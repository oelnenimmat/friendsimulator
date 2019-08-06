// TODO(Leo): heavy-weight testing

// ------------------ DEFINITIONS ---------------------------------
#define VECTOR_TEMPLATE 	template<typename ValueType, int Dimension>
#define VECTOR_TYPE			VectorBase<ValueType, Dimension>

#define VECTOR_LOOP_ELEMENTS for (int i = 0; i < Dimension; ++i)

#define VECTOR_SUBSCRIPT_OPERATORS \
	value_type & operator [](int index) { return values[index]; }\
	value_type operator [] (int index) const { return values[index]; }


VECTOR_TEMPLATE
union VectorBase
{
	ValueType values [Dimension];

	using value_type = ValueType;
	static constexpr int dimension = Dimension;

	VECTOR_SUBSCRIPT_OPERATORS;
};

template<typename ValueType>
union VectorBase<ValueType, 2>
{
	struct { ValueType x, y; };
	struct { ValueType u, v; };
	ValueType values[2];

	using value_type = ValueType;
	static constexpr int dimension = 2;

	VECTOR_SUBSCRIPT_OPERATORS;
};

using Point2 = VectorBase<int32, 2>;
using Vector2 = VectorBase<real32, 2>;

template<typename ValueType>
union VectorBase<ValueType, 3>
{
	struct { ValueType x, y, z; };
	struct { ValueType r, g, b; };
	ValueType values[3];

	using value_type = ValueType;
	static constexpr int dimension = 3;

	VECTOR_SUBSCRIPT_OPERATORS;
};

using Vector3 = VectorBase<real32, 3>;

template<typename ValueType>
union VectorBase<ValueType, 4>
{
	struct { ValueType x, y, z, w; };
	struct { ValueType r, g, b, a; };
	ValueType values[4];

	using value_type = ValueType;
	static constexpr int dimension = 4;

	VECTOR_SUBSCRIPT_OPERATORS;
};

using Vector4 = VectorBase<real32, 4>;

#undef VECTOR_SUBSCRIPT_OPERATORS


// Todo(Leo): rest of operators
// +++++++++ ADDITION +++++++++++++

VECTOR_TEMPLATE auto
operator + (VECTOR_TYPE a, VECTOR_TYPE b)
{
	VECTOR_TYPE result;
	VECTOR_LOOP_ELEMENTS { result.values[i] = a.values[i] + b.values[i]; }
 	return result;
}

VECTOR_TEMPLATE
auto & operator += (VECTOR_TYPE & a, VECTOR_TYPE b)
{
	VECTOR_LOOP_ELEMENTS { a.values[i] += b.values[i]; }
 	return a;
}

VECTOR_TEMPLATE VECTOR_TYPE
operator + (VECTOR_TYPE vec, ValueType num)
{
	VECTOR_TYPE result;
	VECTOR_LOOP_ELEMENTS { result.values[i] = vec.values[i] + num; }
	return result;
}

// ---------- SUBTRACTION ------------------------------
VECTOR_TEMPLATE VECTOR_TYPE &
operator -= (VECTOR_TYPE & lhs, VECTOR_TYPE rhs)
{
	VECTOR_LOOP_ELEMENTS { lhs.values[i] -= rhs.values[i]; }
	return lhs;
}

VECTOR_TEMPLATE auto
operator - (VECTOR_TYPE lhs, VECTOR_TYPE rhs)
{
	VECTOR_TYPE result;
	VECTOR_LOOP_ELEMENTS { result.values[i] = lhs.values[i] - rhs.values[i]; }
	return result;
}

VECTOR_TEMPLATE VECTOR_TYPE
operator - (VECTOR_TYPE vec, ValueType num)
{
	VECTOR_TYPE result;
	VECTOR_LOOP_ELEMENTS { result.values[i] = vec.values[i] - num; }
	return result;
}


// ************* MULTIPLICATION **************************

VECTOR_TEMPLATE auto
operator * (VECTOR_TYPE vec, ValueType num)
{
	VECTOR_TYPE result;
	VECTOR_LOOP_ELEMENTS { result.values[i] = vec.values[i] * num; }
	return result;
}

VECTOR_TEMPLATE VECTOR_TYPE &
operator *= (VECTOR_TYPE & vec, ValueType num)
{
	VECTOR_LOOP_ELEMENTS { vec.values[i] *= num; }
	return vec;
}

VECTOR_TEMPLATE VECTOR_TYPE &
operator *= (VECTOR_TYPE & a, VECTOR_TYPE b)
{
	VECTOR_LOOP_ELEMENTS { a.values[i] *= b.values[i]; }
	return a;
}

