#ifndef ALLEGRO_PARSER_H_
#define ALLEGRO_PARSER_H_

#include <cstdarg>

#include <boost/version.hpp>
#if BOOST_VERSION >= 108100
#include <boost/unordered/unordered_flat_map.hpp>
#else
#include <unordered_map>
#endif
#include <optional>

#include <wx/log.h>

#include <board.h>
#include <geometry/eda_angle.h>
#include <footprint.h>
#include <layer_ids.h>
#include <lib_id.h>
#include <pad.h>
#include <pcb_shape.h>
#include <pcb_track.h>
#include <trigo.h>
#include <zone.h>

#include "allegro_elem_parsers.h"
#include "allegro_file.h"
#include "allegro_structs.h"

#define PRINT_ALL_ITEMS 0

template <ALLEGRO::MAGIC magic>
class ALLEGRO_PARSER
{
public:
    ALLEGRO_PARSER( BOARD* aBoard, const ALLEGRO_FILE& aAllegroBrdFile );
    void Parse();

private:
    void    BuildBoard();
    uint8_t GetCopperLayerCount();
    void    AddAnnotation( BOARD_ITEM_CONTAINER& aContainer, const ALLEGRO::T_14<magic>& i14 );
    void    AddFootprint( const ALLEGRO::T_2B<magic>& i2B );
    void    AddPad( FOOTPRINT* fp, const ALLEGRO::T_32<magic>& i32 );
    void    AddText( BOARD_ITEM_CONTAINER& aContainer, const ALLEGRO::T_30<magic>& i30 );
    void    AddTrack( const ALLEGRO::T_1B<magic>& i1B, const ALLEGRO::T_05_TRACK<magic>& i05 );
    void    AddVia( const ALLEGRO::T_33<magic>& i33 );
    void AddZone( BOARD_ITEM_CONTAINER& aContainer, const std::optional<ALLEGRO::T_1B<magic>>& i1B,
                  const ALLEGRO::T_28_ZONE<magic>& i28 );

    void                    Skip( std::size_t n );
    void                    Log( const char* fmt... );
    bool                    IsType( uint32_t k, uint8_t t );
    std::optional<wxString> StringLookup( uint32_t k );
    NETINFO_ITEM*           NetInfo( uint32_t k );
    NETINFO_ITEM*           NetInfo( const ALLEGRO::T_04<magic>& i04 );
    SHAPE_LINE_CHAIN        ShapeStartingAt( uint32_t* k );
    PCB_LAYER_ID            EtchLayerToKi( uint8_t a_layer );
    double                  Scale( double a_allegroValue );

    std::optional<wxString> RefdesLookup( const ALLEGRO::T_2D<magic>& i2D );

    template <template <ALLEGRO::MAGIC> typename T>
    static uint32_t DefaultParser( ALLEGRO_PARSER& parser );

    static uint32_t Parse03( ALLEGRO_PARSER& parser );
    static uint32_t Parse1C( ALLEGRO_PARSER& parser );
    static uint32_t Parse1D( ALLEGRO_PARSER& parser );
    static uint32_t Parse1E( ALLEGRO_PARSER& parser );
    static uint32_t Parse1F( ALLEGRO_PARSER& parser );
    static uint32_t Parse21( ALLEGRO_PARSER& parser );
    static uint32_t Parse27( ALLEGRO_PARSER& parser );
    static uint32_t Parse2A( ALLEGRO_PARSER& parser );
    static uint32_t Parse31( ALLEGRO_PARSER& parser );
    static uint32_t Parse35( ALLEGRO_PARSER& parser );
    static uint32_t Parse36( ALLEGRO_PARSER& parser );
    static uint32_t Parse3B( ALLEGRO_PARSER& parser );
    static uint32_t Parse3C( ALLEGRO_PARSER& parser );

    typedef uint32_t ( *PARSER_FUNC )( ALLEGRO_PARSER& parser );

    static constexpr PARSER_FUNC PARSER_TABLE[] = {
        // 0x00
        nullptr,
        // 0x01
        &DefaultParser<ALLEGRO::T_01>,
        // 0x02
        nullptr,
        // 0x03
        &Parse03,
        // 0x04
        &DefaultParser<ALLEGRO::T_04>,
        // 0x05
        &DefaultParser<ALLEGRO::T_05_TRACK>,
        // 0x06
        &DefaultParser<ALLEGRO::T_06>,
        // 0x07
        &DefaultParser<ALLEGRO::T_07>,
        // 0x08
        &DefaultParser<ALLEGRO::T_08>,
        // 0x09
        &DefaultParser<ALLEGRO::T_09>,
        // 0x0A
        &DefaultParser<ALLEGRO::T_0A>,
        // 0x0B
        nullptr,
        // 0x0C
        &DefaultParser<ALLEGRO::T_0C>,
        // 0x0D
        &DefaultParser<ALLEGRO::T_0D>,
        // 0x0E
        &DefaultParser<ALLEGRO::T_0E>,
        // 0x0F
        &DefaultParser<ALLEGRO::T_0F>,
        // 0x10
        &DefaultParser<ALLEGRO::T_10>,
        // 0x11
        &DefaultParser<ALLEGRO::T_11>,
        // 0x12
        &DefaultParser<ALLEGRO::T_12>,
        // 0x13
        nullptr,
        // 0x14
        &DefaultParser<ALLEGRO::T_14>,
        // 0x15
        &DefaultParser<ALLEGRO::T_15>,
        // 0x16
        &DefaultParser<ALLEGRO::T_16>,
        // 0x17
        &DefaultParser<ALLEGRO::T_17>,
        // 0x18
        nullptr,
        // 0x19
        nullptr,
        // 0x1A
        nullptr,
        // 0x1B
        &DefaultParser<ALLEGRO::T_1B>,
        // 0x1C
        &Parse1C,
        // 0x1D
        &Parse1D,
        // 0x1E
        &Parse1E,
        // 0x1F
        &Parse1F,
        // 0x20
        &DefaultParser<ALLEGRO::T_20>,
        // 0x21
        &Parse21,
        // 0x22
        &DefaultParser<ALLEGRO::T_22>,
        // 0x23
        &DefaultParser<ALLEGRO::T_23>,
        // 0x24
        &DefaultParser<ALLEGRO::T_24>,
        // 0x25
        nullptr,
        // 0x26
        &DefaultParser<ALLEGRO::T_26>,
        // 0x27
        &Parse27,
        // 0x28
        &DefaultParser<ALLEGRO::T_28_ZONE>,
        // 0x29
        nullptr,
        // 0x2A
        &Parse2A,
        // 0x2B
        &DefaultParser<ALLEGRO::T_2B>,
        // 0x2C
        &DefaultParser<ALLEGRO::T_2C>,
        // 0x2D
        &DefaultParser<ALLEGRO::T_2D>,
        // 0x2E
        &DefaultParser<ALLEGRO::T_2E>,
        // 0x2F
        &DefaultParser<ALLEGRO::T_2F>,
        // 0x30
        &DefaultParser<ALLEGRO::T_30>,
        // 0x31
        &Parse31,
        // 0x32
        &DefaultParser<ALLEGRO::T_32>,
        // 0x33
        &DefaultParser<ALLEGRO::T_33>,
        // 0x34
        &DefaultParser<ALLEGRO::T_34>,
        // 0x35
        &Parse35,
        // 0x36
        &Parse36,
        // 0x37
        &DefaultParser<ALLEGRO::T_37>,
        // 0x38
        &DefaultParser<ALLEGRO::T_38_FILM>,
        // 0x39
        &DefaultParser<ALLEGRO::T_39_FILM_LAYER_LIST>,
        // 0x3A
        &DefaultParser<ALLEGRO::T_3A_FILM_LAYER_LIST_NODE>,
        // 0x3B
        &Parse3B,
        // 0x3C
        &Parse3C,
    };

    BOARD*              m_board;
    const ALLEGRO_FILE& m_brd;

    ALLEGRO::HEADER* m_header = nullptr;
    void*            m_baseAddr = nullptr;
    void*            m_curAddr = nullptr;

