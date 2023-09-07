/*
 * This program source code file is part of KICAD, a free EDA CAD application.
 *
 * Copyright (C) 2017 Bernhard Stegmaier <stegmaier@sw-systems.de>
 * Copyright (C) 2016-2021 Kicad Developers, see AUTHORS.txt for contributors.
 *
 * Base class for HiDPI aware wxGLCanvas implementations.
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

#ifndef HIDPI_GL_CANVAS_H
#define HIDPI_GL_CANVAS_H

#include <pgm_base.h>
#include <settings/common_settings.h>
#include <view/view_controls.h>
#include <wx/glcanvas.h>


/**
 * wxGLCanvas wrapper for HiDPI/Retina support.
 *
 * This is a small wrapper class to enable HiDPI/Retina support for wxGLCanvas.
 */
class HIDPI_GL_CANVAS : public wxGLCanvas
{
public:
    // wxGLCanvas constructor
    HIDPI_GL_CANVAS( const KIGFX::VC_SETTINGS& aSettings, wxWindow* aParent,
                     wxWindowID aId = wxID_ANY, const int* aAttribList = nullptr,
                     const wxPoint& aPos = wxDefaultPosition, const wxSize& aSize = wxDefaultSize,
                     long aStyle = 0, const wxString& aName = wxGLCanvasName,
                     const wxPalette& aPalette = wxNullPalette );

    virtual wxSize GetNativePixelSize() const;

    /**
     * Convert the given point from client coordinates to native pixel coordinates.
     */
    wxPoint GetNativePosition( const wxPoint& aPoint ) const;

    /**
     * Set the canvas scale factor, probably for a hi-DPI display.
     */
    void SetScaleFactor( double aFactor );

    /**
     * Get the current scale factor
     */
    double GetScaleFactor() const;

    void SetVcSettings( const KIGFX::VC_SETTINGS& aVcSettings ) { m_settings = aVcSettings; }

protected:
    ///< Current VIEW_CONTROLS settings.
    KIGFX::VC_SETTINGS m_settings;

private:
    /**
     * The current scale factor (e.g. for hi-DPI displays)
     */
    double m_scale_factor;
};

#endif // HIDPI_GL_CANVAS_H
