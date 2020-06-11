/*
Leo Tamminen

Monuments
*/

struct Monuments
{
	s32 			baseCount;
	m44 *			baseTransforms;
	MeshHandle 		baseMesh;
	MaterialHandle 	baseMaterial;

	s32 			archCount;
	m44 * 			archTransforms;
	MeshHandle 		archMesh;
	MaterialHandle 	archMaterial;

	s32 				ornamentTypeCount;
	s32 * 				ornamentCounts;
	m44 ** 				ornamentTransforms;
	MeshHandle * 		ornamentMeshes;
	MaterialHandle * 	ornamentMaterials;
};

internal void scene3d_draw_monuments(Monuments const & monuments)
{
	platformApi->draw_meshes(platformGraphics, monuments.baseCount, monuments.baseTransforms, monuments.baseMesh, monuments.baseMaterial);
	platformApi->draw_meshes(platformGraphics, monuments.archCount, monuments.archTransforms, monuments.archMesh, monuments.archMaterial);

	for (s32 i = 0; i < monuments.ornamentTypeCount; ++i)
	{
		platformApi->draw_meshes(platformGraphics, 	monuments.ornamentCounts[i],
													monuments.ornamentTransforms[i],
													monuments.ornamentMeshes[i],
													monuments.ornamentMaterials[i]);
	}
}

