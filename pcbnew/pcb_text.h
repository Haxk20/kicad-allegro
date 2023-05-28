/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2004 Jean-Pierre Charras, jaen-pierre.charras@gipsa-lab.inpg.com
 * Copyright (C) 1992-2023 KiCad Developers, see AUTHORS.txt for contributors.
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

#ifndef PCB_TEXT_H
#define PCB_TEXT_H

#include <eda_text.h>
#include <board_item.h>


class LINE_READER;
class MSG_PANEL_ITEM;


class PCB_TEXT : public BOARD_ITEM, public EDA_TEXT
{
public:
    PCB_TEXT( BOARD_ITEM* parent, KICAD_T idtype = PCB_TEXT_T );

    // Do not create a copy constructor & operator=.
    // The ones generated by the compiler are adequate.

    ~PCB_TEXT();

    static inline bool ClassOf( const EDA_ITEM* aItem )
    {
        return aItem && PCB_TEXT_T == aItem->Type();
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

    wxString GetShownText( bool aAllowExtraText, int aDepth = 0 ) const override;

    /// PCB_TEXTs are always visible:
    void SetVisible( bool aVisible ) override { /* do nothing */}
    bool IsVisible() const override { return true; }

    bool Matches( const EDA_SEARCH_DATA& aSearchData, void* aAuxData ) const override;

    virtual VECTOR2I GetPosition() const override
    {
        return EDA_TEXT::GetTextPos();
    }

    virtual void SetPosition( const VECTOR2I& aPos ) override
    {
        EDA_TEXT::SetTextPos( aPos );
    }

    void Move( const VECTOR2I& aMoveVector ) override
    {
        EDA_TEXT::Offset( aMoveVector );
    }

    void Rotate( const VECTOR2I& aRotCentre, const EDA_ANGLE& aAngle ) override;

    void Mirror( const VECTOR2I& aCentre, bool aMirrorAroundXAxis );

    void Flip( const VECTOR2I& aCentre, bool aFlipLeftRight ) override;

    void GetMsgPanelInfo( EDA_DRAW_FRAME* aFrame, std::vector<MSG_PANEL_ITEM>& aList ) override;

    bool TextHitTest( const VECTOR2I& aPoint, int aAccuracy = 0 ) const override;
    bool TextHitTest( const BOX2I& aRect, bool aContains, int aAccuracy = 0 ) const override;

    bool HitTest( const VECTOR2I& aPosition, int aAccuracy ) const override
    {
        return TextHitTest( aPosition, aAccuracy );
    }

    bool HitTest( const BOX2I& aRect, bool aContained, int aAccuracy = 0 ) const override
    {
        return TextHitTest( aRect, aContained, aAccuracy );
    }

    wxString GetClass() const override
    {
        return wxT( "PCB_TEXT" );
    }

    /**
     * Function TransformTextToPolySet
     * Convert the text to a polygonSet describing the actual character strokes (one per segment).
     * Circles and arcs are approximated by segments.
     * @aBuffer = SHAPE_POLY_SET to store the polygon corners
     * @aClearance = the clearance around the text
     * @aError = the maximum error to allow when approximating curves
     */
    void TransformTextToPolySet( SHAPE_POLY_SET& aBuffer, int aClearance, int aError,
                                 ERROR_LOC aErrorLoc ) const;

    void TransformShapeToPolygon( SHAPE_POLY_SET& aBuffer, PCB_LAYER_ID aLayer, int aClearance,
                                  int aError, ERROR_LOC aErrorLoc,
                                  bool aIgnoreLineWidth = false ) const override;

    // @copydoc BOARD_ITEM::GetEffectiveShape
    virtual std::shared_ptr<SHAPE>
    GetEffectiveShape( PCB_LAYER_ID aLayer = UNDEFINED_LAYER,
                       FLASHING aFlash = FLASHING::DEFAULT ) const override;

    wxString GetItemDescription( UNITS_PROVIDER* aUnitsProvider ) const override;

    BITMAPS GetMenuImage() const override;

    ///< @copydoc VIEW_ITEM::ViewGetLOD
    double ViewGetLOD( int aLayer, KIGFX::VIEW* aView ) const override;

    // Virtual function
    const BOX2I GetBoundingBox() const override;

    EDA_ITEM* Clone() const override;

#if defined(DEBUG)
    virtual void Show( int nestLevel, std::ostream& os ) const override { ShowDummy( os ); }
#endif

protected:
    /**
     * Build a nominally rectangular bounding box for the rendered text.  (It's not a BOX2I
     * because it will be a diamond shape for non-cardinally rotated text.)
     */
    void buildBoundingHull( SHAPE_POLY_SET* aBuffer, const SHAPE_POLY_SET& aRenderedText,
                            int aClearance ) const;

    virtual void swapData( BOARD_ITEM* aImage ) override;

    int getKnockoutMargin() const;
};

#endif  // #define PCB_TEXT_H
