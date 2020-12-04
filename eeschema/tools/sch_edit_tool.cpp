/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2019 CERN
 * Copyright (C) 2019-2020 KiCad Developers, see AUTHORS.txt for contributors.
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

#include <kiway.h>
#include <tool/picker_tool.h>
#include <tools/sch_edit_tool.h>
#include <tools/ee_selection_tool.h>
#include <tools/sch_line_wire_bus_tool.h>
#include <tools/sch_move_tool.h>
#include <widgets/infobar.h>
#include <ee_actions.h>
#include <bitmaps.h>
#include <confirm.h>
#include <eda_item.h>
#include <reporter.h>
#include <kicad_string.h>
#include <sch_item.h>
#include <sch_component.h>
#include <sch_sheet.h>
#include <sch_text.h>
#include <sch_bitmap.h>
#include <sch_view.h>
#include <sch_line.h>
#include <sch_bus_entry.h>
#include <sch_junction.h>
#include <sch_edit_frame.h>
#include <schematic.h>
#include <page_layout/ws_proxy_view_item.h>
#include <page_layout/ws_proxy_undo_item.h>
#include <eeschema_id.h>
#include <status_popup.h>
#include <wx/gdicmn.h>
#include <dialogs/dialog_change_symbols.h>
#include <dialogs/dialog_image_editor.h>
#include <dialogs/dialog_edit_line_style.h>
#include <dialogs/dialog_symbol_properties.h>
#include <dialogs/dialog_sheet_pin_properties.h>
#include <dialogs/dialog_edit_one_field.h>
#include <dialogs/dialog_junction_props.h>
#include "sch_drawing_tools.h"
#include <math/util.h>      // for KiROUND
#include <pgm_base.h>
#include <settings/settings_manager.h>
#include <symbol_editor_settings.h>
#include <dialogs/dialog_edit_label.h>

class SYMBOL_UNIT_MENU : public ACTION_MENU
{
public:
    SYMBOL_UNIT_MENU() :
        ACTION_MENU( true )
    {
        SetIcon( component_select_unit_xpm );
        SetTitle( _( "Symbol Unit" ) );
    }

protected:
    ACTION_MENU* create() const override
    {
        return new SYMBOL_UNIT_MENU();
    }

private:
    void update() override
    {
        EE_SELECTION_TOOL* selTool = getToolManager()->GetTool<EE_SELECTION_TOOL>();
        EE_SELECTION&      selection = selTool->GetSelection();
        SCH_COMPONENT*     component = dynamic_cast<SCH_COMPONENT*>( selection.Front() );

        Clear();

        if( !component )
        {
            Append( ID_POPUP_SCH_UNFOLD_BUS, _( "no symbol selected" ), wxEmptyString );
            Enable( ID_POPUP_SCH_UNFOLD_BUS, false );
            return;
        }

        int  unit = component->GetUnit();

        if( !component->GetPartRef() || component->GetPartRef()->GetUnitCount() < 2 )
        {
            Append( ID_POPUP_SCH_UNFOLD_BUS, _( "symbol is not multi-unit" ), wxEmptyString );
            Enable( ID_POPUP_SCH_UNFOLD_BUS, false );
            return;
        }

        for( int ii = 0; ii < component->GetPartRef()->GetUnitCount(); ii++ )
        {
            wxString num_unit;
            num_unit.Printf( _( "Unit %s" ), LIB_PART::SubReference( ii + 1, false ) );

            wxMenuItem * item = Append( ID_POPUP_SCH_SELECT_UNIT1 + ii, num_unit, wxEmptyString,
                                        wxITEM_CHECK );
            if( unit == ii + 1 )
                item->Check(true);

            // The ID max for these submenus is ID_POPUP_SCH_SELECT_UNIT_CMP_MAX
            // See eeschema_id to modify this value.
            if( ii >= (ID_POPUP_SCH_SELECT_UNIT_CMP_MAX - ID_POPUP_SCH_SELECT_UNIT1) )
                break;      // We have used all IDs for these submenus
        }
    }
};


SCH_EDIT_TOOL::SCH_EDIT_TOOL() :
        EE_TOOL_BASE<SCH_EDIT_FRAME>( "eeschema.InteractiveEdit" )
{
    m_pickerItem = nullptr;
}


using E_C = EE_CONDITIONS;

