/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2015-2016 Mario Luzeiro <mrluzeiro@ua.pt>
 * Copyright (C) 2023 CERN
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

/**
 * @file  create_layer_items.cpp
 * @brief This file implements the creation of the pcb board.
 *
 * It is based on the function found in the files:
 *  board_items_to_polygon_shape_transform.cpp
 *  board_items_to_polygon_shape_transform.cpp
 */

#include "board_adapter.h"
#include "../3d_rendering/raytracing/shapes2D/filled_circle_2d.h"
#include <board_design_settings.h>
#include <footprint.h>
#include <pad.h>
#include <pcb_text.h>
#include <pcb_textbox.h>
#include <pcb_shape.h>
#include <zone.h>
#include <convert_basic_shapes_to_polygon.h>
#include <trigo.h>
#include <vector>
#include <thread>
#include <core/arraydim.h>
#include <algorithm>
#include <atomic>
#include <wx/log.h>

#ifdef PRINT_STATISTICS_3D_VIEWER
#include <core/profile.h>
#endif




/*
 * This is used to draw pad outlines on silk layers.
 */
void buildPadOutlineAsPolygon( const PAD* aPad, SHAPE_POLY_SET& aBuffer, int aWidth, int aMaxError,
                               ERROR_LOC aErrorLoc )
{
    if( aPad->GetShape() == PAD_SHAPE::CIRCLE )    // Draw a ring
    {
        TransformRingToPolygon( aBuffer, aPad->ShapePos(), aPad->GetSize().x / 2, aWidth,
                                aMaxError, aErrorLoc );
    }
    else
    {
        // For other shapes, add outlines as thick segments in polygon buffer
        const SHAPE_LINE_CHAIN& path = aPad->GetEffectivePolygon()->COutline( 0 );

        for( int ii = 0; ii < path.PointCount(); ++ii )
        {
            const VECTOR2I& a = path.CPoint( ii );
            const VECTOR2I& b = path.CPoint( ii + 1 );

            TransformOvalToPolygon( aBuffer, a, b, aWidth, aMaxError, aErrorLoc );
        }
    }
}


void transformFPShapesToPolySet( const FOOTPRINT* aFootprint, PCB_LAYER_ID aLayer,
                                 SHAPE_POLY_SET& aBuffer, int aMaxError, ERROR_LOC aErrorLoc )
{
    for( BOARD_ITEM* item : aFootprint->GraphicalItems() )
    {
        if( item->Type() == PCB_SHAPE_T || BaseType( item->Type() ) == PCB_DIMENSION_T )
        {
            if( item->GetLayer() == aLayer )
                item->TransformShapeToPolygon( aBuffer, aLayer, 0, aMaxError, aErrorLoc );
        }
    }
}


void BOARD_ADAPTER::destroyLayers()
{
#define DELETE_AND_FREE( ptr ) \
    {                          \
        delete ptr;            \
        ptr = nullptr;         \
    }                          \

#define DELETE_AND_FREE_MAP( map )         \
    {                                      \
        for( auto& [ layer, poly ] : map ) \
            delete poly;                   \
                                           \
        map.clear();                       \
    }

    DELETE_AND_FREE_MAP( m_layers_poly );

    DELETE_AND_FREE( m_frontPlatedPadPolys )
    DELETE_AND_FREE( m_backPlatedPadPolys )

    DELETE_AND_FREE_MAP( m_layerHoleOdPolys )
    DELETE_AND_FREE_MAP( m_layerHoleIdPolys )

    m_NPTH_ODPolys.RemoveAllContours();
    m_TH_ODPolys.RemoveAllContours();
    m_viaTH_ODPolys.RemoveAllContours();
    m_THAnnularRingPolys.RemoveAllContours();

    DELETE_AND_FREE_MAP( m_layerMap )
    DELETE_AND_FREE_MAP( m_layerHoleMap )

    DELETE_AND_FREE( m_platedPadsFront )
    DELETE_AND_FREE( m_platedPadsBack )

    m_TH_ODs.Clear();
    m_TH_IDs.Clear();
    m_THAnnularRings.Clear();
    m_viaTH_ODs.Clear();
}