// ////////////// DIVISION ///////////////////////////
VECTOR_TEMPLATE auto
operator / (VECTOR_TYPE vec, ValueType num)
{
	VECTOR_TYPE result;
	VECTOR_LOOP_ELEMENTS { result.values[i] = vec.values[i] / num; }
	return result;
}

VECTOR_TEMPLATE VECTOR_TYPE
operator /= (VECTOR_TYPE & vec, ValueType num)
{
	VECTOR_LOOP_ELEMENTS { vec.values[i] /= num; }
	return vec;
}

// %%%%%%%%%%%%%% MODULUS %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
VECTOR_TEMPLATE VECTOR_TYPE
operator % (VECTOR_TYPE vec, ValueType num)
{
	VECTOR_TYPE result;
	VECTOR_LOOP_ELEMENTS { result.values[i] = vec.values[i] % num; }
	return result;
}

// ------------ OTHER OPERATORS ----------------------------------
VECTOR_TEMPLATE auto
operator == (VECTOR_TYPE a, VECTOR_TYPE b)
{
	// Todo(Can we do something smart, like bitwise AND both items)
	bool result = true;
	VECTOR_LOOP_ELEMENTS { result = result && (a.values[i] == b.values[i]);	}
	return result;
}

VECTOR_TEMPLATE bool
operator != (VECTOR_TYPE a, VECTOR_TYPE b)
{
	return !(a == b);
}

VECTOR_TEMPLATE VECTOR_TYPE
operator - (VECTOR_TYPE vec)
{
	VECTOR_LOOP_ELEMENTS { vec.values [i] = -vec.values[i]; }
	return vec;
}



// ---------- CASTS -----------------------------

/*
Cast a vector type to an other vector type with different value type.
*/
template<typename NewVectorType, typename OldValueType, int Dimension>
auto type_cast(VectorBase<OldValueType, Dimension> vec)
{
	using new_value_type = typename NewVectorType::value_type;
	using result_type = VectorBase<new_value_type, Dimension>;

	static_assert(NewVectorType::dimension == Dimension, "Must type_cast to a vector of equal dimension");

	result_type result;
	VECTOR_LOOP_ELEMENTS
	{
		result.values[i] = static_cast<new_value_type>(vec.values[i]);
	}
	return result;
}

template<typename NewVectorType, typename ValueType, int Dimension>
auto size_cast(VECTOR_TYPE vec)
{
	constexpr int new_dimension = NewVectorType::dimension;
	using result_type = VectorBase<ValueType, new_dimension>;

	static_assert(std::is_same_v<NewVectorType::value_type, ValueType>, "Must size_cast to a vector of same value type");

	result_type result;

	// Note(Leo): Copy values at up to old dimension and set rest to 0
	for (int i = 0; i < Dimension && i < new_dimension; ++i)
		result.values[i] = vec.values[i];
	
	for (int i = Dimension; i < new_dimension; ++i)
		result.values[i] = 0;

	return result;
}

// ------- VECTOR MATH COMBOS -------------------------
VECTOR_TEMPLATE VECTOR_TYPE
Abs(VECTOR_TYPE vec)
{
	VECTOR_LOOP_ELEMENTS
	{
		vec.values[i] = Abs(vec.values[i]);
	}
	return vec;
}

VECTOR_TEMPLATE ValueType 
Min(VECTOR_TYPE vec)
{
	ValueType result = vec.values[0];
	for (int i = 1; i < Dimension; ++i)
	{
		result = Min(result, vec.values[i]);
	}
	return result;
}

VECTOR_TEMPLATE ValueType
Max(VECTOR_TYPE vec)
{
	ValueType result = vec.values[0];
	for (int i = 1; i < Dimension; ++i)
	{
		result = Max(result, vec.values[i]);
	}
	return result;
}

template<typename IntegerVectorType, typename FloatType, int Dimension>
auto FloorTo(VectorBase<FloatType, Dimension> vec)
{
	// TODO(Leo): assert that IntegerVectorType is of vector base
	static_assert(
		IntegerVectorType::dimension == Dimension,
		"IntegerVectorType must be vector type with same dimension");

	using integer_type = typename IntegerVectorType::value_type;
	using result_type = VectorBase<integer_type, Dimension>;

	result_type result;
	VECTOR_LOOP_ELEMENTS { result.values [i] = FloorTo<integer_type>(vec.values[i]); }
	return result;
}

