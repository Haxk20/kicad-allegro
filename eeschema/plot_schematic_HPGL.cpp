/** @file plot_schematic_HPGL.cpp
 */

/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 1992-2010 Jean-Pierre Charras jp.charras at wanadoo.fr
 * Copyright (C) 1992-2021 KiCad Developers, see AUTHORS.txt for contributors.
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

#include <fctsys.h>
#include <plotter.h>
#include <sch_edit_frame.h>
#include <base_units.h>
#include <sch_sheet_path.h>
#include <project.h>

#include <dialog_plot_schematic.h>
#include <wx_html_report_panel.h>

enum HPGL_PAGEZ_T {
    PAGE_DEFAULT = 0,
    HPGL_PAGE_SIZE_A4,
    HPGL_PAGE_SIZE_A3,
    HPGL_PAGE_SIZE_A2,
    HPGL_PAGE_SIZE_A1,
    HPGL_PAGE_SIZE_A0,
    HPGL_PAGE_SIZE_A,
    HPGL_PAGE_SIZE_B,
    HPGL_PAGE_SIZE_C,
    HPGL_PAGE_SIZE_D,
    HPGL_PAGE_SIZE_E,
};


static const wxChar* plot_sheet_list( int aSize )
{
    switch( aSize )
    {
    default:
    case PAGE_DEFAULT:      return nullptr;
    case HPGL_PAGE_SIZE_A4: return wxT( "A4" );
    case HPGL_PAGE_SIZE_A3: return wxT( "A3" );
    case HPGL_PAGE_SIZE_A2: return wxT( "A2" );
    case HPGL_PAGE_SIZE_A1: return wxT( "A1" );
    case HPGL_PAGE_SIZE_A0: return wxT( "A0" );
    case HPGL_PAGE_SIZE_A:  return wxT( "A" );
    case HPGL_PAGE_SIZE_B:  return wxT( "B" );
    case HPGL_PAGE_SIZE_C:  return wxT( "C" );
    case HPGL_PAGE_SIZE_D:  return wxT( "D" );
    case HPGL_PAGE_SIZE_E:  return wxT( "E" );
    }
}


void DIALOG_PLOT_SCHEMATIC::SetHPGLPenWidth()
{
    m_HPGLPenSize = m_penWidth.GetValue();

    if( m_HPGLPenSize > Millimeter2iu( 2 ) )
        m_HPGLPenSize = Millimeter2iu( 2 );

    if( m_HPGLPenSize < Millimeter2iu( 0.01 ) )
        m_HPGLPenSize = Millimeter2iu( 0.01 );
}


void DIALOG_PLOT_SCHEMATIC::createHPGLFile( bool aPlotAll, bool aPlotFrameRef )
{
    SCH_SCREEN*     screen = m_parent->GetScreen();
    SCH_SHEET_PATH  oldsheetpath = m_parent->GetCurrentSheet();

    /* When printing all pages, the printed page is not the current page.
     *  In complex hierarchies, we must setup references and other parameters
     *  in the printed SCH_SCREEN
     *  because in complex hierarchies a SCH_SCREEN (a schematic drawings)
     *  is shared between many sheets
     */
    SCH_SHEET_LIST  sheetList;

    if( aPlotAll )
        sheetList.BuildSheetList( g_RootSheet, false );
    else
        sheetList.push_back( m_parent->GetCurrentSheet() );

    REPORTER& reporter = m_MessagesBox->Reporter();

    SetHPGLPenWidth();

    for( unsigned i = 0; i < sheetList.size(); i++ )
    {
        m_parent->SetCurrentSheet( sheetList[i] );
        m_parent->GetCurrentSheet().UpdateAllScreenReferences();
        m_parent->SetSheetNumberAndCount();

        screen = m_parent->GetCurrentSheet().LastScreen();

        if( !screen ) // LastScreen() may return NULL
            screen = m_parent->GetScreen();

        const PAGE_INFO&    curPage = screen->GetPageSettings();

        PAGE_INFO           plotPage = curPage;

        // if plotting on a page size other than curPage
        if( m_paperSizeOption->GetSelection() != PAGE_DEFAULT )
            plotPage.SetType( plot_sheet_list( m_paperSizeOption->GetSelection() ) );

        // Calculation of conversion scales.
        double  plot_scale = (double) plotPage.GetWidthMils() / curPage.GetWidthMils();

        // Calculate offsets
        wxPoint plotOffset;
        wxString msg;

        if( GetPlotOriginCenter() )
        {
            plotOffset.x    = plotPage.GetWidthIU() / 2;
            plotOffset.y    = -plotPage.GetHeightIU() / 2;
        }

        try
        {
            wxString fname = m_parent->GetUniqueFilenameForCurrentSheet();
            wxString ext = HPGL_PLOTTER::GetDefaultFileExtension();
            wxFileName plotFileName = createPlotFileName( m_outputDirectoryName, fname,
                                                          ext, &reporter );

            LOCALE_IO toggle;

            if( Plot_1_Page_HPGL( plotFileName.GetFullPath(), screen, plotPage, plotOffset,
                                  plot_scale, aPlotFrameRef ) )
            {
                msg.Printf( _( "Plot: \"%s\" OK.\n" ), GetChars( plotFileName.GetFullPath() ) );
                reporter.Report( msg, REPORTER::RPT_ACTION );
            }
            else
            {
                msg.Printf( _( "Unable to create file \"%s\".\n" ),
                            GetChars( plotFileName.GetFullPath() ) );
                reporter.Report( msg, REPORTER::RPT_ERROR );
            }
        }
        catch( IO_ERROR& e )
        {
            msg.Printf( wxT( "HPGL Plotter exception: %s"), GetChars( e.What() ) );
            reporter.Report( msg, REPORTER::RPT_ERROR );
        }

    }

    m_parent->SetCurrentSheet( oldsheetpath );
    m_parent->GetCurrentSheet().UpdateAllScreenReferences();
    m_parent->SetSheetNumberAndCount();
}


bool DIALOG_PLOT_SCHEMATIC::Plot_1_Page_HPGL( const wxString&   aFileName,
                                              SCH_SCREEN*       aScreen,
                                              const PAGE_INFO&  aPageInfo,
                                              wxPoint           aPlot0ffset,
                                              double            aScale,
                                              bool              aPlotFrameRef )
{
    HPGL_PLOTTER* plotter = new HPGL_PLOTTER();

    plotter->SetPageSettings( aPageInfo );
    // Currently, plot units are in decimil
    plotter->SetViewport( aPlot0ffset, IU_PER_MILS/10, aScale, false );

    // Init :
    plotter->SetCreator( wxT( "Eeschema-HPGL" ) );

    if( ! plotter->OpenFile( aFileName ) )
    {
        delete plotter;
        return false;
    }

    LOCALE_IO toggle;

    // Pen num and pen speed are not initialized here.
    // Default HPGL driver values are used
    plotter->SetPenDiameter( m_HPGLPenSize );
    plotter->StartPlot();

    plotter->SetColor( BLACK );

    if( getPlotFrameRef() )
        PlotWorkSheet( plotter, m_parent->GetTitleBlock(),
                       m_parent->GetPageSettings(),
                       aScreen->m_ScreenNumber, aScreen->m_NumberOfScreens,
                       m_parent->GetScreenDesc(),
                       aScreen->GetFileName(),
                       GetLayerColor( ( SCH_LAYER_ID )LAYER_WORKSHEET ) );

    aScreen->Plot( plotter );

    plotter->EndPlot();
    delete plotter;

    return true;
}
