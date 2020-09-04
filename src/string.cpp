/*
Leo Tamminen

String, tah-daa
*/

struct String
{
	s32 	length;
	char * 	memory;

	char & operator [] (s32 index) { return memory[index]; }
	char operator [] (s32 index) const { return memory[index]; }

	char * begin() { return memory; }
	char * end() { return memory + length; }

	char const * begin() const { return memory; }
	char const * end() const { return memory + length; }
};

LogInput & operator << (LogInput & log, String const & string)
{
	for (s32 i = 0; i < string.length; ++i)
	{
		log << string.memory[i];
	}

	return log;
}

internal void reset_string(String & string, s32 capacity)
{
	fill_memory(string.memory, 0, capacity);
	string.length = 0;
}

internal bool is_whitespace_character(char character)
{
	bool result = character == ' ' || character == '\t';// || character == '\r' || character == '\n';
	return result;
}

internal bool is_newline_character(char character)
{
	bool result = character == '\r' || character == '\n';
	return result;
}

internal bool is_number_character(char character)
{
	bool result = character >= '0' && character <= '9';
	return result;
}

inline internal char string_first_character(String string)
{
	return string.memory[0];
}

inline internal char string_last_character(String string)
{
	return string.memory[string.length - 1];
}

internal void string_eat_leading_spaces_and_lines(String & string)
{
	while(string.length > 0 && (is_whitespace_character(string[0]) || is_newline_character(string[0])))
	{
		string.memory += 1;
		string.length -= 1;
	}
}

internal void string_eat_leading_spaces(String & string)
{
	while(string.length > 0 && is_whitespace_character(string_first_character(string)))
	{
		string.memory += 1;
		string.length -= 1;
	}
}

internal void string_eat_following_spaces(String & string)
{
	while(string.length > 0 && is_whitespace_character(string_last_character(string)))
	{
		string.length -= 1;
	}
}

internal void string_eat_spaces(String & string)
{
	string_eat_leading_spaces(string);
	string_eat_following_spaces(string);
}

internal char string_eat_first_character(String & string)
{
	char first = string.memory[0];

	string.memory += 1;
	string.length -= 1;

	return first;
}

String string_extract_line(String & source)
{
	s32 length 		= 0;
	s32 skipLength 	= 0;
	while(true)
	{
		if (length == source.length)
		{
			break;
		}

		if (source.memory[length] == '\r' && source.memory[length + 1] == '\n')
		{
			skipLength = 2;
			break;
		}

		if (source.memory[length] == '\n')
		{
			skipLength = 1;
			break;
		}

		length += 1;
	}

	String result = 
	{
		.length = length,
		.memory = source.memory
	};
	string_eat_spaces(result);

	#ifndef _WIN32
	static_assert(false, "Not sure if newline is still the same on this os");
	#endif

	// Note(Leo) add two for \r and \n
	source.memory += length + skipLength;
	source.length -= length + skipLength;

	return result;
}

String string_extract_until_character(String & source, char delimiter)
{
	s32 length = 0;
	while(true)
	{
		if (length == source.length)
		{
			break;
		}

		if (source.memory[length] == delimiter)
		{
			break;
		}

		length += 1;
	}

	String result =
	{
		.length = length,
		.memory = source.memory,
	};
	string_eat_spaces(result);

	// Note(Leo): Add one for delimiting character
	source.memory += length + 1;
	source.length -= length + 1;

	string_eat_leading_spaces(source);

	return result;
}

// Note(Leo): string is passed by value since it is small, and can therefore be modified in function
internal bool32 string_equals(String string, char const * cstring)
{
	while(true)
	{
		if (string.length == 0 && *cstring == 0)
		{
			return true;
		}

		if (string.memory[0] != *cstring)
		{
			return false;
		}

		if (string.length == 0 && *cstring != 0)
		{
			return false;
		}

		if (string.length > 0 && *cstring == 0)
		{
			return false;
		}

		string.memory += 1;
		string.length -= 1;

		cstring += 1;
	}
}

internal f32 string_parse_f32(String string)
{
	// Note(Leo): We get 'string' as a copy, since it is just int and a pointer, so we can modify it
	// Todo(Leo): prevent modifying contents though
	f32 value = 0;
	f32 sign = 0;

	if (string.memory[0] == '-')
	{
		sign = -1;
		string_eat_first_character(string);
	}
	else
	{
		sign = 1;
	}

	while(string.length > 0 && string.memory[0] != '.')
	{
		char current = string_eat_first_character(string);
		Assert(is_number_character(current));

		f32 currentValue 	= current - '0';
		value 				*= 10;
		value 				+= currentValue;

	}

	if (string.length == 0)
	{
		return value;
	}

	// Note(Leo): this is decimal point
	string_eat_first_character(string);

	f32 power = 0.1;

	while(string.length > 0)
	{
		char current = string_eat_first_character(string);
		Assert(is_number_character(current));

		f32 currentValue 	= current - '0';
		value 				+= power * currentValue;
		power 				*= 0.1;
	}

	return sign * value;
}

internal v3 string_parse_v3(String string)
{
	v3 result;
	String part;

	part 		= string_extract_until_character(string, ',');
	result.x 	= string_parse_f32(part);

	part 		= string_extract_until_character(string, ',');
	result.y 	= string_parse_f32(part);

	result.z 	= string_parse_f32(string);

	return result;
}

template<typename T>
internal void string_parse(String string, T * value)
{
	static_assert(false, "string_parse is not implemented for T");
}

