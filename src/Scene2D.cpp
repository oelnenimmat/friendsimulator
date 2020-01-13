/*=============================================================================
Leo Tamminen
shophorn @ internet

Scene description for 2d development scene
=============================================================================*/

namespace scene_2d
{
	struct Scene
	{
		ArenaArray<RenderSystemEntry> renderSystem;
		ArenaArray<Handle<Animator>> animatorSystem;

		// Todo(Leo): make this similar 'system' to others
		CollisionManager2D collisionManager;


		Camera worldCamera;
		// CameraController3rdPerson cameraController;
		CameraControllerSideScroller cameraController;
		CharacterControllerSideScroller characterController;

		// Todo(Leo): make animation state controller or similar for these
		AnimationClip 	laddersUpAnimation;
		AnimationClip 	laddersDownAnimation;

		// Todo(Leo): make controller for these
		CharacterControllerSideScroller::LadderTriggerFunc ladderTrigger1;
		CharacterControllerSideScroller::LadderTriggerFunc ladderTrigger2;
		bool32 ladderOn = false;
		bool32 ladder2On = false;
	};

	internal uint64
	get_alloc_size() { return sizeof(Scene); };

	internal void
	update(void * scenePtr, game::Input * input, game::RenderInfo * renderer, game::PlatformInfo * platform);

	internal void
	load(void * scenePtr, MemoryArena * persistentMemory, MemoryArena * transientMemory, game::PlatformInfo * platformInfo);
}

global_variable SceneInfo 
scene2dInfo = make_scene_info(	scene_2d::get_alloc_size,
								scene_2d::load,
								scene_2d::update);

internal void
scene_2d::update(void * scenePtr, game::Input * input, game::RenderInfo * renderer, game::PlatformInfo * platform)
{
	Scene * scene = reinterpret_cast<Scene*>(scenePtr);

	scene->collisionManager.do_collisions();

	/// Update Character
	scene->characterController.update(input, &scene->collisionManager);
	update_animator_system(input, scene->animatorSystem);

	scene->cameraController.update(input);
    update_camera_system(renderer, platform, input, &scene->worldCamera, platform->graphicsContext);
	update_render_system(platform->graphicsContext, renderer, scene->renderSystem);
}

