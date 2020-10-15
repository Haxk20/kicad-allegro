/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2019 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 2011 Wayne Stambaugh <stambaughw@gmail.com>
 * Copyright (C) 2004-2020 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include <kiface_i.h>
#include <kiway_express.h>
#include <eda_dde.h>
#include <connection_graph.h>
#include <sch_component.h>
#include <schematic.h>
#include <reporter.h>
#include <kicad_string.h>
#include <netlist_exporters/netlist_exporter_kicad.h>
#include <project/project_file.h>
#include <project/net_settings.h>
#include <tools/ee_actions.h>
#include <tools/reannotate.h>
#include <tools/sch_editor_control.h>
#include <advanced_config.h>
#include <netclass.h>

SCH_ITEM* SCH_EDITOR_CONTROL::FindComponentAndItem( const wxString& aReference,
                                                    bool            aSearchHierarchy,
                                                    SCH_SEARCH_T    aSearchType,
                                                    const wxString& aSearchText )
{
    SCH_SHEET_PATH* sheetWithComponentFound = NULL;
    SCH_COMPONENT*  component               = NULL;
    wxPoint         pos;
    SCH_PIN*        pin = nullptr;
    SCH_SHEET_LIST  sheetList;
    SCH_ITEM*       foundItem = nullptr;

    if( !aSearchHierarchy )
        sheetList.push_back( m_frame->GetCurrentSheet() );
    else
        sheetList = m_frame->Schematic().GetSheets();

    for( SCH_SHEET_PATH& sheet : sheetList )
    {
        SCH_SCREEN* screen = sheet.LastScreen();

        for( auto item : screen->Items().OfType( SCH_COMPONENT_T ) )
        {
            SCH_COMPONENT* candidate = static_cast<SCH_COMPONENT*>( item );

            if( aReference.CmpNoCase( candidate->GetRef( &sheet ) ) == 0 )
            {
                component = candidate;
                sheetWithComponentFound = &sheet;

                if( aSearchType == HIGHLIGHT_PIN )
                {
                    pos = component->GetPosition();  // temporary: will be changed if the pin is found.
                    pin = component->GetPin( aSearchText );

                    if( pin )
                    {
                        pos = pin->GetPosition();
                        foundItem = component;
                        break;
                    }
                }
                else
                {
                    pos = component->GetPosition();
                    foundItem = component;
                    break;
                }
            }
        }

        if( foundItem )
            break;
    }

    CROSS_PROBING_SETTINGS& crossProbingSettings = m_frame->eeconfig()->m_CrossProbing;


    if( component )
    {
        if( *sheetWithComponentFound != m_frame->GetCurrentSheet() )
        {
            m_frame->Schematic().SetCurrentSheet( *sheetWithComponentFound );
            m_frame->DisplayCurrentSheet();
        }

        wxPoint delta;
        pos -= component->GetPosition();
        delta = component->GetTransform().TransformCoordinate( pos );
        pos   = delta + component->GetPosition();

        if( crossProbingSettings.center_on_items )
        {
            m_frame->GetCanvas()->GetViewControls()->SetCrossHairCursorPosition( pos, false );
            m_frame->CenterScreen( pos, false );

            if( crossProbingSettings.zoom_to_fit )
            {
//#define COMP_1_TO_1_RATIO // Un-comment for normal KiCad full screen zoom cross-probe
#ifdef COMP_1_TO_1_RATIO
                // Pass "false" to only include visible fields of component in bbox calculations
                EDA_RECT bbox       = component->GetBoundingBox( false );
                wxSize   bbSize     = bbox.Inflate( bbox.GetWidth() * 0.2f ).GetSize();
                VECTOR2D screenSize = getView()->GetViewport().GetSize();

                // NOTE: The 1:1 here is using the default KiCad sizing, which adds a margin of 20%

                screenSize.x = std::max( 10.0, screenSize.x );
                screenSize.y = std::max( 10.0, screenSize.y );
                double ratio = std::max(
                        fabs( bbSize.x / screenSize.x ), fabs( bbSize.y / screenSize.y ) );

                // Try not to zoom on every cross-probe; it gets very noisy
                if( ratio < 0.5 || ratio > 1.0 )
                    getView()->SetScale( getView()->GetScale() / ratio );
#endif // COMP_1_TO_1_RATIO

#ifndef COMP_1_TO_1_RATIO // Do the scaled zoom
                // Pass "false" to only include visible fields of component in bbox calculations
                EDA_RECT bbox       = component->GetBoundingBox( false );
                wxSize   bbSize     = bbox.Inflate( bbox.GetWidth() * 0.2f ).GetSize();
                VECTOR2D screenSize = getView()->GetViewport().GetSize();

                // This code tries to come up with a zoom factor that doesn't simply zoom in
                // to the cross probed component, but instead shows a reasonable amount of the
                // circuit around it to provide context.  This reduces or eliminates the need
                // to manually change the zoom because it's too close.

                // Using the default text height as a constant to compare against, use the
                // height of the bounding box of visible items for a footprint to figure out
                // if this is a big symbol (like a processor) or a small symbol (like a resistor).
                // This ratio is not useful by itself as a scaling factor.  It must be "bent" to
                // provide good scaling at varying component sizes.  Bigger components need less
                // scaling than small ones.
                double currTextHeight = Mils2iu( DEFAULT_TEXT_SIZE );

                double compRatio = bbSize.y / currTextHeight; // Ratio of component to text height
                double compRatioBent = 1.0;

                // LUT to scale zoom ratio to provide reasonable schematic context.  Must work
                // with symbols of varying sizes (e.g. 0402 package and 200 pin BGA).
                // "first" is used as the input and "second" as the output
                //
                // "first" = compRatio (symbol height / default text height)
                // "second" = Amount to scale ratio by
                std::vector<std::pair<double, double>> lut
                {
                    {1.25, 16}, // 32
                    {2.5, 12}, //24
                    {5, 8}, // 16
                    {6, 6}, //
                    {10, 4}, //8
                    {20, 2}, //4
                    {40, 1.5}, // 2
                    {100, 1}
                };

                std::vector<std::pair<double, double>>::iterator it;

                compRatioBent =
                        lut.back().second; // Large component default is last LUT entry (1:1)

                // Use LUT to do linear interpolation of "compRatio" within "first", then
                // use that result to linearly interpolate "second" which gives the scaling
                // factor needed.

                if( compRatio >= lut.front().first )
                {
                    for( it = lut.begin(); it < lut.end() - 1; it++ )
                    {
                        if( it->first <= compRatio && next( it )->first >= compRatio )
                        {

                            double diffx = compRatio - it->first;
                            double diffn = next( it )->first - it->first;

                            compRatioBent = it->second
                                            + ( next( it )->second - it->second ) * diffx / diffn;
                            break; // We have our interpolated value
                        }
                    }
                }
                else
                    compRatioBent = lut.front().second; // Small component default is first entry

                // This is similar to the original KiCad code that scaled the zoom to make sure
                // components were visible on screen.  It's simply a ratio of screen size to component
                // size, and its job is to zoom in to make the component fullscreen.  Earlier in the
                // code the component BBox is given a 20% margin to add some breathing room. We compare
                // the height of this enlarged component bbox to the default text height.  If a
                // component will end up with the sides clipped, we adjust later to make sure it fits
                // on screen.
                screenSize.x      = std::max( 10.0, screenSize.x );
                screenSize.y      = std::max( 10.0, screenSize.y );
                double ratio      = std::max( -1.0, fabs( bbSize.y / screenSize.y ) );
                // Original KiCad code for how much to scale the zoom
                double kicadRatio = std::max( fabs( bbSize.x / screenSize.x ),
                                              fabs( bbSize.y / screenSize.y ) );

                // If the width of the part we're probing is bigger than what the screen width will be
                // after the zoom, then punt and use the KiCad zoom algorithm since it guarantees the
                // part's width will be encompassed within the screen.

                if( bbSize.x > screenSize.x * ratio * compRatioBent )
                {
                    ratio = kicadRatio; // Use standard KiCad zoom for parts too wide to fit on screen
                    compRatioBent = 1.0; // Reset so we don't modify the "KiCad" ratio
                    wxLogTrace( "CROSS_PROBE_SCALE",
                                "Part TOO WIDE for screen.  Using normal KiCad zoom ratio: %1.5f",
                                ratio );
                }

                // Now that "compRatioBent" holds our final scaling factor we apply it to the original
                // fullscreen zoom ratio to arrive at the final ratio itself.
                ratio *= compRatioBent;

                bool alwaysZoom = false; // DEBUG - allows us to minimize zooming or not
                // Try not to zoom on every cross-probe; it gets very noisy
                if( ( ratio < 0.5 || ratio > 1.0 ) || alwaysZoom )
                    getView()->SetScale( getView()->GetScale() / ratio );
#endif // ifndef COMP_1_TO_1_RATIO
            }
        }
    }

    /* Print diag */
    wxString msg_item;
    wxString msg;

    if( aSearchType == HIGHLIGHT_PIN )
        msg_item.Printf( _( "pin %s" ), aSearchText );
    else
        msg_item = _( "component" );

    if( component )
    {
        if( foundItem )
            msg.Printf( _( "%s %s found" ), aReference, msg_item );
        else
            msg.Printf( _( "%s found but %s not found" ), aReference, msg_item );
    }
    else
        msg.Printf( _( "Component %s not found" ), aReference );

    m_frame->SetStatusText( msg );

    m_probingPcbToSch = true;   // recursion guard
    {
        // Clear any existing highlighting
        m_toolMgr->RunAction( EE_ACTIONS::clearSelection, true );

        if( foundItem )
            m_toolMgr->RunAction( EE_ACTIONS::addItemToSel, true, foundItem );
    }
    m_probingPcbToSch = false;

    m_frame->GetCanvas()->Refresh();

    return foundItem;
}


