/*=============================================================================
Leo Tamminen

:MAZEGAME:'s vector structures and functions.

TODO(Leo): I would like to get some unary operator type functions (like normalize())
as member functions, but I have not yet figured good way to do that

TODO(Leo):
	- Full testing
	- SIMD / SSE / AVX
=============================================================================*/


// ------------------ DEFINITIONS ---------------------------------
#define VECTOR_TEMPLATE 	template<typename Scalar, int Dimension>
#define VECTOR_TYPE			VectorBase<Scalar, Dimension>

#define VECTOR_LOOP_ELEMENTS for (int i = 0; i < Dimension; ++i)

#define VECTOR_SUBSCRIPT_OPERATORS 											\
	scalar_type & operator [](int index) { return components[index]; }			\
	scalar_type operator [] (int index) const { return components[index]; }


VECTOR_TEMPLATE
union VectorBase
{
	using scalar_type = Scalar;
	static constexpr int dimension = Dimension;

	scalar_type components [dimension];

	static_assert(dimension > 1, "No vectors of 1 dimension allowed");

	VECTOR_SUBSCRIPT_OPERATORS;
};

template<typename Scalar>
union VectorBase<Scalar, 2>
{
	using scalar_type = Scalar;
	static constexpr int dimension = 2;

	struct { scalar_type x, y; };
	struct { scalar_type u, v; };

	scalar_type components [dimension];

	VECTOR_SUBSCRIPT_OPERATORS;
};

using Point2 = VectorBase<int32, 2>;
using Vector2 = VectorBase<real32, 2>;

template<typename Scalar>
union VectorBase<Scalar, 3>
{
	using scalar_type = Scalar;
	static constexpr int dimension = 3;

	struct { scalar_type x, y, z; };
	struct { scalar_type r, g, b; };

	scalar_type components [dimension];

	VECTOR_SUBSCRIPT_OPERATORS;
};

#define VECTOR_3_TEMPLATE 	template<typename Scalar>
#define VECTOR_3_TYPE		VectorBase<Scalar, 3>

using Vector3 = VectorBase<real32, 3>;

template<typename Scalar>
union VectorBase<Scalar, 4>
{
	using scalar_type = Scalar;
	static constexpr int dimension = 4;

	struct { scalar_type x, y, z, w; };
	struct { scalar_type r, g, b, a; };

	scalar_type components [dimension];

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
	VECTOR_LOOP_ELEMENTS { result.components[i] = a.components[i] + b.components[i]; }
 	return result;
}

VECTOR_TEMPLATE
auto & operator += (VECTOR_TYPE & a, VECTOR_TYPE b)
{
	VECTOR_LOOP_ELEMENTS { a.components[i] += b.components[i]; }
 	return a;
}

// ---------- SUBTRACTION ------------------------------
VECTOR_TEMPLATE VECTOR_TYPE &
operator -= (VECTOR_TYPE & lhs, VECTOR_TYPE rhs)
{
	VECTOR_LOOP_ELEMENTS { lhs.components[i] -= rhs.components[i]; }
	return lhs;
}

VECTOR_TEMPLATE auto
operator - (VECTOR_TYPE lhs, VECTOR_TYPE rhs)
{
	VECTOR_TYPE result;
	VECTOR_LOOP_ELEMENTS { result.components[i] = lhs.components[i] - rhs.components[i]; }
	return result;
}

namespace vector
{
	VECTOR_TEMPLATE VECTOR_TYPE
	coeff_add(VECTOR_TYPE vec, Scalar value)
	{
		VECTOR_LOOP_ELEMENTS { vec[i] += value;	}
		return vec;
	}

	VECTOR_TEMPLATE VECTOR_TYPE
	coeff_subtract(VECTOR_TYPE vec, Scalar value)
	{
		VECTOR_LOOP_ELEMENTS { vec[i] -= value; }
		return vec;
	}
}

// ************* MULTIPLICATION **************************

VECTOR_TEMPLATE auto
operator * (VECTOR_TYPE vec, Scalar num)
{
	VECTOR_TYPE result;
	VECTOR_LOOP_ELEMENTS { result.components[i] = vec.components[i] * num; }
	return result;
}

VECTOR_TEMPLATE VECTOR_TYPE &
operator *= (VECTOR_TYPE & vec, Scalar num)
{
	VECTOR_LOOP_ELEMENTS { vec.components[i] *= num; }
	return vec;
}