template<typename IntegerVectorType, typename FloatType, int Dimension>
auto CeilTo(VectorBase<FloatType, Dimension> vec)
{
	// Todo(Leo): Assert that IntegerVectorType is of vector base
	static_assert(
		IntegerVectorType::dimension == Dimension,
		"IntegerVectorType must be vector type with same dimension");

	using integer_type = typename IntegerVectorType::value_type;
	using result_type = VectorBase<integer_type, Dimension>;

	result_type result;
	VECTOR_LOOP_ELEMENTS { result.values [i] = CeilTo<integer_type>(vec.values[i]); }
	return result;	
}

VECTOR_TEMPLATE VECTOR_TYPE
CoeffClamp(VECTOR_TYPE vec, ValueType coeffMin, ValueType coeffMax)
{
	VECTOR_LOOP_ELEMENTS { vec.values[i] = Clamp(vec.values[i], coeffMin, coeffMax);  }
	return vec;
}

// ----------------- COMPARISON HELPERS ---------------------
VECTOR_TEMPLATE bool
CoeffLessThan(VECTOR_TYPE vec, ValueType num)
{
	bool result = true;
	VECTOR_LOOP_ELEMENTS { result = result && vec.values[i] < num; }
	return result;
}

VECTOR_TEMPLATE bool
CoeffLessThanOrEqual(VECTOR_TYPE vec, ValueType num)
{
	bool result = true;
	VECTOR_LOOP_ELEMENTS { result = result && vec.values[i] <= num; }
	return result;
}

VECTOR_TEMPLATE bool
CoeffGreaterThan(VECTOR_TYPE vec, ValueType num)
{
	bool result = true;
	VECTOR_LOOP_ELEMENTS { result = result && vec.values[i] > num; }
	return result;
}

VECTOR_TEMPLATE bool
CoeffGreaterThanOrEqual(VECTOR_TYPE vec, ValueType num)
{
	bool result = true;
	VECTOR_LOOP_ELEMENTS { result = result && vec.values[i] >= num; }
	return result;
}

VECTOR_TEMPLATE bool
CoeffLessThan(VECTOR_TYPE rhs, VECTOR_TYPE lhs)
{
	bool result = true;
	VECTOR_LOOP_ELEMENTS { result = result && rhs.values[i] < lhs.values[i]; }
	return result;
}

VECTOR_TEMPLATE bool
CoeffLessThanOrEqual(VECTOR_TYPE rhs, VECTOR_TYPE lhs)
{
	bool result = true;
	VECTOR_LOOP_ELEMENTS { result = result && rhs.values[i] <= lhs.values[i]; }
	return result;
}

VECTOR_TEMPLATE bool
CoeffGreaterThan(VECTOR_TYPE rhs, VECTOR_TYPE lhs)
{
	bool result = true;
	VECTOR_LOOP_ELEMENTS { result = result && rhs.values[i] > lhs.values[i]; }
	return result;
}

VECTOR_TEMPLATE bool
CoeffGreaterThanOrEqual(VECTOR_TYPE rhs, VECTOR_TYPE lhs)
{
	bool result = true;
	VECTOR_LOOP_ELEMENTS { result = result && rhs.values[i] >= lhs.values[i]; }
	return result;
}

// --------------- PRODUCTS ----------------------
VECTOR_TEMPLATE ValueType
Dot(VECTOR_TYPE a, VECTOR_TYPE b)
{
	ValueType result;
	VECTOR_LOOP_ELEMENTS { result += a[i] * b[i]; }
	return result;
}

VECTOR_TEMPLATE VECTOR_TYPE
Cross(VECTOR_TYPE lhs, VECTOR_TYPE rhs)
{
	static_assert (Dimension == 3, "Cross product is only defined for 3 dimensional vectors");

	VECTOR_TYPE result = {
		lhs.y * rhs.z - lhs.z * rhs.y,
		lhs.z * rhs.x - lhs.x * rhs.z,
		lhs.x * rhs.y - lhs.y * rhs.x
	};

	return result;
}

// --------------- OTHER HANDY STUFF --------------------
VECTOR_TEMPLATE ValueType
Length (VECTOR_TYPE vec)
{
	ValueType result = {};
	VECTOR_LOOP_ELEMENTS { result += vec.values[i] * vec.values[i]; }
	result = Root2(result);
	return result;
}

VECTOR_TEMPLATE ValueType
Distance(VECTOR_TYPE a, VECTOR_TYPE b)
{
	ValueType result = Length(a - b);
	return result;
}

VECTOR_TEMPLATE VECTOR_TYPE
Normalize(VECTOR_TYPE vec)
{
	VECTOR_TYPE result = vec / Length(vec);
	return result;
}

#undef VECTOR_TEMPLATE
#undef VECTOR_TYPE
#undef VECTOR_LOOP_ELEMENTS
