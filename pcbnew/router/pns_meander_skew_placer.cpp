/*
 * KiRouter - a push-and-(sometimes-)shove PCB router
 *
 * Copyright (C) 2013-2015 CERN
 * Copyright (C) 2016 KiCad Developers, see AUTHORS.txt for contributors.
 * Author: Tomasz Wlostowski <tomasz.wlostowski@cern.ch>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <base_units.h> // God forgive me doing this...

#include "pns_node.h"
#include "pns_itemset.h"
#include "pns_topology.h"
#include "pns_meander_skew_placer.h"
#include "pns_solid.h"

#include "pns_router.h"
#include "pns_debug_decorator.h"

namespace PNS {

MEANDER_SKEW_PLACER::MEANDER_SKEW_PLACER ( ROUTER* aRouter ) :
    MEANDER_PLACER ( aRouter )
{
    // Init temporary variables (do not leave uninitialized members)
    m_coupledLength = 0;
    m_padToDieN = 0;
    m_padToDieP = 0;
}


MEANDER_SKEW_PLACER::~MEANDER_SKEW_PLACER( )
{
}


bool MEANDER_SKEW_PLACER::Start( const VECTOR2I& aP, ITEM* aStartItem )
{
    if( !aStartItem || !aStartItem->OfKind( ITEM::SEGMENT_T | ITEM::ARC_T) )
    {
        Router()->SetFailureReason( _( "Please select a differential pair trace you want to tune." ) );
        return false;
    }

    m_initialSegment = static_cast<LINKED_ITEM*>( aStartItem );
    m_currentNode    = nullptr;
    m_currentStart   = getSnappedStartPoint( m_initialSegment, aP );

    m_world = Router()->GetWorld( )->Branch();
    m_originLine = m_world->AssembleLine( m_initialSegment );

    TOPOLOGY topo( m_world );
    m_tunedPath = topo.AssembleTrivialPath( m_initialSegment );

    if( !topo.AssembleDiffPair ( m_initialSegment, m_originPair ) )
    {
        Router()->SetFailureReason( _( "Unable to find complementary differential pair "
                                       "net for skew tuning. Make sure the names of the nets belonging "
                                       "to a differential pair end with either _N/_P or +/-." ) );
        return false;
    }

    if( m_originPair.Gap() < 0 )
        m_originPair.SetGap( Router()->Sizes().DiffPairGap() );

    if( !m_originPair.PLine().SegmentCount() ||
        !m_originPair.NLine().SegmentCount() )
        return false;

    SOLID* padA = nullptr;
    SOLID* padB = nullptr;

    m_tunedPathP = topo.AssembleTuningPath( m_originPair.PLine().GetLink( 0 ), &padA, &padB );

    m_padToDieP = 0;

    if( padA )
        m_padToDieP += padA->GetPadToDie();

    if( padB )
        m_padToDieP += padB->GetPadToDie();

    m_tunedPathN = topo.AssembleTuningPath( m_originPair.NLine().GetLink( 0 ), &padA, &padB );

    m_padToDieN = 0;

    if( padA )
        m_padToDieN += padA->GetPadToDie();

    if( padB )
        m_padToDieN += padB->GetPadToDie();

    m_world->Remove( m_originLine );

    m_currentWidth = m_originLine.Width();
    m_currentEnd = VECTOR2I( 0, 0 );

    if ( m_originPair.PLine().Net() == m_originLine.Net() )
    {
        m_padToDieLength = m_padToDieN;
        m_coupledLength = lineLength( m_tunedPathN );
        m_tunedPath = m_tunedPathP;
    }
    else
    {
        m_padToDieLength = m_padToDieP;
        m_coupledLength = lineLength( m_tunedPathP );
        m_tunedPath = m_tunedPathN;
    }

    return true;
}


long long int MEANDER_SKEW_PLACER::origPathLength() const
{
    return m_padToDieLength + lineLength( m_tunedPath );
}


long long int MEANDER_SKEW_PLACER::currentSkew() const
{
    return m_lastLength - m_coupledLength;
}


bool MEANDER_SKEW_PLACER::Move( const VECTOR2I& aP, ITEM* aEndItem )
{
    for( const ITEM* item : m_tunedPathP.CItems() )
    {
        if( const LINE* l = dyn_cast<const LINE*>( item ) )
            PNS_DBG( Dbg(), AddItem, l, BLUE, 10000, wxT( "tuned-path-skew-p" ) );
    }

    for( const ITEM* item : m_tunedPathN.CItems() )
    {
        if( const LINE* l = dyn_cast<const LINE*>( item ) )
            PNS_DBG( Dbg(), AddItem, l, YELLOW, 10000, wxT( "tuned-path-skew-n" ) );
    }

    return doMove( aP, aEndItem, m_coupledLength + m_settings.m_targetSkew );
}


const wxString MEANDER_SKEW_PLACER::TuningInfo( EDA_UNITS aUnits ) const
{
    wxString status;

    switch( m_lastStatus )
    {
    case TOO_LONG:
        status = _( "Too long: skew " );
        break;
    case TOO_SHORT:
        status = _( "Too short: skew " );
        break;
    case TUNED:
        status = _( "Tuned: skew " );
        break;
    default:
        return _( "?" );
    }

    status += ::MessageTextFromValue( aUnits, m_lastLength - m_coupledLength );
    status += wxT( "/" );
    status += ::MessageTextFromValue( aUnits, m_settings.m_targetSkew );

    return status;
}

}