    double m_scaleFactor = 0;

    // 0 = not calculated yet
    uint16_t m_layerCount = 0;

    std::vector<std::tuple<uint32_t, uint32_t>> m_layers;
    std::map<uint32_t, char*>                   m_strings;
    std::map<uint32_t, ALLEGRO::T_2A>           m_t2A_map;
    std::map<uint32_t, ALLEGRO::T_1E>           m_t1E_map;


#if BOOST_VERSION >= 108100
    boost::unordered_flat_map<uint32_t, void*> m_ptrs;
#else
    std::unordered_map<uint32_t, void*> m_ptrs;
#endif
};

// Helpers
uint32_t round_to_word( uint32_t len )
{
    if( len % 4 != 0 )
    {
        return len / 4 * 4 + 4;
    }
    else
    {
        return len;
    }
}

double cfp_to_double( ALLEGRO::CADENCE_FP r )
{
    ALLEGRO::CADENCE_FP swapped;
    swapped.x = r.y;
    swapped.y = r.x;
    double g;
    std::memcpy( &g, &swapped, 8 );
    return g;
}

// Implementation in header file is necessary because of templating.

template <ALLEGRO::MAGIC magic>
ALLEGRO_PARSER<magic>::ALLEGRO_PARSER( BOARD* aBoard, const ALLEGRO_FILE& aAllegroBrdFile ) :
        m_board( aBoard ), m_brd( aAllegroBrdFile ){};

template <ALLEGRO::MAGIC magic>
void ALLEGRO_PARSER<magic>::Parse()
{
    m_baseAddr = m_brd.region.get_address();
    size_t size = m_brd.region.get_size();

    m_curAddr = m_baseAddr;

    m_header = static_cast<ALLEGRO::HEADER*>( m_curAddr );
    Skip( sizeof( ALLEGRO::HEADER ) );

    m_ptrs.reserve( m_header->object_count );

    // Cache a fixed scale factor
    switch( m_header->units )
    {
    case ALLEGRO::BRD_UNITS::IMPERIAL:
        m_scaleFactor = ( (double) 25400. ) / m_header->unit_divisor;
        break;
    case ALLEGRO::BRD_UNITS::METRIC:
        m_scaleFactor = ( (double) 1000000. ) / m_header->unit_divisor;
        break;
    default: wxLogError( "Unrecognized units type: 0x%02X", m_header->units ); return;
    };

    // Layer map
    for( uint8_t i = 0; i < 25; i++ )
    {
        uint32_t xs[2] = { *( (uint32_t*) m_curAddr ), *( ( (uint32_t*) m_curAddr ) + 1 ) };
        Skip( sizeof( xs ) );
        m_layers.push_back( std::make_tuple( xs[0], xs[1] ) );
    }

    // Strings map
    m_curAddr = (char*) m_baseAddr + 0x1200;
    for( uint32_t i = 0; i < m_header->strings_count; i++ )
    {
        uint32_t id = *( (uint32_t*) ( m_curAddr ) );
        Skip( sizeof( id ) );

        m_strings[id] = (char*) m_curAddr;

        // Add one to include the NULL byte that might force the length to one
        // word longer.
        uint32_t len = strlen( (char*) m_curAddr );
        Skip( round_to_word( len + 1 ) );

        // wxLogMessage( "%08X = %s\n", id, m_strings[id] );
    }


    // All other objects
    while( m_curAddr < (char*) m_baseAddr + size && *static_cast<uint8_t*>( m_curAddr ) != 0x00 )
    {
        uint8_t t = *static_cast<uint8_t*>( m_curAddr );
        if( PRINT_ALL_ITEMS )
        {
            Log( "Handling t=0x%02X: ", t );
        }

        const PARSER_FUNC parser = PARSER_TABLE[t];
        if( t < sizeof( PARSER_TABLE ) / sizeof( PARSER_FUNC ) && parser != nullptr )
        {
            parser( *this );
        }
        else
        {
            Log( "Do not have parser t=%02X available", t );
            return;
        }
    }

    BuildBoard();
}

template <ALLEGRO::MAGIC magic>
template <template <ALLEGRO::MAGIC> typename T>
uint32_t ALLEGRO_PARSER<magic>::DefaultParser( ALLEGRO_PARSER<magic>& parser )
{
    T<magic>* inst = static_cast<T<magic>*>( parser.m_curAddr );
    parser.m_ptrs[inst->k] = parser.m_curAddr;
    size_t size = ALLEGRO::sizeof_allegro_obj<T<magic>>();
    parser.Skip( size );
    return inst->k;
};

template <ALLEGRO::MAGIC magic>
uint32_t ALLEGRO_PARSER<magic>::Parse03( ALLEGRO_PARSER<magic>& parser )
{
    ALLEGRO::T_03<magic>* i = static_cast<ALLEGRO::T_03<magic>*>( parser.m_curAddr );
    uint32_t              k = ALLEGRO_PARSER<magic>::DefaultParser<ALLEGRO::T_03>( parser );

    char* buf;
    // inst.has_str = false;
    uint32_t size;

    // log(base_addr_glb, address, "- Subtype.t = 0x%02X\n", i->subtype.t);
    switch( i->subtype.t & 0xFF )
    {
    case 0x65: break;
    case 0x64:
    case 0x66:
    case 0x67:
    case 0x6A: parser.Skip( 4 ); break;
    case 0x6D:
    case 0x6E:
    case 0x6F:
    case 0x68:
    case 0x6B:
    case 0x71:
    case 0x73:
    case 0x78: parser.Skip( round_to_word( i->subtype.size ) ); break;
    case 0x69: parser.Skip( 8 ); break;
    case 0x6C:
        size = *( static_cast<uint32_t*>( (void*) parser.m_curAddr ) );
        parser.Skip( 4 + 4 * size );
        break;
    case 0x70:
    case 0x74:
        uint16_t x[2];
        x[0] = *( static_cast<uint16_t*>( (void*) parser.m_curAddr ) );
        parser.Skip( 2 );
        x[1] = *( static_cast<uint16_t*>( (void*) parser.m_curAddr ) );
        parser.Skip( 2 );
        parser.Skip( x[1] + 4 * x[0] );
        break;
    case 0xF6: parser.Skip( 80 ); break;
    default:
        // log( fs.region.get_address(), address, "- Unexpected value subtype=0x%02X\n",
        //      i->subtype.t );
        parser.Log( "Unexpected value subtype = 0x%02X", i->subtype.t );
        exit( 1 );
    };

    return 0;
};

template <ALLEGRO::MAGIC magic>
uint32_t ALLEGRO_PARSER<magic>::Parse1C( ALLEGRO_PARSER<magic>& parser )
{
    ALLEGRO::T_1C<magic>* i = static_cast<ALLEGRO::T_1C<magic>*>( parser.m_curAddr );
    uint32_t              k = ALLEGRO_PARSER<magic>::DefaultParser<ALLEGRO::T_1C>( parser );

    uint16_t size = 0;
    if constexpr( magic < ALLEGRO::A_172 )
    {
        // printf("layer count 1 %d\n", i->layer_count);
        size = 10 + i->layer_count * 3;
    }
    else
    {
        // printf("layer count 2 %d\n", i->layer_count);
        size = 21 + i->layer_count * 4;
    }

    for( uint32_t i = 0; i < size; i++ )
    {
        // log(fs.region.get_address(), address, "- Skipping t13 %d\n", i);
        // log_n_words(address, 4);
        parser.Skip( ALLEGRO::sizeof_allegro_obj<ALLEGRO::t13<magic>>() );
    }
    // skip(address, size * sizeof_until_tail<t13<magic>>());

    if constexpr( magic >= ALLEGRO::A_172 )
    {
        parser.Skip( 4 );
    }

    if constexpr( magic >= ALLEGRO::A_172 )
    {
        parser.Skip( i->n * 40 );
    }
    else
    {
        parser.Skip( i->n * 32 );
    }

    return k;
};

