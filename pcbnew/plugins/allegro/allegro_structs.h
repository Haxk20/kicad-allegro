#ifndef ALLEGRO_STRUCTS_H_
#define ALLEGRO_STRUCTS_H_

#include <cstdint>
#include <string>
#include <optional>
#include <vector>

namespace ALLEGRO
{

#if _MSC_VER
#define COND_FIELD( COND, T, NAME )                                                                \
    [[msvc::no_unique_address]] std::conditional_t<( COND ), T, std::monostate>( NAME );
#else
#define COND_FIELD( COND, T, NAME )                                                                \
    [[no_unique_address]] std::conditional_t<( COND ), T, std::monostate>( NAME );
#endif

// This alternative to `sizeof` is used where conditional fields are at the end
// of a `struct`. Without a `uint32_t TAIL` at the end, the size is incorrect.
template <typename T>
constexpr size_t sizeof_allegro_obj()
{
    return offsetof( T, TAIL );
}

enum MAGIC
{
    A_160 = 0x00130000,
    A_162 = 0x00130400,
    A_164 = 0x00130C00,
    A_165 = 0x00131000,
    A_166 = 0x00131500,
    A_172 = 0x00140400,
    A_174 = 0x00140900,
    A_MAX = 0x00150000,
};

// Linked list
struct LL_PTRS
{
    uint32_t tail;
    uint32_t head;
};

/* Cadence seems to use a double for storing some types, but the first and
 * second 32 bit words seem to be flipped. This type provides a function to
 * swap them.
 */
typedef struct
{
    uint32_t x;
    uint32_t y;
} CADENCE_FP;

struct HEADER
{
    uint32_t magic;
    uint32_t un1[4];
    uint32_t object_count;
    uint32_t un2[9];
    LL_PTRS  ll_x04;
    LL_PTRS  ll_x06;
    LL_PTRS  ll_x0C_2;
    LL_PTRS  ll_x0E_x28;
    LL_PTRS  ll_x14;
    LL_PTRS  ll_x1B;
    LL_PTRS  ll_x1C;
    LL_PTRS  ll_x24_x28;
    LL_PTRS  ll_unused_1;
    LL_PTRS  ll_x2B;
    LL_PTRS  ll_x03_x30;
    LL_PTRS  ll_x0A_2;
    LL_PTRS  ll_x1D_x1E_x1F;
    LL_PTRS  ll_unused_2;
    LL_PTRS  ll_x38;
    LL_PTRS  ll_x2C;
    LL_PTRS  ll_x0C;
    LL_PTRS  ll_unused_3;

    // Is there only ever one x35? This points to both the start and end?
    uint32_t x35_start;
    uint32_t x35_end;

    LL_PTRS  ll_x36;
    LL_PTRS  ll_x21;
    LL_PTRS  ll_unused_4;
    LL_PTRS  ll_x0A;
    uint32_t un5;
    char     allegro_version[60];
    uint32_t un6;
    uint32_t max_key;
    uint32_t un7[20];
    uint32_t x27_end_offset;
    uint32_t un8;
    uint32_t strings_count;
    uint32_t un9[166];
};

// Curved line segment
template <MAGIC magic>
struct T_01
{
    uint16_t t;
    uint8_t  un0;

    // Determines whether the shape extends outwards or cuts into the shape.
    uint8_t subtype;

    uint32_t k;

    uint32_t next;
    uint32_t parent;

    // Some bit mask?
    uint32_t un1;

    // New in 17.2
    COND_FIELD( magic >= A_172, uint32_t, un6 );

    uint32_t width;

    // Start and end coordinates
    int32_t coords[4];

    CADENCE_FP x;
    CADENCE_FP y;
    CADENCE_FP r;

    int32_t bbox[4];

    uint32_t TAIL;
};

static_assert( sizeof_allegro_obj<T_01<A_160>>() == 80 );
static_assert( sizeof_allegro_obj<T_01<A_172>>() == 84 );
static_assert( sizeof_allegro_obj<T_01<A_174>>() == 84 );

struct x03_hdr_subtype
{
    uint8_t  t;
    uint8_t  un3;
    uint16_t size;
};

template <MAGIC magic>
struct T_03
{
    uint32_t t;
    uint32_t k;