bool SCH_EDIT_TOOL::Init()
{
    EE_TOOL_BASE::Init();

    SCH_DRAWING_TOOLS* drawingTools = m_toolMgr->GetTool<SCH_DRAWING_TOOLS>();
    SCH_MOVE_TOOL*     moveTool = m_toolMgr->GetTool<SCH_MOVE_TOOL>();

    wxASSERT_MSG( drawingTools, "eeshema.InteractiveDrawing tool is not available" );

    auto hasElements =
            [ this ] ( const SELECTION& aSel )
            {
                return !m_frame->GetScreen()->Items().empty();
            };

    auto sheetTool =
            [ this ] ( const SELECTION& aSel )
            {
                return ( m_frame->IsCurrentTool( EE_ACTIONS::drawSheet ) );
            };

    auto anyTextTool =
            [ this ] ( const SELECTION& aSel )
            {
                return ( m_frame->IsCurrentTool( EE_ACTIONS::placeLabel )
                      || m_frame->IsCurrentTool( EE_ACTIONS::placeGlobalLabel )
                      || m_frame->IsCurrentTool( EE_ACTIONS::placeHierLabel )
                      || m_frame->IsCurrentTool( EE_ACTIONS::placeSchematicText ) );
            };

    auto duplicateCondition =
            [] ( const SELECTION& aSel )
            {
                if( SCH_LINE_WIRE_BUS_TOOL::IsDrawingLineWireOrBus( aSel ) )
                    return false;

                return true;
            };

    auto orientCondition =
            [] ( const SELECTION& aSel )
            {
                if( aSel.Empty() )
                    return false;

                if( SCH_LINE_WIRE_BUS_TOOL::IsDrawingLineWireOrBus( aSel ) )
                    return false;

                SCH_ITEM* item = (SCH_ITEM*) aSel.Front();

                if( aSel.GetSize() > 1 )
                    return true;

                switch( item->Type() )
                {
                case SCH_MARKER_T:
                case SCH_JUNCTION_T:
                case SCH_NO_CONNECT_T:
                case SCH_PIN_T:
                    return false;
                case SCH_LINE_T:
                    return item->GetLayer() != LAYER_WIRE && item->GetLayer() != LAYER_BUS;
                default:
                    return true;
                }
            };

    auto propertiesCondition =
            []( const SELECTION& aSel )
            {
                if( aSel.GetSize() == 0 )
                    return true;            // Show worksheet properties

                SCH_ITEM*           firstItem   = dynamic_cast<SCH_ITEM*>( aSel.Front() );
                const EE_SELECTION* eeSelection = dynamic_cast<const EE_SELECTION*>( &aSel );

                if( !firstItem || !eeSelection )
                    return false;

                switch( firstItem->Type() )
                {
                case SCH_COMPONENT_T:
                case SCH_SHEET_T:
                case SCH_SHEET_PIN_T:
                case SCH_TEXT_T:
                case SCH_LABEL_T:
                case SCH_GLOBAL_LABEL_T:
                case SCH_HIER_LABEL_T:
                case SCH_FIELD_T:
                case SCH_BITMAP_T:
                    return aSel.GetSize() == 1;

                case SCH_LINE_T:
                case SCH_BUS_WIRE_ENTRY_T:
                    return eeSelection->AllItemsHaveLineStroke();

                case SCH_JUNCTION_T:
                    return eeSelection->AreAllItemsIdentical();

                default:
                    return false;
                }
            };

    static KICAD_T toLabelTypes[] = { SCH_GLOBAL_LABEL_T, SCH_HIER_LABEL_T, SCH_TEXT_T, EOT };
    auto toLabelCondition = E_C::Count( 1 ) && E_C::OnlyTypes( toLabelTypes );

    static KICAD_T toHLableTypes[] = { SCH_LABEL_T, SCH_GLOBAL_LABEL_T, SCH_TEXT_T, EOT };
    auto toHLabelCondition = E_C::Count( 1 ) && E_C::OnlyTypes( toHLableTypes );

    static KICAD_T toGLableTypes[] = { SCH_LABEL_T, SCH_HIER_LABEL_T, SCH_TEXT_T, EOT };
    auto toGLabelCondition = E_C::Count( 1 ) && E_C::OnlyTypes( toGLableTypes );

    static KICAD_T toTextTypes[] = { SCH_LABEL_T, SCH_GLOBAL_LABEL_T, SCH_HIER_LABEL_T, EOT };
    auto toTextlCondition = E_C::Count( 1 ) && E_C::OnlyTypes( toTextTypes );

    static KICAD_T entryTypes[] = { SCH_BUS_WIRE_ENTRY_T, SCH_BUS_BUS_ENTRY_T, EOT };
    auto entryCondition = E_C::MoreThan( 0 ) && E_C::OnlyTypes( entryTypes );

    static KICAD_T fieldParentTypes[] = { SCH_COMPONENT_T, SCH_SHEET_T, SCH_GLOBAL_LABEL_T, EOT };
    auto singleFieldParentCondition = E_C::Count( 1 ) && E_C::OnlyTypes( fieldParentTypes );

    auto singleSymbolCondition = E_C::Count( 1 ) && E_C::OnlyType( SCH_COMPONENT_T );

    auto singleSheetCondition =  E_C::Count( 1 ) && E_C::OnlyType( SCH_SHEET_T );
    //
    // Add edit actions to the move tool menu
    //
    if( moveTool )
    {
        CONDITIONAL_MENU& moveMenu = moveTool->GetToolMenu().GetMenu();

        moveMenu.AddSeparator();
        moveMenu.AddItem( EE_ACTIONS::rotateCCW,       orientCondition );
        moveMenu.AddItem( EE_ACTIONS::rotateCW,        orientCondition );
        moveMenu.AddItem( EE_ACTIONS::mirrorX,         orientCondition );
        moveMenu.AddItem( EE_ACTIONS::mirrorY,         orientCondition );
        moveMenu.AddItem( ACTIONS::doDelete,           E_C::NotEmpty );

        moveMenu.AddItem( EE_ACTIONS::properties,      propertiesCondition );
        moveMenu.AddItem( EE_ACTIONS::editReference,   singleSymbolCondition );
        moveMenu.AddItem( EE_ACTIONS::editValue,       singleSymbolCondition );
        moveMenu.AddItem( EE_ACTIONS::editFootprint,   singleSymbolCondition );
        moveMenu.AddItem( EE_ACTIONS::toggleDeMorgan,  E_C::SingleDeMorganSymbol );

        std::shared_ptr<SYMBOL_UNIT_MENU> symUnitMenu = std::make_shared<SYMBOL_UNIT_MENU>();
        symUnitMenu->SetTool( this );
        m_menu.AddSubMenu( symUnitMenu );
        moveMenu.AddMenu( symUnitMenu.get(), E_C::SingleMultiUnitSymbol, 1 );

        moveMenu.AddSeparator();
        moveMenu.AddItem( ACTIONS::cut,                E_C::IdleSelection );
        moveMenu.AddItem( ACTIONS::copy,               E_C::IdleSelection );
        moveMenu.AddItem( ACTIONS::duplicate,          duplicateCondition );

        moveMenu.AddSeparator();
        moveMenu.AddItem( ACTIONS::selectAll,          hasElements );
    }

    //
    // Add editing actions to the drawing tool menu
    //
    CONDITIONAL_MENU& drawMenu = drawingTools->GetToolMenu().GetMenu();

    drawMenu.AddItem( EE_ACTIONS::rotateCCW,        orientCondition, 200 );
    drawMenu.AddItem( EE_ACTIONS::rotateCW,         orientCondition, 200 );
    drawMenu.AddItem( EE_ACTIONS::mirrorX,          orientCondition, 200 );
    drawMenu.AddItem( EE_ACTIONS::mirrorY,          orientCondition, 200 );

    drawMenu.AddItem( EE_ACTIONS::properties,       propertiesCondition, 200 );
    drawMenu.AddItem( EE_ACTIONS::editReference,    singleSymbolCondition, 200 );
    drawMenu.AddItem( EE_ACTIONS::editValue,        singleSymbolCondition, 200 );
    drawMenu.AddItem( EE_ACTIONS::editFootprint,    singleSymbolCondition, 200 );
    drawMenu.AddItem( EE_ACTIONS::autoplaceFields,  singleFieldParentCondition, 200 );
    drawMenu.AddItem( EE_ACTIONS::toggleDeMorgan,   E_C::SingleDeMorganSymbol, 200 );

    std::shared_ptr<SYMBOL_UNIT_MENU> symUnitMenu2 = std::make_shared<SYMBOL_UNIT_MENU>();
    symUnitMenu2->SetTool( drawingTools );
    drawingTools->GetToolMenu().AddSubMenu( symUnitMenu2 );
    drawMenu.AddMenu( symUnitMenu2.get(), E_C::SingleMultiUnitSymbol, 1 );

    drawMenu.AddItem( EE_ACTIONS::editWithLibEdit,     singleSymbolCondition && E_C::Idle, 200 );

    drawMenu.AddItem( EE_ACTIONS::toLabel,             anyTextTool && E_C::Idle, 200 );
    drawMenu.AddItem( EE_ACTIONS::toHLabel,            anyTextTool && E_C::Idle, 200 );
    drawMenu.AddItem( EE_ACTIONS::toGLabel,            anyTextTool && E_C::Idle, 200 );
    drawMenu.AddItem( EE_ACTIONS::toText,              anyTextTool && E_C::Idle, 200 );
    drawMenu.AddItem( EE_ACTIONS::cleanupSheetPins,    sheetTool && E_C::Idle, 250 );

    //
    // Add editing actions to the selection tool menu
    //
    CONDITIONAL_MENU& selToolMenu = m_selectionTool->GetToolMenu().GetMenu();

    selToolMenu.AddItem( EE_ACTIONS::rotateCCW,        orientCondition, 200 );
    selToolMenu.AddItem( EE_ACTIONS::rotateCW,         orientCondition, 200 );
    selToolMenu.AddItem( EE_ACTIONS::mirrorX,          orientCondition, 200 );
    selToolMenu.AddItem( EE_ACTIONS::mirrorY,          orientCondition, 200 );
    selToolMenu.AddItem( ACTIONS::doDelete,            E_C::NotEmpty, 200 );

    selToolMenu.AddItem( EE_ACTIONS::properties,       propertiesCondition, 200 );
    selToolMenu.AddItem( EE_ACTIONS::editReference,    E_C::SingleSymbol, 200 );
    selToolMenu.AddItem( EE_ACTIONS::editValue,        E_C::SingleSymbol, 200 );
    selToolMenu.AddItem( EE_ACTIONS::editFootprint,    E_C::SingleSymbol, 200 );
    selToolMenu.AddItem( EE_ACTIONS::autoplaceFields,  singleFieldParentCondition, 200 );
    selToolMenu.AddItem( EE_ACTIONS::toggleDeMorgan,   E_C::SingleSymbol, 200 );

    std::shared_ptr<SYMBOL_UNIT_MENU> symUnitMenu3 = std::make_shared<SYMBOL_UNIT_MENU>();
    symUnitMenu3->SetTool( m_selectionTool );
    m_selectionTool->GetToolMenu().AddSubMenu( symUnitMenu3 );
    selToolMenu.AddMenu( symUnitMenu3.get(), E_C::SingleMultiUnitSymbol, 1 );

    selToolMenu.AddItem( EE_ACTIONS::editWithLibEdit,  singleSymbolCondition && E_C::Idle, 200 );
    selToolMenu.AddItem( EE_ACTIONS::changeSymbol,     E_C::SingleSymbol, 200 );
    selToolMenu.AddItem( EE_ACTIONS::updateSymbol,     E_C::SingleSymbol, 200 );

    selToolMenu.AddItem( EE_ACTIONS::toLabel,          toLabelCondition, 200 );
    selToolMenu.AddItem( EE_ACTIONS::toHLabel,         toHLabelCondition, 200 );
    selToolMenu.AddItem( EE_ACTIONS::toGLabel,         toGLabelCondition, 200 );
    selToolMenu.AddItem( EE_ACTIONS::toText,           toTextlCondition, 200 );
    selToolMenu.AddItem( EE_ACTIONS::cleanupSheetPins, singleSheetCondition, 250 );

    selToolMenu.AddSeparator( 300 );
    selToolMenu.AddItem( ACTIONS::cut,                 E_C::IdleSelection, 300 );
    selToolMenu.AddItem( ACTIONS::copy,                E_C::IdleSelection, 300 );
    selToolMenu.AddItem( ACTIONS::paste,               E_C::Idle, 300 );
    selToolMenu.AddItem( ACTIONS::pasteSpecial,        E_C::Idle, 300 );
    selToolMenu.AddItem( ACTIONS::duplicate,           duplicateCondition, 300 );

    selToolMenu.AddSeparator( 400 );
    selToolMenu.AddItem( ACTIONS::selectAll,           hasElements, 400 );


    return true;
}


