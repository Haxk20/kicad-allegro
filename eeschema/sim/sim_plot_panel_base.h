/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2016-2023 CERN
 * Copyright (C) 2016-2023 KiCad Developers, see AUTHORS.txt for contributors.
 * @author Sylwester Kocjan <s.kocjan@o2.pl>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * https://www.gnu.org/licenses/gpl-3.0.html
 * or you may search the http://www.gnu.org website for the version 3 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifndef __SIM_PLOT_PANEL_BASE_H
#define __SIM_PLOT_PANEL_BASE_H

#include "sim_types.h"
#include "ngspice_circuit_model.h"
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>


class SIM_PLOT_PANEL_BASE : public wxWindow
{
public:
    SIM_PLOT_PANEL_BASE();
    SIM_PLOT_PANEL_BASE( const wxString& aCommand, int aOptions, wxWindow* parent, wxWindowID id,
                         const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
                         long style = 0, const wxString& name = wxPanelNameStr );
    virtual ~SIM_PLOT_PANEL_BASE();

    static bool IsPlottable( SIM_TYPE aSimType );

    virtual void OnLanguageChanged() = 0;

    SIM_TYPE GetType() const;

    const wxString& GetSimCommand() const { return m_simCommand; }
    void SetSimCommand( const wxString& aSimCommand )
    {
        wxCHECK_RET( GetType() == NGSPICE_CIRCUIT_MODEL::CommandToSimType( aSimCommand ),
                     "Cannot change the type of simulation of the existing plot panel" );

        m_simCommand = aSimCommand;
    }

    int GetSimOptions() const { return m_simOptions; }
    void SetSimOptions( int aOptions ) { m_simOptions = aOptions; }

private:
    wxString m_simCommand;
    int      m_simOptions;
};


class SIM_NOPLOT_PANEL : public SIM_PLOT_PANEL_BASE
{
public:
    SIM_NOPLOT_PANEL( const wxString& aCommand, int aOptions, wxWindow* parent, wxWindowID id,
                      const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
                      long style = 0, const wxString& name = wxPanelNameStr );

    virtual ~SIM_NOPLOT_PANEL();

    void OnLanguageChanged() override;

private:
    wxSizer*      m_sizer;
    wxStaticText* m_textInfo;
};


#endif