void SCH_EDIT_FRAME::ExecuteRemoteCommand( const char* cmdline )
{
    SCH_EDITOR_CONTROL* editor = m_toolManager->GetTool<SCH_EDITOR_CONTROL>();
    char                line[1024];

    strncpy( line, cmdline, sizeof(line) - 1 );
    line[ sizeof(line) - 1 ] = '\0';

    char* idcmd = strtok( line, " \n\r" );
    char* text  = strtok( NULL, "\"\n\r" );

    if( idcmd == NULL )
        return;

    CROSS_PROBING_SETTINGS& crossProbingSettings = eeconfig()->m_CrossProbing;

    if( strcmp( idcmd, "$NET:" ) == 0 )
    {
        if( !crossProbingSettings.auto_highlight )
            return;

        wxString netName = FROM_UTF8( text );

        if( auto sg = Schematic().ConnectionGraph()->FindFirstSubgraphByName( netName ) )
            m_highlightedConn = sg->m_driver_connection;

        GetToolManager()->RunAction( EE_ACTIONS::updateNetHighlighting, true );

        SetStatusText( _( "Selected net: " ) + UnescapeString( netName ) );
        return;
    }

    if( strcmp( idcmd, "$CLEAR:" ) == 0 )
    {
        // Cross-probing is now done through selection so we no longer need a clear command
        return;
    }

    if( text == NULL )
        return;

    if( strcmp( idcmd, "$PART:" ) != 0 )
        return;

    wxString part_ref = FROM_UTF8( text );

    /* look for a complement */
    idcmd = strtok( NULL, " \n\r" );

    if( idcmd == NULL )    // Highlight component only (from Cvpcb or Pcbnew)
    {
        // Highlight component part_ref, or clear Highlight, if part_ref is not existing
        editor->FindComponentAndItem( part_ref, true, HIGHLIGHT_COMPONENT, wxEmptyString );
        return;
    }

    text = strtok( NULL, "\"\n\r" );

    if( text == NULL )
        return;

    wxString msg = FROM_UTF8( text );

    if( strcmp( idcmd, "$REF:" ) == 0 )
    {
        // Highlighting the reference itself isn't actually that useful, and it's harder to
        // see.  Highlight the parent and display the message.
        editor->FindComponentAndItem( part_ref, true, HIGHLIGHT_COMPONENT, msg );
    }
    else if( strcmp( idcmd, "$VAL:" ) == 0 )
    {
        // Highlighting the value itself isn't actually that useful, and it's harder to see.
        // Highlight the parent and display the message.
        editor->FindComponentAndItem( part_ref, true, HIGHLIGHT_COMPONENT, msg );
    }
    else if( strcmp( idcmd, "$PAD:" ) == 0 )
    {
        editor->FindComponentAndItem( part_ref, true, HIGHLIGHT_PIN, msg );
    }
    else
    {
        editor->FindComponentAndItem( part_ref, true, HIGHLIGHT_COMPONENT, wxEmptyString );
    }
}