internal void 
scene_2d::load(void * scenePtr, MemoryArena * persistentMemory, MemoryArena * transientMemory, game::PlatformInfo * platformInfo)
{
	Scene * scene = reinterpret_cast<Scene*>(scenePtr);

	// auto * persistentMemory = &state->persistentMemoryArena;
	// auto * transientMemory = &state->transientMemoryArena;

	allocate_for_handle<Transform3D>(persistentMemory, 100);
	allocate_for_handle<Collider2D>(persistentMemory, 100);
	allocate_for_handle<Renderer>(persistentMemory, 100);
	allocate_for_handle<Animator>(persistentMemory, 100);

	scene->renderSystem = reserve_array<RenderSystemEntry>(persistentMemory, 100);
	scene->animatorSystem = reserve_array<Handle<Animator>>(persistentMemory, 100);

	scene->collisionManager =
	{
		.colliders 	= reserve_array<Handle<Collider2D>>(persistentMemory, 200),
		.collisions = reserve_array<Collision2D>(persistentMemory, 300) // Todo(Leo): Not enough..
	};

	struct MaterialCollection {
		MaterialHandle character;
		MaterialHandle environment;
	} materials;

	// Create MateriaLs
	{
		PipelineHandle shader = platformInfo->push_pipeline(platformInfo->graphicsContext, "shaders/vert.spv", "shaders/frag.spv", {.textureCount = 3});

		TextureAsset whiteTextureAsset = make_texture_asset(push_array<uint32>(transientMemory, {0xffffffff}), 1, 1);
		TextureAsset blackTextureAsset = make_texture_asset(push_array<uint32>(transientMemory, {0xff000000}), 1, 1);

		TextureHandle whiteTexture = platformInfo->push_texture(platformInfo->graphicsContext, &whiteTextureAsset);
		TextureHandle blackTexture = platformInfo->push_texture(platformInfo->graphicsContext, &blackTextureAsset);

		auto load_and_push_texture = [transientMemory, platformInfo](const char * path) -> TextureHandle
		{
			auto asset = load_texture_asset(path, transientMemory);
			auto result = platformInfo->push_texture(platformInfo->graphicsContext, &asset);
			return result;
		};

		auto tilesTexture 	= load_and_push_texture("textures/tiles.jpg");
		auto lavaTexture 	= load_and_push_texture("textures/lava.jpg");
		auto faceTexture 	= load_and_push_texture("textures/texture.jpg");

		auto push_material = [shader, platformInfo, transientMemory](MaterialType type, TextureHandle a, TextureHandle b, TextureHandle c) -> MaterialHandle
		{
			MaterialAsset asset = make_material_asset(shader, push_array(transientMemory, {a, b, c}));
			MaterialHandle handle = platformInfo->push_material(platformInfo->graphicsContext, &asset);
			return handle;
		};

		materials =
		{
			.character 	= push_material(	MaterialType::Character,
											lavaTexture,
											faceTexture,
											blackTexture),

			.environment = push_material(	MaterialType::Character,
											tilesTexture,
											blackTexture,
											blackTexture)
		};
	}

    auto push_mesh = [platformInfo] (MeshAsset * asset) -> MeshHandle
    {
    	auto handle = platformInfo->push_mesh(platformInfo->graphicsContext, asset);
    	return handle;
    };

    auto push_model = [platformInfo] (MeshHandle mesh, MaterialHandle material) -> ModelHandle
    {
    	auto handle = platformInfo->push_model(platformInfo->graphicsContext, mesh, material);
    	return handle;
    };

	// Characters
	Handle<Transform3D> characterTransform = {};
	{
		auto characterMesh 			= load_model_obj(transientMemory, "models/character.obj");
		auto characterMeshHandle 	= push_mesh(&characterMesh);

		// Our dude
		auto transform = make_handle<Transform3D>({});
		auto renderer = make_handle<Renderer>({push_model(characterMeshHandle, materials.character)});

		characterTransform = transform;

		push_one(scene->renderSystem, {transform, renderer});

		scene->characterController 	= 
		{
			.transform 	= transform,
			.collider 	= push_collider(&scene->collisionManager, transform, {0.4f, 0.8f}, {0.0f, 0.5f}),
		};


		scene->characterController.OnTriggerLadder1 = &scene->ladderTrigger1;
		scene->characterController.OnTriggerLadder2 = &scene->ladderTrigger2;

		// Other dude
		transform 	= make_handle<Transform3D>({2, 0.5f, 12.25f});
		renderer 	= make_handle<Renderer>({push_model(characterMeshHandle, materials.character)});
		push_one(scene->renderSystem, {transform, renderer});
	}

    scene->worldCamera =
    {
    	.forward 		= World::Forward,
    	.fieldOfView 	= 60,
    	.nearClipPlane 	= 0.1f,
    	.farClipPlane 	= 1000.0f,
    	.aspectRatio 	= (float)platformInfo->windowWidth / (float)platformInfo->windowHeight	
    };

    scene->cameraController =
    {
    	.camera 		= &scene->worldCamera,
    	.target 		= characterTransform
    };

	// Environment
	{
		constexpr float depth = 100;
		constexpr float width = 100;
		constexpr float ladderHeight = 1.0f;

		constexpr bool32 addPillars 	= true;
		constexpr bool32 addLadders 	= true;
		constexpr bool32 addButtons 	= true;
		constexpr bool32 addPlatforms 	= true;

		{
			auto groundQuad 	= mesh_primitives::create_quad(transientMemory, false);
			auto meshTransform	= Matrix44::Translate({-width / 2, -depth /2, 0}) * Matrix44::Scale({width, depth, 0});
			mesh_ops::transform(&groundQuad, meshTransform);
			mesh_ops::transform_tex_coords(&groundQuad, {0,0}, {width / 2, depth / 2});

			auto groundQuadHandle 	= push_mesh(&groundQuad);
			auto renderer 			= make_handle<Renderer>({push_model(groundQuadHandle, materials.environment)});
			auto transform 			= make_handle<Transform3D>({});

			push_one(scene->renderSystem, {transform, renderer});
			push_collider(&scene->collisionManager, transform, {100, 1}, {0, -1.0f});
		}

		if (addPillars)
		{
			auto pillarMesh 		= load_model_glb(transientMemory, "models/big_pillar.glb", "big_pillar");
			auto pillarMeshHandle 	= push_mesh(&pillarMesh);

			auto renderer 	= make_handle<Renderer>({push_model(pillarMeshHandle, materials.environment)});
			auto transform 	= make_handle<Transform3D>({-width / 4, 0, 0});

			push_one(scene->renderSystem, {transform, renderer});
			push_collider(&scene->collisionManager, transform, {2, 25});

			renderer = make_handle<Renderer>({push_model(pillarMeshHandle, materials.environment)});
			transform = make_handle<Transform3D>({width / 4, 0, 0});

			push_one(scene->renderSystem, {transform, renderer});
			push_collider(&scene->collisionManager, transform, {2, 25});
		}

		if (addLadders)
		{
			auto ladderMesh 		= load_model_glb(transientMemory, "models/ladder.glb", "LadderSection");
			auto ladderMeshHandle 	= push_mesh(&ladderMesh);

			auto root1 	= make_handle<Transform3D>({0, 0.5f, -ladderHeight});
			auto root2 	= make_handle<Transform3D>({10, 0.5f, 6 - ladderHeight});
			auto bones1 = reserve_array<Handle<Transform3D>>(persistentMemory, 6);
			auto bones2 = reserve_array<Handle<Transform3D>>(persistentMemory, 6);

			int ladderRigBoneCount = 6;
			auto animations = reserve_array<Animation>(persistentMemory, ladderRigBoneCount);

			auto parent1 = root1;
			auto parent2 = root2;
			
			int ladder2StartIndex = 6;
			int ladderCount = 12;
			for (int ladderIndex = 0; ladderIndex < ladderCount; ++ladderIndex)
			{
				auto renderer 	= make_handle<Renderer>({push_model(ladderMeshHandle, materials.environment)});
				auto transform 	= make_handle<Transform3D>({});

				push_one(scene->renderSystem, {transform, renderer});
				push_collider(&scene->collisionManager, transform,
														{1.0f, 0.5f},
														{0, 0.5f},
														ColliderTag::Ladder);

				if (ladderIndex < ladder2StartIndex)
				{
					transform->parent = parent1;
					parent1 = transform;
					push_one(bones1, transform);
	

					// Todo(Leo): only one animation needed, move somewhere else				
					auto keyframes = push_array(persistentMemory, {
						Keyframe{(ladderIndex % ladder2StartIndex) * 0.12f, {0, 0, 0}},
						Keyframe{((ladderIndex % ladder2StartIndex) + 1) * 0.15f, {0, 0, ladderHeight}}
					});
					push_one(animations, {keyframes});
				}
				else
				{
					transform->parent = parent2;
					parent2 = transform;	
					push_one(bones2, transform);
				}
			}	

			scene->laddersUpAnimation 	= make_animation_clip(animations);
			scene->laddersDownAnimation = duplicate_animation_clip(persistentMemory, &scene->laddersUpAnimation);
			reverse_animation_clip(&scene->laddersDownAnimation);

			auto keyframeCounters1 = push_array<uint64>(persistentMemory, bones1.count());
			auto keyframeCounters2 = push_array<uint64>(persistentMemory, bones2.count());

			auto rig1 = make_animation_rig(root1, bones1, keyframeCounters1);
			auto rig2 = make_animation_rig(root2, bones2, keyframeCounters2);

			auto animator1 = make_animator(rig1);
			auto animator2 = make_animator(rig2);

			push_one(scene->animatorSystem, make_handle(animator1));
			push_one(scene->animatorSystem, make_handle(animator2));

			scene->ladderTrigger1 = [scene]() -> void
			{
				scene->ladderOn = !scene->ladderOn;
				if (scene->ladderOn)
					play_animation_clip(scene->animatorSystem[0], &scene->laddersUpAnimation);
				else
					play_animation_clip(scene->animatorSystem[0], &scene->laddersDownAnimation);
			};

			scene->ladderTrigger2 = [scene]() -> void
			{
				scene->ladder2On = !scene->ladder2On;
				if (scene->ladder2On)
					play_animation_clip(scene->animatorSystem[1], &scene->laddersUpAnimation);
				else
					play_animation_clip(scene->animatorSystem[1], &scene->laddersDownAnimation);
			};
		}

		if (addPlatforms)
		{
			Vector3 platformPositions [] =
			{
				{-6, 0, 6},
				{-4, 0, 6},
				{-2, 0, 6},
	
				{2, 0, 6},
				{4, 0, 6},
				{6, 0, 6},
				{8, 0, 6},
				{10, 0, 6},

				{2, 0, 12},
				{4, 0, 12},
				{6, 0, 12},
				{8, 0, 12},
			};

			auto platformMeshAsset 	= load_model_obj(transientMemory, "models/platform.obj");
			auto platformMeshHandle = push_mesh(&platformMeshAsset);

			int platformCount = 12;
			for (int platformIndex = 0; platformIndex < platformCount; ++platformIndex)
			{
				auto renderer 	= make_handle<Renderer>({push_model(platformMeshHandle, materials.environment)});
				auto transform 	= make_handle<Transform3D>({platformPositions[platformIndex]});

				push_one(scene->renderSystem, {transform, renderer});
				push_collider(&scene->collisionManager, transform, {1.0f, 0.25f});
			}
		}

		if(addButtons)
		{
			auto keyholeMeshAsset 	= load_model_obj(transientMemory, "models/keyhole.obj");
			auto keyholeMeshHandle 	= push_mesh (&keyholeMeshAsset);

			auto renderer 	= make_handle<Renderer>({push_model(keyholeMeshHandle, materials.environment)});
			auto transform 	= make_handle<Transform3D>({Vector3{5, 0, 0}});

			push_one(scene->renderSystem, {transform, renderer});
			push_collider(&scene->collisionManager, transform, {0.3f, 0.6f}, {0, 0.3f}, ColliderTag::Trigger);

			renderer 	= make_handle<Renderer>({push_model(keyholeMeshHandle, materials.environment)});
			transform 	= make_handle<Transform3D>({Vector3{4, 0, 6}});

			push_one(scene->renderSystem, {transform, renderer});
			push_collider(&scene->collisionManager, transform, {0.3f, 0.6f}, {0, 0.3f}, ColliderTag::Trigger2);
		}
	}
}