template <ALLEGRO::MAGIC magic>
uint32_t ALLEGRO_PARSER<magic>::Parse1D( ALLEGRO_PARSER<magic>& parser )
{
    ALLEGRO::T_1D<magic>* i = static_cast<ALLEGRO::T_1D<magic>*>( parser.m_curAddr );
    uint32_t              k = ALLEGRO_PARSER<magic>::DefaultParser<ALLEGRO::T_1D>( parser );

    // log(&f, "size_a = %d, size_b = %d\n", size_a, size_b);
    parser.Skip( i->size_b * ( magic >= ALLEGRO::A_162 ? 56 : 48 ) );
    parser.Skip( i->size_a * 256 );
    if( magic >= ALLEGRO::A_172 )
    {
        parser.Skip( 4 );
    }
    return 0;
};

template <ALLEGRO::MAGIC magic>
uint32_t ALLEGRO_PARSER<magic>::Parse1E( ALLEGRO_PARSER<magic>& parser )
{
    ALLEGRO::T_1E* x1E_inst = new ALLEGRO::T_1E;
    memcpy( x1E_inst, parser.m_curAddr, sizeof( ALLEGRO::T_1E_HEADER ) );
    parser.Skip( sizeof( ALLEGRO::T_1E_HEADER ) );
    // f.read((char*)x1E_inst, sizeof(x1E_hdr));
    x1E_inst->s = (char*) parser.m_curAddr;
    parser.Skip( round_to_word( x1E_inst->hdr.size ) );
    // f.read(x1E_inst->s, round_to_word(x1E_inst->hdr.size));
    parser.m_t1E_map[x1E_inst->hdr.k] = *x1E_inst;
    if( magic >= ALLEGRO::A_172 )
    {
        parser.Skip( 4 );
    }
    return 0;
}

template <ALLEGRO::MAGIC magic>
uint32_t ALLEGRO_PARSER<magic>::Parse1F( ALLEGRO_PARSER<magic>& parser )
{
    ALLEGRO::T_1F<magic>* i = static_cast<ALLEGRO::T_1F<magic>*>( parser.m_curAddr );
    uint32_t              k = ALLEGRO_PARSER<magic>::DefaultParser<ALLEGRO::T_1F>( parser );

    if constexpr( magic >= ALLEGRO::A_172 )
    {
        parser.Skip( i->size * 280 + 8 );
    }
    else if constexpr( magic >= ALLEGRO::A_162 )
    {
        parser.Skip( i->size * 280 + 4 );
    }
    else
    {
        parser.Skip( i->size * 240 + 4 );
    }

    return k;
}

template <ALLEGRO::MAGIC magic>
uint32_t ALLEGRO_PARSER<magic>::Parse21( ALLEGRO_PARSER& parser )
{
    ALLEGRO::T_21_HEADER* i = static_cast<ALLEGRO::T_21_HEADER*>( parser.m_curAddr );
    if( i->r == 1304 )
    {
        ALLEGRO::STACKUP_MATERIAL ps;
        memcpy( &ps, parser.m_curAddr, sizeof( ALLEGRO::STACKUP_MATERIAL ) );
        parser.Skip( sizeof( ALLEGRO::STACKUP_MATERIAL ) );
        // fs.stackup_materials[ps.hdr.k] = ps;
        // log(&f, "- - Stackup material... %02d->%s\n", ps.layer_id,
        // ps.material);
    }
    else if( i->r == 14093 )
    {
        // log(&f, "- - i->r=\x1b[31m14093\x1b[0m...?\n");
        parser.Skip( i->size );
        // } else if (i->r == 2050) {
    }
    else if( i->r == 0x0407 )
    {
        ALLEGRO::META_NETLIST_PATH r;
        parser.Skip( ALLEGRO::sizeof_allegro_obj<ALLEGRO::META_NETLIST_PATH>() );
        parser.Skip( 1028 );
        // char s[1028];
        // f.read((char*)&r, sizeof_until_tail<meta_netlist_path>());
        // f.read(s, sizeof(s));
        // r.path = std::string(s);
        // fs.netlist_path = r;
    }
    else
    {
        parser.Skip( i->size );
    }
    return 0;
};

template <ALLEGRO::MAGIC magic>
uint32_t ALLEGRO_PARSER<magic>::Parse27( ALLEGRO_PARSER& parser )
{
    parser.m_curAddr = (char*) parser.m_baseAddr + parser.m_header->x27_end_offset - 1;
    return 0;
}

template <ALLEGRO::MAGIC magic>
uint32_t ALLEGRO_PARSER<magic>::Parse2A( ALLEGRO_PARSER& parser )
{
    ALLEGRO::T_2A x2A_inst;
    memcpy( &x2A_inst, parser.m_curAddr, sizeof( ALLEGRO::x2A_hdr ) );

    // f.read((char*)&x2A_inst.hdr, sizeof(x2A_hdr));
    parser.Skip( sizeof( ALLEGRO::x2A_hdr ) );
    if( magic >= ALLEGRO::A_174 )
    {
        parser.Skip( 4 );
    }

    if( magic <= ALLEGRO::A_164 )
    {
        x2A_inst.references = false;
        for( uint16_t i = 0; i < x2A_inst.hdr.size; i++ )
        {
            char buf[36] = { 0 };
            // f.read((char*)buf, 36);
            memcpy( buf, parser.m_curAddr, 36 );
            parser.Skip( 36 );
            ALLEGRO::x2A_layer_properties suffix = *( (ALLEGRO::x2A_layer_properties*) &buf[32] );
            ALLEGRO::x2A_local_entry entry = ALLEGRO::x2A_local_entry{ std::string( buf ), suffix };
            x2A_inst.local_entries.push_back( entry );
        }
    }
    else
    {
        x2A_inst.references = true;
        for( uint16_t i = 0; i < x2A_inst.hdr.size; i++ )
        {
            ALLEGRO::x2A_reference_entry entry;
            // f.read((char*)&entry, sizeof(x2A_reference_entry));
            memcpy( &entry, parser.m_curAddr, sizeof( ALLEGRO::x2A_reference_entry ) );
            parser.Skip( sizeof( ALLEGRO::x2A_reference_entry ) );
            x2A_inst.reference_entries.push_back( entry );
        }
    }
    // f.read((char*)&x2A_inst.k, 4);
    x2A_inst.k = *static_cast<uint32_t*>( parser.m_curAddr );
    parser.Skip( 4 );
    parser.m_t2A_map[x2A_inst.k] = x2A_inst;
    return 0;
}

template <ALLEGRO::MAGIC magic>
uint32_t ALLEGRO_PARSER<magic>::Parse31( ALLEGRO_PARSER& parser )
{
    ALLEGRO::T_31<magic>* i = static_cast<ALLEGRO::T_31<magic>*>( parser.m_curAddr );
    uint32_t              k = ALLEGRO_PARSER<magic>::DefaultParser<ALLEGRO::T_31>( parser );

    if( i->len > 0 )
    {
        uint32_t len = round_to_word( i->len );
        parser.Skip( len );
    }

    return 0;
}

template <ALLEGRO::MAGIC magic>
uint32_t ALLEGRO_PARSER<magic>::Parse35( ALLEGRO_PARSER& parser )
{
    parser.Skip( 124 );
    return 0;
}

