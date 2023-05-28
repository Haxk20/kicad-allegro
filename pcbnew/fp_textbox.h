/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2022 KiCad Developers, see AUTHORS.txt for contributors.
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

#ifndef FP_TEXTBOX_H
#define FP_TEXTBOX_H

#include <eda_text.h>
#include <fp_shape.h>

class LINE_READER;
class FOOTPRINT;
class MSG_PANEL_ITEM;
class PCB_BASE_FRAME;
class SHAPE;


class FP_TEXTBOX : public FP_SHAPE, public EDA_TEXT
{
public:
    FP_TEXTBOX( FOOTPRINT* aParentFootprint );

    // Do not create a copy constructor & operator=.
    // The ones generated by the compiler are adequate.

    ~FP_TEXTBOX();

    static inline bool ClassOf( const EDA_ITEM* aItem )
    {
        return aItem && aItem->Type() == PCB_FP_TEXT_T;
    }

    bool IsType( const std::vector<KICAD_T>& aScanTypes ) const override
    {
        if( BOARD_ITEM::IsType( aScanTypes ) )
            return true;

        for( KICAD_T scanType : aScanTypes )
        {
            if( scanType == PCB_LOCATE_TEXT_T )
                return true;
        }

        return false;
    }

    VECTOR2I GetTopLeft() const override;
    VECTOR2I GetBotRight() const override;

    void SetTop( int aVal ) override;
    void SetLeft( int aVal ) override;
    void SetRight( int aVal ) override;
    void SetBottom( int aVal ) override;

    wxString GetParentAsString() const;

    bool Matches( const EDA_SEARCH_DATA& aSearchData, void* aAuxData ) const override
    {
        return BOARD_ITEM::Matches( GetShownText( false ), aSearchData );
    }

    int GetTextMargin() const;

    virtual EDA_ANGLE GetDrawRotation() const override;

    VECTOR2I GetDrawPos() const override;

    std::vector<VECTOR2I> GetCorners() const override;
    std::vector<VECTOR2I> GetNormalizedCorners() const;

    void Move( const VECTOR2I& aMoveVector ) override;

    void Rotate( const VECTOR2I& aOffset, const EDA_ANGLE& aAngle ) override;

    void Flip( const VECTOR2I& aCentre, bool aFlipLeftRight ) override;

    /**
     * Mirror the textbox's position, but not the text (or its justification).
     */
    void Mirror( const VECTOR2I& aCentre, bool aMirrorAroundXAxis ) override;

    void GetMsgPanelInfo( EDA_DRAW_FRAME* aFrame, std::vector<MSG_PANEL_ITEM>& aList ) override;

    bool HitTest( const VECTOR2I& aPosition, int aAccuracy ) const override;
    bool HitTest( const BOX2I& aRect, bool aContained, int aAccuracy = 0 ) const override;

    void TransformShapeToPolygon( SHAPE_POLY_SET& aBuffer, PCB_LAYER_ID aLayer, int aClearance,
                                  int aError, ERROR_LOC aErrorLoc,
                                  bool aIgnoreLineWidth = false ) const override;

    void TransformTextToPolySet( SHAPE_POLY_SET& aBuffer, int aClearance, int aError,
                                 ERROR_LOC aErrorLoc ) const;

    // @copydoc BOARD_ITEM::GetEffectiveShape
    std::shared_ptr<SHAPE> GetEffectiveShape( PCB_LAYER_ID aLayer = UNDEFINED_LAYER,
            FLASHING aFlash = FLASHING::DEFAULT ) const override;

    wxString GetClass() const override
    {
        return wxT( "FP_TEXTBOX" );
    }

    wxString GetItemDescription( UNITS_PROVIDER* aUnitsProvider ) const override;

    BITMAPS GetMenuImage() const override;

    EDA_ITEM* Clone() const override;

    virtual wxString GetShownText( bool aAllowExtraText, int aDepth = 0 ) const override;

    virtual void ViewGetLayers( int aLayers[], int& aCount ) const override;

    double ViewGetLOD( int aLayer, KIGFX::VIEW* aView ) const override;

#if defined(DEBUG)
    virtual void Show( int nestLevel, std::ostream& os ) const override { ShowDummy( os ); }
#endif

private:
    VECTOR2I  m_Pos0;           ///< text coordinates relative to the footprint anchor, orient 0.
                                ///< text coordinate ref point is the text center
};

#endif // FP_TEXTBOX_H