void BOARD_ADAPTER::createLayers( REPORTER* aStatusReporter )
{
    destroyLayers();

    // Build Copper layers
    // Based on:
    //    https://github.com/KiCad/kicad-source-mirror/blob/master/3d-viewer/3d_draw.cpp#L692

#ifdef PRINT_STATISTICS_3D_VIEWER
    unsigned stats_startCopperLayersTime = GetRunningMicroSecs();

    unsigned start_Time = stats_startCopperLayersTime;
#endif

    PCB_LAYER_ID cu_seq[MAX_CU_LAYERS];
    LSET         cu_set = LSET::AllCuMask( m_copperLayersCount );

    std::bitset<LAYER_3D_END> visibilityFlags = GetVisibleLayers();

    m_trackCount               = 0;
    m_averageTrackWidth        = 0;
    m_viaCount                 = 0;
    m_averageViaHoleDiameter   = 0;
    m_holeCount                = 0;
    m_averageHoleDiameter      = 0;

    if( !m_board )
        return;

    // Prepare track list, convert in a vector. Calc statistic for the holes
    std::vector<const PCB_TRACK*> trackList;
    trackList.clear();
    trackList.reserve( m_board->Tracks().size() );

    int maxError = m_board->GetDesignSettings().m_MaxError;

    for( PCB_TRACK* track : m_board->Tracks() )
    {
        if( !Is3dLayerEnabled( track->GetLayer(), visibilityFlags ) ) // Skip non enabled layers
            continue;

        // Note: a PCB_TRACK holds normal segment tracks and also vias circles (that have also
        // drill values)
        trackList.push_back( track );

        if( track->Type() == PCB_VIA_T )
        {
            const PCB_VIA *via = static_cast< const PCB_VIA*>( track );
            m_viaCount++;
            m_averageViaHoleDiameter += static_cast<float>( via->GetDrillValue() * m_biuTo3Dunits );
        }
        else
        {
            m_trackCount++;
        }

        m_averageTrackWidth += static_cast<float>( track->GetWidth() * m_biuTo3Dunits );
    }

    if( m_trackCount )
        m_averageTrackWidth /= (float)m_trackCount;

    if( m_viaCount )
        m_averageViaHoleDiameter /= (float)m_viaCount;

    // Prepare copper layers index and containers
    std::vector<PCB_LAYER_ID> layer_ids;
    layer_ids.clear();
    layer_ids.reserve( m_copperLayersCount );

    for( unsigned i = 0; i < arrayDim( cu_seq ); ++i )
        cu_seq[i] = ToLAYER_ID( B_Cu - i );

    for( LSEQ cu = cu_set.Seq( cu_seq, arrayDim( cu_seq ) ); cu; ++cu )
    {
        const PCB_LAYER_ID layer = *cu;

        if( !Is3dLayerEnabled( layer, visibilityFlags ) ) // Skip non enabled layers
            continue;

        layer_ids.push_back( layer );

        BVH_CONTAINER_2D *layerContainer = new BVH_CONTAINER_2D;
        m_layerMap[layer] = layerContainer;

        if( m_Cfg->m_Render.opengl_copper_thickness
                && m_Cfg->m_Render.engine == RENDER_ENGINE::OPENGL )
        {
            SHAPE_POLY_SET* layerPoly    = new SHAPE_POLY_SET;
            m_layers_poly[layer] = layerPoly;
        }
    }

    if( m_Cfg->m_Render.renderPlatedPadsAsPlated )
    {
        m_frontPlatedPadPolys = new SHAPE_POLY_SET;
        m_backPlatedPadPolys = new SHAPE_POLY_SET;

        m_platedPadsFront = new BVH_CONTAINER_2D;
        m_platedPadsBack = new BVH_CONTAINER_2D;

    }

    if( aStatusReporter )
        aStatusReporter->Report( _( "Create tracks and vias" ) );

    // Create tracks as objects and add it to container
    for( PCB_LAYER_ID layer : layer_ids )
    {
        wxASSERT( m_layerMap.find( layer ) != m_layerMap.end() );

        BVH_CONTAINER_2D *layerContainer = m_layerMap[layer];

        for( const PCB_TRACK* track : trackList )
        {
            // NOTE: Vias can be on multiple layers
            if( !track->IsOnLayer( layer ) )
                continue;

            // Skip vias annulus when not flashed on this layer
            if( track->Type() == PCB_VIA_T
                    && !static_cast<const PCB_VIA*>( track )->FlashLayer( layer ) )
            {
                continue;
            }

            // Add object item to layer container
            createTrack( track, layerContainer );
        }
    }

    // Create VIAS and THTs objects and add it to holes containers
    for( PCB_LAYER_ID layer : layer_ids )
    {
        // ADD TRACKS
        unsigned int nTracks = trackList.size();

        for( unsigned int trackIdx = 0; trackIdx < nTracks; ++trackIdx )
        {
            const PCB_TRACK *track = trackList[trackIdx];

            if( !track->IsOnLayer( layer ) )
                continue;

            // ADD VIAS and THT
            if( track->Type() == PCB_VIA_T )
            {
                const PCB_VIA* via               = static_cast<const PCB_VIA*>( track );
                const VIATYPE  viatype           = via->GetViaType();
                const double   holediameter      = via->GetDrillValue() * BiuTo3dUnits();
                const double   viasize           = via->GetWidth() * BiuTo3dUnits();
                const double   plating           = GetHolePlatingThickness() * BiuTo3dUnits();

                // holes and layer copper extend half info cylinder wall to hide transition
                const float    thickness         = static_cast<float>( plating / 2.0f );
                const float    hole_inner_radius = static_cast<float>( holediameter / 2.0f );
                const float    ring_radius       = static_cast<float>( viasize / 2.0f );

                const SFVEC2F via_center( via->GetStart().x * m_biuTo3Dunits,
                                          -via->GetStart().y * m_biuTo3Dunits );

                if( viatype != VIATYPE::THROUGH )
                {
                    // Add hole objects
                    BVH_CONTAINER_2D *layerHoleContainer = nullptr;

                    // Check if the layer is already created
                    if( m_layerHoleMap.find( layer ) == m_layerHoleMap.end() )
                    {
                        // not found, create a new container
                        layerHoleContainer = new BVH_CONTAINER_2D;
                        m_layerHoleMap[layer] = layerHoleContainer;
                    }
                    else
                    {
                        // found
                        layerHoleContainer = m_layerHoleMap[layer];
                    }

                    // Add a hole for this layer
                    layerHoleContainer->Add( new FILLED_CIRCLE_2D( via_center,
                                                                   hole_inner_radius + thickness,
                                                                   *track ) );
                }
                else if( layer == layer_ids[0] ) // it only adds once the THT holes
                {
                    // Add through hole object
                    m_TH_ODs.Add( new FILLED_CIRCLE_2D( via_center, hole_inner_radius + thickness,
                                                        *track ) );
                    m_viaTH_ODs.Add( new FILLED_CIRCLE_2D( via_center, hole_inner_radius + thickness,
                                                           *track ) );

                    if( m_Cfg->m_Render.clip_silk_on_via_annulus && ring_radius > 0.0 )
                    {
                        m_THAnnularRings.Add( new FILLED_CIRCLE_2D( via_center, ring_radius,
                                                                    *track ) );
                    }

                    if( hole_inner_radius > 0.0 )
                    {
                        m_TH_IDs.Add( new FILLED_CIRCLE_2D( via_center, hole_inner_radius, *track ) );
                    }
                }
            }
        }
    }

    // Create VIAS and THTs objects and add it to holes containers
    for( PCB_LAYER_ID layer : layer_ids )
    {
        // ADD TRACKS
        const unsigned int nTracks = trackList.size();

        for( unsigned int trackIdx = 0; trackIdx < nTracks; ++trackIdx )
        {
            const PCB_TRACK *track = trackList[trackIdx];

            if( !track->IsOnLayer( layer ) )
                continue;

            // ADD VIAS and THT
            if( track->Type() == PCB_VIA_T )
            {
                const PCB_VIA* via = static_cast<const PCB_VIA*>( track );
                const VIATYPE  viatype = via->GetViaType();

                if( viatype != VIATYPE::THROUGH )
                {
                    // Add PCB_VIA hole contours

                    // Add outer holes of VIAs
                    SHAPE_POLY_SET *layerOuterHolesPoly = nullptr;
                    SHAPE_POLY_SET *layerInnerHolesPoly = nullptr;

                    // Check if the layer is already created
                    if( m_layerHoleOdPolys.find( layer ) == m_layerHoleOdPolys.end() )
                    {
                        // not found, create a new container
                        layerOuterHolesPoly = new SHAPE_POLY_SET;
                        m_layerHoleOdPolys[layer] = layerOuterHolesPoly;

                        wxASSERT( m_layerHoleIdPolys.find( layer ) == m_layerHoleIdPolys.end() );

                        layerInnerHolesPoly = new SHAPE_POLY_SET;
                        m_layerHoleIdPolys[layer] = layerInnerHolesPoly;
                    }
                    else
                    {
                        // found
                        layerOuterHolesPoly = m_layerHoleOdPolys[layer];

                        wxASSERT( m_layerHoleIdPolys.find( layer ) != m_layerHoleIdPolys.end() );

                        layerInnerHolesPoly = m_layerHoleIdPolys[layer];
                    }

                    const int holediameter = via->GetDrillValue();
                    const int hole_outer_radius = (holediameter / 2) + GetHolePlatingThickness();

                    TransformCircleToPolygon( *layerOuterHolesPoly, via->GetStart(),
                                              hole_outer_radius, maxError, ERROR_INSIDE );

                    TransformCircleToPolygon( *layerInnerHolesPoly, via->GetStart(),
                                              holediameter / 2, maxError, ERROR_INSIDE );
                }
                else if( layer == layer_ids[0] ) // it only adds once the THT holes
                {
                    const int holediameter = via->GetDrillValue();
                    const int hole_outer_radius = (holediameter / 2) + GetHolePlatingThickness();
                    const int hole_outer_ring_radius = KiROUND( via->GetWidth() / 2.0 );

                    // Add through hole contours
                    TransformCircleToPolygon( m_TH_ODPolys, via->GetStart(), hole_outer_radius,
                                              maxError, ERROR_INSIDE );

                    // Add same thing for vias only
                    TransformCircleToPolygon( m_viaTH_ODPolys, via->GetStart(), hole_outer_radius,
                                              maxError, ERROR_INSIDE );

                    if( m_Cfg->m_Render.clip_silk_on_via_annulus )
                    {
                        TransformCircleToPolygon( m_THAnnularRingPolys, via->GetStart(),
                                                  hole_outer_ring_radius, maxError, ERROR_INSIDE );
                    }
                }
            }
        }
    }

    // Creates vertical outline contours of the tracks and add it to the poly of the layer
    if( m_Cfg->m_Render.opengl_copper_thickness && m_Cfg->m_Render.engine == RENDER_ENGINE::OPENGL )
    {
        for( PCB_LAYER_ID layer : layer_ids )
        {
            wxASSERT( m_layers_poly.find( layer ) != m_layers_poly.end() );

            SHAPE_POLY_SET *layerPoly = m_layers_poly[layer];

            // ADD TRACKS
            unsigned int nTracks = trackList.size();

            for( unsigned int trackIdx = 0; trackIdx < nTracks; ++trackIdx )
            {
                const PCB_TRACK *track = trackList[trackIdx];

                if( !track->IsOnLayer( layer ) )
                    continue;

                // Skip vias annulus when not flashed on this layer
                if( track->Type() == PCB_VIA_T
                        && !static_cast<const PCB_VIA*>( track )->FlashLayer( layer )  )
                {
                    continue;
                }

                // Add the track/via contour
                track->TransformShapeToPolygon( *layerPoly, layer, 0, maxError, ERROR_INSIDE );
            }
        }
    }

    // Add holes of footprints
    for( FOOTPRINT* footprint : m_board->Footprints() )
    {
        for( PAD* pad : footprint->Pads() )
        {
            const VECTOR2I padHole = pad->GetDrillSize();

            if( !padHole.x )    // Not drilled pad like SMD pad
                continue;

            // The hole in the body is inflated by copper thickness, if not plated, no copper
            int inflate = 0;

            if( pad->GetAttribute () != PAD_ATTRIB::NPTH )
                inflate = KiROUND( GetHolePlatingThickness() / 2.0 );

            m_holeCount++;
            double holeDiameter = ( pad->GetDrillSize().x + pad->GetDrillSize().y ) / 2.0;
            m_averageHoleDiameter += static_cast<float>( holeDiameter * m_biuTo3Dunits );

            createPadWithHole( pad, &m_TH_ODs, inflate );

            if( m_Cfg->m_Render.clip_silk_on_via_annulus )
                createPadWithHole( pad, &m_THAnnularRings, inflate );

            createPadWithHole( pad, &m_TH_IDs, 0 );
        }
    }

    if( m_holeCount )
        m_averageHoleDiameter /= (float)m_holeCount;

    // Add contours of the pad holes (pads can be Circle or Segment holes)
    for( FOOTPRINT* footprint : m_board->Footprints() )
    {
        for( PAD* pad : footprint->Pads() )
        {
            const VECTOR2I padHole = pad->GetDrillSize();

            if( !padHole.x ) // Not drilled pad like SMD pad
                continue;

            // The hole in the body is inflated by copper thickness.
            const int inflate = GetHolePlatingThickness();

            if( pad->GetAttribute () != PAD_ATTRIB::NPTH )
            {
                if( m_Cfg->m_Render.clip_silk_on_via_annulus )
                    pad->TransformHoleToPolygon( m_THAnnularRingPolys, inflate, maxError, ERROR_INSIDE );

                pad->TransformHoleToPolygon( m_TH_ODPolys, inflate, maxError, ERROR_INSIDE );
            }
            else
            {
                // If not plated, no copper.
                if( m_Cfg->m_Render.clip_silk_on_via_annulus )
                    pad->TransformHoleToPolygon( m_THAnnularRingPolys, 0, maxError, ERROR_INSIDE );

                pad->TransformHoleToPolygon( m_NPTH_ODPolys, 0, maxError, ERROR_INSIDE );
            }
        }
    }

    // Add footprints PADs objects to containers
    for( PCB_LAYER_ID layer : layer_ids )
    {
        wxASSERT( m_layerMap.find( layer ) != m_layerMap.end() );

        BVH_CONTAINER_2D *layerContainer = m_layerMap[layer];

        // ADD PADS
        for( FOOTPRINT* footprint : m_board->Footprints() )
        {
            addPads( footprint, layerContainer, layer, m_Cfg->m_Render.renderPlatedPadsAsPlated,
                     false );

            // Micro-wave footprints may have items on copper layers
            addFootprintShapes( footprint, layerContainer, layer, visibilityFlags );
        }
    }

    if( m_Cfg->m_Render.renderPlatedPadsAsPlated )
    {
        // ADD PLATED PADS
        for( FOOTPRINT* footprint : m_board->Footprints() )
        {
            addPads( footprint, m_platedPadsFront, F_Cu, false, true );
            addPads( footprint, m_platedPadsBack, B_Cu, false, true );
        }

        m_platedPadsFront->BuildBVH();
        m_platedPadsBack->BuildBVH();
    }

    // Add footprints PADs poly contours (vertical outlines)
    if( m_Cfg->m_Render.opengl_copper_thickness && m_Cfg->m_Render.engine == RENDER_ENGINE::OPENGL )
    {
        for( PCB_LAYER_ID layer : layer_ids )
        {
            wxASSERT( m_layers_poly.find( layer ) != m_layers_poly.end() );

            SHAPE_POLY_SET *layerPoly = m_layers_poly[layer];

            // Add pads to polygon list
            for( FOOTPRINT* footprint : m_board->Footprints() )
            {
                // Note: NPTH pads are not drawn on copper layers when the pad has same shape as
                // its hole
                footprint->TransformPadsToPolySet( *layerPoly, layer, 0, maxError, ERROR_INSIDE,
                                                   true, m_Cfg->m_Render.renderPlatedPadsAsPlated,
                                                   false );

                transformFPShapesToPolySet( footprint, layer, *layerPoly, maxError, ERROR_INSIDE );
            }
        }

        if( m_Cfg->m_Render.renderPlatedPadsAsPlated )
        {
            // ADD PLATED PADS contours
            for( FOOTPRINT* footprint : m_board->Footprints() )
            {
                footprint->TransformPadsToPolySet( *m_frontPlatedPadPolys, F_Cu, 0, maxError,
                                                   ERROR_INSIDE, true, false, true );

                footprint->TransformPadsToPolySet( *m_backPlatedPadPolys, B_Cu, 0, maxError,
                                                   ERROR_INSIDE, true, false, true );
            }
        }
    }

    // Add graphic item on copper layers to object containers
    for( PCB_LAYER_ID layer : layer_ids )
    {
        wxASSERT( m_layerMap.find( layer ) != m_layerMap.end() );

        BVH_CONTAINER_2D *layerContainer = m_layerMap[layer];

        // Add graphic items on copper layers (texts and other graphics)
        for( BOARD_ITEM* item : m_board->Drawings() )
        {
            if( !item->IsOnLayer( layer ) )
                continue;

            switch( item->Type() )
            {
            case PCB_SHAPE_T:
                addShape( static_cast<PCB_SHAPE*>( item ), layerContainer, item );
                break;

            case PCB_TEXT_T:
                addText( static_cast<PCB_TEXT*>( item ), layerContainer, item );
                break;

            case PCB_TEXTBOX_T:
                addText( static_cast<PCB_TEXTBOX*>( item ), layerContainer, item );
                    addShape( static_cast<PCB_TEXTBOX*>( item ), layerContainer, item );
                break;

            case PCB_DIM_ALIGNED_T:
            case PCB_DIM_CENTER_T:
            case PCB_DIM_RADIAL_T:
            case PCB_DIM_ORTHOGONAL_T:
            case PCB_DIM_LEADER_T:
                addShape( static_cast<PCB_DIMENSION_BASE*>( item ), layerContainer, item );
                break;

            default:
                wxLogTrace( m_logTrace, wxT( "createLayers: item type: %d not implemented" ),
                            item->Type() );
                break;
            }
        }
    }

    // Add graphic item on copper layers to poly contours (vertical outlines)
    if( m_Cfg->m_Render.opengl_copper_thickness && m_Cfg->m_Render.engine == RENDER_ENGINE::OPENGL )
    {
        for( PCB_LAYER_ID layer : layer_ids )
        {
            wxASSERT( m_layers_poly.find( layer ) != m_layers_poly.end() );

            SHAPE_POLY_SET *layerPoly = m_layers_poly[layer];

            // Add graphic items on copper layers (texts and other )
            for( BOARD_ITEM* item : m_board->Drawings() )
            {
                if( !item->IsOnLayer( layer ) )
                    continue;

                switch( item->Type() )
                {
                case PCB_SHAPE_T:
                    item->TransformShapeToPolygon( *layerPoly, layer, 0, maxError, ERROR_INSIDE );
                    break;

                case PCB_TEXT_T:
                {
                    PCB_TEXT* text = static_cast<PCB_TEXT*>( item );

                    text->TransformTextToPolySet( *layerPoly, 0, maxError, ERROR_INSIDE );
                    break;
                }

                case PCB_TEXTBOX_T:
                {
                    PCB_TEXTBOX* textbox = static_cast<PCB_TEXTBOX*>( item );

                    textbox->TransformTextToPolySet( *layerPoly, 0, maxError, ERROR_INSIDE );
                    break;
                }

                default:
                    wxLogTrace( m_logTrace, wxT( "createLayers: item type: %d not implemented" ),
                                item->Type() );
                    break;
                }
            }
        }
    }

    if( m_Cfg->m_Render.show_zones )
    {
        if( aStatusReporter )
            aStatusReporter->Report( _( "Create zones" ) );

        std::vector<std::pair<ZONE*, PCB_LAYER_ID>> zones;
        std::unordered_map<PCB_LAYER_ID, std::unique_ptr<std::mutex>> layer_lock;

        for( ZONE* zone : m_board->Zones() )
        {
            for( PCB_LAYER_ID layer : zone->GetLayerSet().Seq() )
            {
                zones.emplace_back( std::make_pair( zone, layer ) );
                layer_lock.emplace( layer, std::make_unique<std::mutex>() );
            }
        }

        // Add zones objects
        std::atomic<size_t> nextZone( 0 );
        std::atomic<size_t> threadsFinished( 0 );

        size_t parallelThreadCount = std::min<size_t>( zones.size(),
                std::max<size_t>( std::thread::hardware_concurrency(), 2 ) );

        for( size_t ii = 0; ii < parallelThreadCount; ++ii )
        {
            std::thread t = std::thread( [&]()
            {
                for( size_t areaId = nextZone.fetch_add( 1 );
                            areaId < zones.size();
                            areaId = nextZone.fetch_add( 1 ) )
                {
                    ZONE* zone = zones[areaId].first;

                    if( zone == nullptr )
                        break;

                    PCB_LAYER_ID layer = zones[areaId].second;

                    auto layerContainer = m_layerMap.find( layer );
                    auto layerPolyContainer = m_layers_poly.find( layer );

                    if( layerContainer != m_layerMap.end() )
                        addSolidAreasShapes( zone, layerContainer->second, layer );

                    if( m_Cfg->m_Render.opengl_copper_thickness
                          && m_Cfg->m_Render.engine == RENDER_ENGINE::OPENGL
                          && layerPolyContainer != m_layers_poly.end() )
                    {
                        auto mut_it = layer_lock.find( layer );

                        std::lock_guard< std::mutex > lock( *( mut_it->second ) );
                        zone->TransformSolidAreasShapesToPolygon( layer, *layerPolyContainer->second );
                    }
                }

                threadsFinished++;
            } );

            t.detach();
        }

        while( threadsFinished < parallelThreadCount )
            std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );

    }

    // Simplify layer polygons

    if( aStatusReporter )
        aStatusReporter->Report( _( "Simplifying copper layers polygons" ) );

    if( m_Cfg->m_Render.opengl_copper_thickness && m_Cfg->m_Render.engine == RENDER_ENGINE::OPENGL )
    {
        if( m_Cfg->m_Render.renderPlatedPadsAsPlated )
        {
            if( m_frontPlatedPadPolys && ( m_layers_poly.find( F_Cu ) != m_layers_poly.end() ) )
            {
                if( aStatusReporter )
                    aStatusReporter->Report( _( "Simplifying polygons on F_Cu" ) );

                SHAPE_POLY_SET *layerPoly_F_Cu = m_layers_poly[F_Cu];
                layerPoly_F_Cu->BooleanSubtract( *m_frontPlatedPadPolys, SHAPE_POLY_SET::PM_FAST );

                 m_frontPlatedPadPolys->Simplify( SHAPE_POLY_SET::PM_FAST );
            }

            if( m_backPlatedPadPolys && ( m_layers_poly.find( B_Cu ) != m_layers_poly.end() ) )
            {
                if( aStatusReporter )
                    aStatusReporter->Report( _( "Simplifying polygons on B_Cu" ) );

                SHAPE_POLY_SET *layerPoly_B_Cu = m_layers_poly[B_Cu];
                layerPoly_B_Cu->BooleanSubtract( *m_backPlatedPadPolys, SHAPE_POLY_SET::PM_FAST );

                m_backPlatedPadPolys->Simplify( SHAPE_POLY_SET::PM_FAST );
            }
        }

        std::vector<PCB_LAYER_ID> &selected_layer_id = layer_ids;
        std::vector<PCB_LAYER_ID> layer_id_without_F_and_B;

        if( m_Cfg->m_Render.renderPlatedPadsAsPlated )
        {
            layer_id_without_F_and_B.clear();
            layer_id_without_F_and_B.reserve( layer_ids.size() );

            for( PCB_LAYER_ID layer: layer_ids )
            {
                if( layer != F_Cu && layer != B_Cu )
                    layer_id_without_F_and_B.push_back( layer );
            }

            selected_layer_id = layer_id_without_F_and_B;
        }

        if( selected_layer_id.size() > 0 )
        {
            if( aStatusReporter )
            {
                aStatusReporter->Report( wxString::Format( _( "Simplifying %d copper layers" ),
                                                           (int) selected_layer_id.size() ) );
            }

            std::atomic<size_t> nextItem( 0 );
            std::atomic<size_t> threadsFinished( 0 );

            size_t parallelThreadCount = std::min<size_t>(
                    std::max<size_t>( std::thread::hardware_concurrency(), 2 ),
                    selected_layer_id.size() );

            for( size_t ii = 0; ii < parallelThreadCount; ++ii )
            {
                std::thread t = std::thread(
                        [&nextItem, &threadsFinished, &selected_layer_id, this]()
                        {
                            for( size_t i = nextItem.fetch_add( 1 );
                                        i < selected_layer_id.size();
                                        i = nextItem.fetch_add( 1 ) )
                            {
                                auto layerPoly = m_layers_poly.find( selected_layer_id[i] );

                                if( layerPoly != m_layers_poly.end() )
                                    // This will make a union of all added contours
                                    layerPoly->second->Simplify( SHAPE_POLY_SET::PM_FAST );
                            }

                            threadsFinished++;
                        } );

                t.detach();
            }

            while( threadsFinished < parallelThreadCount )
                std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
        }
    }

    // Simplify holes polygon contours
    if( aStatusReporter )
        aStatusReporter->Report( _( "Simplify holes contours" ) );

    for( PCB_LAYER_ID layer : layer_ids )
    {
        if( m_layerHoleOdPolys.find( layer ) != m_layerHoleOdPolys.end() )
        {
            // found
            SHAPE_POLY_SET *polyLayer = m_layerHoleOdPolys[layer];
            polyLayer->Simplify( SHAPE_POLY_SET::PM_FAST );

            wxASSERT( m_layerHoleIdPolys.find( layer ) != m_layerHoleIdPolys.end() );

            polyLayer = m_layerHoleIdPolys[layer];
            polyLayer->Simplify( SHAPE_POLY_SET::PM_FAST );
        }
    }

    // End Build Copper layers

    // This will make a union of all added contours
    m_TH_ODPolys.Simplify( SHAPE_POLY_SET::PM_FAST );
    m_NPTH_ODPolys.Simplify( SHAPE_POLY_SET::PM_FAST );
    m_viaTH_ODPolys.Simplify( SHAPE_POLY_SET::PM_FAST );
    m_THAnnularRingPolys.Simplify( SHAPE_POLY_SET::PM_FAST );

    // Build Tech layers
    // Based on:
    //    https://github.com/KiCad/kicad-source-mirror/blob/master/3d-viewer/3d_draw.cpp#L1059
    if( aStatusReporter )
        aStatusReporter->Report( _( "Build Tech layers" ) );

    // draw graphic items, on technical layers

    // Vertical walls (layer thickness) around shapes is really time consumming
    // They are built on request
    bool buildVerticalWallsForTechLayers = m_Cfg->m_Render.opengl_copper_thickness
                                              && m_Cfg->m_Render.engine == RENDER_ENGINE::OPENGL;

    static const PCB_LAYER_ID techLayerList[] = {
            B_Adhes,
            F_Adhes,
            B_Paste,
            F_Paste,
            B_SilkS,
            F_SilkS,
            B_Mask,
            F_Mask,

            // Aux Layers
            Dwgs_User,
            Cmts_User,
            Eco1_User,
            Eco2_User
        };

    for( LSEQ seq = LSET::AllNonCuMask().Seq( techLayerList, arrayDim( techLayerList ) );
         seq;
         ++seq )
    {
        const PCB_LAYER_ID layer = *seq;

        if( !Is3dLayerEnabled( layer, visibilityFlags ) )
            continue;

        if( aStatusReporter )
            aStatusReporter->Report( wxString::Format( _( "Build Tech layer %d" ), (int) layer ) );

        BVH_CONTAINER_2D *layerContainer = new BVH_CONTAINER_2D;
        m_layerMap[layer] = layerContainer;

        SHAPE_POLY_SET *layerPoly = new SHAPE_POLY_SET;
        m_layers_poly[layer] = layerPoly;

        // Add drawing objects
        for( BOARD_ITEM* item : m_board->Drawings() )
        {
            if( !item->IsOnLayer( layer ) )
                continue;

            switch( item->Type() )
            {
            case PCB_SHAPE_T:
                addShape( static_cast<PCB_SHAPE*>( item ), layerContainer, item );
                break;

            case PCB_TEXT_T:
                addText( static_cast<PCB_TEXT*>( item ), layerContainer, item );
                break;

            case PCB_TEXTBOX_T:
                addText( static_cast<PCB_TEXTBOX*>( item ), layerContainer, item );
                if( static_cast<PCB_TEXTBOX*>( item )->IsBorderEnabled() )
                    addShape( static_cast<PCB_TEXTBOX*>( item ), layerContainer, item );
                break;

            case PCB_DIM_ALIGNED_T:
            case PCB_DIM_CENTER_T:
            case PCB_DIM_RADIAL_T:
            case PCB_DIM_ORTHOGONAL_T:
            case PCB_DIM_LEADER_T:
                addShape( static_cast<PCB_DIMENSION_BASE*>( item ), layerContainer, item );
                break;

            default:
                break;
            }
        }

        // Add drawing contours (vertical walls)
        if( buildVerticalWallsForTechLayers )
        {
            for( BOARD_ITEM* item : m_board->Drawings() )
            {
                if( !item->IsOnLayer( layer ) )
                    continue;

                switch( item->Type() )
                {
                case PCB_SHAPE_T:
                    item->TransformShapeToPolygon( *layerPoly, layer, 0, maxError, ERROR_INSIDE );
                    break;

                case PCB_TEXT_T:
                {
                    PCB_TEXT* text = static_cast<PCB_TEXT*>( item );

                    text->TransformTextToPolySet( *layerPoly, 0, maxError, ERROR_INSIDE );
                    break;
                }

                case PCB_TEXTBOX_T:
                {
                    PCB_TEXTBOX* textbox = static_cast<PCB_TEXTBOX*>( item );

                    textbox->TransformTextToPolySet( *layerPoly, 0, maxError, ERROR_INSIDE );
                    break;
                }

                default:
                    break;
                }
            }
        }

        // Add via tech layers
        if( ( layer == F_Mask || layer == B_Mask ) && !m_board->GetTentVias() )
        {
            int maskExpansion = GetBoard()->GetDesignSettings().m_SolderMaskExpansion;

            for( PCB_TRACK* track : m_board->Tracks() )
            {
                if( track->Type() == PCB_VIA_T
                        && static_cast<const PCB_VIA*>( track )->FlashLayer( layer )  )
                {
                    createViaWithMargin( track, layerContainer, maskExpansion );
                }
            }

            // Add via tech layers - contours (vertical walls)
            if( buildVerticalWallsForTechLayers )
            {
                for( PCB_TRACK* track : m_board->Tracks() )
                {
                    if( track->Type() == PCB_VIA_T
                            && static_cast<const PCB_VIA*>( track )->FlashLayer( layer )  )
                    {
                        track->TransformShapeToPolygon( *layerPoly, layer, maskExpansion, maxError,
                                                        ERROR_INSIDE );
                    }
                }
            }
        }

        // Add footprints tech layers - objects
        for( FOOTPRINT* footprint : m_board->Footprints() )
        {
            if( layer == F_SilkS || layer == B_SilkS )
            {
                int linewidth = m_board->GetDesignSettings().m_LineThickness[ LAYER_CLASS_SILK ];

                for( PAD* pad : footprint->Pads() )
                {
                    if( !pad->IsOnLayer( layer ) )
                        continue;

                    buildPadOutlineAsSegments( pad, layerContainer, linewidth );
                }
            }
            else
            {
                addPads( footprint, layerContainer, layer, false, false );
            }

            addFootprintShapes( footprint, layerContainer, layer, visibilityFlags );
        }


        // Add footprints tech layers - contours (vertical walls)
        if( buildVerticalWallsForTechLayers )
        {
            for( FOOTPRINT* footprint : m_board->Footprints() )
            {
                if( layer == F_SilkS || layer == B_SilkS )
                {
                    int linewidth = m_board->GetDesignSettings().m_LineThickness[ LAYER_CLASS_SILK ];

                    for( PAD* pad : footprint->Pads() )
                    {
                        if( pad->IsOnLayer( layer ) )
                        {
                            buildPadOutlineAsPolygon( pad, *layerPoly, linewidth, maxError,
                                                      ERROR_INSIDE );
                        }
                    }
                }
                else
                {
                    footprint->TransformPadsToPolySet( *layerPoly, layer, 0, maxError, ERROR_INSIDE );
                }

                // On tech layers, use a poor circle approximation, only for texts (stroke font)
                footprint->TransformFPTextToPolySet( *layerPoly, layer, 0, maxError, ERROR_INSIDE );

                // Add the remaining things with dynamic seg count for circles
                transformFPShapesToPolySet( footprint, layer, *layerPoly, maxError, ERROR_INSIDE );
            }
        }

        // Draw non copper zones
        if( m_Cfg->m_Render.show_zones )
        {
            for( ZONE* zone : m_board->Zones() )
            {
                if( zone->IsOnLayer( layer ) )
                    addSolidAreasShapes( zone, layerContainer, layer );
            }

            if( buildVerticalWallsForTechLayers )
            {
                for( ZONE* zone : m_board->Zones() )
                {
                    if( zone->IsOnLayer( layer ) )
                        zone->TransformSolidAreasShapesToPolygon( layer, *layerPoly );
                }
            }
        }

        // This will make a union of all added contours
        layerPoly->Simplify( SHAPE_POLY_SET::PM_FAST );
    }
    // End Build Tech layers