const KICAD_T rotatableItems[] = {
    SCH_TEXT_T,
    SCH_LABEL_T,
    SCH_GLOBAL_LABEL_T,
    SCH_HIER_LABEL_T,
    SCH_FIELD_T,
    SCH_COMPONENT_T,
    SCH_SHEET_PIN_T,
    SCH_SHEET_T,
    SCH_BITMAP_T,
    SCH_BUS_BUS_ENTRY_T,
    SCH_BUS_WIRE_ENTRY_T,
    SCH_LINE_T,
    SCH_JUNCTION_T,
    SCH_NO_CONNECT_T,
    EOT
};


int SCH_EDIT_TOOL::Rotate( const TOOL_EVENT& aEvent )
{
    EE_SELECTION& selection = m_selectionTool->RequestSelection( rotatableItems );

    if( selection.GetSize() == 0 )
        return 0;

    wxPoint   rotPoint;
    bool      clockwise = ( aEvent.Matches( EE_ACTIONS::rotateCW.MakeEvent() ) );
    SCH_ITEM* item = static_cast<SCH_ITEM*>( selection.Front() );
    bool      connections = false;
    bool      moving = item->IsMoving();

    if( selection.GetSize() == 1 )
    {
        if( !moving )
            saveCopyInUndoList( item, UNDO_REDO::CHANGED );

        switch( item->Type() )
        {
        case SCH_COMPONENT_T:
        {
            SCH_COMPONENT* component = static_cast<SCH_COMPONENT*>( item );

            if( clockwise )
                component->SetOrientation( CMP_ROTATE_CLOCKWISE );
            else
                component->SetOrientation( CMP_ROTATE_COUNTERCLOCKWISE );

            if( m_frame->eeconfig()->m_AutoplaceFields.enable )
                component->AutoAutoplaceFields( m_frame->GetScreen() );

            break;
        }

        case SCH_TEXT_T:
        case SCH_LABEL_T:
        case SCH_GLOBAL_LABEL_T:
        case SCH_HIER_LABEL_T:
        {
            SCH_TEXT* textItem = static_cast<SCH_TEXT*>( item );
            textItem->Rotate90( clockwise );
            break;
        }

        case SCH_SHEET_PIN_T:
        {
            // Rotate pin within parent sheet
            SCH_SHEET_PIN* pin   = static_cast<SCH_SHEET_PIN*>( item );
            SCH_SHEET*     sheet = pin->GetParent();

            for( int i = 0; clockwise ? i < 1 : i < 3; ++i )
                pin->Rotate( sheet->GetBoundingBox().GetCenter() );

            break;
        }

        case SCH_LINE_T:
        case SCH_BUS_BUS_ENTRY_T:
        case SCH_BUS_WIRE_ENTRY_T:
            for( int i = 0; clockwise ? i < 1 : i < 3; ++i )
                item->Rotate( item->GetPosition() );

            break;

        case SCH_FIELD_T:
        {
            SCH_FIELD* field = static_cast<SCH_FIELD*>( item );

            if( field->GetTextAngle() == TEXT_ANGLE_HORIZ )
                field->SetTextAngle( TEXT_ANGLE_VERT );
            else
                field->SetTextAngle( TEXT_ANGLE_HORIZ );

            // Now that we're moving a field, they're no longer autoplaced.
            static_cast<SCH_ITEM*>( item->GetParent() )->ClearFieldsAutoplaced();

            break;
        }

        case SCH_BITMAP_T:
            for( int i = 0; clockwise ? i < 1 : i < 3; ++i )
                item->Rotate( item->GetPosition() );

            // The bitmap is cached in Opengl: clear the cache to redraw
            getView()->RecacheAllItems();
            break;

        case SCH_SHEET_T:
        {
            SCH_SHEET* sheet = static_cast<SCH_SHEET*>( item );

            // Rotate the sheet on itself. Sheets do not have an anchor point.
            for( int i = 0; clockwise ? i < 3 : i < 1; ++i )
            {
                rotPoint = m_frame->GetNearestGridPosition( sheet->GetRotationCenter() );
                sheet->Rotate( rotPoint );
            }
            break;
        }

        default:
            break;
        }

        connections = item->IsConnectable();
        m_frame->UpdateItem( item );
    }
    else if( selection.GetSize() > 1 )
    {
        rotPoint = m_frame->GetNearestGridPosition( (wxPoint)selection.GetCenter() );

        for( unsigned ii = 0; ii < selection.GetSize(); ii++ )
        {
            item = static_cast<SCH_ITEM*>( selection.GetItem( ii ) );

            if( !moving )
                saveCopyInUndoList( item, UNDO_REDO::CHANGED, ii > 0 );

            for( int i = 0; clockwise ? i < 1 : i < 3; ++i )
            {
                if( item->Type() == SCH_LINE_T )
                {
                    SCH_LINE* line = (SCH_LINE*) item;

                    if( item->HasFlag( STARTPOINT ) )
                        line->RotateStart( rotPoint );

                    if( item->HasFlag( ENDPOINT ) )
                        line->RotateEnd( rotPoint );
                }
                else if( item->Type() == SCH_SHEET_PIN_T )
                {
                    if( item->GetParent()->IsSelected() )
                    {
                        // parent will rotate us
                    }
                    else
                    {
                        // rotate within parent
                        SCH_SHEET_PIN* pin = static_cast<SCH_SHEET_PIN*>( item );
                        SCH_SHEET*     sheet = pin->GetParent();

                        pin->Rotate( sheet->GetBoundingBox().GetCenter() );
                    }
                }
                else
                {
                    item->Rotate( rotPoint );
                }
            }

            connections |= item->IsConnectable();
            m_frame->UpdateItem( item );
        }
    }

    m_toolMgr->PostEvent( EVENTS::SelectedItemsModified );

    if( item->IsMoving() )
    {
        m_toolMgr->RunAction( ACTIONS::refreshPreview );
    }
    else
    {
        if( selection.IsHover() )
            m_toolMgr->RunAction( EE_ACTIONS::clearSelection, true );

        if( connections )
            m_frame->TestDanglingEnds();

        m_frame->OnModify();
    }

    return 0;
}


