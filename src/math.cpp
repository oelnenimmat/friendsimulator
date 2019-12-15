// TODO(Leo): Proper unit testing!!
// Note(Leo): For now. Actually implement these ourself, for the kicks of course.
#include <cmath>

internal real32
Root2(real32 value)
{
    real32 result = sqrt(value);
    return result; 
}

internal int32
FloorToInt32(real32 value)
{
    int32 result = static_cast<int32>(value);
    if (value < 0)
        result -= 1;
    return result;
}

template<typename IntegerType, typename FloatType>
IntegerType
FloorTo(FloatType value)
{
    int64 floorValue = static_cast<int64>(value);
    if (value < 0)
        floorValue = floorValue -1;
    return static_cast<IntegerType>(floorValue);
}

template<typename IntegerType, typename FloatType>
IntegerType
CeilTo(FloatType value)
{
    int64 ceilValue = static_cast<int64>(value);
    if (value > 0)
        ceilValue = ceilValue + 1;
    return static_cast<IntegerType>(ceilValue);
}

internal int32
RoundToInt32(real32 value)
{
    if (value < 0.0f)
        value -= 0.5f;
    else
        value += 0.5f;
    return static_cast<int32>(value);
}

template<typename Number>
internal Number 
Min(Number a, Number b)
{
    return (a < b) ? a : b;
}

template<typename Number>
internal Number 
Max(Number a, Number b)
{
    return (a > b) ? a : b;
}

template<typename Number>
internal Number
Abs(Number num)
{
    if (num < 0)
        return -num;
    return num;
}

template<typename Number>
internal Number
Clamp(Number value, Number min, Number max)
{
    if (value < min)
        value = min;
    if (value > max)
        value = max;
    return value;
}

template<typename Float> inline Float
Modulo(Float dividend, Float divisor)
{
    Float result = fmod(dividend, divisor);
    return result;
}


// Todo(Leo): Add namespace
inline real32
Sine (real32 value)
{
    real32 result = sinf(value);
    return result;
}

inline real32
Cosine(real32 value)
{
    real32 result = cosf(value);
    return result;
}

inline real32
Tan(real32 value)
{
    real32 result = tanf(value);
    return result;
}

inline real32
ArcSine(real32 value)
{
    real32 result = asinf(value);
    return result;
}

inline real32
ArcCosine(real32 value)
{
    real32 result = acosf(value);
    return result;
}

inline real32
ArcTan2(real32 cosValue, real32 sinValue)
{
    real32 result = atan2f(cosValue, sinValue);
    return result;
}

template<typename Number>
inline Number
Sign(Number value)
{
    if (value < 0) return -1;
    if (value > 0) return 1;
    return 0;
}

internal constexpr real32 pi = 3.141592653589793f;

internal constexpr real32 degToRad = pi / 180.f;
internal constexpr real32 radToDeg = 180.0f / pi;

// Todo(Leo): remove or convert to functions
internal constexpr real32 DegToRad = pi / 180.f;
internal constexpr real32 RadToDeg = 180.0f / pi;

template<typename Number>
Number ToDegrees(Number radians)
{
    Number result = 180.0f * radians / pi;
    return result; 
}

template<typename Number>
Number ToRadians(Number degrees)
{
    Number result = pi * degrees / 180.0f;
    return result;
}

template<typename Number>
Number Interpolate(Number from, Number to, float time, int32 smooth = 0, bool32 clamp = true)
{
    switch (smooth)
    {
        case 1:
        {
            float t = time;
            time = t * t * (-2 * t + 3);
         } break;
        
        case 2:
        { 
            float t = time;
            time = t * t * t * (t * (6 * t - 15) + 10);
        }break;
    }

    if (clamp)
    {
        time = Clamp(time, 0.0f, 1.0f);
    }

    Number result = (1 - time) * from + time * to;
    return result;
}