VECTOR_TEMPLATE VECTOR_3_TYPE
operator * (Scalar scalar, VECTOR_TYPE vec)
{
	VECTOR_LOOP_ELEMENTS
	{
		vec[i] *= scalar;
	}
	return vec;
}

// ////////////// DIVISION ///////////////////////////
VECTOR_TEMPLATE auto
operator / (VECTOR_TYPE vec, Scalar num)
{
	VECTOR_TYPE result;
	VECTOR_LOOP_ELEMENTS { result.components[i] = vec.components[i] / num; }
	return result;
}

VECTOR_TEMPLATE VECTOR_TYPE
operator /= (VECTOR_TYPE & vec, Scalar num)
{
	VECTOR_LOOP_ELEMENTS { vec.components[i] /= num; }
	return vec;
}

VECTOR_TEMPLATE VECTOR_TYPE
operator / (VECTOR_TYPE a, VECTOR_TYPE b)
{
	VECTOR_TYPE result;
	VECTOR_LOOP_ELEMENTS { result[i] = a[i] / b[i]; }
	return result; 
}

// %%%%%%%%%%%%%% MODULUS %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
VECTOR_TEMPLATE VECTOR_TYPE
operator % (VECTOR_TYPE vec, Scalar num)
{
	VECTOR_TYPE result;
	VECTOR_LOOP_ELEMENTS { result.components[i] = vec.components[i] % num; }
	return result;
}

// ------------ OTHER OPERATORS ----------------------------------
VECTOR_TEMPLATE auto
operator == (VECTOR_TYPE a, VECTOR_TYPE b)
{
	// Todo(Can we do something smart, like bitwise AND both items)
	bool32 result = true;
	VECTOR_LOOP_ELEMENTS { result = result && (a.components[i] == b.components[i]);	}
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
	VECTOR_LOOP_ELEMENTS { vec.components [i] = -vec.components[i]; }
	return vec;
}



// ---------- CASTS -----------------------------

/*
Cast a vector type to an other vector type with different value type.
*/
template<typename NewVectorType, typename OldValueType, int Dimension>
auto type_cast(VectorBase<OldValueType, Dimension> vec)
{
	using new_value_type = typename NewVectorType::scalar_type;
	using result_type = VectorBase<new_value_type, Dimension>;

	static_assert(NewVectorType::dimension == Dimension, "Must type_cast to a vector of equal dimension");

	result_type result;
	VECTOR_LOOP_ELEMENTS
	{
		result.components[i] = static_cast<new_value_type>(vec.components[i]);
	}
	return result;
}

template<typename NewVectorType, typename Scalar, int Dimension>
auto size_cast(VECTOR_TYPE vec)
{
	constexpr int new_dimension = NewVectorType::dimension;
	using result_type = VectorBase<Scalar, new_dimension>;

	static_assert(std::is_same_v<typename NewVectorType::scalar_type, Scalar>, "Must size_cast to a vector of same value type");

	result_type result;

	// Note(Leo): Copy components at up to old dimension and set rest to 0
	for (int i = 0; i < Dimension && i < new_dimension; ++i)
		result.components[i] = vec.components[i];
	
	for (int i = Dimension; i < new_dimension; ++i)
		result.components[i] = 0;

	return result;
}

// ------- VECTOR MATH COMBOS -------------------------
VECTOR_TEMPLATE VECTOR_TYPE
Abs(VECTOR_TYPE vec)
{
	VECTOR_LOOP_ELEMENTS
	{
		vec.components[i] = Abs(vec.components[i]);
	}
	return vec;
}

VECTOR_TEMPLATE Scalar 
Min(VECTOR_TYPE vec)
{
	Scalar result = vec.components[0];
	for (int i = 1; i < Dimension; ++i)
	{
		result = Min(result, vec.components[i]);
	}
	return result;
}