std::string FormatProbeItem( EDA_ITEM* aItem, SCH_COMPONENT* aComp )
{
    // This is a keyword followed by a quoted string.

    // Cross probing to Pcbnew if a pin or a component is found
    switch( aItem->Type() )
    {
    case SCH_FIELD_T:
        if( aComp )
            return StrPrintf( "$PART: \"%s\"", TO_UTF8( aComp->GetField( REFERENCE )->GetText() ) );
        break;

    case SCH_COMPONENT_T:
        aComp = (SCH_COMPONENT*) aItem;
        return StrPrintf( "$PART: \"%s\"", TO_UTF8( aComp->GetField( REFERENCE )->GetText() ) );

    case SCH_SHEET_T:
        {
            // For cross probing, we need the full path of the sheet, because
            // in complex hierarchies the sheet uuid of not unique
            SCH_SHEET* sheet = (SCH_SHEET*)aItem;
            wxString full_path;

            SCH_SHEET* parent = sheet;
            while( (parent = dynamic_cast<SCH_SHEET*>( parent->GetParent() ) ) )
            {
                if( parent->GetParent() )   // The root sheet has no parent and path is just "/"
                {
                    full_path.Prepend( parent->m_Uuid.AsString() );
                    full_path.Prepend( "/" );
                }
            }

            full_path += "/" + sheet->m_Uuid.AsString();

            return StrPrintf( "$SHEET: \"%s\"", TO_UTF8( full_path ) );
        }

    case SCH_PIN_T:
        {
            SCH_PIN* pin = (SCH_PIN*) aItem;
            aComp = pin->GetParentComponent();

            if( !pin->GetNumber().IsEmpty() )
            {
                return StrPrintf( "$PIN: \"%s\" $PART: \"%s\"",
                                  TO_UTF8( pin->GetNumber() ),
                                  TO_UTF8( aComp->GetField( REFERENCE )->GetText() ) );
            }
            else
            {
                return StrPrintf( "$PART: \"%s\"",
                                  TO_UTF8( aComp->GetField( REFERENCE )->GetText() ) );
            }
        }

    default:
        break;
    }

    return "";
}