    // May point to `x36_x0F` object, among other types?
    uint32_t next;

    COND_FIELD( magic >= A_172, uint32_t, un1 );
    x03_hdr_subtype subtype;
    COND_FIELD( magic >= A_172, uint32_t, un2 );

    uint32_t TAIL;

    bool        has_str;
    std::string s;
    uint32_t    ptr;
};

template <MAGIC magic>
struct T_04
{
    uint32_t t;
    uint32_t k;
    uint32_t next;

    // Points to `x1B`
    uint32_t ptr1;

    // Points to `x05`
    uint32_t ptr2;

    COND_FIELD( magic >= A_174, uint32_t, un1 );

    uint32_t TAIL;
};

template <MAGIC magic>
struct T_06
{
    uint32_t t;
    uint32_t k;

    // Pointer to x06
    uint32_t next;

    // Pointer to string
    uint32_t ptr1;

    uint32_t ptr2;

    // Points to instance
    uint32_t ptr3;

    // Points to footprint
    uint32_t ptr4;

    // Points to x08
    uint32_t ptr5;

    // Points to x03, schematic symbol (e.g. RESISTOR)
    uint32_t ptr6;

    // Added in 17.x?
    COND_FIELD( magic >= A_172, uint32_t, un2 );

    uint32_t TAIL;
};

template <MAGIC magic>
struct T_07
{
    uint32_t t;
    uint32_t k;
    uint32_t un1; // Points to another instance

    COND_FIELD( magic >= A_172, uint32_t, ptr0 );
    COND_FIELD( magic >= A_172, uint32_t, un4 );
    COND_FIELD( magic >= A_172, uint32_t, un2 );

    uint32_t ptr1; // 0x2D

    COND_FIELD( magic < A_172, uint32_t, un5 );

    uint32_t refdes_string_ref;

    uint32_t ptr2;

    uint32_t ptr3; // x03 or null

    uint32_t un3; // Always null?

    uint32_t ptr4; // x32 or null

    uint32_t TAIL;
};

static_assert( sizeof_allegro_obj<T_07<A_172>>() == 48 );
static_assert( sizeof_allegro_obj<T_07<A_174>>() == 48 );

template <MAGIC magic>
struct T_08
{
    uint32_t t;
    uint32_t k;

    // x08?
    COND_FIELD( magic >= A_172, uint32_t, ptr1 );

    // String
    // Note: String pointers swap position around `ptr2` in different magics.
    COND_FIELD( magic < A_172, uint32_t, str_ptr_16x );

    // x06
    uint32_t ptr2;

    // String
    COND_FIELD( magic >= A_172, uint32_t, str_ptr );

    // x11
    uint32_t ptr3;

    COND_FIELD( magic >= A_172, uint32_t, un1 );

    // Can be string, usually null
    uint32_t ptr4;

    uint32_t TAIL;
};

template <MAGIC magic>
struct T_09
{
    uint32_t t;
    uint32_t k;

    // Always null?
    uint32_t un1[4];
    COND_FIELD( magic >= A_172, uint32_t, un3 );

    // Points to `x28`, `x32`, or `x33` (sometimes null)
    uint32_t ptr1;

    // Points to `x05` or `x09`
    uint32_t ptr2;

    // Always null?
    uint32_t un2;

    // Points to `x28`, `x32`, or `x33
    uint32_t ptr3;

    uint32_t ptr4;

    COND_FIELD( magic >= A_174, uint32_t, un4 );

    uint32_t TAIL;
};

// DRC, not fully decoded
template <MAGIC magic>
struct T_0A
{
    uint16_t t;
    uint8_t  subtype;
    uint8_t  layer;
    uint32_t k;
    uint32_t next;
    uint32_t un1; // Always null?
    COND_FIELD( magic >= A_172, uint32_t, un2 );
    int32_t  coords[4];
    uint32_t un4[4];
    uint32_t un5[5];
    COND_FIELD( magic >= A_174, uint32_t, un3 );

