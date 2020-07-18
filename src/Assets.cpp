/*=============================================================================
Leo Tamminen

Asset things for :MAZEGAME:
=============================================================================*/

enum HandleType
{
	INVALID,

	MESH,
	TEXTURE,
	MATERIAL,
	MODEL,
	GUITEXTURE,
};

template<HandleType T>
struct BaseHandle
{
	s64 index_ =  -1;
	operator s64() const { return index_; }	
	static constexpr HandleType type = T;

	inline static const BaseHandle Null = {};
};

using MeshHandle  		= BaseHandle<MESH>;
using TextureHandle  	= BaseHandle<TEXTURE>;
using MaterialHandle  	= BaseHandle<MATERIAL>;
using ModelHandle  		= BaseHandle<MODEL>;
using GuiTextureHandle 	= BaseHandle<GUITEXTURE>;

template<HandleType T>
bool32 is_valid_handle(BaseHandle<T> handle)
{
	return (BaseHandle<T>::Null.index_ != handle.index_);
}

template <HandleType T>
bool32 operator==(BaseHandle<T> a, BaseHandle<T> b)
{
	return a.index_ == b.index_;
}

struct Vertex
{
	v3 position;
	v3 normal;
	v3 color;
	v2 texCoord;

	u32 boneIndices[4];
	f32 boneWeights[4];

	v3 tangent;
	v3 biTangent;
};

enum struct IndexType : u32 { UInt16, UInt32 };

struct MeshAsset
{
	Array<Vertex> vertices;
	Array<u16> indices;

	// TODO(Leo): would this be a good way to deal with different index types?
	// union
	// {
	// 	Array<u16> indices16;
	// 	Array<u32> indices32;
	// };

	IndexType indexType = IndexType::UInt16;
};

internal MeshAsset
make_mesh_asset(Array<Vertex> vertices, Array<u16> indices)
{
	MeshAsset result =
	{
		.vertices 	= std::move(vertices),
		.indices 	= std::move(indices),
		.indexType 	= IndexType::UInt16,
	};
	return result;
}

using Pixel = u32;

struct TextureAsset
{
	Array<Pixel> pixels;

	s32 	width;
	s32 	height;
	s32 	channels;
};

inline float
get_red(Pixel color)
{
	u32 value = (color & 0x00ff0000) >> 16;
	return (float)value / 255.0f; 
}

inline float
get_green(Pixel color)
{
	u32 value = (color & 0x0000ff00) >> 8;
	return (float)value / 255.0f; 
}

inline Pixel 
get_pixel(TextureAsset * texture, u32 x, u32 y)
{
	AssertMsg(x < texture->width && y < texture->height, "Invalid pixel coordinates!");

	s64 index = x + y * texture->width;
	return texture->pixels[index];
}

inline u8
value_to_byte(float value)
{
	return (u8)(value * 255);
}

internal u32 color_rgba_32(v4 color)
{
	u32 pixel = (u32)value_to_byte(color.a) << 24
				| (u32)value_to_byte(color.b) << 16
				| (u32)value_to_byte(color.g) << 8
				| (u32)value_to_byte(color.r);
	return pixel;
}

internal constexpr v4 colour_v4_from_rgb_255(u8 r, u8 g, u8 b)
{
	v4 colour = {r / 255.0f, g / 255.0f, b / 255.0f, 1.0f};
	return colour;
}

internal constexpr v4 colour_rgb_alpha(v3 rgb, f32 alpha)
{
	v4 colour = {rgb.x, rgb.y, rgb.z, alpha};
	return colour;
}

internal u32 colour_rgb_alpha_32(v3 colour, f32 alpha)
{
	return color_rgba_32(colour_rgb_alpha(colour, alpha));
}

internal v4 colour_multiply(v4 a, v4 b)
{
	a = {	a.r * b.r,
			a.g * b.g,
			a.b * b.b,
			a.a * b.a};
	return a;
}

internal Pixel
get_closest_pixel(TextureAsset * texture, v2 texcoord)
{
	u32 u = (u32)round_f32(texture->width * texcoord.x) % texture->width;
	u32 v = (u32)round_f32(texture->height * texcoord.y) % texture->height;

	s64 index = u + v * texture->width;
	return texture->pixels[index];
}

struct MaterialAsset
{
    GraphicsPipeline 	pipeline;
    Array<TextureHandle> textures;
};