VECTOR_TEMPLATE Scalar
Max(VECTOR_TYPE vec)
{
	Scalar result = vec.components[0];
	for (int i = 1; i < Dimension; ++i)
	{
		result = Max(result, vec.components[i]);
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

	using integer_type = typename IntegerVectorType::scalar_type;
	using result_type = VectorBase<integer_type, Dimension>;

	result_type result;
	VECTOR_LOOP_ELEMENTS { result.components [i] = FloorTo<integer_type>(vec.components[i]); }
	return result;
}

template<typename IntegerVectorType, typename FloatType, int Dimension>
auto CeilTo(VectorBase<FloatType, Dimension> vec)
{
	// Todo(Leo): Assert that IntegerVectorType is of vector base
	static_assert(
		IntegerVectorType::dimension == Dimension,
		"IntegerVectorType must be vector type with same dimension");

	using integer_type = typename IntegerVectorType::scalar_type;
	using result_type = VectorBase<integer_type, Dimension>;

	result_type result;
	VECTOR_LOOP_ELEMENTS { result.components [i] = CeilTo<integer_type>(vec.components[i]); }
	return result;	
}

VECTOR_TEMPLATE VECTOR_TYPE
CoeffClamp(VECTOR_TYPE vec, Scalar coeffMin, Scalar coeffMax)
{
	VECTOR_LOOP_ELEMENTS { vec.components[i] = Clamp(vec.components[i], coeffMin, coeffMax);  }
	return vec;
}

// ----------------- COMPARISON HELPERS ---------------------
namespace vector
{
	VECTOR_TEMPLATE bool32
	coeff_less_than(VECTOR_TYPE vec, Scalar value)
	{
		bool32 result = true;
		VECTOR_LOOP_ELEMENTS { result = result && vec.components[i] < value; }
		return result;
	}
	
	VECTOR_TEMPLATE bool32
	coeff_less_than_or_equal(VECTOR_TYPE vec, Scalar value)
	{
		bool32 result = true;
		VECTOR_LOOP_ELEMENTS { result = result && vec.components[i] <= value; }
		return result;
	}
	
	VECTOR_TEMPLATE bool32
	coeff_greater_than(VECTOR_TYPE vec, Scalar value)
	{
		bool32 result = true;
		VECTOR_LOOP_ELEMENTS { result = result && vec.components[i] > value; }
		return result;
	}

	VECTOR_TEMPLATE bool32
	coeff_greater_than_or_equal(VECTOR_TYPE vec, Scalar value)
	{
		bool32 result = true;
		VECTOR_LOOP_ELEMENTS { result = result && vec.components[i] >= value; }
		return result;
	}
}



VECTOR_TEMPLATE bool32
CoeffLessThan(VECTOR_TYPE rhs, VECTOR_TYPE lhs)
{
	bool32 result = true;
	VECTOR_LOOP_ELEMENTS { result = result && rhs.components[i] < lhs.components[i]; }
	return result;
}

VECTOR_TEMPLATE bool32
CoeffLessThanOrEqual(VECTOR_TYPE rhs, VECTOR_TYPE lhs)
{
	bool32 result = true;
	VECTOR_LOOP_ELEMENTS { result = result && rhs.components[i] <= lhs.components[i]; }
	return result;
}

VECTOR_TEMPLATE bool32
CoeffGreaterThan(VECTOR_TYPE rhs, VECTOR_TYPE lhs)
{
	bool32 result = true;
	VECTOR_LOOP_ELEMENTS { result = result && rhs.components[i] > lhs.components[i]; }
	return result;
}

VECTOR_TEMPLATE bool32
CoeffGreaterThanOrEqual(VECTOR_TYPE rhs, VECTOR_TYPE lhs)
{
	bool32 result = true;
	VECTOR_LOOP_ELEMENTS { result = result && rhs.components[i] >= lhs.components[i]; }
	return result;
}

namespace vector
{
// --------------- PRODUCTS ----------------------
	VECTOR_TEMPLATE Scalar
	dot(VECTOR_TYPE a, VECTOR_TYPE b)
	{
		Scalar result = 0;
		VECTOR_LOOP_ELEMENTS { result += a[i] * b[i]; }
		return result;
	}

	VECTOR_3_TEMPLATE VECTOR_3_TYPE
	cross(VECTOR_3_TYPE lhs, VECTOR_3_TYPE rhs)
	{
		lhs = {
			lhs.y * rhs.z - lhs.z * rhs.y,
			lhs.z * rhs.x - lhs.x * rhs.z,
			lhs.x * rhs.y - lhs.y * rhs.x
		};
		return lhs;
	}

// --------------- OTHER HANDY STUFF --------------------
	VECTOR_TEMPLATE Scalar
	get_length (VECTOR_TYPE vec)
	{
		Scalar result = 0;
		VECTOR_LOOP_ELEMENTS
		{ 
			result += vec.components[i] * vec.components[i];
		}
		result = Root2(result);
		return result;
	}

	VECTOR_TEMPLATE VECTOR_TYPE
	normalize(VECTOR_TYPE vec)
	{
		VECTOR_TYPE result = vec / get_length(vec);
		return result;
	}

	VECTOR_TEMPLATE void
	dissect(VECTOR_TYPE vec, VECTOR_TYPE * outDirection, Scalar * outLength)
	{
		*outLength = get_length(vec);

		if (Abs(*outLength) == 0.0f)
		{
			*outLength = 0;
			*outDirection = {0, 0, 0};
		}
		else
		{
			*outDirection = vec / (*outLength);
		}
	}

	VECTOR_TEMPLATE VECTOR_TYPE
	project(VECTOR_TYPE vec, VECTOR_TYPE projectionTarget)
	{
		Scalar c = dot(projectionTarget, vec) / dot(vec, vec);
		return projectionTarget * c;
	}
	
	VECTOR_TEMPLATE Scalar
	get_distance(VECTOR_TYPE a, VECTOR_TYPE b)
	{
		Scalar result = get_length(a - b);
		return result;
	}
	
	VECTOR_TEMPLATE Scalar
	get_sqr_length(VECTOR_TYPE vec)
	{
		Scalar result = 0;
		VECTOR_LOOP_ELEMENTS { result += vec.components[i] * vec.components[i]; }
		return result;
	}

	VECTOR_TEMPLATE Scalar
	get_sqr_distance(VECTOR_TYPE a, VECTOR_TYPE b)
	{
		Scalar distance = 0;
		VECTOR_LOOP_ELEMENTS
		{
			Scalar s = a[i] - b[i];
			distance += s * s;
		}
		return distance;
	}

	VECTOR_TEMPLATE VECTOR_TYPE
	clamp_length(VECTOR_TYPE vec, Scalar maxLength)
	{
		Scalar length = get_length(vec);
		if (length > maxLength)
		{
			vec *= (maxLength / length);
		}
		return vec;
	}
	
	VECTOR_3_TEMPLATE Scalar
	get_signed_angle(VECTOR_3_TYPE from, VECTOR_3_TYPE to, VECTOR_3_TYPE axis)
	{
		// Todo(Leo): Check if this relly is correct
		Scalar a = dot(cross(from, to), axis);
		Scalar b = dot(to, from);

		Scalar result = ArcTan2(a, b);
		return result;
	}
	
	VECTOR_TEMPLATE VECTOR_TYPE
	interpolate(VECTOR_TYPE a, VECTOR_TYPE b, Scalar t)
	{
		VECTOR_LOOP_ELEMENTS { a.components[i] = ::interpolate(a.components[i], b.components[i], t); }
		return a;
	}

	VECTOR_TEMPLATE VECTOR_TYPE
	scale(VECTOR_TYPE vec, VECTOR_TYPE scale)
	{
		VECTOR_LOOP_ELEMENTS { vec[i] *= scale[i]; }
		return vec;
	}

	VECTOR_3_TEMPLATE VECTOR_3_TYPE
	rotate(VECTOR_3_TYPE vec, VECTOR_3_TYPE axis, Scalar angleInRadians)
	{
		// https://math.stackexchange.com/questions/511370/how-to-rotate-one-vector-about-another
		VECTOR_3_TYPE projection = project(vec, axis);
		VECTOR_3_TYPE rejection = vec - projection;

		VECTOR_3_TYPE w = cross(axis, rejection);
		
		Scalar rejectionLength = get_length(rejection);
		Scalar x1 = Cosine(angleInRadians) / rejectionLength;
		Scalar x2 = Sine(angleInRadians) / get_length(w);

		VECTOR_3_TYPE rotatedRejection = rejectionLength * (x1 * rejection + x2 * w);

		vec = projection + rotatedRejection;
		return vec;
	}
}




#if MAZEGAME_INCLUDE_STD_IOSTREAM

namespace std
{	
	VECTOR_TEMPLATE ostream &
	operator << (ostream & os, VECTOR_TYPE vec)
	{
		os << "(" << vec[0];
		for (int i = 1; i < Dimension; ++i)
		{
			os << "," << vec[i];
		}
		os << ")";
		return os;
	}
}

#endif


/* Note(Leo): Undefine macros here, since they definetly are not intended
to be used around. */
#undef VECTOR_TEMPLATE
#undef VECTOR_TYPE

#undef VECTOR_3_TEMPLATE
#undef VECTOR_3_TYPE

#undef VECTOR_LOOP_ELEMENTS