    uint32_t TAIL;
};

template <MAGIC magic>
struct T_0C
{
    uint16_t t;
    uint8_t  subtype;
    uint8_t  layer;
    uint32_t k;
    uint32_t next;
    COND_FIELD( magic >= A_172, uint32_t, un2 );
    COND_FIELD( magic >= A_172, uint32_t, un3 );
    union
    {
        uint32_t un[11];
        struct
        {
            uint32_t un1[2]; // Typically (always?) null
            uint32_t kind;
            uint32_t un5;
            int32_t  coords[4];
            uint32_t un6[3];
        };
    };
    COND_FIELD( magic >= A_174, uint32_t, un4 );

    uint32_t TAIL;
};

template <MAGIC magic>
struct T_0D
{
    uint32_t t;
    uint32_t k;
    uint32_t str_ptr;
    uint32_t ptr2;      // Points to a random different `x0D`?
    int32_t  coords[2]; // Relative to symbol origin
    COND_FIELD( magic >= A_174, uint32_t, un3 );
    uint32_t pad_ptr; // Points to `x1C`
    uint32_t un1;     // Always null?
    COND_FIELD( magic >= A_172, uint32_t, un2 );
    uint32_t bitmask;
    uint32_t rotation;

    uint32_t TAIL;
};

template <MAGIC magic>
struct T_0E
{
    uint32_t t;
    uint32_t k;
    uint32_t un[13];

    COND_FIELD( magic >= A_172, uint32_t[2], un1 );

    uint32_t TAIL;
};

// Footprint
// There's a copy for every instance, not just every kind of footprint.
template <MAGIC magic>
struct T_0F
{
    uint32_t t;
    uint32_t k;
    uint32_t ptr1; // Refers to `G1`, `G2`, `G3`, etc. string...?
    char     s[32];
    uint32_t ptr2; // Points to x06
    uint32_t ptr3; // Points to x11
    uint32_t un;   // Always null?

    COND_FIELD( magic >= A_172, uint32_t, un2 );
    COND_FIELD( magic >= A_174, uint32_t, un3 );

    uint32_t TAIL;
};

template <MAGIC magic>
struct T_10
{
    uint32_t t;
    uint32_t k;
    COND_FIELD( magic >= A_172, uint32_t, un2 );

    // Instance
    uint32_t ptr1;

    COND_FIELD( magic >= A_174, uint32_t, un3 );

    uint32_t ptr2;

    // Always null?
    uint32_t un1;

    uint32_t ptr3;

    // Points to a footprint (x0F) or so that I can't print right now?
    uint32_t ptr4;

    // Something like:
    // "@beagle_xm_revcb.schematic(sch_1):ins21222351@beagle_d.\tfp410.normal\(chips)"
    // or occasionally like:
    // ".\pstchip.dat"
    // or even like:
    // "L:/hdllib2/library/mechanical/rpv64101#2f02r1/chips/chips.prt"
    uint32_t path_str;

    uint32_t TAIL;
};

template <MAGIC magic>
struct T_11
{
    uint32_t t;
    uint32_t k;

    // String
    uint32_t ptr1;

    // Points to: x11, footprint
    uint32_t ptr2;

    // Points to: x08
    uint32_t ptr3;

    uint32_t un;

    COND_FIELD( magic >= A_174, uint32_t, un1 );

    uint32_t TAIL;
};

template <MAGIC magic>
struct T_12
{
    uint32_t t;
    uint32_t k;

    // x10 or x12
    uint32_t ptr1;

    // x11
    uint32_t ptr2;

    // x32
    uint32_t ptr3;

    uint32_t un0;
    COND_FIELD( magic >= A_165, uint32_t, un1 );
    COND_FIELD( magic >= A_174, uint32_t, un2 );

    uint32_t TAIL;
};

template <MAGIC magic>
struct T_14
{
    uint16_t type;
    uint8_t  subtype;
    uint8_t  layer;

    uint32_t k;
    uint32_t next;

    uint32_t ptr1;

    uint32_t un2;
    COND_FIELD( magic >= A_172, uint32_t, un3 );

    uint32_t ptr2;

    // Null or sometimes `x03`
    uint32_t ptr3;

    // `x26`
    uint32_t ptr4;

    uint32_t TAIL;
};

// Line segment
template <MAGIC magic>
struct T_15
{
    uint32_t t;
    uint32_t k;