int SCH_EDIT_TOOL::Mirror( const TOOL_EVENT& aEvent )
{
    EE_SELECTION& selection = m_selectionTool->RequestSelection( rotatableItems );

    if( selection.GetSize() == 0 )
        return 0;

    wxPoint   mirrorPoint;
    bool      xAxis = ( aEvent.Matches( EE_ACTIONS::mirrorX.MakeEvent() ) );
    SCH_ITEM* item = static_cast<SCH_ITEM*>( selection.Front() );
    bool      connections = false;
    bool      moving = item->IsMoving();

    if( selection.GetSize() == 1 )
    {
        if( !moving )
            saveCopyInUndoList( item, UNDO_REDO::CHANGED );

        switch( item->Type() )
        {
        case SCH_COMPONENT_T:
        {
            SCH_COMPONENT* component = static_cast<SCH_COMPONENT*>( item );

            if( xAxis )
                component->SetOrientation( CMP_MIRROR_X );
            else
                component->SetOrientation( CMP_MIRROR_Y );

            if( m_frame->eeconfig()->m_AutoplaceFields.enable )
                component->AutoAutoplaceFields( m_frame->GetScreen() );

            break;
        }

        case SCH_TEXT_T:
        case SCH_LABEL_T:
        case SCH_GLOBAL_LABEL_T:
        case SCH_HIER_LABEL_T:
        {
            SCH_TEXT* textItem = static_cast<SCH_TEXT*>( item );
            textItem->MirrorSpinStyle( !xAxis );
            break;
        }

        case SCH_SHEET_PIN_T:
        {
            // mirror within parent sheet
            SCH_SHEET_PIN* pin = static_cast<SCH_SHEET_PIN*>( item );
            SCH_SHEET*     sheet = pin->GetParent();

            if( xAxis )
                pin->MirrorX( sheet->GetBoundingBox().GetCenter().y );
            else
                pin->MirrorY( sheet->GetBoundingBox().GetCenter().x );

            break;
        }

        case SCH_BUS_BUS_ENTRY_T:
        case SCH_BUS_WIRE_ENTRY_T:
            if( xAxis )
                item->MirrorX( item->GetPosition().y );
            else
                item->MirrorY( item->GetPosition().x );
            break;

        case SCH_FIELD_T:
        {
            SCH_FIELD* field = static_cast<SCH_FIELD*>( item );

            if( xAxis )
                field->SetVertJustify( (EDA_TEXT_VJUSTIFY_T)-field->GetVertJustify() );
            else
                field->SetHorizJustify( (EDA_TEXT_HJUSTIFY_T)-field->GetHorizJustify() );

            // Now that we're re-justifying a field, they're no longer autoplaced.
            static_cast<SCH_ITEM*>( item->GetParent() )->ClearFieldsAutoplaced();

            break;
        }

        case SCH_BITMAP_T:
            if( xAxis )
                item->MirrorX( item->GetPosition().y );
            else
                item->MirrorY( item->GetPosition().x );

            // The bitmap is cached in Opengl: clear the cache to redraw
            getView()->RecacheAllItems();
            break;

        case SCH_SHEET_T:
            // Mirror the sheet on itself. Sheets do not have a anchor point.
            mirrorPoint = m_frame->GetNearestGridPosition( item->GetBoundingBox().Centre() );

            if( xAxis )
                item->MirrorX( mirrorPoint.y );
            else
                item->MirrorY( mirrorPoint.x );

            break;

        default:
            break;
        }

        connections = item->IsConnectable();
        m_frame->UpdateItem( item );
    }
    else if( selection.GetSize() > 1 )
    {
        mirrorPoint = m_frame->GetNearestGridPosition( (wxPoint)selection.GetCenter() );

        for( unsigned ii = 0; ii < selection.GetSize(); ii++ )
        {
            item = static_cast<SCH_ITEM*>( selection.GetItem( ii ) );

            if( !moving )
                saveCopyInUndoList( item, UNDO_REDO::CHANGED, ii > 0 );

            if( item->Type() == SCH_SHEET_PIN_T )
            {
                if( item->GetParent()->IsSelected() )
                {
                    // parent will mirror us
                }
                else
                {
                    // mirror within parent sheet
                    SCH_SHEET_PIN* pin = static_cast<SCH_SHEET_PIN*>( item );
                    SCH_SHEET*     sheet = pin->GetParent();

                    if( xAxis )
                        pin->MirrorX( sheet->GetBoundingBox().GetCenter().y );
                    else
                        pin->MirrorY( sheet->GetBoundingBox().GetCenter().x );
                }
            }
            else
            {
                if( xAxis )
                    item->MirrorX( mirrorPoint.y );
                else
                    item->MirrorY( mirrorPoint.x );
            }

            connections |= item->IsConnectable();
            m_frame->UpdateItem( item );
        }
    }

    m_toolMgr->PostEvent( EVENTS::SelectedItemsModified );

    // Update R-Tree for modified items
    for( EDA_ITEM* selected : selection )
        updateItem( selected, true );

    if( item->IsMoving() )
    {
        m_toolMgr->RunAction( ACTIONS::refreshPreview );
    }
    else
    {
        if( selection.IsHover() )
            m_toolMgr->RunAction( EE_ACTIONS::clearSelection, true );

        if( connections )
            m_frame->TestDanglingEnds();

        m_frame->OnModify();
    }

    return 0;
}


static KICAD_T duplicatableItems[] =
{
    SCH_JUNCTION_T,
    SCH_LINE_T,
    SCH_BUS_BUS_ENTRY_T,
    SCH_BUS_WIRE_ENTRY_T,
    SCH_TEXT_T,
    SCH_LABEL_T,
    SCH_GLOBAL_LABEL_T,
    SCH_HIER_LABEL_T,
    SCH_NO_CONNECT_T,
    SCH_SHEET_T,
    SCH_COMPONENT_T,
    EOT
};


int SCH_EDIT_TOOL::Duplicate( const TOOL_EVENT& aEvent )
{
    EE_SELECTION& selection = m_selectionTool->RequestSelection( duplicatableItems );

    if( selection.GetSize() == 0 )
        return 0;

    // Doing a duplicate of a new object doesn't really make any sense; we'd just end
    // up dragging around a stack of objects...
    if( selection.Front()->IsNew() )
        return 0;

    EDA_ITEMS newItems;

    // Keep track of existing sheet paths. Duplicating a selection can modify this list
    bool copiedSheets = false;
    SCH_SHEET_LIST initial_sheetpathList = m_frame->Schematic().GetSheets();

    for( unsigned ii = 0; ii < selection.GetSize(); ++ii )
    {
        SCH_ITEM* oldItem = static_cast<SCH_ITEM*>( selection.GetItem( ii ) );
        SCH_ITEM* newItem = oldItem->Duplicate();
        newItem->SetFlags( IS_NEW );
        newItems.push_back( newItem );
        saveCopyInUndoList( newItem, UNDO_REDO::NEWITEM, ii > 0 );

        switch( newItem->Type() )
        {
        case SCH_JUNCTION_T:
        case SCH_LINE_T:
        case SCH_BUS_BUS_ENTRY_T:
        case SCH_BUS_WIRE_ENTRY_T:
        case SCH_TEXT_T:
        case SCH_LABEL_T:
        case SCH_GLOBAL_LABEL_T:
        case SCH_HIER_LABEL_T:
        case SCH_NO_CONNECT_T:
            newItem->SetParent( m_frame->GetScreen() );
            m_frame->AddToScreen( newItem, m_frame->GetScreen() );
            break;

        case SCH_SHEET_T:
        {
            SCH_SHEET_LIST hierarchy     = m_frame->Schematic().GetSheets();
            SCH_SHEET*     sheet         = (SCH_SHEET*) newItem;
            SCH_FIELD&     nameField     = sheet->GetFields()[SHEETNAME];
            wxString       baseName      = nameField.GetText();
            wxString       number;

            while( !baseName.IsEmpty() && wxIsdigit( baseName.Last() ) )
            {
                number = baseName.Last() + number;
                baseName.RemoveLast();
            }

            int      uniquifier = std::max( 0, wxAtoi( number ) ) + 1;
            wxString candidateName = wxString::Format( wxT( "%s%d" ), baseName, uniquifier++ );

            while( hierarchy.NameExists( candidateName ) )
                candidateName = wxString::Format( wxT( "%s%d" ), baseName, uniquifier++ );

            nameField.SetText( candidateName );

            sheet->SetParent( m_frame->GetCurrentSheet().Last() );
            m_frame->AddToScreen( sheet, m_frame->GetScreen() );

            copiedSheets = true;
            break;
        }

        case SCH_COMPONENT_T:
        {
            SCH_COMPONENT* component = (SCH_COMPONENT*) newItem;
            component->ClearAnnotation( NULL );
            component->SetParent( m_frame->GetScreen() );
            m_frame->AddToScreen( component, m_frame->GetScreen() );
            break;
        }

        default:
            break;
        }
    }

    if( copiedSheets )
    {
        // We clear annotation of new sheet paths.
        // Annotation of new components added in current sheet is already cleared.
        SCH_SCREENS screensList( &m_frame->Schematic().Root() );
        screensList.ClearAnnotationOfNewSheetPaths( initial_sheetpathList );
        m_frame->SetSheetNumberAndCount();
    }

    m_toolMgr->RunAction( EE_ACTIONS::clearSelection, true );
    m_toolMgr->RunAction( EE_ACTIONS::addItemsToSel, true, &newItems );
    m_toolMgr->RunAction( EE_ACTIONS::move, false );

    return 0;
}


