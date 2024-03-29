struct DynamicMesh
{
	Array<Vertex> 	vertices;
	Array<u16> 	indices;
};

internal DynamicMesh push_dynamic_mesh(MemoryArena & allocator, u64 vertexCapacity, u64 indexCapacity)
{
	DynamicMesh mesh =
	{
		.vertices 	= push_array<Vertex>(allocator, vertexCapacity, ALLOC_ZERO_MEMORY),
		.indices 	= push_array<u16>(allocator, indexCapacity, ALLOC_ZERO_MEMORY)
	};
	return mesh;
}

internal void flush_dynamic_mesh(DynamicMesh & mesh)
{
	mesh.vertices.count = 0;
	mesh.indices.count 	= 0;
};