    uint32_t next;
    uint32_t parent;

    // Usually null, sometimes one bit is set
    uint32_t un3;
    COND_FIELD( magic >= A_172, uint32_t, un4 );

    // Often 0
    uint32_t width;

    int32_t coords[4];

    uint32_t TAIL;
};

// Line segment
template <MAGIC magic>
struct T_16
{
    uint32_t t;
    uint32_t k;

    uint32_t next;
    uint32_t parent;

    // Some bit mask?
    uint32_t bitmask;

    COND_FIELD( magic >= A_172, uint32_t, un );

    uint32_t width;
    int32_t  coords[4];

    uint32_t TAIL;
};

// Line segment
template <MAGIC magic>
struct T_17
{
    uint32_t t;
    uint32_t k;

    uint32_t next;
    uint32_t parent;

    uint32_t un3;
    COND_FIELD( magic >= A_172, uint32_t, un4 );

    uint32_t width;
    int32_t  coords[4];

    uint32_t TAIL;
};

// A net
template <MAGIC magic>
struct T_1B
{
    uint32_t t;
    uint32_t k;

    // Points to another `x1B`
    uint32_t next;

    uint32_t net_name;

    // Null
    uint32_t un2;

    // 17.x?
    COND_FIELD( magic >= A_172, uint32_t, un4 );

    // Bit mask, not fully decoded
    // 0x0000 0002 = has net name?
    uint32_t type;

    // `x04`
    uint32_t ptr1;
    uint32_t ptr2;

    // `x03`
    // Points to a string like
    // `@\cc256xqfn-em_102612a_added_cap\.schematic(sch_1):aud_fsync_1v8`
    uint32_t path_str_ptr;

    // `x26`
    uint32_t ptr4;

    // `x1E`
    uint32_t model_ptr;

    // Null?
    uint32_t un3[2];

    // `x22`
    uint32_t ptr6;

    uint32_t TAIL;
};

template <MAGIC magic>
struct t13
{
    uint32_t str_ptr; // Often null
    uint32_t t;

    COND_FIELD( magic >= A_172, uint32_t, z0 );

    int32_t w;
    int32_t h;
    int32_t x2;
    int32_t x3;

    // This should be _after_ `x4`, but conditional fields at the end of
    // the struct are flakey?
    COND_FIELD( magic >= A_172, uint32_t, z );

    int32_t x4;

    uint32_t TAIL;
};

static_assert( sizeof_allegro_obj<t13<A_160>>() == 28 );
static_assert( sizeof_allegro_obj<t13<A_174>>() == 36 );

// x1C shows how to draw pads
enum PAD_TYPE : uint8_t
{
    ThroughVia = 0,
    Via = 1,
    SmtPin = 2,
    Slot = 4,
    HoleA = 8, // Maybe? Low confidence
    HoleB = 10 // Maybe?
};

struct PAD_INFO
{
    PAD_TYPE pad_type : 4;
    uint8_t  a : 4;
    uint8_t  b;
    uint8_t  c;
    uint8_t  d;
};

template <MAGIC magic>
struct T_1C
{
    uint16_t t;
    uint8_t  n;
    uint8_t  un1;
    uint32_t k;
    uint32_t next;
    uint32_t pad_str;
    uint32_t un2;
    uint32_t un3;
    uint32_t pad_path;
    COND_FIELD( magic < A_172, uint32_t[4], un4 );
    PAD_INFO pad_info;
    COND_FIELD( magic >= A_172, uint32_t[3], un5 );
    COND_FIELD( magic < A_172, uint16_t, un6 );
    uint16_t layer_count;
    COND_FIELD( magic >= A_172, uint16_t, un7 );
    uint32_t un8[7];
    COND_FIELD( magic >= A_172, uint32_t[28], un9 );
    COND_FIELD( magic == A_165 || magic == A_166, uint32_t[8], un10 );

