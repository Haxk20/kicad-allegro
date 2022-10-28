/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2022 Mikolaj Wielgus
 * Copyright (C) 2022 KiCad Developers, see AUTHORS.txt for contributors.
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

#include <sim/sim_model_diode.h>


std::string SPICE_GENERATOR_DIODE::ItemLine( const SPICE_ITEM& aItem ) const
{
    SPICE_ITEM item = aItem;

    // FIXME: This `if` is there because Preview() calls this function with empty pinNetNames vector.
    // This shouldn't be necessary.
    if( item.pinNetNames.size() >= 2 )
        std::swap( item.pinNetNames.at( 0 ), item.pinNetNames.at( 1 ) );

    return SPICE_GENERATOR::ItemLine( item );
}


std::vector<std::string> SPICE_GENERATOR_DIODE::CurrentNames( const SPICE_ITEM& aItem ) const
{
    return SPICE_GENERATOR::CurrentNames( aItem ); // NOLINT
}


SIM_MODEL_DIODE::SIM_MODEL_DIODE() :
    SIM_MODEL_NGSPICE( TYPE::D, std::make_unique<SPICE_GENERATOR_DIODE>( *this ) )
{
}
