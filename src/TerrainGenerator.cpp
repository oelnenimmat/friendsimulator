/*=============================================================================
Leo Tamminen
shophorn @ internet

Terrain generator prototype
=============================================================================*/

struct HeightMap
{
	// gridSize rows and gridSize colums
	f32 * 	values;
	u32 	gridSize;
	f32		worldSize;
	f32 	minHeight;
	f32		maxHeight;
};

internal f32
get_height_at(HeightMap const * map, v2 worldScalePosition)
{
	v2 gridSpacePosition = worldScalePosition * (map->gridSize / map->worldSize);

	s32 x0 		= (s32)floor_f32(gridSpacePosition.x);
	s32 x1 		= x0 + 1;
	f32 xFraction = gridSpacePosition.x - x0;

	s32 y0 		= (s32)floor_f32(gridSpacePosition.y);
	s32 y1 		= y0 + 1;
	f32 yFraction = gridSpacePosition.y - y0;

	if ((x0 < 0) || (x1 >= map->gridSize) || (y0 < 0) || (y1 >= map->gridSize))
	{
		return 0;
	}

	auto get_value = [map](s32 x, s32 y) -> f32
	{
		u64 valueIndex = x + y * map->gridSize;
		return map->values[valueIndex];
	};

	f32 value00 	= get_value(x0, y0);
	f32 value10 	= get_value(x1, y0);
	f32 value01 	= get_value(x0, y1);
	f32 value11 	= get_value(x1, y1);

	f32 value0 	= f32_lerp(value00, value10, xFraction);
	f32 value1 	= f32_lerp(value01, value11, xFraction);

	f32 value 	= f32_lerp(value0, value1, yFraction);

	return f32_lerp(map->minHeight, map->maxHeight, value);
}

internal HeightMap
make_heightmap(MemoryArena * memory, TextureAssetData * texture, u32 gridSize, f32 worldSize, f32 minHeight, f32 maxHeight)
{
	u64 pixelCount 		= gridSize * gridSize;
	auto heightValues 	= push_memory<f32>(*memory, pixelCount, ALLOC_GARBAGE);

	f32 textureScale 	= 1.0f / gridSize;

	auto get_value = [texture](u32 u, u32 v) -> f32
	{
		s64 index 		= u + v * texture->width;
		u32 * u32memory = reinterpret_cast<u32*>(texture->pixelMemory);
		u32 value 		= u32memory[index];

		// Todo(Leo): seems like not red, but whatever
		f32 red 		= ((0x00ff0000 & value) >> 16) / 255.0f;
		return red;
	};

	for (f32 y = 0; y < gridSize; ++y)
	{
		for (f32 x = 0; x < gridSize; ++x)
		{
			v2 pixelCoord 	= {	x * textureScale * (texture->width - 1),
									y * textureScale * (texture->height - 1)};

			u32 u0 		= (u32)floor_f32(pixelCoord.x);
			u32 u1 		= u0 + 1;
			f32 uFraction = pixelCoord.x - u0;
			
			u32 v0 		= (u32)floor_f32(pixelCoord.y);
			u32 v1 		= v0 + 1;
			f32 vFraction	= pixelCoord.y - v0;

			f32 height00  = get_value(u0, v0);
			f32 height10  = get_value(u1, v0);
			f32 height01  = get_value(u0, v1);
			f32 height11  = get_value(u1, v1);

			f32 height0 	= f32_lerp(height00, height10, uFraction);
			f32 height1 	= f32_lerp(height01, height11, uFraction);

			f32 height 	= f32_lerp (height0, height1, vFraction);

			u64 mapIndex 		= x + gridSize * y;
			heightValues[mapIndex] 	= height;
		}
	}

	return {
		.values 	= heightValues,
		.gridSize 	= gridSize,
		.worldSize 	= worldSize,

		.minHeight = minHeight,
		.maxHeight = maxHeight,
	};
}


internal void mesh_generate_tangents(s32 vertexCount, Vertex * vertices, s32 indexCount, u16 * indices)
{
	s64 triangleCount = indexCount / 3;

	v3 * vertexTangents = push_memory<v3>(*global_transientMemory, vertexCount, ALLOC_ZERO_MEMORY);
	
	for(s64 i = 0; i < triangleCount; ++i)
	{
		u32 index0 = indices[i * 3];
		u32 index1 = indices[i * 3 + 1];
		u32 index2 = indices[i * 3 + 2];

		v3 p0 = vertices[index0].position;
		v2 uv0 = vertices[index0].texCoord;

		v3 p1 = vertices[index1].position;
		v2 uv1 = vertices[index1].texCoord;
		
		v3 p2 = vertices[index2].position;
		v2 uv2 = vertices[index2].texCoord;

		v3 p01 = p1 - p0;
		v3 p02 = p2 - p0;

		v2 uv01 = uv1 - uv0;
		v2 uv02 = uv2 - uv0;

		// Note(Leo): Why is this called r? ThinMatrix has link to explanation in his normal map video
		// Note(Leo): Also this looks suspiciously like cross_2d, and someone named that "det", determinant?
		f32 r = 1.0f / (uv01.x * uv02.y - uv01.y * uv02.x);

		v3 tangent = (p01 * uv02.y - p02 * uv01.y) * r;
		tangent = v3_normalize(tangent);

		vertexTangents[index0] += tangent;
		vertexTangents[index1] += tangent;
		vertexTangents[index2] += tangent;
	}

	for (u32 i = 0; i < vertexCount; ++i)
	{
		vertices[i].tangent = v3_normalize(vertexTangents[i]);
	}
}