    uint32_t                TAIL;
    std::vector<t13<magic>> parts;
};

static_assert( sizeof_allegro_obj<T_1C<A_164>>() == 20 * 4 );
static_assert( sizeof_allegro_obj<T_1C<A_165>>() == 28 * 4 );
static_assert( sizeof_allegro_obj<T_1C<A_172>>() == 47 * 4 );

static_assert( offsetof( T_1C<A_164>, layer_count ) == 50 );
static_assert( offsetof( T_1C<A_172>, layer_count ) == 44 );
static_assert( offsetof( T_1C<A_164>, pad_info ) == 44 );
static_assert( offsetof( T_1C<A_172>, pad_info ) == 28 );

template <MAGIC magic>
struct T_1D
{
    uint32_t t;
    uint32_t k;
    uint32_t un[3];
    uint16_t size_a;
    uint16_t size_b;

    uint32_t TAIL;
};

template <MAGIC magic>
struct T_1F
{
    uint32_t t;
    uint32_t k;
    uint32_t un[4];
    uint16_t un1;
    uint16_t size;

    uint32_t TAIL;
};

struct T_21_HEADER
{
    uint16_t t;
    uint16_t r;
    uint32_t size;
    uint32_t k;
};

struct META_NETLIST_PATH
{
    T_21_HEADER hdr;
    uint32_t    un1;
    uint32_t    bitmask;
    uint32_t    TAIL;
    std::string path;
};

struct STACKUP_MATERIAL
{
    T_21_HEADER hdr;
    uint32_t    a;
    uint32_t    layer_id;
    char        material[20];
    char        thickness[20];
    char        thermal_conductivity[20];
    char        electrical_conductivity[20];
    char        d_k[20];
    char        kind[20]; // E.g. DIELECTRIC, PLANE, SURFACE, CONDUCTOR
    char        d_f[20];
    char        unknown[20];
};

// Connection (rat). Draws a line between two connected pads.
template <MAGIC magic>
struct T_23
{
    uint16_t type;
    uint8_t  subtype;
    uint8_t  layer;

    uint32_t k;
    uint32_t next;

    uint32_t bitmask[2];

    // Matching placed symbol pad (`x32`)
    uint32_t ptr1;

    // Another `x23`
    uint32_t ptr2;

    // Another `x32`
    uint32_t ptr3;

    int32_t coords[5];

    uint32_t un[4];
    COND_FIELD( magic >= A_164, uint32_t[4], un2 );
    COND_FIELD( magic >= A_174, uint32_t, un1 );

    uint32_t TAIL;
};

template <MAGIC magic>
struct T_26
{
    uint32_t t;
    uint32_t k;

    uint32_t member_ptr;

    COND_FIELD( magic >= A_172, uint32_t, un );

    // Points to instance of `x22`, `x2C`, or `x33`.
    // Indicates the group that the member is a member of.
    uint32_t group_ptr;

    // Always null?
    uint32_t const_ptr;

    COND_FIELD( magic >= A_174, uint32_t, un1 );

    uint32_t TAIL;
};

// Shape
template <MAGIC magic>
struct T_28
{
    uint16_t type;
    uint8_t  subtype;
    uint8_t  layer;

    uint32_t k;
    uint32_t un1;

    // Points to one of: header value, `x04`, `x2B`, `x2D`
    uint32_t ptr1;

    // Null?
    uint32_t un2;

    COND_FIELD( magic >= A_172, uint32_t[2], un5 );

    // Points to `x28`, `x2D`, or `x33`
    uint32_t next;

    // Points to `x05` or `x09` (much more frequently `x09`)
    uint32_t ptr3;

    // Points to `x34`?
    uint32_t ptr4;

    // Line segments that form shape (x01, x15, x16, x17)
    uint32_t first_segment_ptr;

    // Null?
    uint32_t un3;

    // Has a few random bits set?
    uint32_t un4;

    // Points to `x26`, `x2C`
    COND_FIELD( magic >= A_172, uint32_t, ptr7 );

    // Points to `x03`
    uint32_t ptr6;

    COND_FIELD( magic < A_172, uint32_t, ptr7_16x );

    // Bounding box
    int32_t coords[4];

    uint32_t TAIL;
};

template <MAGIC magic>
struct T_2B
{
    uint32_t t;
    uint32_t k;
    uint32_t footprint_string_ref;

    uint32_t un1; // Always null?

    int32_t coords[4];

    // Points to another x2B object? Just the next item, not interesting.
    uint32_t next;