#if 1
    // A somewhat experimental feature: if we're rendering off-board silk, turn any pads of
    // footprints which are entirely outside the board outline into silk.  This makes off-board
    // footprints more visually recognizable.
    if( m_Cfg->m_Render.show_off_board_silk )
    {
        BOX2I boardBBox = m_board_poly.BBox();

        for( FOOTPRINT* footprint : m_board->Footprints() )
        {
            if( !footprint->GetBoundingBox().Intersects( boardBBox ) )
            {
                if( footprint->IsFlipped() )
                {
                    BVH_CONTAINER_2D *layerContainer = m_layerMap[ B_SilkS ];
                    addPads( footprint, layerContainer, B_Cu, false, false );
                }
                else
                {
                    BVH_CONTAINER_2D *layerContainer = m_layerMap[ F_SilkS ];
                    addPads( footprint, layerContainer, F_Cu, false, false );
                }
            }
        }
    }
#endif

    // Build BVH (Bounding volume hierarchy) for holes and vias

    if( aStatusReporter )
        aStatusReporter->Report( _( "Build BVH for holes and vias" ) );

    m_TH_IDs.BuildBVH();
    m_TH_ODs.BuildBVH();
    m_THAnnularRings.BuildBVH();

    if( !m_layerHoleMap.empty() )
    {
        for( std::pair<const PCB_LAYER_ID, BVH_CONTAINER_2D*>& hole : m_layerHoleMap )
            hole.second->BuildBVH();
    }

    // We only need the Solder mask to initialize the BVH
    // because..?
    if( m_layerMap[B_Mask] )
        m_layerMap[B_Mask]->BuildBVH();

    if( m_layerMap[F_Mask] )
        m_layerMap[F_Mask]->BuildBVH();
}