template <ALLEGRO::MAGIC magic>
uint32_t ALLEGRO_PARSER<magic>::Parse36( ALLEGRO_PARSER& parser )
{
    ALLEGRO::T_36<magic> inst;
    memcpy( &inst, parser.m_curAddr, ALLEGRO::sizeof_allegro_obj<ALLEGRO::T_36<magic>>() );
    parser.Skip( ALLEGRO::sizeof_allegro_obj<ALLEGRO::T_36<magic>>() );
    // f.read((char*)inst, sizeof_until_tail<x36<version>>());

    // log(&f, "- x36\n");
    // log(&f, "- - inst->c    = 0x %04X\n", ntohs(inst->c));
    // log(&f, "- - inst->k    = 0x %08X\n", ntohl(inst->k));
    // log(&f, "- - inst->next = 0x %08X\n", ntohl(inst->next));
    // if constexpr (!std::is_same_v<decltype(inst->un1), std::monostate>) {
    //     log(&f, "- - inst->un1  = 0x %08X\n", ntohl(inst->un1));
    // } else {
    //     log(&f, "- - inst->un1  = n/a\n");
    // }
    // log(&f, "- - inst->size = %d\n", inst->size);
    // log(&f,
    //     "- - inst->count = %d, inst->last_idx = %d, inst->un3 = 0x"
    //     " % 08X\n ",
    //     inst->count, inst->last_idx, ntohl(inst->un3));
    // if constexpr (!std::is_same_v<decltype(inst->un2), std::monostate>) {
    //     log(&f, "- - inst->un2  = 0x %08X\n", ntohl(inst->un2));
    // } else {
    //     log(&f, "- - inst->un2  = n/a\n");
    // }

    switch( inst.c )
    {
    case 0x02:
        for( uint32_t i = 0; i < inst.size; i++ )
        {
            // Discard
            ALLEGRO::x36_x02<magic> inst_x02 =
                    *static_cast<ALLEGRO::x36_x02<magic>*>( parser.m_curAddr );
            parser.Skip( ALLEGRO::sizeof_allegro_obj<ALLEGRO::x36_x02<magic>>() );
            // f.read((char*)&inst_x02,
            // sizeof_until_tail<x36_x02<version>>()); log(&f, "- x02.str =
            // %s\n", inst_x02.str);
        }
        break;
    case 0x06:
        for( uint32_t i = 0; i < inst.size; i++ )
        {
            // Currently just read the object and immediately throw it
            // away, because we don't know of any use for it.
            ALLEGRO::x36_x06<magic> inst_x06 =
                    *static_cast<ALLEGRO::x36_x06<magic>*>( parser.m_curAddr );
            parser.Skip( ALLEGRO::sizeof_allegro_obj<ALLEGRO::x36_x06<magic>>() );
            // f.read((char*)&inst_x06,
            // sizeof_until_tail<x36_x06<version>>()); log(&f, "- x06.n =
            // %d\n", inst_x06.n); log(&f, "- x06.r = %d\n", inst_x06.r);
            // log(&f, "- x06.s = %d\n", inst_x06.s);
            // log(&f, "- x06.un1 = %d\n\n", inst_x06.un1);
        }
        break;
    case 0x03:
        for( uint32_t i = 0; i < inst.size; i++ )
        {
            // Just throw this away after reading it.
            ALLEGRO::x36_x03<magic> inst_x03 =
                    *static_cast<ALLEGRO::x36_x03<magic>*>( parser.m_curAddr );
            parser.Skip( ALLEGRO::sizeof_allegro_obj<ALLEGRO::x36_x03<magic>>() );
            // f.read((char*)&inst_x03,
            // sizeof_until_tail<x36_x03<version>>()); if (version >= A_172)
            // {
            //     log(&f, "- - s = \"%s\"\n", inst_x03.str);
            // } else {
            //     log(&f, "- - s = \"%s\"\n", inst_x03.str_16x);
            // }
        }
        break;
    case 0x05: parser.Skip( inst.size * 28 ); break;
    case 0x08:
        for( uint32_t i = 0; i < inst.size; i++ )
        {
            ALLEGRO::x36_x08<magic> x = *static_cast<ALLEGRO::x36_x08<magic>*>( parser.m_curAddr );
            parser.Skip( ALLEGRO::sizeof_allegro_obj<ALLEGRO::x36_x08<magic>>() );
            // f.read((char*)&x, sizeof_until_tail<x36_x08<version>>());
            inst.x08s.push_back( x );
            // log(&f,
            //     "- - 0x%02X %08x %08x"
            //     " xs={h=% 7d w=% 7d % 7d % 7d % 7d %7d}",
            //     i, ntohl(x.a), ntohl(x.b), x.char_height, x.char_width,
            //     x.xs[0], x.xs[1], x.xs[2], x.xs[3]);
            // if constexpr (!std::is_same_v<decltype(x.ys),
            // std::monostate>) {
            //     printf(" ys={% 7d % 7d % 7d}", x.ys[0], x.ys[1],
            //     x.ys[2]);
            // }
            // printf("\n");
        }
        break;
    case 0x0B:
        // No clue what this represents?
        parser.Skip( inst.size * 1016 );
        // for (uint32_t i = 0; i < inst.size; i++) {
        //     log(&f, "- - Next words:");
        //     log_n_words(&f, 6);
        //     skip(&f, 1016 - 4 * 6);
        // }
        break;
    case 0x0C: parser.Skip( inst.size * 232 ); break;
    case 0x0D: parser.Skip( inst.size * 200 ); break;
    case 0x0F:
        for( uint32_t i = 0; i < inst.size; i++ )
        {
            ALLEGRO::x36_x0F<magic> x = *static_cast<ALLEGRO::x36_x0F<magic>*>( parser.m_curAddr );
            parser.Skip( sizeof( ALLEGRO::x36_x0F<magic> ) );
            // f.read((char*)&x, sizeof(x36_x0F<version>));
            inst.x0Fs.push_back( x );
        }
        break;
    case 0x10:
        parser.Skip( inst.size * 108 );
        // for (uint32_t i = 0; i < inst.size; i++) {
        //     log(&f, "- - Next words:");
        //     log_n_words(&f, 6);
        //     skip(&f, 108 - 4 * 6);
        // }
        break;
    default:
        // log( fs.region.get_address(), address, "- Don\'t know how to handle c=%X\n", inst.c );
        parser.Log( "Don\'t know how to handle c=%X", inst.c );
        exit( 1 );
    }

    // fs.x36_map[inst.k] = upgrade<version, A_174>( inst );
    return 0;
}

template <ALLEGRO::MAGIC magic>
uint32_t ALLEGRO_PARSER<magic>::Parse3B( ALLEGRO_PARSER& parser )
{
    ALLEGRO::T_3B<magic>* i = (ALLEGRO::T_3B<magic>*) parser.m_curAddr;

    parser.Skip( ALLEGRO::sizeof_allegro_obj<ALLEGRO::T_3B<magic>>() );
    parser.Skip( round_to_word( i->len ) );

    return 0;
}

template <ALLEGRO::MAGIC magic>
uint32_t ALLEGRO_PARSER<magic>::Parse3C( ALLEGRO_PARSER& parser )
{
    ALLEGRO::T_3C<magic>* i = static_cast<ALLEGRO::T_3C<magic>*>( parser.m_curAddr );
    uint32_t              k = ALLEGRO_PARSER<magic>::DefaultParser<ALLEGRO::T_3C>( parser );

    parser.Skip( i->size * 4 );

    return 0;
}

template <ALLEGRO::MAGIC magic>
uint8_t ALLEGRO_PARSER<magic>::GetCopperLayerCount()
{
    if( m_layerCount > 1 )
    {
        return m_layerCount;
    }

    std::tuple<uint32_t, uint32_t> tup = m_layers[4];
    uint32_t                       ptr = std::get<1>( tup );

    if( m_t2A_map.count( ptr ) > 0 )
    {
        const ALLEGRO::T_2A* x = &m_t2A_map.at( ptr );
        if( x->references )
        {
            m_layerCount = x->reference_entries.size();
        }
        else
        {
            m_layerCount = x->local_entries.size();
        }
        return m_layerCount;
    }
    else
    {
        wxLogMessage( "Unable to determine layer count" );
        return -1;
    }
}