int SCH_EDIT_TOOL::RepeatDrawItem( const TOOL_EVENT& aEvent )
{
    SCH_ITEM* sourceItem = m_frame->GetRepeatItem();

    if( !sourceItem )
        return 0;

    m_toolMgr->RunAction( EE_ACTIONS::clearSelection, true );

    SCH_ITEM* newItem = sourceItem->Duplicate();
    bool      performDrag = false;

    // If cloning a component then put into 'move' mode.
    if( newItem->Type() == SCH_COMPONENT_T )
    {
        wxPoint cursorPos = (wxPoint) getViewControls()->GetCursorPosition( true );
        newItem->Move( cursorPos - newItem->GetPosition() );
        performDrag = true;
    }
    else
    {
        if( m_isSymbolEditor )
        {
            auto* cfg = Pgm().GetSettingsManager().GetAppSettings<SYMBOL_EDITOR_SETTINGS>();

            if( dynamic_cast<SCH_TEXT*>( newItem ) )
            {
                SCH_TEXT* text = static_cast<SCH_TEXT*>( newItem );
                text->IncrementLabel( cfg->m_Repeat.label_delta );
            }

            newItem->Move( wxPoint( Mils2iu( cfg->m_Repeat.x_step ),
                                    Mils2iu( cfg->m_Repeat.y_step ) ) );
        }
        else
        {
            EESCHEMA_SETTINGS* cfg = Pgm().GetSettingsManager().GetAppSettings<EESCHEMA_SETTINGS>();

            if( dynamic_cast<SCH_TEXT*>( newItem ) )
            {
                SCH_TEXT* text = static_cast<SCH_TEXT*>( newItem );

                // If incrementing tries to go below zero, tell user why the value is repeated

                if( !text->IncrementLabel( cfg->m_Drawing.repeat_label_increment ) )
                    m_frame->ShowInfoBarWarning( _( "Label value cannot go below zero" ), true );
            }

            newItem->Move( wxPoint( Mils2iu( cfg->m_Drawing.default_repeat_offset_x ),
                                    Mils2iu( cfg->m_Drawing.default_repeat_offset_y ) ) );
        }

    }

    newItem->SetFlags( IS_NEW );
    m_frame->AddToScreen( newItem, m_frame->GetScreen() );
    m_frame->SaveCopyInUndoList( m_frame->GetScreen(), newItem, UNDO_REDO::NEWITEM, false );

    m_selectionTool->AddItemToSel( newItem );

    if( performDrag )
        m_toolMgr->RunAction( EE_ACTIONS::move, true );

    newItem->ClearFlags();

    if( newItem->IsConnectable() )
    {
        auto selection = m_selectionTool->GetSelection();

        m_toolMgr->RunAction( EE_ACTIONS::addNeededJunctions, true, &selection );
        m_frame->SchematicCleanUp();
        m_frame->TestDanglingEnds();
    }

    // newItem newItem, now that it has been moved, thus saving new position.
    m_frame->SaveCopyForRepeatItem( newItem );

    return 0;
}


static KICAD_T deletableItems[] =
{
    SCH_MARKER_T,
    SCH_JUNCTION_T,
    SCH_LINE_T,
    SCH_BUS_BUS_ENTRY_T,
    SCH_BUS_WIRE_ENTRY_T,
    SCH_TEXT_T,
    SCH_LABEL_T,
    SCH_GLOBAL_LABEL_T,
    SCH_HIER_LABEL_T,
    SCH_NO_CONNECT_T,
    SCH_SHEET_T,
    SCH_SHEET_PIN_T,
    SCH_COMPONENT_T,
    SCH_BITMAP_T,
    EOT
};


int SCH_EDIT_TOOL::DoDelete( const TOOL_EVENT& aEvent )
{
    SCH_SCREEN*          screen = m_frame->GetScreen();
    auto                 items = m_selectionTool->RequestSelection( deletableItems ).GetItems();
    bool                 appendToUndo = false;
    std::vector<wxPoint> pts;

    if( items.empty() )
        return 0;

    // Don't leave a freed pointer in the selection
    m_toolMgr->RunAction( EE_ACTIONS::clearSelection, true );

    for( EDA_ITEM* item : items )
        item->ClearFlags( STRUCT_DELETED );

    for( EDA_ITEM* item : items )
    {
        SCH_ITEM* sch_item = dynamic_cast<SCH_ITEM*>( item );

        if( !sch_item )
            continue;

        if( sch_item->IsConnectable() )
        {
            std::vector<wxPoint> tmp_pts = sch_item->GetConnectionPoints();
            pts.insert( pts.end(), tmp_pts.begin(), tmp_pts.end() );
        }

        if( sch_item->Type() == SCH_JUNCTION_T )
        {
            sch_item->SetFlags( STRUCT_DELETED );
            // clean up junctions at the end
        }
        else
        {
            sch_item->SetFlags( STRUCT_DELETED );
            saveCopyInUndoList( item, UNDO_REDO::DELETED, appendToUndo );
            appendToUndo = true;

            updateItem( sch_item, false );

            if( sch_item->Type() == SCH_SHEET_PIN_T )
            {
                SCH_SHEET_PIN* pin = (SCH_SHEET_PIN*) sch_item;
                SCH_SHEET*     sheet = pin->GetParent();

                sheet->RemovePin( pin );
            }
            else
            {
                m_frame->RemoveFromScreen( sch_item, m_frame->GetScreen() );
            }

            if( sch_item->Type() == SCH_SHEET_T )
                m_frame->UpdateHierarchyNavigator();
        }
    }

    for( auto point : pts )
    {
        SCH_ITEM* junction = screen->GetItem( point, 0, SCH_JUNCTION_T );

        if( !junction )
            continue;

        if( junction->HasFlag( STRUCT_DELETED ) || !screen->IsJunctionNeeded( point ) )
            m_frame->DeleteJunction( junction, appendToUndo );
    }

    m_frame->TestDanglingEnds();

    m_frame->GetCanvas()->Refresh();
    m_frame->OnModify();

    return 0;
}


#define HITTEST_THRESHOLD_PIXELS 5


int SCH_EDIT_TOOL::DeleteItemCursor( const TOOL_EVENT& aEvent )
{
    std::string  tool = aEvent.GetCommandStr().get();
    PICKER_TOOL* picker = m_toolMgr->GetTool<PICKER_TOOL>();

    m_toolMgr->RunAction( EE_ACTIONS::clearSelection, true );
    m_pickerItem = nullptr;

    // Deactivate other tools; particularly important if another PICKER is currently running
    Activate();

    picker->SetCursor( KICURSOR::REMOVE );

    picker->SetClickHandler(
            [this]( const VECTOR2D& aPosition ) -> bool
            {
                if( m_pickerItem )
                {
                    SCH_ITEM* sch_item = dynamic_cast<SCH_ITEM*>( m_pickerItem );

                    if( sch_item && sch_item->IsLocked() )
                    {
                        STATUS_TEXT_POPUP statusPopup( m_frame );
                        statusPopup.SetText( _( "Item locked." ) );
                        statusPopup.PopupFor( 2000 );
                        statusPopup.Move( wxGetMousePosition() + wxPoint( 20, 20 ) );
                        return true;
                    }

                    EE_SELECTION_TOOL* selectionTool = m_toolMgr->GetTool<EE_SELECTION_TOOL>();
                    selectionTool->UnbrightenItem( m_pickerItem );
                    selectionTool->AddItemToSel( m_pickerItem, true /*quiet mode*/ );
                    m_toolMgr->RunAction( ACTIONS::doDelete, true );
                    m_pickerItem = nullptr;
                }

                return true;
            } );

    picker->SetMotionHandler(
            [this]( const VECTOR2D& aPos )
            {
                EE_COLLECTOR collector;
                collector.m_Threshold = KiROUND( getView()->ToWorld( HITTEST_THRESHOLD_PIXELS ) );
                collector.Collect( m_frame->GetScreen(), deletableItems, (wxPoint) aPos );

                EE_SELECTION_TOOL* selectionTool = m_toolMgr->GetTool<EE_SELECTION_TOOL>();
                selectionTool->GuessSelectionCandidates( collector, aPos );

                EDA_ITEM* item = collector.GetCount() == 1 ? collector[ 0 ] : nullptr;

                if( m_pickerItem != item )
                {
                    if( m_pickerItem )
                        selectionTool->UnbrightenItem( m_pickerItem );

                    m_pickerItem = item;

                    if( m_pickerItem )
                        selectionTool->BrightenItem( m_pickerItem );
                }
            } );

    picker->SetFinalizeHandler(
            [this]( const int& aFinalState )
            {
                if( m_pickerItem )
                    m_toolMgr->GetTool<EE_SELECTION_TOOL>()->UnbrightenItem( m_pickerItem );

                // Wake the selection tool after exiting to ensure the cursor gets updated
                m_toolMgr->RunAction( EE_ACTIONS::selectionActivate, false );
            } );

    m_toolMgr->RunAction( ACTIONS::pickerTool, true, &tool );

    return 0;
}


void SCH_EDIT_TOOL::editFieldText( SCH_FIELD* aField )
{
    // Save old component in undo list if not already in edit, or moving.
    if( aField->GetEditFlags() == 0 )    // i.e. not edited, or moved
        saveCopyInUndoList( aField, UNDO_REDO::CHANGED );

    wxString title = wxString::Format( _( "Edit %s Field" ), aField->GetName() );

    DIALOG_SCH_EDIT_ONE_FIELD dlg( m_frame, title, aField );

    // The footprint field dialog can invoke a KIWAY_PLAYER so we must use a quasi-modal
    if( dlg.ShowQuasiModal() != wxID_OK )
        return;

    dlg.UpdateField( aField, &m_frame->GetCurrentSheet() );

    if( m_frame->eeconfig()->m_AutoplaceFields.enable || aField->GetParent()->Type() == SCH_SHEET_T )
        static_cast<SCH_ITEM*>( aField->GetParent() )->AutoAutoplaceFields( m_frame->GetScreen() );

    m_frame->UpdateItem( aField );
    m_frame->OnModify();

    // This must go after OnModify() so that the connectivity graph will have been updated.
    m_toolMgr->PostEvent( EVENTS::SelectedItemsModified );
}