void SCH_EDIT_FRAME::SendMessageToPCBNEW( EDA_ITEM* aObjectToSync, SCH_COMPONENT* aLibItem )
{
    wxASSERT( aObjectToSync );     // fix the caller

    if( !aObjectToSync )
        return;

    std::string packet = FormatProbeItem( aObjectToSync, aLibItem );

    if( !packet.empty() )
    {
        if( Kiface().IsSingle() )
            SendCommand( MSG_TO_PCB, packet.c_str() );
        else
        {
            // Typically ExpressMail is going to be s-expression packets, but since
            // we have existing interpreter of the cross probe packet on the other
            // side in place, we use that here.
            Kiway().ExpressMail( FRAME_PCB_EDITOR, MAIL_CROSS_PROBE, packet, this );
        }
    }
}


void SCH_EDIT_FRAME::SendCrossProbeNetName( const wxString& aNetName )
{
    // The command is a keyword followed by a quoted string.

    std::string packet = StrPrintf( "$NET: \"%s\"", TO_UTF8( aNetName ) );

    if( !packet.empty() )
    {
        if( Kiface().IsSingle() )
            SendCommand( MSG_TO_PCB, packet.c_str() );
        else
        {
            // Typically ExpressMail is going to be s-expression packets, but since
            // we have existing interpreter of the cross probe packet on the other
            // side in place, we use that here.
            Kiway().ExpressMail( FRAME_PCB_EDITOR, MAIL_CROSS_PROBE, packet, this );
        }
    }
}


void SCH_EDIT_FRAME::SetCrossProbeConnection( const SCH_CONNECTION* aConnection )
{
    if( !aConnection )
    {
        SendCrossProbeClearHighlight();
        return;
    }

    if( aConnection->IsNet() )
    {
        SendCrossProbeNetName( aConnection->Name() );
        return;
    }

    if( aConnection->Members().empty() )
        return;

    auto all_members = aConnection->AllMembers();

    wxString nets = all_members[0]->Name();

    if( all_members.size() == 1 )
    {
        SendCrossProbeNetName( nets );
        return;
    }

    // TODO: This could be replaced by just sending the bus name once we have bus contents
    // included as part of the netlist sent from eeschema to pcbnew (and thus pcbnew can
    // natively keep track of bus membership)

    for( size_t i = 1; i < all_members.size(); i++ )
        nets << "," << all_members[i]->Name();

    std::string packet = StrPrintf( "$NETS: \"%s\"", TO_UTF8( nets ) );

    if( !packet.empty() )
    {
        if( Kiface().IsSingle() )
            SendCommand( MSG_TO_PCB, packet.c_str() );
        else
        {
            // Typically ExpressMail is going to be s-expression packets, but since
            // we have existing interpreter of the cross probe packet on the other
            // side in place, we use that here.
            Kiway().ExpressMail( FRAME_PCB_EDITOR, MAIL_CROSS_PROBE, packet, this );
        }
    }
}