template <ALLEGRO::MAGIC magic>
void ALLEGRO_PARSER<magic>::AddAnnotation( BOARD_ITEM_CONTAINER&       aContainer,
                                           const ALLEGRO::T_14<magic>& i14 )
{
    PCB_LAYER_ID layer = User_1;

    if( i14.subtype == 0x9 && i14.layer == 246 )
    {
        layer = B_SilkS;
    }
    if( i14.subtype == 0x9 && i14.layer == 247 )
    {
        layer = F_SilkS;
    }
    else if( i14.subtype == 0x1 && i14.layer == 241 )
    {
        layer = F_SilkS;
    }

    uint32_t k = i14.ptr2;
    while( true )
    {
        if( IsType( k, 0x01 ) )
        {
            ALLEGRO::T_01<magic>* i01 = static_cast<ALLEGRO::T_01<magic>*>( m_ptrs[k] );

            double r = cfp_to_double( i01->r );

            VECTOR2I start, end, center, mid;
            start.x = Scale( i01->coords[0] );
            start.y = Scale( -i01->coords[1] );
            end.x = Scale( i01->coords[2] );
            end.y = Scale( -i01->coords[3] );
            center.x = Scale( (int32_t) cfp_to_double( i01->x ) );
            center.y = Scale( -(int32_t) cfp_to_double( i01->y ) );
            mid = CalcArcMid( start, end, center );

            PCB_SHAPE* arc = new PCB_SHAPE( &aContainer, SHAPE_T::ARC );

            arc->SetLayer( layer );
            arc->SetWidth( Scale( i01->width ) );
            arc->SetArcGeometry( start, mid, end );

            aContainer.Add( arc, ADD_MODE::APPEND );

            k = i01->next;
        }
        else if( IsType( k, 0x15 ) )
        {
            ALLEGRO::T_15<magic>* i15 = static_cast<ALLEGRO::T_15<magic>*>( m_ptrs[k] );

            VECTOR2D start, end;
            start.x = Scale( i15->coords[0] );
            start.y = Scale( -i15->coords[1] );
            end.x = Scale( i15->coords[2] );
            end.y = Scale( -i15->coords[3] );

            PCB_SHAPE* segment = new PCB_SHAPE( &aContainer, SHAPE_T::SEGMENT );

            segment->SetLayer( layer );
            segment->SetWidth( Scale( i15->width ) );
            segment->SetStart( start );
            segment->SetEnd( end );

            aContainer.Add( segment, ADD_MODE::APPEND );

            k = i15->next;
        }
        else if( IsType( k, 0x16 ) )
        {
            ALLEGRO::T_16<magic>* i16 = static_cast<ALLEGRO::T_16<magic>*>( m_ptrs[k] );

            VECTOR2D start, end;
            start.x = Scale( i16->coords[0] );
            start.y = Scale( -i16->coords[1] );
            end.x = Scale( i16->coords[2] );
            end.y = Scale( -i16->coords[3] );

            PCB_SHAPE* segment = new PCB_SHAPE( &aContainer, SHAPE_T::SEGMENT );

            segment->SetLayer( layer );
            segment->SetWidth( Scale( i16->width ) );
            segment->SetStart( start );
            segment->SetEnd( end );

            aContainer.Add( segment, ADD_MODE::APPEND );

            k = i16->next;
        }
        else if( IsType( k, 0x17 ) )
        {
            ALLEGRO::T_17<magic>* i17 = static_cast<ALLEGRO::T_17<magic>*>( m_ptrs[k] );

            VECTOR2D start, end;
            start.x = Scale( i17->coords[0] );
            start.y = Scale( -i17->coords[1] );
            end.x = Scale( i17->coords[2] );
            end.y = Scale( -i17->coords[3] );

            PCB_SHAPE* segment = new PCB_SHAPE( &aContainer, SHAPE_T::SEGMENT );

            segment->SetLayer( layer );
            segment->SetWidth( Scale( i17->width ) );
            segment->SetStart( start );
            segment->SetEnd( end );

            aContainer.Add( segment, ADD_MODE::APPEND );

            k = i17->next;
        }
        else
        {
            break;
        }
    }
}

template <ALLEGRO::MAGIC magic>
void ALLEGRO_PARSER<magic>::AddFootprint( const ALLEGRO::T_2B<magic>& i2B )
{
    // wxLogMessage( "Adding footprint" );
    wxASSERT_MSG( m_strings.count( i2B.footprint_string_ref ) == 1, "Expected a string" );
    wxString footprint_name = m_strings[i2B.footprint_string_ref];
    LIB_ID   lib_id = LIB_ID( "my_lib", footprint_name );

    uint32_t k = i2B.ptr2;
    while( IsType( k, 0x2D ) )
    {
        ALLEGRO::T_2D<magic>* i2D = static_cast<ALLEGRO::T_2D<magic>*>( m_ptrs[k] );

        std::unique_ptr<FOOTPRINT> fp = std::make_unique<FOOTPRINT>( m_board );
        fp->SetAttributes( FOOTPRINT_ATTR_T::FP_SMD );
        fp->SetFPID( lib_id );

        // Refdes
        std::optional<wxString> refdes = RefdesLookup( *i2D );
        if( refdes )
        {
            fp->SetReference( *refdes );
        }
        else
        {
            fp->SetReference( "A0" );
        }

        // Position the object
        fp->SetPosition( VECTOR2I( Scale( i2D->coords[0] ), Scale( -i2D->coords[1] ) ) );
        fp->SetOrientationDegrees( i2D->rotation / 1000. );
        fp->SetLayerAndFlip( i2D->layer == 0 ? F_Cu : B_Cu );

        // Add annotations
        uint32_t k_marking = i2D->ptr1;
        while( true )
        {
            if( IsType( k_marking, 0x14 ) )
            {
                ALLEGRO::T_14<magic>* i14 = static_cast<ALLEGRO::T_14<magic>*>( m_ptrs[k_marking] );
                AddAnnotation( *fp, *i14 );
                k_marking = i14->next;
            }
            else
            {
                break;
            }
        }

        // Add text
        uint32_t k_text = i2D->ptr3;
        while( true )
        {
            if( IsType( k_text, 0x30 ) )
            {
                ALLEGRO::T_30<magic>* i30 = static_cast<ALLEGRO::T_30<magic>*>( m_ptrs[k_text] );
                AddText( *fp, *i30 );
                k_text = i30->next;
            }
            else if( IsType( k_text, 0x03 ) )
            {
                ALLEGRO::T_03<magic>* i03 = static_cast<ALLEGRO::T_03<magic>*>( m_ptrs[k_text] );
                // wxLogMessage( "%s T_03.k=%08X", *refdes, ntohl( i03->k ) );
                k_text = i03->next;
            }
            else
            {
                break;
            }
        }

        // Add zones
        uint32_t k_shape = i2D->ptr4[1];
        while( IsType( k_shape, 0x28 ) )
        {
            ALLEGRO::T_28_ZONE<magic>* i28 =
                    static_cast<ALLEGRO::T_28_ZONE<magic>*>( m_ptrs[k_shape] );
            AddZone( *fp, {}, *i28 );
            k_shape = i28->ptr5;
        }

        fp->AddField( PCB_FIELD( &*fp, -1, "ABC" ) );

        // Draw pads
        uint32_t k_pad = i2D->first_pad_ptr;
        while( IsType( k_pad, 0x32 ) )
        {
            ALLEGRO::T_32<magic>* i32 = static_cast<ALLEGRO::T_32<magic>*>( m_ptrs[k_pad] );
            AddPad( &*fp, *i32 );
            k_pad = i32->next;
        }

        k = i2D->next;

        m_board->Add( fp.release(), ADD_MODE::APPEND );
    }
}