int SCH_EDIT_TOOL::EditField( const TOOL_EVENT& aEvent )
{
    static KICAD_T Nothing[]        = { EOT };
    static KICAD_T CmpOrReference[] = { SCH_FIELD_LOCATE_REFERENCE_T, SCH_COMPONENT_T, EOT };
    static KICAD_T CmpOrValue[]     = { SCH_FIELD_LOCATE_VALUE_T,     SCH_COMPONENT_T, EOT };
    static KICAD_T CmpOrFootprint[] = { SCH_FIELD_LOCATE_FOOTPRINT_T, SCH_COMPONENT_T, EOT };

    KICAD_T* filter = Nothing;

    if( aEvent.IsAction( &EE_ACTIONS::editReference ) )
        filter = CmpOrReference;
    else if( aEvent.IsAction( &EE_ACTIONS::editValue ) )
        filter = CmpOrValue;
    else if( aEvent.IsAction( &EE_ACTIONS::editFootprint ) )
        filter = CmpOrFootprint;

    EE_SELECTION& selection = m_selectionTool->RequestSelection( filter );

    if( selection.Empty() )
        return 0;

    SCH_ITEM* item = (SCH_ITEM*) selection.Front();

    if( item->Type() == SCH_COMPONENT_T )
    {
        SCH_COMPONENT* component = (SCH_COMPONENT*) item;

        if( aEvent.IsAction( &EE_ACTIONS::editReference ) )
            editFieldText( component->GetField( REFERENCE_FIELD ) );
        else if( aEvent.IsAction( &EE_ACTIONS::editValue ) )
            editFieldText( component->GetField( VALUE_FIELD ) );
        else if( aEvent.IsAction( &EE_ACTIONS::editFootprint ) )
            editFieldText( component->GetField( FOOTPRINT_FIELD ) );
    }
    else if( item->Type() == SCH_FIELD_T )
    {
        editFieldText( (SCH_FIELD*) item );
    }

    if( selection.IsHover() )
        m_toolMgr->RunAction( EE_ACTIONS::clearSelection, true );

    return 0;
}


int SCH_EDIT_TOOL::AutoplaceFields( const TOOL_EVENT& aEvent )
{
    EE_SELECTION& selection = m_selectionTool->RequestSelection( EE_COLLECTOR::FieldOwners );

    if( selection.Empty() )
        return 0;

    SCH_ITEM* item = static_cast<SCH_ITEM*>( selection.Front() );

    if( !item->IsNew() )
        saveCopyInUndoList( item, UNDO_REDO::CHANGED );

    item->AutoplaceFields( m_frame->GetScreen(), /* aManual */ true );

    updateItem( item, true );
    m_frame->OnModify();

    if( selection.IsHover() )
        m_toolMgr->RunAction( EE_ACTIONS::clearSelection, true );

    return 0;
}


int SCH_EDIT_TOOL::ChangeSymbols( const TOOL_EVENT& aEvent )
{
    SCH_COMPONENT* selectedSymbol = nullptr;
    EE_SELECTION& selection = m_selectionTool->RequestSelection( EE_COLLECTOR::ComponentsOnly );

    if( !selection.Empty() )
        selectedSymbol = dynamic_cast<SCH_COMPONENT*>( selection.Front() );

    DIALOG_CHANGE_SYMBOLS::MODE mode = DIALOG_CHANGE_SYMBOLS::MODE::UPDATE;

    if( aEvent.IsAction( &EE_ACTIONS::changeSymbol )
            || aEvent.IsAction( &EE_ACTIONS::changeSymbols ) )
    {
        mode = DIALOG_CHANGE_SYMBOLS::MODE::CHANGE;
    }

    DIALOG_CHANGE_SYMBOLS dlg( m_frame, selectedSymbol, mode );

    dlg.ShowModal();

    return 0;
}


int SCH_EDIT_TOOL::ConvertDeMorgan( const TOOL_EVENT& aEvent )
{
    EE_SELECTION& selection = m_selectionTool->RequestSelection( EE_COLLECTOR::ComponentsOnly );

    if( selection.Empty() )
        return 0;

    SCH_COMPONENT* component = (SCH_COMPONENT*) selection.Front();

    if( aEvent.IsAction( &EE_ACTIONS::showDeMorganStandard )
            && component->GetConvert() == LIB_ITEM::LIB_CONVERT::BASE )
    {
        return 0;
    }

    if( aEvent.IsAction( &EE_ACTIONS::showDeMorganAlternate )
            && component->GetConvert() != LIB_ITEM::LIB_CONVERT::DEMORGAN )
    {
        return 0;
    }

    if( !component->IsNew() )
        saveCopyInUndoList( component, UNDO_REDO::CHANGED );

    m_frame->ConvertPart( component );

    if( component->IsNew() )
        m_toolMgr->RunAction( ACTIONS::refreshPreview );

    if( selection.IsHover() )
        m_toolMgr->RunAction( EE_ACTIONS::clearSelection, true );

    return 0;
}


