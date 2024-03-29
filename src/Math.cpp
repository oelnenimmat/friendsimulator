/*
Leo Tamminen

Scalar types math wrapper

TODO(Leo): Proper unit testing!!
*/


constexpr f32 highest_f32   = std::numeric_limits<f32>::max();
constexpr f32 lowest_f32    = std::numeric_limits<f32>::lowest();
constexpr f32 smallest_f32  = std::numeric_limits<f32>::min();

constexpr s32 min_value_s32 = std::numeric_limits<s32>::min();

constexpr s16 max_value_s16 = std::numeric_limits<s16>::max();
constexpr s32 max_value_s32 = std::numeric_limits<s32>::max();
constexpr s64 max_value_s64 = std::numeric_limits<s64>::max();

constexpr s16 max_value_u16 = std::numeric_limits<u16>::max();
constexpr u32 max_value_u32 = std::numeric_limits<u32>::max();
constexpr u64 max_value_u64 = std::numeric_limits<u64>::max();

inline f32 abs_f32(f32 value)
{
    /* Story(Leo): I tested optimizing this using both pointer casting and type punning
    and manually setting the sign bit, and it was significantly faster on some types.
    That was, until I tried setting compiler optimization level up, and then this was
    fastest at least as significantly.

    Always optimize with proper compiler options :) */
    return fabsf(value);
}

inline f32 mod_f32(f32 dividend, f32 divisor)
{
    f32 result = fmodf(dividend, divisor);
    return result;
}

inline f32 floor_f32(f32 value)
{
    return floorf(value);
}

inline f32 ceil_f32(f32 value)
{
    return ceilf(value);
}

inline f32 round_f32(f32 value)
{
    return roundf(value);
}

inline f32 sign_f32(f32 value)
{
    return copysign(1.0f, value);
}

inline f32 square_f32(f32 value)
{
    return value * value;
}

inline f32 f32_sqr_root (f32 value)
{
    return sqrtf(value);
}

inline f32 f32_lerp(f32 a, f32 b, f32 t)
{
    return a + t * (b - a);
}

static f32 f32_unlerp(f32 a, f32 b, f32 value)
{
    // Todo(Leo): comment on commentors comment on naming
    // https://www.gamedev.net/articles/programming/general-and-gameplay-programming/inverse-lerp-a-super-useful-yet-often-overlooked-function-r5230/
    return (value - a) / (b - a);
}

f32 f32_min(f32 a, f32 b)
{
    return (a < b) ? a : b;
}

f32 f32_max(f32 a, f32 b)
{
    return (a > b) ? a : b;
}

s32 s32_min(s32 a, s32 b)
{
    return (a < b) ? a : b;
}

s32 s32_max(s32 a, s32 b)
{
    return (a > b) ? a : b;
}

internal s32 s32_clamp(s32 value, s32 minValue, s32 maxValue)
{
    value = (value < minValue) ? minValue : value;
    value = (value > maxValue) ? maxValue : value;
    return value;
}

internal s64 s64_clamp(s64 value, s64 minValue, s64 maxValue)
{
    value = (value < minValue) ? minValue : value;
    value = (value > maxValue) ? maxValue : value;
    return value;
}

internal f32 f32_clamp(f32 value, f32 min, f32 max)
{
    value = value < min ? min : value;
    value = value > max ? max : value;
    return value;
}

internal f32 pow_f32(f32 value, f32 pow)
{
    return std::pow(value, pow);
}

/// ******************************************************************
/// MATHFUN, fun math stuff :)
f32 f32_mathfun_smooth (f32 v)
{
    v = v * v * (v * -2 + 3);
    return v;
}

f32 mathfun_smoother_f32(f32 v)
{
    v = v * v * v * (v * (v * 6 - 15) + 10);
    return v;
}

f32 mathfun_pingpong_f32(f32 value, f32 length)
{
    f32 length2 = 2 * length;
    f32 result  = length - abs_f32((value - floor_f32(value / length2) * length2) - length);
    return result;
}

/// ******************************************************************
/// TRIGONOMETRY

internal constexpr f32 π = 3.141592653589793f;

// TODO(Leo): Look for constexpr implementations...
// https://github.com/Morwenn/static_math

inline f32 sine (f32 value)
{
    f32 result = sinf(value);
    return result;
}

inline f32 f32_cos(f32 value)
{
    f32 result = cosf(value);
    return result;
}

inline f32 Tan(f32 value)
{
    f32 result = tanf(value);
    return result;
}

inline f32 arc_sine(f32 value)
{
    f32 result = asinf(value);
    return result;
}

inline f32 arc_cosine(f32 value)
{
    f32 result = acosf(value);
    return result;
}

// Todo(Leo): Study this.
// https://www.quora.com/Why-do-we-not-use-cross-product-instead-of-dot-product-for-finding-out-the-angle-between-the-two-vectors
inline f32 arctan2(f32 y, f32 x)
{
    f32 result = atan2f(y, x);
    return result;
}

constexpr internal f32 to_radians(f32 degrees)
{
    constexpr f32 conversion = π / 180.0f;
    return conversion * degrees;
}

constexpr internal f32 to_degrees(f32 radians)
{
    constexpr f32 conversion = 180.0f / π;
    return conversion * radians;
}