template <ALLEGRO::MAGIC magic>
void ALLEGRO_PARSER<magic>::AddPad( FOOTPRINT* fp, const ALLEGRO::T_32<magic>& i32 )
{
    std::unique_ptr<PAD> pad = std::make_unique<PAD>( fp );

    // Find net information
    if( IsType( i32.ptr1, 0x04 ) )
    {
        const ALLEGRO::T_04<magic>* i04 = static_cast<ALLEGRO::T_04<magic>*>( m_ptrs[i32.ptr1] );

        NETINFO_ITEM* netinfo = NetInfo( *i04 );
        pad->SetNet( netinfo );
    }

    const ALLEGRO::T_0D<magic>* i0D = static_cast<ALLEGRO::T_0D<magic>*>( m_ptrs[i32.ptr5] );
    VECTOR2I                    center;
    center.x = Scale( i0D->coords[0] );
    center.y = Scale( -i0D->coords[1] );
    pad->SetPosition( center );
    pad->SetOrientationDegrees( i0D->rotation / 1000. );

    std::optional<wxString> pad_number = StringLookup( i0D->str_ptr );
    if( pad_number )
    {
        pad->SetNumber( *pad_number );
    }
    else
    {
        pad->SetNumber( "MISSING" );
    }

    const ALLEGRO::T_1C<magic>* i1C = static_cast<ALLEGRO::T_1C<magic>*>( m_ptrs[i0D->pad_ptr] );

    switch( i1C->pad_info.pad_type )
    {
    case ALLEGRO::SmtPin:
        pad->SetAttribute( PAD_ATTRIB::SMD );
        pad->SetLayer( F_Cu );
        pad->SetLayerSet( LSET( 1, F_Cu ) );
        break;
    case ALLEGRO::ThroughVia:
        pad->SetAttribute( PAD_ATTRIB::PTH );
        pad->SetLayer( F_Cu );
        pad->SetLayerSet( LSET::AllCuMask() );
        break;
    default: wxLogMessage( "Unknown pad type %d", i1C->pad_info.pad_type );
    }

    // FIXME: The list of pads immediately follows i1C in memory. This is an
    // ugly hack to get that address, though.
    const ALLEGRO::t13<magic>* first_t13 =
            (ALLEGRO::t13<magic>*) ( (char*) i1C
                                     + ALLEGRO::sizeof_allegro_obj<ALLEGRO::T_1C<magic>>() );

    pad->SetSize( VECTOR2I( Scale( first_t13->w ), Scale( first_t13->h ) ) );

    switch( first_t13->t )
    {
    case 0x02: pad->SetShape( PAD_SHAPE::CIRCLE ); break;
    case 0x05:
    case 0x06:
        pad->SetShape( PAD_SHAPE::RECTANGLE );
        break;
        // default: wxLogMessage( "Unrecognized type on %s: t=%d", fp->GetReference(), first_t13->t );
    }

    fp->Add( pad.release(), ADD_MODE::APPEND );
}

template <ALLEGRO::MAGIC magic>
void ALLEGRO_PARSER<magic>::AddText( BOARD_ITEM_CONTAINER&       aContainer,
                                     const ALLEGRO::T_30<magic>& i30 )
{
    ALLEGRO::T_31<magic>* i31 = static_cast<ALLEGRO::T_31<magic>*>( m_ptrs[i30.str_graphic_ptr] );

    const char* s = (char*) ( &i31->TAIL );

    if( strlen( s ) == 0 )
    {
        // wxLogMessage( "empty string" );
    }
    else
    {
        // wxLogMessage( "T_30.k=%08X T_31.k=%08X %04X \"%s\"", ntohl( i30.k ), ntohl( i31->k ),
        //               i31->layer, s );

        PCB_LAYER_ID layer = User_2;

        // FIXME: This probably needs to be flipped relative to the placement
        switch( i31->layer )
        {
        case ALLEGRO::STR_LAYER::TOP_TEXT:
        case ALLEGRO::STR_LAYER::TOP_PIN:
        case ALLEGRO::STR_LAYER::TOP_REFDES: layer = User_4; break;
        }

        std::unique_ptr<PCB_TEXT> t = std::make_unique<PCB_TEXT>( &aContainer );
        t->SetLayer( layer );
        t->SetPosition( VECTOR2I( Scale( i30.coords[0] ), Scale( -i30.coords[1] ) ) );
        t->Rotate( t->GetPosition(), EDA_ANGLE( i30.rotation / 1000., DEGREES_T ) );
        t->SetText( wxString( s ) );
        aContainer.Add( t.release(), ADD_MODE::APPEND );
    }
}

template <ALLEGRO::MAGIC magic>
void ALLEGRO_PARSER<magic>::AddTrack( const ALLEGRO::T_1B<magic>&       i1B,
                                      const ALLEGRO::T_05_TRACK<magic>& i05 )
{
    uint32_t k = i05.first_segment_ptr;

    NETINFO_ITEM* netinfo = NetInfo( i1B.net_name );

    while( true )
    {
        if( IsType( k, 0x01 ) )
        {
            ALLEGRO::T_01<magic>* i01 = static_cast<ALLEGRO::T_01<magic>*>( m_ptrs[k] );

            double r = cfp_to_double( i01->r );

            VECTOR2I start, end, center, mid;
            start.x = Scale( i01->coords[0] );
            start.y = Scale( -i01->coords[1] );
            end.x = Scale( i01->coords[2] );
            end.y = Scale( -i01->coords[3] );
            center.x = Scale( (int32_t) cfp_to_double( i01->x ) );
            center.y = Scale( -(int32_t) cfp_to_double( i01->y ) );

            SHAPE_ARC shape_arc;
            shape_arc.ConstructFromStartEndCenter( start, end, center, i01->subtype == 0x00, 0 );

            std::unique_ptr<PCB_ARC> arc = std::make_unique<PCB_ARC>( m_board, &shape_arc );

            arc->SetLayer( EtchLayerToKi( i05.layer ) );
            arc->SetWidth( Scale( i01->width ) );
            arc->SetNet( netinfo );

            m_board->Add( arc.release(), ADD_MODE::APPEND );

            k = i01->next;
        }
        else if( IsType( k, 0x15 ) )
        {
            ALLEGRO::T_15<magic>* i15 = static_cast<ALLEGRO::T_15<magic>*>( m_ptrs[k] );

            VECTOR2D start, end;
            start.x = Scale( i15->coords[0] );
            start.y = Scale( -i15->coords[1] );
            end.x = Scale( i15->coords[2] );
            end.y = Scale( -i15->coords[3] );

            std::unique_ptr<PCB_TRACK> track = std::make_unique<PCB_TRACK>( m_board );

            track->SetLayer( EtchLayerToKi( i05.layer ) );
            track->SetWidth( Scale( i15->width ) );
            track->SetStart( start );
            track->SetEnd( end );
            track->SetNet( netinfo );

            m_board->Add( track.release(), ADD_MODE::APPEND );

            k = i15->next;
        }
        else if( IsType( k, 0x16 ) )
        {
            ALLEGRO::T_16<magic>* i16 = static_cast<ALLEGRO::T_16<magic>*>( m_ptrs[k] );

            VECTOR2D start, end;
            start.x = Scale( i16->coords[0] );
            start.y = Scale( -i16->coords[1] );
            end.x = Scale( i16->coords[2] );
            end.y = Scale( -i16->coords[3] );

            std::unique_ptr<PCB_TRACK> track = std::make_unique<PCB_TRACK>( m_board );

            track->SetLayer( EtchLayerToKi( i05.layer ) );
            track->SetWidth( Scale( i16->width ) );
            track->SetStart( start );
            track->SetEnd( end );
            track->SetNet( netinfo );

            m_board->Add( track.release(), ADD_MODE::APPEND );

            k = i16->next;
        }
        else if( IsType( k, 0x17 ) )
        {
            ALLEGRO::T_17<magic>* i17 = static_cast<ALLEGRO::T_17<magic>*>( m_ptrs[k] );

            VECTOR2D start, end;
            start.x = Scale( i17->coords[0] );
            start.y = Scale( -i17->coords[1] );
            end.x = Scale( i17->coords[2] );
            end.y = Scale( -i17->coords[3] );

            std::unique_ptr<PCB_TRACK> track = std::make_unique<PCB_TRACK>( m_board );

            track->SetLayer( EtchLayerToKi( i05.layer ) );
            track->SetWidth( Scale( i17->width ) );
            track->SetStart( start );
            track->SetEnd( end );
            track->SetNet( netinfo );

            m_board->Add( track.release(), ADD_MODE::APPEND );

            k = i17->next;
        }
        else
        {
            break;
        }
    }
}