    // Points to placed symbol (x2D)
    uint32_t ptr2;

    // Points to pad of some type (x0D)?
    uint32_t ptr3;

    // Symbol pad (x32)
    uint32_t ptr4;

    // x14?
    uint32_t ptr5;

    // Usually (but not always) points to footprint file path.
    uint32_t str_ptr;

    // x0E
    uint32_t ptr6;

    // x24
    uint32_t ptr7;

    // x28
    uint32_t ptr8;

    COND_FIELD( magic >= A_164, uint32_t, un2 ); // Always null?

    // 17.x?
    COND_FIELD( magic >= A_172, uint32_t, un3 );

    uint32_t TAIL;
};

struct x2A_hdr
{
    uint16_t t;
    uint16_t size;
    // uint32_t k;
};

struct x2A_layer_properties
{
    uint8_t has_bot_reference : 1;
    uint8_t has_top_reference : 1;
    uint8_t : 0; // Skips to next byte
    uint8_t is_power : 1;
    uint8_t is_inner : 1;
    uint8_t is_inner2 : 1;
    uint8_t is_power2 : 1;
    uint8_t : 3;
    uint8_t is_signal : 1;
    uint8_t : 3;
    uint8_t is_top : 1;
    uint8_t is_bot : 1;
    uint8_t : 0;
    uint8_t empty;
};

static_assert( sizeof( x2A_layer_properties ) == 4 );

struct x2A_local_entry
{
    std::string          s;
    x2A_layer_properties properties;
};

struct x2A_reference_entry
{
    uint32_t             ptr;
    x2A_layer_properties properties;

    // Always null?
    uint32_t un1;
};

// Layer names
struct T_2A
{
    x2A_hdr                          hdr;
    bool                             references;
    std::vector<x2A_local_entry>     local_entries;
    std::vector<x2A_reference_entry> reference_entries;
    uint32_t                         k;
};

// Table?
template <MAGIC magic>
struct T_2C
{
    uint32_t t;
    uint32_t k;
    uint32_t next;

    COND_FIELD( magic >= A_172, uint32_t, un2 );
    COND_FIELD( magic >= A_172, uint32_t, un3 );
    COND_FIELD( magic >= A_172, uint32_t, un4 );

    // May be null
    uint32_t string_ptr;

    COND_FIELD( magic < A_172, uint32_t, un5 );

    // Points to instance of `x37` or `x3C` (or null).
    uint32_t ptr1;

    // Points to instance of `x03` (string or constant)
    uint32_t ptr2;

    // Points to instance of `x26` or `x2C`
    uint32_t ptr3;

    // Often but not always `0x0`.
    uint32_t bitmask;

    uint32_t TAIL;
};

// Placed symbol
template <MAGIC magic>
struct T_2D
{
    uint16_t t;
    uint8_t  layer;
    uint8_t  un0;

    uint32_t k;
    uint32_t next; // Points to x2B or x2D

    // Points to x2B?
    COND_FIELD( magic >= A_172, uint32_t, un4 );

    // Points to instance (x07) or null
    COND_FIELD( magic < A_172, uint32_t, inst_ref_16x );

    uint16_t un2;
    uint16_t un3;

    COND_FIELD( magic >= A_172, uint32_t, un5 );

    // Bit 0 = part is rotated to non-90 deg angle?
    uint32_t bitmask1;

    uint32_t rotation;
    int32_t  coords[2];

    // Points to instance (x07) or null
    COND_FIELD( magic >= A_172, uint32_t, inst_ref );

    uint32_t ptr1; // x14

    uint32_t first_pad_ptr; // x32

    uint32_t ptr3; // x03 or x30

    // ptr4[1] = bounding box?
    uint32_t ptr4[3]; // x24 or x28 or x0E

    // x26 instance indicating group membership
    uint32_t group_assignment_ptr;