#define STRING_GENERIC_PARSE(type) template<> void string_parse<type>(String string, type * value){*value = string_parse_##type(string);}

STRING_GENERIC_PARSE(f32);
STRING_GENERIC_PARSE(v3);

#undef STRING_GENERIC_PARSE

// template<> void string_parse<f32>(String string, f32 * value)
// {
// 	*value = string_parse_f32(string);
// }

// template<> void string_parse<v3>(String string, v3 * value)
// {
// 	*value = string_parse_v3(string);
// }


internal void string_append(String & a, String b, s32 capacity)
{
	s32 bIndex = 0;
	while(a.length < capacity && bIndex < b.length)
	{
		a[a.length++] = b[bIndex++];
	}
}

internal void string_append_cstring(String & string, char const * cstring, s32 capacity)
{
	while(string.length < capacity && *cstring != 0)
	{
		string.memory[string.length] = *cstring;

		string.length 	+= 1;
		cstring 		+= 1;
	}
}

internal void string_append_f32(String & string, f32 value, s32 capacity)
{	
	// Todo(Leo): check that range is valid

	constexpr s32 digitCapacity = 8;
	char digits[digitCapacity]	= {};
	fill_memory(digits, '0', digitCapacity);

	// Note(Leo): added one is for possible negative sign
	Assert((capacity - string.length) > (digitCapacity + 1))
	
	f32 sign 					= value < 0 ? -1 : 1;
	value 						= abs_f32(value);

	constexpr s32 maxExponent 	= 6;
	
	constexpr f32 powers [2 * maxExponent + 1] =
	{
		0.000001,
		0.00001,
		0.0001,
		0.001,
		0.01,
		0.1,
		1,
		10,
		100,
		1000,
		10000,
		100000,
		1000000,
	};

	auto get_digit = [&powers, value](s32 exponent) -> char
	{
		f32 power 	= powers[exponent + maxExponent];
	
		// Note(Leo): Modulo limits upper bound, max limits lower bound;
		s32 digit 	= ((s32)(value / power)) % 10;
		digit 		= max_s32(0, digit);
	
		return '0' + (char)digit;
	};

	s32 exponent 	= maxExponent;

	// Note(Leo): first loop to find first number, so we don't write leading zeros wastefully
	while(digits[0] == '0' && exponent >= 0)
	{
		digits[0] = get_digit(exponent);
		exponent--;
	}

	s32 digitCount = 1;

	// Note(Leo): second loop to find rest
	while((digitCount < digitCapacity) && (exponent >= -maxExponent))
	{
		if (exponent == -1)
		{
			digits[digitCount] = '.';
			digitCount++;
		}
		
		digits[digitCount] = get_digit(exponent);

		digitCount++;
		exponent--;
	}

	if (sign < 0)
	{
		string.memory[string.length++] = '-';
	}

	for(s32 i = 0; i < digitCount; ++i)
	{
		string.memory[string.length++] = digits[i];
	}
}

internal void string_append_s32(String & string, s32 value, s32 capacity)
{	
	// Todo(Leo): check that range is valid

	// Note(Leo): these are connected
	constexpr s32 digitCapacity 	= 8;
	constexpr s32 startMultiplier 	= 10'000'000;

	// Note(Leo): added one is for possible negative sign
	char digits[digitCapacity + 1] = {};
	fill_memory(digits, '0', array_count(digits));
	s32 digitCount = 0;

	Assert((capacity - string.length) > array_count(digits));

	if (value == 0)
	{
		*string.end() = '0';
		string.length += 1;
	}
	else
	{
		if (value < 0)
		{
			value *= -1;
			digits[digitCount++] = '-';
		}

		s32 multiplier 				= startMultiplier;
		bool32 firstNonZeroFound 	= false;

		for (s32 i = 0; i < digitCapacity; ++i)
		{
			s32 valueAtMultiplier = (value / multiplier) % 10;
			multiplier /= 10;

			firstNonZeroFound = firstNonZeroFound || (valueAtMultiplier > 0);

			if (firstNonZeroFound)
			{
				digits[digitCount++] = '0' + valueAtMultiplier;
			}
		}

		string_append(string, {digitCount, digits}, capacity);
	}
}

internal void string_append_v3(String & string, v3 value, s32 capacity)
{
	string_append_f32(string, value.x, capacity);	
	string_append_cstring(string, ", ", capacity);	
	string_append_f32(string, value.y, capacity);	
	string_append_cstring(string, ", ", capacity);	
	string_append_f32(string, value.z, capacity);	
}

template<typename T>
internal void string_append_format(String & string, s32 capacity, T thing)
{
	if constexpr (std::is_same_v<T, f32>)
	{
		string_append_f32(string, thing, capacity);
	}
	
	if constexpr (std::is_same_v<T, char const *>)
	{
		string_append_cstring(string, thing, capacity);
	}	

	if constexpr (std::is_same_v<T, v3>)
	{
		string_append_v3(string, thing, capacity);
	}

	if constexpr (std::is_same_v<T, String>)
	{
		string_append(string, thing, capacity);
	}
}

#define STRING_GENERIC_APPEND(type) template<> void string_append_format<type>(String & string, s32 capacity, type thing) { string_append_##type(string, thing, capacity); }

STRING_GENERIC_APPEND(s32)

#undef STRING_GENERIC_APPEND

template<typename TFirst, typename ... TOthers>
internal void string_append_format(String & string, s32 capacity, TFirst first, TOthers ... others)
{
	string_append_format(string, capacity, first);
	string_append_format(string, capacity, others...);
}