template <ALLEGRO::MAGIC magic>
void ALLEGRO_PARSER<magic>::AddVia( const ALLEGRO::T_33<magic>& i33 )
{
    std::unique_ptr<PCB_VIA> via = std::make_unique<PCB_VIA>( m_board );
    via->SetPosition( VECTOR2D( Scale( i33.coords[0] ), Scale( -i33.coords[1] ) ) );

    // Find net information
    if( IsType( i33.ptr1, 0x04 ) )
    {
        const ALLEGRO::T_04<magic>* i04 = static_cast<ALLEGRO::T_04<magic>*>( m_ptrs[i33.ptr1] );

        NETINFO_ITEM* netinfo = NetInfo( *i04 );
        via->SetNet( netinfo );
    }

    m_board->Add( via.release(), ADD_MODE::APPEND );
}

template <ALLEGRO::MAGIC magic>
void ALLEGRO_PARSER<magic>::AddZone( BOARD_ITEM_CONTAINER&                      aContainer,
                                     const std::optional<ALLEGRO::T_1B<magic>>& i1B,
                                     const ALLEGRO::T_28_ZONE<magic>&           i28 )
{
    uint32_t k = i28.first_segment_ptr;

    std::unique_ptr<SHAPE_POLY_SET> outline = std::make_unique<SHAPE_POLY_SET>();
    outline->AddOutline( ShapeStartingAt( &k ) );

    uint32_t hole_k = i28.ptr4;
    while( IsType( hole_k, 0x34 ) )
    {
        ALLEGRO::T_34<magic>* i34 = static_cast<ALLEGRO::T_34<magic>*>( m_ptrs[hole_k] );
        // wxLogMessage( "Handling x34 0x%08X", ntohl( i34->k ) );
        uint32_t k = i34->ptr2;

        SHAPE_LINE_CHAIN hole_chain = ShapeStartingAt( &k );
        if( hole_chain.PointCount() >= 3 )
        {
            hole_chain.SetClosed( true );
            // zone->AddPolygon( hole_chain );
            outline->AddHole( hole_chain );
        }
        else
        {
            // wxLogMessage( "Skipping, why?" );
        }
        hole_k = i34->next;
    }


    if( i28.subtype == ALLEGRO::ZONE_TYPE_NET )
    {
        std::unique_ptr<ZONE> zone = std::make_unique<ZONE>( &aContainer );
        zone->SetZoneName( wxString::Format( "x28: 0x%08X", ntohl( i28.k ) ) );
        zone->SetLocalClearance( 0 );
        zone->SetMinThickness( 0 );
        zone->SetOutline( outline.release() );

        PCB_LAYER_ID layer = (PCB_LAYER_ID) ( ( (int) F_Cu ) + i28.layer );

        zone->SetLayer( layer );

        if( i1B )
        {
            NETINFO_ITEM* netinfo = NetInfo( i1B->net_name );
            zone->SetNet( netinfo );
        }

        // zone->SetFilledPolysList( layer, outline );

        aContainer.Add( zone.release(), ADD_MODE::APPEND );
    }
    else if( i28.subtype == ALLEGRO::ZONE_TYPE_SILK )
    {
        std::unique_ptr<PCB_SHAPE> shape =
                std::make_unique<PCB_SHAPE>( &aContainer, SHAPE_T::POLY );
        shape->SetPolyShape( *outline.release() );
        shape->SetFilled( true );

        if( i28.layer == 0xFD )
        {
            shape->SetLayer( F_SilkS );
        }
        else
        {
            shape->SetLayer( User_3 );
        }

        aContainer.Add( shape.release(), ADD_MODE::APPEND );
    }
    /*
    else if( i28.subtype == ALLEGRO::ZONE_TYPE_TBD )

    {
        // PCB_LAYER_ID layer = (PCB_LAYER_ID) ( ( (int) F_Cu ) + i28.layer );
        zone->SetLayer( F_Cu );

        // zone->SetIsRuleArea( true );
        // zone->SetDoNotAllowCopperPour( true );

        aContainer.Add( zone.release(), ADD_MODE::APPEND );
    }
    */
    else
    {
        // zone->SetLayer( User_3 );
        // aContainer.Add( zone.release(), ADD_MODE::APPEND );
    }
}

template <ALLEGRO::MAGIC magic>
void ALLEGRO_PARSER<magic>::BuildBoard()
{
    // FIXME: Add one so we can see the bottom layer, which is currently tagged
    // incorrectly on an inner layer.
    m_board->SetCopperLayerCount( GetCopperLayerCount() );

    ALLEGRO::T_1B<magic>* i = static_cast<ALLEGRO::T_1B<magic>*>( m_ptrs[m_header->ll_x1B.head] );
    if( i != nullptr )
    {
        do
        {
            if( i->ptr1 != 0 )
            {
                ALLEGRO::T_04<magic>* i04 = static_cast<ALLEGRO::T_04<magic>*>( m_ptrs[i->ptr1] );
                do
                {
                    uint32_t k = i04->ptr2;
                    while( true )
                    {
                        if( IsType( k, 0x33 ) )
                        {
                            ALLEGRO::T_33<magic>* i33 =
                                    static_cast<ALLEGRO::T_33<magic>*>( m_ptrs[k] );
                            // auto& i = fs->get_x33( k );
                            // printf("- - Found x33 w/ key = 0x %08X\n", ntohl(k));
                            AddVia( *i33 );
                            k = i33->un1;
                        }
                        else if( IsType( k, 0x32 ) )
                        {
                            ALLEGRO::T_32<magic>* i =
                                    static_cast<ALLEGRO::T_32<magic>*>( m_ptrs[k] );
                            // printf("- - Found x32 w/ key = 0x %08X\n", ntohl(k));
                            k = i->un1;
                        }
                        else if( IsType( k, 0x2E ) )
                        {
                            ALLEGRO::T_2E<magic>* i =
                                    static_cast<ALLEGRO::T_2E<magic>*>( m_ptrs[k] );
                            // printf("- - Found x2E w/ key = 0x %08X\n", ntohl(k));
                            k = i->un[0];
                        }
                        else if( IsType( k, 0x28 ) )
                        {
                            ALLEGRO::T_28_ZONE<magic>* i28 =
                                    static_cast<ALLEGRO::T_28_ZONE<magic>*>( m_ptrs[k] );
                            AddZone( *m_board, *i, *i28 );

                            k = i28->ptr5;
                        }
                        else if( IsType( k, 0x0E ) )
                        {
                            ALLEGRO::T_0E<magic>* i =
                                    static_cast<ALLEGRO::T_0E<magic>*>( m_ptrs[k] );
                            k = i->un[0];
                        }
                        else if( IsType( k, 0x05 ) )
                        {
                            ALLEGRO::T_05_TRACK<magic>* i05 =
                                    static_cast<ALLEGRO::T_05_TRACK<magic>*>( m_ptrs[k] );
                            AddTrack( *i, *i05 );

                            k = i05->ptr0;
                        }
                        else if( IsType( k, 0x04 ) | IsType( k, 0x1B ) )
                        {
                            break;
                        }
                        else
                        {
                            wxLogMessage( "Unexpected key = 0x %08X :(", ntohl( k ) );
                            break;
                        }
                    }

                    if( i04->next == i->k )
                    {
                        break;
                    }
                    else
                    {
                        i04 = static_cast<ALLEGRO::T_04<magic>*>( m_ptrs[i04->next] );
                    }
                } while( true );
            }

            i = static_cast<ALLEGRO::T_1B<magic>*>( m_ptrs[i->next] );
        } while( i->next != m_header->ll_x1B.tail );
    }

    uint32_t k = m_header->ll_x0E_x28.head;
    if( k != 0 )
    {
        while( k != m_header->ll_x0E_x28.tail )
        {
            if( IsType( k, 0x0E ) )
            {
                // Do not know what this represents
                auto i0E = static_cast<ALLEGRO::T_0E<magic>*>( m_ptrs[k] );
                k = i0E->next;
            }
            else if( IsType( k, 0x28 ) )
            {
                auto i28 = static_cast<ALLEGRO::T_28_ZONE<magic>*>( m_ptrs[k] );
                AddZone( *m_board, {}, *i28 );
                k = i28->ptr5;
            }
            else
            {
                // wxLogMessage( "Unexpected type with key = 0x%08X!", ntohl( k ) );
                break;
            }
        }
    }

    k = m_header->ll_x14.head;
    if( k != 0 )
    {
        while( k != m_header->ll_x14.tail )
        {
            auto i14 = static_cast<ALLEGRO::T_14<magic>*>( m_ptrs[k] );
            AddAnnotation( *m_board, *i14 );
            k = i14->next;
        }
    }

    k = m_header->ll_x2B.head;
    if( k != 0 )
    {
        while( k != m_header->ll_x2B.tail )
        {
            auto i2B = static_cast<ALLEGRO::T_2B<magic>*>( m_ptrs[k] );
            AddFootprint( *i2B );
            k = i2B->next;
        }
    }

    k = m_header->ll_x03_x30.head;
    if( k != 0 )
    {
        while( k != m_header->ll_x03_x30.tail )
        {
            if( IsType( k, 0x30 ) )
            {
                ALLEGRO::T_30<magic>* i30 = static_cast<ALLEGRO::T_30<magic>*>( m_ptrs[k] );
                AddText( *m_board, *i30 );
                k = i30->next;
            }
            else if( IsType( k, 0x03 ) )
            {
                ALLEGRO::T_03<magic>* i03 = static_cast<ALLEGRO::T_03<magic>*>( m_ptrs[k] );
                k = i03->next;
            }
            else
            {
                wxLogMessage( "Unexpected type in ll_x03_x30" );
                break;
            }
        }
    }
}