internal Monuments scene3d_load_monuments(MemoryArena & persistentMemory, MaterialHandle environmentMaterial, CollisionSystem3D & collisionSystem)
{
	push_memory_checkpoint(*global_transientMemory);

	constexpr s32 ornamentTypeCount = 3;

	f32 monumentRange 					= 600;
	f32 halfMonumentRange				= monumentRange / 2;
	s32 monumentSlotCountPerDirection 	= 10;
	s32 maxMonumentCapacity = monumentSlotCountPerDirection * monumentSlotCountPerDirection;


	// These are temp
	s32 counts[ornamentTypeCount] 	= {};
	s32 offsets[ornamentTypeCount] 	= {};

	m44 * tempTransforms 	= push_memory<m44>(persistentMemory, maxMonumentCapacity, 0);
	m44 * inverseTransforms 		= push_memory<m44>(*global_transientMemory, maxMonumentCapacity, 0);

	s32 totalMonumentCount 	= 0;

	for (s32 y = 0; y < monumentSlotCountPerDirection; ++y)
	{
		if ((y % 4) > 0)
			continue;

		for (s32 x = 0; x < monumentSlotCountPerDirection; ++x)
		{
			if ((x % 7) < 2)
				continue;

			v3 position;
			position.x = (f32)x / monumentSlotCountPerDirection * monumentRange - halfMonumentRange;
			position.y = (f32)y / monumentSlotCountPerDirection * monumentRange - halfMonumentRange;
			position.z = get_terrain_height(&collisionSystem, position.xy);

			quaternion rotation = axis_angle_quaternion(up_v3, RandomRange(-pi / 8, pi / 8));

			m44 transform 			= translation_matrix(position) * rotation_matrix(rotation);
			m44 inverseTransform 	= inverse_transform_matrix(position, rotation, {1,1,1}); 

			s32 typeIndex = RandomRange(0, ornamentTypeCount);
			
			if (typeIndex == 0)
			{	
				tempTransforms[totalMonumentCount] = tempTransforms[offsets[2]];
				tempTransforms[offsets[2]] = tempTransforms[offsets[1]];
				tempTransforms[offsets[1]] = transform;

				inverseTransforms[totalMonumentCount] 	= inverseTransforms[offsets[2]];
				inverseTransforms[offsets[2]] 		= inverseTransforms[offsets[1]];
				inverseTransforms[offsets[1]] 		= inverseTransform;
				
				offsets[1] += 1;
				offsets[2] += 1;

				counts[0] += 1;

			}
			else if (typeIndex == 1)
			{
				tempTransforms[totalMonumentCount] = tempTransforms[offsets[2]];
				tempTransforms[offsets[2]] = transform;

				inverseTransforms[totalMonumentCount] = inverseTransforms[offsets[2]];
				inverseTransforms[offsets[2]] = inverseTransform;

				offsets[2] += 1;

				counts[1] += 1;


			}
			else if (typeIndex == 2)
			{
				tempTransforms[totalMonumentCount] = transform;
				
				inverseTransforms[totalMonumentCount] = inverseTransform;

				counts[2] += 1;
			}

			++totalMonumentCount;
		}
	}

	auto gltfFile = read_gltf_file(*global_transientMemory, "assets/models/monuments.glb");

	auto archesMeshAsset 	= load_mesh_glb(*global_transientMemory, gltfFile, "monument_arches");
	auto baseMeshAsset 		= load_mesh_glb(*global_transientMemory, gltfFile, "monument_base");

	MeshAsset ornamentMeshAssets[ornamentTypeCount] = 
	{
		load_mesh_glb(*global_transientMemory, gltfFile, "monument_ornament_01"),
		load_mesh_glb(*global_transientMemory, gltfFile, "monument_ornament_02"),
		load_mesh_glb(*global_transientMemory, gltfFile, "monument_ornament_03"),
	};

	m44 * transforms = push_memory<m44>(persistentMemory, totalMonumentCount, ALLOC_NO_CLEAR);
	copy_memory(transforms, tempTransforms, sizeof(m44) * totalMonumentCount);

	Monuments monuments = {};

	monuments.baseCount 		= totalMonumentCount;
	monuments.baseTransforms 	= transforms;
	monuments.baseMesh 			= platformApi->push_mesh(platformGraphics, &baseMeshAsset);
	monuments.baseMaterial 		= environmentMaterial;

	monuments.archCount 		= totalMonumentCount;
	monuments.archTransforms 	= transforms;
	monuments.archMesh 			= platformApi->push_mesh(platformGraphics, &archesMeshAsset);
	monuments.archMaterial 		= environmentMaterial;

	monuments.ornamentTypeCount 	= ornamentTypeCount;
	monuments.ornamentCounts 		= push_memory<s32>(persistentMemory, monuments.ornamentTypeCount, 0);
	monuments.ornamentTransforms 	= push_memory<m44*>(persistentMemory, monuments.ornamentTypeCount, 0);
	monuments.ornamentMeshes 		= push_memory<MeshHandle>(persistentMemory, monuments.ornamentTypeCount, 0);
	monuments.ornamentMaterials 	= push_memory<MaterialHandle>(persistentMemory, monuments.ornamentTypeCount, 0);

	for (s32 i = 0; i < monuments.ornamentTypeCount; ++i)
	{
		monuments.ornamentCounts[i] 	= counts[i];
		monuments.ornamentTransforms[i] = transforms + offsets[i];
		monuments.ornamentMeshes[i] 	= platformApi->push_mesh(platformGraphics, &ornamentMeshAssets[i]);
		monuments.ornamentMaterials[i] 	= environmentMaterial;
	}

	Array<BoxCollider> archColliders = {};
	load_all_transforms_glb(*global_transientMemory, gltfFile, "monument_arches", &archColliders);

	Array<BoxCollider> baseColliders = {};
	load_all_transforms_glb(*global_transientMemory, gltfFile, "monument_base", &baseColliders);

	for (s32 i = 0; i < monuments.baseCount; ++i)
	{
		for (auto collider : archColliders)
		{
			m44 colliderTransform 			= transform_matrix(collider.center, collider.orientation, collider.extents);
			m44 colliderInverseTransform 	= inverse_transform_matrix(collider.center, collider.orientation, collider.extents);

			collisionSystem.staticBoxColliders.push({	transforms[i] * colliderTransform,
														colliderInverseTransform * inverseTransforms[i] });
		}

		for (auto collider : baseColliders)
		{
			m44 colliderTransform 			= transform_matrix(collider.center, collider.orientation, collider.extents);
			m44 colliderInverseTransform 	= inverse_transform_matrix(collider.center, collider.orientation, collider.extents);

			collisionSystem.staticBoxColliders.push({	transforms[i] * colliderTransform,
														colliderInverseTransform * inverseTransforms[i] });
		}
	}

	pop_memory_checkpoint(*global_transientMemory);

	return monuments;
}