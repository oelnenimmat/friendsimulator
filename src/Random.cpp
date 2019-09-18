/*=============================================================================
Leo Tamminen

Pseudo random generator. This is rather important for the game since levels
are procedurally generated, so we will implement this ourselves.
=============================================================================*/

// Note(Leo): From https://codingforspeed.com/using-faster-psudo-random-generator-xorshift/
uint32 xor128()
{
  	static uint32 x = 123456789;
  	static uint32 y = 362436069;
  	static uint32 z = 521288629;
  	static uint32 w = 88675123;
  	uint32 t;
 	t = x ^ (x << 11);   
  	x = y; y = z; z = w;   
  	return w = w ^ (w >> 19) ^ (t ^ (t >> 8));
}

internal real32
RandomValue()
{
  	real32 value 	= static_cast<real32>(xor128());
  	real32 max 		= static_cast<real32>(MaxValue<uint32>);
  	real32 result 	= value / max;

  	return result;
}

internal real32
RandomRange(real32 min, real32 max)
{
    MAZEGAME_ASSERT (min <= max, "'min' must be smaller than 'max'");

    real32 value = static_cast<real32>(xor128()) / static_cast<real32>(MaxValue<uint32>);
    real32 range = max - min;
    real32 result = min + (value * range);

    return result;    
}