internal void mesh_generate_tangents(MeshAssetData & mesh)
{
	mesh_generate_tangents(mesh.vertexCount, mesh.vertices, mesh.indexCount, mesh.indices);
}

internal void mesh_generate_normals(s32 vertexCount, Vertex * vertices, s32 indexCount, u16 * indices)
{
	v3 * normals = push_memory<v3>(*global_transientMemory, vertexCount, ALLOC_ZERO_MEMORY);

	for (u32 i = 0; i < indexCount; i += 3)
	{
		u32 i0 = indices[i];
		u32 i1 = indices[i + 1];
		u32 i2 = indices[i + 2];

		v3 p0 = vertices[i0].position;
		v3 p1 = vertices[i1].position;
		v3 p2 = vertices[i2].position;

		v3 p01 = p1 - p0;
		v3 p02 = p2 - p0;

		v3 normal = v3_normalize(v3_cross(p01, p02));

		normals[i0] += normal;
		normals[i1] += normal;
		normals[i2] += normal;
	}

	for (s32 i = 0; i < vertexCount; ++i)
	{
		vertices[i].normal = v3_normalize(normals[i]);
	}	
}

internal void mesh_generate_normals (MeshAssetData & mesh)
{
	// Todo(Leo): why does this exist?? 
	Assert(mesh.indexCount <= max_value_u32);

	u32 vertexCount = mesh.vertexCount;
	u32 indexCount 	= mesh.indexCount;

	v3 * normals = push_memory<v3>(*global_transientMemory, vertexCount, ALLOC_ZERO_MEMORY);

	for (u32 i = 0; i < indexCount; i += 3)
	{
		u32 i0 = mesh.indices[i];
		u32 i1 = mesh.indices[i + 1];
		u32 i2 = mesh.indices[i + 2];

		v3 p0 = mesh.vertices[i0].position;
		v3 p1 = mesh.vertices[i1].position;
		v3 p2 = mesh.vertices[i2].position;

		v3 p01 = p1 - p0;
		v3 p02 = p2 - p0;

		v3 normal = v3_normalize(v3_cross(p01, p02));

		normals[i0] += normal;
		normals[i1] += normal;
		normals[i2] += normal;
	}

	for (s32 i = 0; i < vertexCount; ++i)
	{
		mesh.vertices[i].normal = v3_normalize(normals[i]);
	}
}

internal MeshAssetData generate_terrain(MemoryArena & 	allocator,
									HeightMap & 	heightMap,

									/// Note(Leo): relative to scale of map
									v2 	relativeStart,
									v2 	relativeSize,

									s32 meshResolution,
									f32 texCoordScale)
{
	v2 meshSize = relativeSize * heightMap.worldSize;
	v2 mapStart = relativeStart * heightMap.worldSize;

	s32 vertexCount 		= meshResolution * meshResolution;
	s32 triangleIndexCount 	= 6 * (meshResolution - 1) * (meshResolution - 1);

	// Note(Leo): Clear vertices so that attributes we do not set here are initialized to zero
	Vertex * vertices 	= push_memory<Vertex>(allocator, vertexCount, ALLOC_ZERO_MEMORY);
	u16 * indices 		= push_memory<u16> (allocator, triangleIndexCount, ALLOC_ZERO_MEMORY); 

	/// FILL VERTICES
	for (s32 y = 0; y < meshResolution; ++y)
	{
		for (s32 x = 0; x < meshResolution; ++x)
		{
			// Mesh local position in world scale
			v2 meshPosition = { (f32)x / (meshResolution - 1) * meshSize.x,
								(f32)y / (meshResolution - 1) * meshSize.y };

			v2 mapPosition = mapStart + meshPosition;

			f32 height = get_height_at(&heightMap, mapPosition);

			vertices[x + meshResolution * y].position = { 	meshPosition.x, 
															meshPosition.y,
															height };

			// Todo(Leo): this can be scaled, but mind seam/gap between other chunks
			vertices[x + meshResolution * y].texCoord = { 	(f32)x / meshResolution * texCoordScale,
															(f32)y / meshResolution * texCoordScale};
		}
	}

	// FILL INDICES
	for(s32 y = 0, i = 0; y < (meshResolution - 1); ++y)
	{
		for (s32 x = 0; x < (meshResolution - 1); ++x, i += 6)
		{
			u16 a = x + meshResolution * y;

			u16 b = a + 1;
			u16 c = a + meshResolution;
			u16 d = a + meshResolution + 1;

			indices [i] = a;
			indices [i + 1] = b;
			indices [i + 2] = c;
			
			indices [i + 3] = c;
			indices [i + 4] = b;
			indices [i + 5] = d;
		}
	}


	// Todo(Leo): Stop using array in mesh asset
	MeshAssetData result 	= {}; 
	
	result.vertexCount 	= vertexCount;
	result.vertices 	= vertices;

	result.indexCount 	= triangleIndexCount;
	result.indices 		= indices;

	mesh_generate_normals(result);
	mesh_generate_tangents(result);

	return result;
}