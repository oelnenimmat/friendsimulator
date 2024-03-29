/*
Leo Tamminen

Serialization meta function helper things

Inspired by:
https://eliasdaler.github.io/meta-stuff/
*/

template<typename TStruct, typename TProp>
struct SerializedMemberProperty
{
	using property_type = TProp;

	char const * 		name;
	TProp TStruct::* 	address;
};

template<typename TStruct, typename TProp>
constexpr auto serialize_property(char const * name, TProp TStruct::* address)
{
	SerializedMemberProperty<TStruct, TProp> prop = {name, address};
	return prop;
}

template<typename T>
struct SerializedObject
{
	char const * 	name;
	T & 			data;
};

template<typename T>
constexpr auto serialize_object(char const * name, T & data)
{
	SerializedObject<T> object = {name, data};
	return object;
}

////////////////////////////////////////////////////////////

template <typename ... TProps>
struct SerializedPropertyList {};

template <typename TProp>
struct SerializedPropertyList<TProp>
{
	TProp property;	
};

template<typename TProp, typename ... TOthers>
struct SerializedPropertyList<TProp, TOthers...>
{
	TProp 								property;
	SerializedPropertyList<TOthers...> 	others;
};

template<typename ... TProps>
constexpr auto make_property_list(TProps ... args)
{
	return SerializedPropertyList<TProps...>{args...};
}

//////////////////////////////////////////////////////////

template<typename TFunc, typename ... TProps>
void for_each_property(TFunc func, SerializedPropertyList<TProps...> list)
{
	func(list.property);
	if constexpr(sizeof...(TProps) > 1)
	{
		for_each_property(func, list.others);
	}
};

template<typename T>
void serialize_properties(T const & data, String & serializedString, s32 capacity)
{
	for_each_property
	(
		[&](auto property)
		{
			string_append_format(serializedString, capacity, "\t", property.name, " = ", data.*property.address, "\n");
		},
		T::serializedProperties
	);
}

template <typename T>
internal void deserialize_properties(T & data, String serializedString)
{
	while(serializedString.length > 0)
	{
		String line = string_extract_line(serializedString);
		String id 	= string_extract_until_character(line, '=');

		for_each_property
		(
			[&](auto property)
			{
				if (string_equals(id, property.name))
				{	
					string_parse(line, &(data.*property.address));
				}
			},
			T::serializedProperties
		);
	}
}

// Note(Leo): this is more complicated way to do this, but also more strict. Right now we dont need it, but 
// I left it here for later reference
// template<typename TStruct, typename ... TProps>
// auto test_prop_list(SerializedMemberProperty<TStruct,TProps> ... arg)
// {
// 	return PropertyList2<TStruct, TProps...>{arg...};
// };


// template <typename TStruct, typename ... T>
// struct PropertyList2 {};

// template <typename TStruct, typename TProp>
// struct PropertyList2<TStruct, TProp>
// {
// 	SerializedMemberProperty<TStruct, TProp> a;	
// };

// template<typename TStruct, typename TFirst, typename ... TOthers>
// struct PropertyList2<TStruct, TFirst, TOthers...>
// {
// 	SerializedMemberProperty<TStruct, TFirst> a;
// 	PropertyList2<TStruct, TOthers...> b;
// };