void SCH_EDIT_FRAME::SendCrossProbeClearHighlight()
{
    std::string packet = "$CLEAR\n";

    if( Kiface().IsSingle() )
        SendCommand( MSG_TO_PCB, packet.c_str() );
    else
    {
        // Typically ExpressMail is going to be s-expression packets, but since
        // we have existing interpreter of the cross probe packet on the other
        // side in place, we use that here.
        Kiway().ExpressMail( FRAME_PCB_EDITOR, MAIL_CROSS_PROBE, packet, this );
    }
}


void SCH_EDIT_FRAME::KiwayMailIn( KIWAY_EXPRESS& mail )
{
    std::string& payload = mail.GetPayload();

    switch( mail.Command() )
    {
    case MAIL_CROSS_PROBE:
        ExecuteRemoteCommand( payload.c_str() );
        break;

    case MAIL_SCH_GET_NETLIST:
    {
        if( payload.find( "quiet-annotate" ) != std::string::npos )
        {
            Schematic().GetSheets().AnnotatePowerSymbols();
            AnnotateComponents( true, UNSORTED, INCREMENTAL_BY_REF, 0, false, false, true,
                                NULL_REPORTER::GetInstance() );
        }

        if( payload.find( "no-annotate" ) == std::string::npos )
        {
            // Ensure schematic is OK for netlist creation (especially that it is fully annotated):
            if( !ReadyToNetlist() )
                return;
        }

        NETLIST_EXPORTER_KICAD exporter( &Schematic() );
        STRING_FORMATTER formatter;

        // TODO remove once real-time connectivity is a given
        if( !ADVANCED_CFG::GetCfg().m_realTimeConnectivity || !CONNECTION_GRAPH::m_allowRealTime )
            // Ensure the netlist data is up to date:
            RecalculateConnections( NO_CLEANUP );

        exporter.Format( &formatter, GNL_ALL | GNL_OPT_KICAD );

        payload = formatter.GetString();
    }
        break;

    case MAIL_ASSIGN_FOOTPRINTS:
        try
        {
            SCH_EDITOR_CONTROL* controlTool = m_toolManager->GetTool<SCH_EDITOR_CONTROL>();
            controlTool->AssignFootprints( payload );
        }
        catch( const IO_ERROR& )
        {
        }
        break;

    case MAIL_SCH_REFRESH:
    {
        SCH_SCREENS schematic( Schematic().Root() );
        schematic.TestDanglingEnds();

        GetCanvas()->GetView()->UpdateAllItems( KIGFX::ALL );
        GetCanvas()->Refresh();
    }
        break;

    case MAIL_SCH_CLEAN_NETCLASSES:
    {
        NET_SETTINGS& netSettings = Prj().GetProjectFile().NetSettings();

        netSettings.m_NetClassAssignments.clear();

        // Establish the set of nets which is currently valid
        for( const wxString& name : Schematic().GetNetClassAssignmentCandidates() )
            netSettings.m_NetClassAssignments[ name ] = "Default";

        // Copy their netclass assignments, dropping any assignments to non-current nets.
        for( auto& ii : netSettings.m_NetClasses )
        {
            for( const wxString& member : *ii.second )
            {
                if( netSettings.m_NetClassAssignments.count( member ) )
                    netSettings.m_NetClassAssignments[ member ] = ii.first;
            }

            ii.second->Clear();
        }

        // Update the membership lists to contain only the current nets.
        for( const std::pair<const wxString, wxString>& ii : netSettings.m_NetClassAssignments )
        {
            if( ii.second == "Default" )
                continue;

            NETCLASSPTR netclass = netSettings.m_NetClasses.Find( ii.second );

            if( netclass )
                netclass->Add( ii.first );
        }

        netSettings.ResolveNetClassAssignments();
    }
        break;

    case MAIL_IMPORT_FILE:
    {
        // Extract file format type and path (plugin type and path separated with \n)
        size_t split = payload.find( '\n' );
        wxCHECK( split != std::string::npos, /*void*/ );
        int importFormat;

        try
        {
            importFormat = std::stoi( payload.substr( 0, split ) );
        }
        catch( std::invalid_argument& )
        {
            wxFAIL;
            importFormat = -1;
        }

        std::string path = payload.substr( split + 1 );
        wxASSERT( !path.empty() );

        if( importFormat >= 0 )
            importFile( path, importFormat );
    }
        break;

    case MAIL_SCH_SAVE:
        if( SaveProject() )
            payload = "success";

        break;

    case MAIL_SCH_UPDATE:
        m_toolManager->RunAction( ACTIONS::updateSchematicFromPcb, true );
        break;

    case MAIL_REANNOTATE:
        //Reannotate the schematic as per the netlist.
        ReannotateFromPCBNew( this, payload );
        break;

    default:;

    }
}
