#ifndef ALLEGRO_PARSER_H_
#define ALLEGRO_PARSER_H_

#include <cstdarg>

#include <boost/version.hpp>
#if BOOST_VERSION >= 108100
#include <boost/unordered/unordered_flat_map.hpp>
#else
#include <unordered_map>
#endif

#include <wx/log.h>

#include "board.h"

#include "allegro_elem_parsers.h"
#include "allegro_file.h"
#include "allegro_structs.h"

#define PRINT_ALL_ITEMS 1

template <ALLEGRO::MAGIC magic>
class ALLEGRO_PARSER
{
public:
    ALLEGRO_PARSER( BOARD* aBoard, const ALLEGRO_FILE& aAllegroBrdFile );
    void Parse();

private:
    void Skip( std::size_t n );
    void Log( const char* fmt... );

    template <template <ALLEGRO::MAGIC> typename T>
    static uint32_t DefaultParser( ALLEGRO_PARSER& parser );

    static uint32_t Parse03( ALLEGRO_PARSER& parser );
    static uint32_t Parse1C( ALLEGRO_PARSER& parser );
    static uint32_t Parse1D( ALLEGRO_PARSER& parser );
    static uint32_t Parse1F( ALLEGRO_PARSER& parser );
    static uint32_t Parse21( ALLEGRO_PARSER& parser );
    static uint32_t Parse27( ALLEGRO_PARSER& parser );
    static uint32_t Parse2A( ALLEGRO_PARSER& parser );
    static uint32_t Parse31( ALLEGRO_PARSER& parser );
    static uint32_t Parse35( ALLEGRO_PARSER& parser );
    static uint32_t Parse36( ALLEGRO_PARSER& parser );
    static uint32_t Parse3B( ALLEGRO_PARSER& parser );

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
        nullptr,
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
        nullptr,
        // 0x1F
        &Parse1F,
        // 0x20
        nullptr,
        // 0x21
        &Parse21,
        // 0x22
        nullptr,
        // 0x23
        &DefaultParser<ALLEGRO::T_23>,
        // 0x24
        nullptr,
        // 0x25
        nullptr,
        // 0x26
        &DefaultParser<ALLEGRO::T_26>,
        // 0x27
        &Parse27,
        // 0x28
        &DefaultParser<ALLEGRO::T_28>,
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
        nullptr,
        // 0x2F
        nullptr,
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
    };

    BOARD*              m_board;
    const ALLEGRO_FILE& m_brd;

    ALLEGRO::HEADER* m_header = nullptr;
    void*            m_baseAddr = nullptr;
    void*            m_curAddr = nullptr;

    std::vector<std::tuple<uint32_t, uint32_t>> m_layers;
    std::map<uint32_t, char*>                   m_strings;

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
    Log( "Starting to parse" );
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
            Log( "Do not have parser available" );
            break;
        }
    }
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
    // ( fs.x2A_map )[x2A_inst.k] = x2A_inst;
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

#endif // ALLEGRO_PARSER_H_