template <ALLEGRO::MAGIC magic>
void ALLEGRO_PARSER<magic>::Skip( std::size_t n )
{
    m_curAddr = (void*) ( ( (char*) m_curAddr ) + n );
}

template <ALLEGRO::MAGIC magic>
void ALLEGRO_PARSER<magic>::Log( const char* fmt... )
{
    va_list args;
    va_start( args, fmt );

    wxLogMessage( "@ 0x%08lX:", (char*) m_curAddr - (char*) m_baseAddr );
    wxVLogMessage( fmt, args );

    va_end( args );
}

template <ALLEGRO::MAGIC magic>
bool ALLEGRO_PARSER<magic>::IsType( uint32_t k, uint8_t t )
{
    return ( m_ptrs.count( k ) > 0 ) && ( *(uint8_t*) m_ptrs[k] == t );
}

template <ALLEGRO::MAGIC magic>
std::optional<wxString> ALLEGRO_PARSER<magic>::StringLookup( uint32_t k )
{
    if( m_strings.count( k ) > 0 )
    {
        return wxString( m_strings[k] );
    }
    return {};
}

template <ALLEGRO::MAGIC magic>
SHAPE_LINE_CHAIN ALLEGRO_PARSER<magic>::ShapeStartingAt( uint32_t* k )
{
    SHAPE_LINE_CHAIN chain;

    bool first = true;
    while( k != nullptr )
    {
        if( IsType( *k, 0x01 ) )
        {
            ALLEGRO::T_01<magic>* i01 = static_cast<ALLEGRO::T_01<magic>*>( m_ptrs[*k] );

            VECTOR2I start, end, center, mid;
            start.x = Scale( i01->coords[0] );
            start.y = Scale( -i01->coords[1] );
            end.x = Scale( i01->coords[2] );
            end.y = Scale( -i01->coords[3] );
            center.x = Scale( (int32_t) cfp_to_double( i01->x ) );
            center.y = Scale( -(int32_t) cfp_to_double( i01->y ) );

            SHAPE_ARC arc;
            arc.ConstructFromStartEndCenter( start, end, center, i01->subtype == 0x00, 0 );

            if( first )
            {
                chain.Append( start );
            }
            chain.Append( arc );

            *k = i01->next;
        }
        else if( IsType( *k, 0x15 ) )
        {
            ALLEGRO::T_15<magic>* i15 = static_cast<ALLEGRO::T_15<magic>*>( m_ptrs[*k] );

            VECTOR2D start, end;
            start.x = Scale( i15->coords[0] );
            start.y = Scale( -i15->coords[1] );
            end.x = Scale( i15->coords[2] );
            end.y = Scale( -i15->coords[3] );

            if( first )
            {
                chain.Append( start );
            }
            chain.Append( end );

            *k = i15->next;
        }
        else if( IsType( *k, 0x16 ) )
        {
            ALLEGRO::T_16<magic>* i16 = static_cast<ALLEGRO::T_16<magic>*>( m_ptrs[*k] );

            VECTOR2D start, end;
            start.x = Scale( i16->coords[0] );
            start.y = Scale( -i16->coords[1] );
            end.x = Scale( i16->coords[2] );
            end.y = Scale( -i16->coords[3] );

            if( first )
            {
                chain.Append( start );
            }
            chain.Append( end );

            *k = i16->next;
        }
        else if( IsType( *k, 0x17 ) )
        {
            ALLEGRO::T_17<magic>* i17 = static_cast<ALLEGRO::T_17<magic>*>( m_ptrs[*k] );

            VECTOR2D start, end;
            start.x = Scale( i17->coords[0] );
            start.y = Scale( -i17->coords[1] );
            end.x = Scale( i17->coords[2] );
            end.y = Scale( -i17->coords[3] );

            if( first )
            {
                chain.Append( start );
            }
            chain.Append( end );

            *k = i17->next;
        }
        else
        {
            break;
        }

        first = false;
    }

    return chain;
}

template <ALLEGRO::MAGIC magic>
PCB_LAYER_ID ALLEGRO_PARSER<magic>::EtchLayerToKi( uint8_t a_layer )
{
    if( a_layer == GetCopperLayerCount() - 1 )
    {
        return B_Cu;
    }
    else
    {
        return (PCB_LAYER_ID) ( ( (int) F_Cu ) + a_layer );
    }
}

template <ALLEGRO::MAGIC magic>
double ALLEGRO_PARSER<magic>::Scale( double a_allegroValue )
{
    return a_allegroValue * m_scaleFactor;
}

template <ALLEGRO::MAGIC magic>
std::optional<wxString> ALLEGRO_PARSER<magic>::RefdesLookup( const ALLEGRO::T_2D<magic>& i2D )
{
    uint32_t inst_ref = 0;
    if constexpr( std::is_same_v<decltype( i2D.inst_ref ), std::monostate> )
    {
        inst_ref = i2D.inst_ref_16x;
    }
    else
    {
        inst_ref = i2D.inst_ref;
    }

    if( inst_ref == 0 || !IsType( inst_ref, 0x07 ) )
        return {};

    ALLEGRO::T_07<magic>* i07 = static_cast<ALLEGRO::T_07<magic>*>( m_ptrs[inst_ref] );
    wxASSERT_MSG( m_strings.count( i07->refdes_string_ref ) == 1, "Expected a string" );
    return wxString( m_strings[i07->refdes_string_ref] );
}

template <ALLEGRO::MAGIC magic>
NETINFO_ITEM* ALLEGRO_PARSER<magic>::NetInfo( uint32_t k )
{
    NETINFO_ITEM*           netinfo = nullptr;
    std::optional<wxString> netname = StringLookup( k );

    if( netname )
    {
        if( NETINFO_ITEM* item = m_board->FindNet( *netname ) )
        {
            netinfo = item;
        }
        else
        {
            item = new NETINFO_ITEM( m_board, *netname, m_board->GetNetCount() + 1 );
            m_board->Add( item, ADD_MODE::APPEND );
            netinfo = item;
        }
    }
    return netinfo;
}

template <ALLEGRO::MAGIC magic>
NETINFO_ITEM* ALLEGRO_PARSER<magic>::NetInfo( const ALLEGRO::T_04<magic>& i04 )
{
    ALLEGRO::T_1B<magic>* i1B = static_cast<ALLEGRO::T_1B<magic>*>( m_ptrs[i04.ptr1] );
    return NetInfo( i1B->net_name );
}

#endif // ALLEGRO_PARSER_H_