    uint32_t TAIL;
};

// `x30` wraps a string graphic object and includes info like rotation and
// layer.
enum TEXT_ALIGNMENT : uint8_t
{
    TextAlignLeft = 1,
    TextAlignRight,
    TextAlignCenter
};

enum TEXT_REVERSAL : uint8_t
{
    TextStraight = 0,
    TextReversed = 1
};

struct TEXT_PROPERTIES
{
    uint8_t        key;
    uint8_t        bm1;
    TEXT_ALIGNMENT align;
    TEXT_REVERSAL  reversed;
};

template <MAGIC magic>
struct T_30
{
    uint16_t type;
    uint8_t  subtype;
    uint8_t  layer;
    uint32_t k;
    uint32_t next;

    COND_FIELD( magic >= A_172, uint32_t, un4 );
    COND_FIELD( magic >= A_172, uint32_t, un5 );
    COND_FIELD( magic >= A_172, TEXT_PROPERTIES, font );
    COND_FIELD( magic >= A_172, uint32_t, ptr3 );
    COND_FIELD( magic >= A_174, uint32_t, un7 );

    // Pointer to string graphic object
    uint32_t str_graphic_ptr;
    uint32_t un1;
    COND_FIELD( magic < A_172, TEXT_PROPERTIES, font_16x );

    COND_FIELD( magic >= A_172, uint32_t, ptr4 );

    int32_t  coords[2];
    uint32_t un3;

    uint32_t rotation;
    COND_FIELD( magic < A_172, uint32_t, ptr3_16x );

    uint32_t TAIL;
};

// String graphic
template <MAGIC magic>
struct T_31
{
    uint16_t t;
    uint8_t  subtype;
    uint8_t  layer;

    uint32_t k;

    // Points to x30
    uint32_t str_graphic_wrapper_ptr;

    int32_t coords[2];

    uint16_t un;

    uint16_t len;
    COND_FIELD( magic >= A_174, uint32_t, un2 );

    uint32_t TAIL;

    std::string s;
};

// Symbol pins
template <MAGIC magic>
struct T_32
{
    uint16_t type;
    uint8_t  subtype;
    uint8_t  layer;

    uint32_t k;
    uint32_t un1;

    uint32_t ptr1;

    // Don't look for layer here, seems almost always null.
    uint32_t bitmask1;

    COND_FIELD( magic >= A_172, uint32_t, prev );
    uint32_t next;

    uint32_t ptr3;
    uint32_t ptr4;
    uint32_t ptr5;
    uint32_t ptr6;
    uint32_t ptr7;
    uint32_t ptr8;

    uint32_t previous;

    COND_FIELD( magic >= A_172, uint32_t, un2 );

    uint32_t ptr10;
    uint32_t ptr11;

    int32_t coords[4];

    uint32_t TAIL;
};

template <MAGIC magic>
struct T_33
{
    uint16_t t;
    uint8_t  subtype;
    uint8_t  layer;

    uint32_t k;
    uint32_t un1;

    uint32_t ptr1;

    // Some bit field?
    uint32_t un2;
    COND_FIELD( magic >= A_172, uint32_t, un4 );

    uint32_t ptr2;

    COND_FIELD( magic >= A_172, uint32_t, ptr7 );

    int32_t coords[2];

    // Points to instance of `x05` or `0x09` (or null).
    uint32_t ptr3;

    uint32_t ptr4;

    // Null or pointer to (always empty?) string
    uint32_t ptr5;

    uint32_t ptr6;

    // Occassionally non-zero integers? Maybe bit fields?
    uint32_t un3[2];

    int32_t bb_coords[4];

    uint32_t TAIL;
};

// Keepout/keepin/etc.region
template <MAGIC magic>
struct T_34
{
    uint16_t t;
    uint8_t  subtype;
    uint8_t  layer;

    uint32_t k;
    uint32_t next;

    uint32_t ptr1; // x28
    COND_FIELD( magic >= A_172, uint32_t, un2 );
    uint32_t bitmask1;
    uint32_t ptr2; // x01
    uint32_t ptr3; // x03

    uint32_t un;

    uint32_t TAIL;
};

// List of strings, such as `"OBJECT_INSTANCE"`, `"VIASTRUCTUREID"`,
// `"0x00072768"`, or `"DIFF_IMPEDANCE_RULE"`. No obvious pattern?
//
// Variety of other undecoded fields follow, too.
template <MAGIC magic>
struct x36_x02
{
    uint8_t  str[32];
    uint32_t xs[14];
    COND_FIELD( magic >= A_164, uint32_t[3], ys );
    COND_FIELD( magic >= A_172, uint32_t[2], zs );