int SCH_EDIT_TOOL::Properties( const TOOL_EVENT& aEvent )
{
    EE_SELECTION& selection = m_selectionTool->RequestSelection( EE_COLLECTOR::EditableItems );

    if( selection.Empty() )
    {
        if( getView()->IsLayerVisible( LAYER_SCHEMATIC_WORKSHEET ) )
        {
            KIGFX::WS_PROXY_VIEW_ITEM* worksheet = m_frame->GetCanvas()->GetView()->GetWorksheet();
            VECTOR2D cursorPos = getViewControls()->GetCursorPosition( false );

            if( worksheet && worksheet->HitTestWorksheetItems( getView(), (wxPoint) cursorPos ) )
                m_toolMgr->RunAction( ACTIONS::pageSettings );
        }

        return 0;
    }

    SCH_ITEM* item = (SCH_ITEM*) selection.Front();

    switch( item->Type() )
    {
    case SCH_LINE_T:
    case SCH_BUS_WIRE_ENTRY_T:
        if( !selection.AllItemsHaveLineStroke() )
            return 0;

        break;

    case SCH_JUNCTION_T:
        if( !selection.AreAllItemsIdentical() )
            return 0;

        break;

    default:
        if( selection.Size() > 1 )
            return 0;

        break;
    }

    switch( item->Type() )
    {
    case SCH_COMPONENT_T:
    {
        SCH_COMPONENT* component = (SCH_COMPONENT*) item;
        DIALOG_SYMBOL_PROPERTIES symbolPropsDialog( m_frame, component );

        // This dialog itself subsequently can invoke a KIWAY_PLAYER as a quasimodal
        // frame. Therefore this dialog as a modal frame parent, MUST be run under
        // quasimodal mode for the quasimodal frame support to work.  So don't use
        // the QUASIMODAL macros here.
        int retval = symbolPropsDialog.ShowQuasiModal();

        if( retval == SYMBOL_PROPS_EDIT_OK )
        {
            if( m_frame->eeconfig()->m_AutoplaceFields.enable )
                component->AutoAutoplaceFields( m_frame->GetScreen() );

            m_toolMgr->PostEvent( EVENTS::SelectedItemsModified );
            m_frame->OnModify();
        }
        else if( retval == SYMBOL_PROPS_EDIT_SCHEMATIC_SYMBOL )
        {
            auto editor = (SYMBOL_EDIT_FRAME*) m_frame->Kiway().Player( FRAME_SCH_SYMBOL_EDITOR, true );

            editor->LoadSymbolFromSchematic( component->GetPartRef(),
                                             component->GetRef( &m_frame->GetCurrentSheet() ),
                                             component->GetUnit(), component->GetConvert() );

            editor->Show( true );
            editor->Raise();
        }
        else if( retval == SYMBOL_PROPS_EDIT_LIBRARY_SYMBOL )
        {
            auto editor = (SYMBOL_EDIT_FRAME*) m_frame->Kiway().Player( FRAME_SCH_SYMBOL_EDITOR, true );

            editor->LoadSymbolAndSelectLib( component->GetLibId(), component->GetUnit(),
                                            component->GetConvert() );

            editor->Show( true );
            editor->Raise();
        }
        else if( retval == SYMBOL_PROPS_WANT_UPDATE_SYMBOL )
        {
            DIALOG_CHANGE_SYMBOLS dlg( m_frame, component, DIALOG_CHANGE_SYMBOLS::MODE::UPDATE );
            dlg.ShowModal();
        }
        else if( retval == SYMBOL_PROPS_WANT_EXCHANGE_SYMBOL )
        {
            DIALOG_CHANGE_SYMBOLS dlg( m_frame, component, DIALOG_CHANGE_SYMBOLS::MODE::CHANGE );
            dlg.ShowModal();
        }
    }
        break;

    case SCH_SHEET_T:
    {
        SCH_SHEET*     sheet = static_cast<SCH_SHEET*>( item );
        bool           doClearAnnotation;
        bool           doRefresh = false;

        // Keep track of existing sheet paths. EditSheet() can modify this list.
        // Note that we use the validity checking/repairing version here just to make sure
        // we've got a valid hierarchy to begin with.
        SCH_SHEET_LIST initial_sheetpathList( &m_frame->Schematic().Root(), true );

        doRefresh = m_frame->EditSheetProperties( sheet, &m_frame->GetCurrentSheet(),
                                                  &doClearAnnotation );

        // If the sheet file is changed and new sheet contents are loaded then we have to
        // clear the annotations on the new content (as it may have been set from some other
        // sheet path reference)
        if( doClearAnnotation )
        {
            SCH_SCREENS screensList( &m_frame->Schematic().Root() );
            // We clear annotation of new sheet paths here:
            screensList.ClearAnnotationOfNewSheetPaths( initial_sheetpathList );
            // Clear annotation of g_CurrentSheet itself, because its sheetpath is not a new
            // path, but components managed by its sheet path must have their annotation
            // cleared, because they are new:
            sheet->GetScreen()->ClearAnnotation( &m_frame->GetCurrentSheet() );
        }

        if( doRefresh )
        {
            m_toolMgr->PostEvent( EVENTS::SelectedItemsModified );
            m_frame->GetCanvas()->Refresh();
            m_frame->UpdateHierarchyNavigator();
        }

        break;
    }

    case SCH_SHEET_PIN_T:
    {
        SCH_SHEET_PIN*              pin = (SCH_SHEET_PIN*) item;
        DIALOG_SHEET_PIN_PROPERTIES dlg( m_frame, pin );

        // QuasiModal required for help dialog
        if( dlg.ShowQuasiModal() == wxID_OK )
        {
            m_toolMgr->PostEvent( EVENTS::SelectedItemsModified );
            m_frame->OnModify();
        }
    }
        break;

    case SCH_TEXT_T:
    case SCH_LABEL_T:
    case SCH_GLOBAL_LABEL_T:
    case SCH_HIER_LABEL_T:
    {
        DIALOG_LABEL_EDITOR dlg( m_frame, (SCH_TEXT*) item );

        // Must be quasi modal for syntax help
        if( dlg.ShowQuasiModal() == wxID_OK )
        {
            m_toolMgr->PostEvent( EVENTS::SelectedItemsModified );
            m_frame->OnModify();
        }
    }
        break;

    case SCH_FIELD_T:
        editFieldText( (SCH_FIELD*) item );
        break;

    case SCH_BITMAP_T:
    {
        SCH_BITMAP*         bitmap = (SCH_BITMAP*) item;
        DIALOG_IMAGE_EDITOR dlg( m_frame, bitmap->GetImage() );

        if( dlg.ShowModal() == wxID_OK )
        {
            // save old image in undo list if not already in edit
            if( bitmap->GetEditFlags() == 0 )
                saveCopyInUndoList( bitmap, UNDO_REDO::CHANGED );

            dlg.TransferToImage( bitmap->GetImage() );

            // The bitmap is cached in Opengl: clear the cache in case it has become invalid
            getView()->RecacheAllItems();
            m_toolMgr->PostEvent( EVENTS::SelectedItemsModified );
            m_frame->OnModify();
        }
    }
        break;

    case SCH_LINE_T:
    case SCH_BUS_WIRE_ENTRY_T:
    {
        std::deque<SCH_ITEM*> strokeItems;

        for( auto selItem : selection.Items() )
        {
            SCH_ITEM* schItem = dynamic_cast<SCH_ITEM*>( selItem );

            if( schItem && schItem->HasLineStroke() )
                strokeItems.push_back( schItem );
            else
                return 0;
        }

        DIALOG_EDIT_LINE_STYLE dlg( m_frame, strokeItems );

        if( dlg.ShowModal() == wxID_OK )
        {
            m_toolMgr->PostEvent( EVENTS::SelectedItemsModified );
            m_frame->OnModify();
        }
    }
    break;

    case SCH_JUNCTION_T:
    {
        std::deque<SCH_JUNCTION*> junctions;

        for( auto selItem : selection.Items() )
        {
            SCH_JUNCTION* junction = dynamic_cast<SCH_JUNCTION*>( selItem );

            wxCHECK( junction, 0 );

            junctions.push_back( junction );
        }

        DIALOG_JUNCTION_PROPS dlg( m_frame, junctions );

        if( dlg.ShowModal() == wxID_OK )
        {
            m_toolMgr->PostEvent( EVENTS::SelectedItemsModified );
            m_frame->OnModify();
        }
    }
        break;

    case SCH_MARKER_T:        // These items have no properties to edit
    case SCH_NO_CONNECT_T:
        break;

    default:                // Unexpected item
        wxFAIL_MSG( wxString( "Cannot edit schematic item type " ) + item->GetClass() );
    }

    updateItem( item, true );

    if( selection.IsHover() )
        m_toolMgr->RunAction( EE_ACTIONS::clearSelection, true );

    return 0;
}


int SCH_EDIT_TOOL::ChangeTextType( const TOOL_EVENT& aEvent )
{
    KICAD_T       convertTo = aEvent.Parameter<KICAD_T>();
    KICAD_T       allTextTypes[] = { SCH_LABEL_T, SCH_GLOBAL_LABEL_T, SCH_HIER_LABEL_T, SCH_TEXT_T, EOT };
    EE_SELECTION& selection = m_selectionTool->RequestSelection( allTextTypes );

    for( unsigned int i = 0; i < selection.GetSize(); ++i )
    {
        SCH_TEXT* text = dynamic_cast<SCH_TEXT*>( selection.GetItem( i ) );

        if( text && text->Type() != convertTo )
        {
            bool             selected    = text->IsSelected();
            SCH_TEXT*        newtext     = nullptr;
            const wxPoint&   position    = text->GetPosition();
            LABEL_SPIN_STYLE orientation = text->GetLabelSpinStyle();
            wxString         txt         = UnescapeString( text->GetText() );

            // There can be characters in a SCH_TEXT object that can break labels so we have to
            // fix them here.
            if( text->Type() == SCH_TEXT_T )
            {
                txt.Replace( "\n", "_" );
                txt.Replace( "\r", "_" );
                txt.Replace( "\t", "_" );
                txt.Replace( " ", "_" );
            }

            // label strings are "escaped" i.e. a '/' is replaced by "{slash}"
            if( convertTo != SCH_TEXT_T )
                txt = EscapeString( txt, CTX_NETNAME );

            switch( convertTo )
            {
            case SCH_LABEL_T:        newtext = new SCH_LABEL( position, txt );        break;
            case SCH_GLOBAL_LABEL_T: newtext = new SCH_GLOBALLABEL( position, txt );  break;
            case SCH_HIER_LABEL_T:   newtext = new SCH_HIERLABEL( position, txt );    break;
            case SCH_TEXT_T:         newtext = new SCH_TEXT( position, txt );         break;

            default:
                wxFAIL_MSG( wxString::Format( "Invalid text type: %d.", convertTo ) );
                return 0;
            }

            // Copy the old text item settings to the new one.  Justifications are not copied
            // because they are not used in labels.  Justifications will be set to default value
            // in the new text item type.
            //
            newtext->SetFlags( text->GetEditFlags() );
            newtext->SetShape( text->GetShape() );
            newtext->SetLabelSpinStyle( orientation );
            newtext->SetTextSize( text->GetTextSize() );
            newtext->SetTextThickness( text->GetTextThickness() );
            newtext->SetItalic( text->IsItalic() );
            newtext->SetBold( text->IsBold() );
            newtext->SetIsDangling( text->IsDangling() );

            if( selected )
                m_toolMgr->RunAction( EE_ACTIONS::removeItemFromSel, true, text );

            if( !text->IsNew() )
            {
                saveCopyInUndoList( text, UNDO_REDO::DELETED );
                saveCopyInUndoList( newtext, UNDO_REDO::NEWITEM, true );

                m_frame->RemoveFromScreen( text, m_frame->GetScreen() );
                m_frame->AddToScreen( newtext, m_frame->GetScreen() );
            }

            if( selected )
                m_toolMgr->RunAction( EE_ACTIONS::addItemToSel, true, newtext );

            // Otherwise, pointer is owned by the undo stack
            if( text->IsNew() )
                delete text;

            if( convertTo == SCH_TEXT_T )
            {
                if( newtext->IsDangling() )
                {
                    newtext->SetIsDangling( false );
                    getView()->Update( newtext, KIGFX::REPAINT );
                }
            }
            else
                m_frame->TestDanglingEnds();

            m_frame->OnModify();
        }
    }

    if( selection.IsHover() )
        m_toolMgr->RunAction( EE_ACTIONS::clearSelection, true );

    return 0;
}


int SCH_EDIT_TOOL::BreakWire( const TOOL_EVENT& aEvent )
{
    auto cursorPos = wxPoint( getViewControls()->GetCursorPosition( !aEvent.Modifier( MD_ALT ) ) );

    if( m_frame->BreakSegments( cursorPos ) )
    {
        if( m_frame->GetScreen()->IsJunctionNeeded( cursorPos, true ) )
            m_frame->AddJunction( m_frame->GetScreen(), cursorPos, true, false );

        m_frame->TestDanglingEnds();

        m_frame->OnModify();
        m_frame->GetCanvas()->Refresh();
    }

    return 0;
}


int SCH_EDIT_TOOL::CleanupSheetPins( const TOOL_EVENT& aEvent )
{
    EE_SELECTION& selection = m_selectionTool->RequestSelection( EE_COLLECTOR::SheetsOnly );
    SCH_SHEET*    sheet = (SCH_SHEET*) selection.Front();

    if( !sheet )
        return 0;

    if( !sheet->HasUndefinedPins() )
    {
        DisplayInfoMessage( m_frame, _( "There are no unreferenced pins in this sheet to remove." ) );
        return 0;
    }

    if( !IsOK( m_frame, _( "Do you wish to delete the unreferenced pins from this sheet?" ) ) )
        return 0;

    saveCopyInUndoList( sheet, UNDO_REDO::CHANGED );

    sheet->CleanupSheet();

    updateItem( sheet, true );
    m_frame->OnModify();

    if( selection.IsHover() )
        m_toolMgr->RunAction( EE_ACTIONS::clearSelection, true );

    return 0;
}


int SCH_EDIT_TOOL::EditPageNumber( const TOOL_EVENT& aEvent )
{
    EE_SELECTION& selection = m_selectionTool->RequestSelection( EE_COLLECTOR::SheetsOnly );

    if( selection.GetSize() > 1 )
        return 0;

    SCH_SHEET* sheet = (SCH_SHEET*) selection.Front();

    SCH_SHEET_PATH instance = m_frame->GetCurrentSheet();

    SCH_SCREEN* screen;

    if( sheet )
    {
        // When changing the page number of a selected sheet, the current screen owns the sheet.
        screen = m_frame->GetScreen();

        instance.push_back( sheet );
    }
    else
    {
        SCH_SHEET_PATH prevInstance = instance;

        // When change the page number in the screen, the previous screen owns the sheet.
        if( prevInstance.size() )
        {
            prevInstance.pop_back();
            screen = prevInstance.LastScreen();
        }
        else
        {
            // The root sheet and root screen are effectively the same thing.
            screen = m_frame->GetScreen();
        }

        sheet = m_frame->GetCurrentSheet().Last();
    }

    wxString msg;
    wxString sheetPath = instance.PathHumanReadable( false );
    wxString pageNumber = instance.GetPageNumber();

    msg.Printf( _( "Enter page number for sheet path%s" ),
                ( sheetPath.Length() > 20 ) ? "\n" + sheetPath : " " + sheetPath );

    wxTextEntryDialog dlg( m_frame, msg, _( "Edit Page Number" ), pageNumber );

    dlg.SetTextValidator( wxFILTER_ALPHANUMERIC );  // No white space.

    if( dlg.ShowModal() == wxID_CANCEL || dlg.GetValue() == instance.GetPageNumber() )
        return 0;

    m_frame->SaveCopyInUndoList( screen, sheet, UNDO_REDO::CHANGED, false );

    instance.SetPageNumber( dlg.GetValue() );

    if( instance == m_frame->GetCurrentSheet() )
    {
        m_frame->GetScreen()->SetPageNumber( dlg.GetValue() );
        m_frame->OnPageSettingsChange();
    }

    m_frame->OnModify();

    return 0;
}


void SCH_EDIT_TOOL::setTransitions()
{
    Go( &SCH_EDIT_TOOL::Duplicate,          ACTIONS::duplicate.MakeEvent() );
    Go( &SCH_EDIT_TOOL::RepeatDrawItem,     EE_ACTIONS::repeatDrawItem.MakeEvent() );
    Go( &SCH_EDIT_TOOL::Rotate,             EE_ACTIONS::rotateCW.MakeEvent() );
    Go( &SCH_EDIT_TOOL::Rotate,             EE_ACTIONS::rotateCCW.MakeEvent() );
    Go( &SCH_EDIT_TOOL::Mirror,             EE_ACTIONS::mirrorX.MakeEvent() );
    Go( &SCH_EDIT_TOOL::Mirror,             EE_ACTIONS::mirrorY.MakeEvent() );
    Go( &SCH_EDIT_TOOL::DoDelete,           ACTIONS::doDelete.MakeEvent() );
    Go( &SCH_EDIT_TOOL::DeleteItemCursor,   ACTIONS::deleteTool.MakeEvent() );

    Go( &SCH_EDIT_TOOL::Properties,         EE_ACTIONS::properties.MakeEvent() );
    Go( &SCH_EDIT_TOOL::EditField,          EE_ACTIONS::editReference.MakeEvent() );
    Go( &SCH_EDIT_TOOL::EditField,          EE_ACTIONS::editValue.MakeEvent() );
    Go( &SCH_EDIT_TOOL::EditField,          EE_ACTIONS::editFootprint.MakeEvent() );
    Go( &SCH_EDIT_TOOL::AutoplaceFields,    EE_ACTIONS::autoplaceFields.MakeEvent() );
    Go( &SCH_EDIT_TOOL::ChangeSymbols,      EE_ACTIONS::changeSymbols.MakeEvent() );
    Go( &SCH_EDIT_TOOL::ChangeSymbols,      EE_ACTIONS::updateSymbols.MakeEvent() );
    Go( &SCH_EDIT_TOOL::ChangeSymbols,      EE_ACTIONS::changeSymbol.MakeEvent() );
    Go( &SCH_EDIT_TOOL::ChangeSymbols,      EE_ACTIONS::updateSymbol.MakeEvent() );
    Go( &SCH_EDIT_TOOL::ConvertDeMorgan,    EE_ACTIONS::toggleDeMorgan.MakeEvent() );
    Go( &SCH_EDIT_TOOL::ConvertDeMorgan,    EE_ACTIONS::showDeMorganStandard.MakeEvent() );
    Go( &SCH_EDIT_TOOL::ConvertDeMorgan,    EE_ACTIONS::showDeMorganAlternate.MakeEvent() );
    Go( &SCH_EDIT_TOOL::ChangeTextType,     EE_ACTIONS::toLabel.MakeEvent() );
    Go( &SCH_EDIT_TOOL::ChangeTextType,     EE_ACTIONS::toHLabel.MakeEvent() );
    Go( &SCH_EDIT_TOOL::ChangeTextType,     EE_ACTIONS::toGLabel.MakeEvent() );
    Go( &SCH_EDIT_TOOL::ChangeTextType,     EE_ACTIONS::toText.MakeEvent() );

    Go( &SCH_EDIT_TOOL::BreakWire,          EE_ACTIONS::breakWire.MakeEvent() );
    Go( &SCH_EDIT_TOOL::BreakWire,          EE_ACTIONS::breakBus.MakeEvent() );

    Go( &SCH_EDIT_TOOL::CleanupSheetPins,   EE_ACTIONS::cleanupSheetPins.MakeEvent() );
    Go( &SCH_EDIT_TOOL::GlobalEdit,         EE_ACTIONS::editTextAndGraphics.MakeEvent() );
    Go( &SCH_EDIT_TOOL::EditPageNumber,     EE_ACTIONS::editPageNumber.MakeEvent() );
}