    uint32_t TAIL;
};

// List of strings, such as `"NO_TYPE"`, `"DEFAULT"`, `"16MIL"`, or
// `"ETH_DIFF"`. No obvious pattern?
template <MAGIC magic>
struct x36_x03
{
    COND_FIELD( magic >= A_172, uint8_t[64], str );
    COND_FIELD( magic < A_172, uint8_t[32], str_16x );
    COND_FIELD( magic >= A_174, uint32_t, un2 );

    uint32_t TAIL;
};

template <MAGIC magic>
struct x36_x06
{
    uint16_t n;
    uint8_t  r;
    uint8_t  s;
    uint32_t un1;
    COND_FIELD( magic < A_172, uint32_t[50], un2 );
    uint32_t TAIL;
};

// Font dimension information
template <MAGIC magic>
struct x36_x08
{
    uint32_t a, b;
    uint32_t char_height;
    uint32_t char_width;
    COND_FIELD( magic >= A_174, uint32_t, un2 );
    uint32_t xs[4];
    COND_FIELD( magic >= A_172, uint32_t[8], ys );

    uint32_t TAIL;
};

template <MAGIC magic>
struct x36_x0F
{
    uint32_t k;

    // Point to `x2C` instances
    uint32_t ptrs[3];

    // Points to `x03` instance
    uint32_t ptr2;
};

template <MAGIC magic>
struct T_36
{
    uint16_t t;
    uint16_t c;
    uint32_t k;
    uint32_t next;
    COND_FIELD( magic >= A_172, uint32_t, un1 );
    uint32_t size;

    uint32_t count;
    uint32_t last_idx;
    uint32_t un3;

    COND_FIELD( magic >= A_174, uint32_t, un2 );

    uint32_t TAIL;

    std::vector<x36_x08<magic>> x08s;
    std::vector<x36_x0F<magic>> x0Fs;
};

template <MAGIC magic>
struct T_37
{
    uint32_t t;
    uint32_t k;
    uint32_t ptr1;
    uint32_t un2;
    uint32_t capacity;
    uint32_t count;
    uint32_t un3;
    uint32_t ptrs[100];
    COND_FIELD( magic >= A_174, uint32_t, un4 );

    uint32_t TAIL;
};

template <MAGIC magic>
struct T_38_FILM
{
    uint32_t t;
    uint32_t k;
    uint32_t next;
    uint32_t layer_list;
    COND_FIELD( magic < A_166, char[20], film_name );
    COND_FIELD( magic >= A_166, uint32_t, layer_name_str );
    COND_FIELD( magic >= A_166, uint32_t, un2 );
    uint32_t un1[7];
    COND_FIELD( magic >= A_174, uint32_t, un3 );

    uint32_t TAIL;

    std::string s;
};

template <MAGIC magic>
struct T_39_FILM_LAYER_LIST
{
    uint32_t t;
    uint32_t k;
    uint32_t parent;
    uint32_t head;
    uint16_t x[22];

    uint32_t TAIL;
};

static_assert( sizeof_allegro_obj<T_38_FILM<A_160>>() == 64 );
static_assert( sizeof_allegro_obj<T_38_FILM<A_165>>() == 64 );
static_assert( sizeof_allegro_obj<T_38_FILM<A_166>>() == 52 );

template <MAGIC magic>
struct T_3A_FILM_LAYER_LIST_NODE
{
    uint16_t t;
    uint8_t  subtype;
    uint8_t  layer;

    uint32_t k;
    uint32_t next;

    uint32_t un;

    COND_FIELD( magic >= A_174, uint32_t, un1 );

    uint32_t TAIL;
};

// List of SI models
template <MAGIC magic>
struct T_3B
{
    uint16_t t;
    uint16_t subtype;

    uint32_t len;
    char     name[128];
    char     type[32];
    uint32_t un1;
    uint32_t un2;
    COND_FIELD( magic >= A_172, uint32_t, un3 );

    uint32_t TAIL;

    std::string model_str;
};

}; // namespace ALLEGRO

#endif // ALLEGRO_STRUCTS_H_
