/*
Leo Tamminen
shophorn @ internet

Scene description for 3d development game

Todo(Leo):
	crystal trees 

Todo(leo): audio
	clipping, compressor, dynamic gain, soft knee, hard knee
*/

// Todo(Leo): maybe Actor? as opposed to static Scenery
enum EntityType : s32
{ 	
	// Todo(Leo): none should be default value, but really it would be better to have it not
	// contribute to count, basically be below 0 or above count
	EntityType_none,
	
	EntityType_small_pot,
	EntityType_big_pot,
	EntityType_water,
	EntityType_raccoon,
	EntityType_tree_3,
	EntityType_box,

	EntityTypeCount
};

struct EntityReference
{
	EntityType 	type;
	s32 		index;
};

bool operator == (EntityReference const & a, EntityReference const & b)
{
	bool result = a.type == b.type && a.index == b.index;
	return result;
}

bool operator != (EntityReference const & a, EntityReference const & b)
{
	return !(a == b);
}

// Todo(Leo): Maybe try to get rid of this forward declaration
// Should these be global variables or something
struct Game;

internal s32 					game_spawn_tree(Game & game, v3 position, s32 treeTypeIndex, bool32 pushToPhysics = true);
internal CollisionSystem3D & 	game_get_collision_system(Game * game);

internal v3 * 			entity_get_position(Game * game, EntityReference entity);
internal quaternion * 	entity_get_rotation(Game * game, EntityReference entity);

#include "map.cpp"

#include "game_settings.cpp"

#include "CharacterMotor.cpp"
#include "PlayerController3rdPerson.cpp"
#include "FollowerController.cpp"

enum CameraMode : s32
{ 
	CameraMode_player, 
	CameraMode_editor,

	CameraModeCount
};

enum TrainState : s32
{
	TrainState_move,
	TrainState_wait,
};

enum NoblePersonMode : s32
{
	NoblePersonMode_wander_around,
	NoblePersonMode_wait_for_train,
	NoblePersonMode_away,
	NoblePersonMode_arriving_in_train,
};

enum RaccoonMode : s32
{
	RaccoonMode_idle,
	RaccoonMode_flee,
	RaccoonMode_carried,
};

#include "physics.cpp"

#include "metaballs.cpp"
#include "dynamic_mesh.cpp"
#include "sky_settings.cpp"

// Note(Leo): This maybe seems nice?
#include "game_assets.cpp"
#include "game_monuments.cpp"
#include "game_waters.cpp"
#include "game_clouds.cpp"
#include "game_leaves.cpp"
#include "game_trees.cpp"
#include "game_boxes.cpp"

// Todo(Leo): actually other way aroung, but now scene saving button is on blocks editor
#include "scene_data.cpp"
#include "game_building_blocks.cpp"


struct Scenery
{
	MeshAssetId 	mesh;
	MaterialAssetId material;
	s64 	count;
	m44 * 	transforms;
};

struct Game
{
	MemoryArena * 	persistentMemory;
	GameAssets 		assets;

	SkySettings 	skySettings;

	// ---------------------------------------

	CollisionSystem3D 	collisionSystem;
	PhysicsWorld 		physicsWorld;

	Camera 						worldCamera;
	GameCameraController 		gameCamera;	
	EditorCameraController 		editorCamera;
	f32 						cameraSelectPercent = 0;
	f32 						cameraTransitionDuration = 0.5;

	// ---------------------------------------

	Player player;

	// ---------------------------------------

	Transform3D 		noblePersonTransform;
	CharacterMotor 		noblePersonCharacterMotor;
	SkeletonAnimator 	noblePersonSkeletonAnimator;
	AnimatedRenderer 	noblePersonAnimatedRenderer;

	s32 	noblePersonMode;

	v3 		nobleWanderTargetPosition;
	f32 	nobleWanderWaitTimer;
	bool32 	nobleWanderIsWaiting;

	// ---------------------------------------

	static constexpr f32 fullWaterLevel = 1;
	Waters 			waters;
	MeshHandle 		waterMesh;
	MaterialHandle 	waterMaterial;

	/// POTS -----------------------------------
	struct Pots
	{
		s32 capacity;
		s32 count;

		Transform3D * 		transforms;
		f32 * 				waterLevels;
		EntityReference * 	carriedEntities;
	};

	Pots smallPots;

		MeshHandle 		potMesh;
		MaterialHandle 	potMaterial;

		MeshHandle 			bigPotMesh;
		MaterialHandle		bigPotMaterial;
		Array<Transform3D> 	bigPotTransforms;

	// ------------------------------------------------------

	Monuments monuments;

	Array<Scenery> sceneries;

	// ------------------------------------------------------

	s32 				raccoonCount;
	RaccoonMode *		raccoonModes;
	Transform3D * 		raccoonTransforms;
	v3 *				raccoonTargetPositions;
	CharacterMotor * 	raccoonCharacterMotors;

	MeshHandle 		raccoonMesh;
	MaterialHandle 	raccoonMaterial;

	// ------------------------------------------------------

	Transform3D 	trainTransform;
	MeshHandle 		trainMesh;
	MaterialHandle 	trainMaterial;

	v3 trainStopPosition;
	v3 trainFarPositionA;
	v3 trainFarPositionB;

	s32 trainMoveState;
	s32 trainTargetReachedMoveState;

	s32 trainWayPointIndex;

	f32 trainFullSpeed;
	f32 trainStationMinSpeed;
	f32 trainAcceleration;
	f32 trainWaitTimeOnStop;
	f32 trainBrakeBeforeStationDistance;

	f32 trainCurrentWaitTime;
	f32 trainCurrentSpeed;

	v3 trainCurrentTargetPosition;
	v3 trainCurrentDirection;

	// ------------------------------------------------------

	// Note(Leo): There are multiple mesh handles here
	s32 			terrainCount;
	m44 * 			terrainTransforms;
	MeshHandle * 	terrainMeshes;
	MaterialHandle 	terrainMaterial;

	m44 			seaTransform;
	MeshHandle 		seaMesh;
	MaterialHandle 	seaMaterial;

	// ----------------------------------------------------------
	
	bool32 		drawMCStuff;
	
	f32 			metaballGridScale;
	MaterialHandle 	metaballMaterial;

	m44 		metaballTransform;

	u32 		metaballVertexCapacity;
	u32 		metaballVertexCount;
	Vertex * 	metaballVertices;

	u32 		metaballIndexCapacity;
	u32 		metaballIndexCount;
	u16 * 		metaballIndices;


	m44 metaballTransform2;
	
	u32 		metaballVertexCapacity2;
	u32 		metaballVertexCount2;
	Vertex * 	metaballVertices2;

	u32 		metaballIndexCapacity2;
	u32 		metaballIndexCount2;
	u16 *		metaballIndices2;


	// ----------------------------------------------------------

	// Sky
	ModelHandle 	skybox;

	// Random
	Gui 		gui;
	CameraMode 	cameraMode;
	bool32		drawDebugShadowTexture;
	f32 		timeScale = 1;
	bool32 		guiVisible;

	Trees 	trees;
	Boxes 	boxes;
	Clouds 	clouds;

	// ----------------------------------------------

	v3 castlePosition;

	// ----------------------------------------------

	Scene scene;

	s64 selectedBuildingBlockIndex;
	s64 selectedBuildingPipeIndex;

	// ----------------------------------------------
	
	// AUDIO

	AudioAsset * backgroundAudio;
	AudioAsset * stepSFX;
	AudioAsset * stepSFX2;

	AudioClip 			backgroundAudioClip;
	Array<AudioClip> 	audioClipsOnPlay;

	v3 testRayPosition 	= {-10,0,45};
	v3 testRayDirection = {0,1,0};
	f32 testRayLength 	= 1;
};

internal v3 * entity_get_position(Game * game, EntityReference entity)
{
	switch(entity.type)
	{
		case EntityType_raccoon:	return &game->raccoonTransforms[entity.index].position;
		case EntityType_water: 		return &game->waters.positions[entity.index];
		case EntityType_small_pot:	return &game->smallPots.transforms[entity.index].position;
		case EntityType_tree_3: 	return &game->trees.array[entity.index].position;
		case EntityType_box:		return &game->boxes.transforms[entity.index].position;

		default:
			return nullptr;
	}
}

internal quaternion * entity_get_rotation(Game * game, EntityReference entity)
{
	switch(entity.type)
	{
		case EntityType_raccoon:	return &game->raccoonTransforms[entity.index].rotation;
		case EntityType_water: 		return &game->waters.rotations[entity.index];
		case EntityType_small_pot:	return &game->smallPots.transforms[entity.index].rotation;
		case EntityType_tree_3: 	return &game->trees.array[entity.index].rotation;
		case EntityType_box:		return &game->boxes.transforms[entity.index].rotation;

		default:
			return nullptr;
	}
}

internal CollisionSystem3D & game_get_collision_system(Game * game) { return game->collisionSystem; }


internal auto game_get_serialized_objects(Game & game)
{
	auto serializedObjects = make_property_list
	(
		serialize_object("sky", game.skySettings),
		serialize_object("tree_0", game.trees.settings[0]),
		serialize_object("tree_1", game.trees.settings[1]),
		serialize_object("player_camera", game.gameCamera)	, 
		serialize_object("editor_camera", game.editorCamera)
	);

	return serializedObjects;
}

internal void game_spawn_water(Game & game, s32 count)
{
	Waters & waters = game.waters;

	v2 center = game.player.characterTransform.position.xy;

	count = f32_min(count, waters.capacity - waters.count);

	for (s32 i = 0; i < count; ++i)
	{
		f32 distance 	= random_range(1, 5);
		f32 angle 		= random_range(0, 2 * π);

		f32 x = f32_cos(angle) * distance;
		f32 y = sine(angle) * distance;

		v3 position = { x + center.x, y + center.y, 0 };
		position.z 	= get_terrain_height(game.collisionSystem, position.xy);

		waters_instantiate(game.waters, position, game.fullWaterLevel);
	}
}

internal s32 game_spawn_tree(Game & game, v3 position, s32 treeTypeIndex, bool32 pushToPhysics)
{
	// Assert(game.trees.count < game.trees.capacity);
	if (game.trees.array.count >= game.trees.array.capacity)
	{
		// Todo(Leo): do something more interesting
		log_debug(FILE_ADDRESS, "Trying to spawn tree, but out of tree capacity");
		return -1;
	}

	s32 index = game.trees.array.count++;
	Tree & tree = game.trees.array[index];

	reset_tree_3(tree, &game.trees.settings[treeTypeIndex], position);

	tree.typeIndex 	= treeTypeIndex;
	tree.game 		= &game;

	MeshAssetId seedMesh = treeTypeIndex == 0 ? MeshAssetId_seed : MeshAssetId_water_drop;

	tree.leaves.material 	= assets_get_material(game.assets, MaterialAssetId_leaves);
	tree.seedMesh 			= assets_get_mesh(game.assets, seedMesh);
	tree.seedMaterial		= assets_get_material(game.assets, MaterialAssetId_seed);

	if (pushToPhysics)
	{
		physics_world_push_entity(game.physicsWorld, {EntityType_tree_3, (s32)game.trees.array.count - 1});
	}

	return index;
}

internal void game_spawn_tree_on_player(Game & game)
{
	v2 center = game.player.characterTransform.position.xy;

	f32 distance 	= random_range(1, 5);
	f32 angle 		= random_range(0, 2 * π);

	f32 x = f32_cos(angle) * distance;
	f32 y = sine(angle) * distance;

	v3 position = { x + center.x, y + center.y, 0 };
	position.z = get_terrain_height(game.collisionSystem, position.xy);

	local_persist s32 treeTypeIndex = 0;

	game_spawn_tree(game, position, treeTypeIndex);

	treeTypeIndex += 1;
	treeTypeIndex %= 2;
}

// Note(Leo): These seem to naturally depend on the game struct, so they are here.
// Todo(Leo): This may be a case for header file, at least for game itself
#include "game_gui.cpp"
#include "game_render.cpp"

// Todo(Leo): add this to syntax higlight, so that 'GAME UPDATE' is different color
/// ------------------ GAME UPDATE -------------------------
internal bool32 game_game_update(Game * 				game,
								PlatformInput * 		input,
								StereoSoundOutput *		soundOutput,
								f32 					elapsedTimeSeconds)
{
	struct SnapOnGround
	{
		CollisionSystem3D & collisionSystem;

		v3 operator()(v2 position)
		{
			v3 result = {position.x, position.y, get_terrain_height(collisionSystem, position)};
			return result;
		}

		v3 operator()(v3 position)
		{
			v3 result = {position.x, position.y, get_terrain_height(collisionSystem, position.xy)};
			return result;
		}
	};
	SnapOnGround snap_on_ground = {game->collisionSystem};

	/// ****************************************************************************
	/// TIME

	f32 scaledTime 		= elapsedTimeSeconds * game->timeScale;
	f32 unscaledTime 	= elapsedTimeSeconds;

	/// ****************************************************************************

	gui_start_frame(game->gui, input, unscaledTime);

	FS_DEBUG_ALWAYS(debug_draw_circle_xy(game->player.characterTransform.position + v3{0,0,2}, 2, colour_bright_red));



	bool playerInputAvailable = game->cameraMode == CameraMode_player;

	if (input_button_went_down(input, InputButton_keyboard_f1))
	{
		game->cameraMode = 	game->cameraMode == CameraMode_player ?
							CameraMode_editor :
							CameraMode_player;

		platform_window_set(platformWindow,
							PlatformWindowSetting_cursor_hidden_and_locked,
							game->cameraMode == CameraMode_player);
	}

	bool mouseInputAvailable = !ImGui::GetIO().WantCaptureMouse;

	// Game Logic section

	if (mouseInputAvailable)
	{
		switch (game->cameraMode)
		{
			case CameraMode_player:
			{
				player_camera_update(game->gameCamera,	 game->player.characterTransform.position, input, scaledTime);
				game->cameraSelectPercent = f32_max(0, game->cameraSelectPercent - unscaledTime / game->cameraTransitionDuration);
			} break;

			case CameraMode_editor:
			{
				editor_camera_update(game->editorCamera, input, scaledTime);
				game->cameraSelectPercent = f32_min(1, game->cameraSelectPercent + unscaledTime / game->cameraTransitionDuration);
			} break;

			case CameraModeCount:
				Assert(false && "Bad execution path");
				break;
		}
	}

	f32 cameraSelectPercent 		= f32_mathfun_smooth(game->cameraSelectPercent);
	game->worldCamera.position 		= v3_lerp(game->gameCamera.	position, game->editorCamera.position, cameraSelectPercent);
	game->worldCamera.direction 	= v3_lerp(game->gameCamera.	direction, game->editorCamera.direction, cameraSelectPercent);
	game->worldCamera.direction 	= v3_normalize(game->worldCamera.direction);
	game->worldCamera.farClipPlane 	= 1000;


	/// *******************************************************************************************


	{
		CharacterInput playerCharacterMotorInput = {};
		PlayerInput playerInput = {};
		if (playerInputAvailable)
		{

			if (input_button_went_down(input, InputButton_nintendo_y))
			{
				game->nobleWanderTargetPosition = game->player.characterTransform.position;
				game->nobleWanderWaitTimer 		= 0;
				game->nobleWanderIsWaiting 		= false;
			}

			playerInput 				= player_input_from_platform_input(input);
			playerCharacterMotorInput 	= update_player_input(game->worldCamera, playerInput);
		}
		update_character_motor(game->player.characterMotor, playerCharacterMotorInput, game->collisionSystem, scaledTime, DEBUG_LEVEL_PLAYER);


		// Todo(leo): Should these be handled differently????
		f32 playerPickupDistance 	= 1.0f;
		f32 playerInteractDistance 	= 1.0f;

		/// PICKUP OR DROP
		if (playerInput.pickupOrDrop)
		{
			v3 playerPosition = game->player.characterTransform.position;

			switch(game->player.carriedEntity.type)
			{
				case EntityType_none:
				{
					bool pickup = true;

					if (pickup)
					{
						for(s32 i  = 0; i < game->boxes.count; ++i)
						{
							f32 boxPickupDistance = 0.5f + playerPickupDistance;
							if (v3_length(playerPosition - game->boxes.transforms[i].position) < boxPickupDistance)
							{
								bool32 canPickInsides = game->boxes.carriedEntities[i].type != EntityType_none
														&& game->boxes.states[i] == BoxState_open;

								if (canPickInsides == false)
								{
									game->player.carriedEntity = {EntityType_box, i};
								}
								else
								{
									game->player.carriedEntity = game->boxes.carriedEntities[i];
									game->boxes.carriedEntities[i] = {EntityType_none};
								}

								pickup = false;
							}
						}
					}

					v3 playerPosition = game->player.characterTransform.position;

					/* Todo(Leo): Do this properly, taking into account player facing direction and distances etc. */
					auto check_pickup = [&](s32 count, Transform3D const * transforms, EntityType entityType)
					{
						for (s32 i = 0; i < count; ++i)
						{
							if (v3_length(playerPosition - transforms[i].position) < playerPickupDistance)
							{
								game->player.carriedEntity = {entityType, i};
								pickup = false;
							}
						}
					};


					if (pickup)
					{
						check_pickup(game->smallPots.count, game->smallPots.transforms, EntityType_small_pot);
					}

					if (pickup)
					{
						for (s32 i = 0; i < game->waters.count; ++i)
						{
							if (v3_length(playerPosition - game->waters.positions[i]) < playerPickupDistance)
							{
								game->player.carriedEntity = {EntityType_water, i};
								pickup = false;
							}
						}


					}


					if (pickup)
					{
						check_pickup(game->raccoonCount, game->raccoonTransforms, EntityType_raccoon);
					}

					if (pickup)
					{
						for (s32 i = 0; i < game->trees.array.count; ++i)
						{
							Tree & tree = game->trees.array[i];

							if (v3_length(playerPosition - game->trees.array[i].position) < playerPickupDistance)
							{
								// if (game->trees.array[i].planted == false)
								{
									game->player.carriedEntity = {EntityType_tree_3, i};
								}

								pickup = false;
							}

						}
					}

					// Todo(Leo): maybe this is bad, as in should we only do this if we actually started carrying something
					physics_world_remove_entity(game->physicsWorld, game->player.carriedEntity);

				} break;

				case EntityType_raccoon:
					game->raccoonTransforms[game->player.carriedEntity.index].rotation = quaternion_identity;
					// Todo(Leo): notice no break here, little sketcthy, maybe do something about it, probably in raccoon movement code
					// Todo(Leo): keep raccoon oriented normally, add animation or simulation make them look hangin..

				case EntityType_small_pot:
				case EntityType_tree_3:
				case EntityType_box:	
				case EntityType_water:
					physics_world_push_entity(game->physicsWorld, game->player.carriedEntity);
					game->player.carriedEntity = {EntityType_none};
					break;

				default:
					log_debug(FILE_ADDRESS, "cannot pickup or drop that entity: (", game->player.carriedEntity.type, ", #", game->player.carriedEntity.index, ")");
			}
		}

		f32 boxInteractDistance = 0.5f + playerInteractDistance;

		if (playerInput.interact)
		{
			bool32 interact = true;

			// TRY PUT STUFF INTO BOX
			bool playerCarriesContainableEntity = 	game->player.carriedEntity.type != EntityType_none
													&& game->player.carriedEntity.type != EntityType_box;

			if (interact && playerCarriesContainableEntity)
			{
				for (s32 i = 0; i < game->boxes.count; ++i)
				{
					f32 distanceToBox = v3_length(game->boxes.transforms[i].position - game->player.characterTransform.position);
					if (distanceToBox < boxInteractDistance && game->boxes.carriedEntities[i].type == EntityType_none)
					{
						game->boxes.carriedEntities[i] = game->player.carriedEntity;
						game->player.carriedEntity = {EntityType_none};

						interact = false;
					}				
				}
			}

			// TRY PUT STUFF INTO POT
			bool playerCarriesPottableEntity = 	game->player.carriedEntity.type == EntityType_tree_3
												|| game->player.carriedEntity.type == EntityType_raccoon;

			if (interact && playerCarriesPottableEntity)
			{
				log_debug("Try put something to pot");

				for (s32 i = 0; i < game->smallPots.count; ++i)
				{
					f32 distanceToPot = v3_length(game->smallPots.transforms[i].position - game->player.characterTransform.position);
					if (distanceToPot < playerInteractDistance && game->smallPots.carriedEntities[i].type == EntityType_none)
					{
						log_debug(FILE_ADDRESS, "Stuff put into pot");

						game->smallPots.carriedEntities[i] = game->player.carriedEntity;
						game->player.carriedEntity 	= {EntityType_none};

						interact = false;
					}
				}
			}
			
			// TRY OPEN OR CLOSE NEARBY BOX
			bool32 playerCarriesNothing = game->player.carriedEntity.type == EntityType_none;

			if (interact && playerCarriesNothing)
			{
				for (s32 i = 0; i < game->boxes.count; ++i)
				{
					f32 distanceToBox = v3_length(game->boxes.transforms[i].position - game->player.characterTransform.position);
					if (distanceToBox < boxInteractDistance)
					{
						if (game->boxes.states[i] == BoxState_closed)
						{
							game->boxes.states[i] 			= BoxState_opening;
							game->boxes.openStates[i] 	= 0;

							interact = false;
						}

						else if (game->boxes.states[i] == BoxState_open)
						{
							game->boxes.states[i] 			= BoxState_closing;
							game->boxes.openStates[i] 	= 0;

							interact = false;
						}
					}
				}
			}

			// PLANT THE CARRIED TREE
			bool32 playerCarriesTree = game->player.carriedEntity.type == EntityType_tree_3;

			if (interact && playerCarriesTree)
			{
				game->trees.array[game->player.carriedEntity.index].planted = true;
				v3 position = game->trees.array[game->player.carriedEntity.index].position;
				position.z = get_terrain_height(game->collisionSystem, position.xy);
				game->trees.array[game->player.carriedEntity.index].position = position;
				game->player.carriedEntity.index = -1;
				game->player.carriedEntity.type = EntityType_none;			

				interact = false;
			}

		}


		for(s32 i = 0; i < game->boxes.count; ++i)
		{
			v4 colour = game->boxes.carriedEntities[i].type == EntityType_none ? colour_bright_red : colour_bright_green;
			FS_DEBUG_NPC(debug_draw_circle_xy(game->boxes.transforms[i].position + v3{0,0,0.6}, 1, colour));
			FS_DEBUG_NPC(debug_draw_circle_xy(game->boxes.transforms[i].position + v3{0,0,0.6}, 1.5, colour_bright_cyan));
		}
	}

	auto update_carried_entities_transforms = [&](	s32 				count,
													Transform3D * 		transforms,
													EntityReference * 	carriedEntities,
													v3 					carriedOffset)
	{
		for (s32 i = 0; i < count; ++i)
		{
			v3 carriedPosition 			= multiply_point(transform_matrix(transforms[i]), carriedOffset);
			quaternion carriedRotation 	= transforms[i].rotation;
		
			// Todo(Leo): maybe something like this??
			// entity_set_position(game->player.carriedEntity, carriedPosition);
			// entity_set_rotation(game->player.carriedEntity, carriedRotation);
			// entity_set_state(game->player.carriedEntity, EntityState_carried_by_player);

			if (carriedEntities[i].type == EntityType_raccoon)
			{
				game->raccoonTransforms[carriedEntities[i].index].position 	= carriedPosition + v3{0,0,0.2};

				v3 right = quaternion_rotate_v3(carriedRotation, v3_right);
				game->raccoonTransforms[carriedEntities[i].index].rotation 	= carriedRotation * quaternion_axis_angle(right, 1.4f);
			}
			else if (carriedEntities[i].type != EntityType_none)
			{
				// Todo(Leo): if we ever crash here start doing checks
				*entity_get_position(game, carriedEntities[i]) = carriedPosition;
				*entity_get_rotation(game, carriedEntities[i]) = carriedRotation;
			}
		}
	};

	update_carried_entities_transforms(1, &game->player.characterTransform, &game->player.carriedEntity, {0, 0.7, 0.7});
	update_carried_entities_transforms(game->boxes.count, game->boxes.transforms, game->boxes.carriedEntities, {0, 0, 0.1});
	update_carried_entities_transforms(game->smallPots.count, game->smallPots.transforms, game->smallPots.carriedEntities, {0, 0, 0.1});

	// UPDATE BOX COVER
	for (s32 i = 0; i < game->boxes.count; ++i)
	{
		constexpr f32 openAngle 	= 5.0f/8.0f * π;
		constexpr f32 openingTime 	= 0.7f;

		if (game->boxes.states[i] == BoxState_opening)
		{
			game->boxes.openStates[i] += scaledTime / openingTime;

			if (game->boxes.openStates[i] > 1.0f)
			{
				game->boxes.states[i] 			= BoxState_open;
				game->boxes.openStates[i] 	= 1.0f;
			}

			f32 angle = openAngle * game->boxes.openStates[i];
			game->boxes.coverLocalTransforms[i].rotation = quaternion_axis_angle(v3_right, angle);
		}
		else if (game->boxes.states[i] == BoxState_closing)
		{
			game->boxes.openStates[i] += scaledTime / openingTime;

			if (game->boxes.openStates[i] > 1.0f)
			{
				game->boxes.states[i] 			= BoxState_closed;
				game->boxes.openStates[i] 	= 1.0f;
			}

			f32 angle = openAngle * (1 - game->boxes.openStates[i]);
			game->boxes.coverLocalTransforms[i].rotation = quaternion_axis_angle(v3_right, angle);
		}
	}

	update_waters(game->waters, scaledTime);
	update_clouds(game->clouds, game->waters, game->physicsWorld, game->collisionSystem, scaledTime);
	update_physics_world(game->physicsWorld, game, scaledTime);

	
	// -----------------------------------------------------------------------------------------------------------
	/// TRAIN
	{
		v3 trainWayPoints [] = 
		{
			game->trainStopPosition, 
			game->trainFarPositionA,
			game->trainStopPosition, 
			game->trainFarPositionB,
		};

		if (game->trainMoveState == TrainState_move)
		{

			v3 trainMoveVector 	= game->trainCurrentTargetPosition - game->trainTransform.position;
			f32 directionDot 	= v3_dot(trainMoveVector, game->trainCurrentDirection);
			f32 moveDistance	= v3_length(trainMoveVector);

			if (moveDistance > game->trainBrakeBeforeStationDistance)
			{
				game->trainCurrentSpeed += scaledTime * game->trainAcceleration;
				game->trainCurrentSpeed = f32_min(game->trainCurrentSpeed, game->trainFullSpeed);
			}
			else
			{
				game->trainCurrentSpeed -= scaledTime * game->trainAcceleration;
				game->trainCurrentSpeed = f32_max(game->trainCurrentSpeed, game->trainStationMinSpeed);
			}

			if (directionDot > 0)
			{
				game->trainTransform.position += game->trainCurrentDirection * game->trainCurrentSpeed * scaledTime;
			}
			else
			{
				game->trainMoveState 		= TrainState_wait;
				game->trainCurrentWaitTime = 0;
			}

		}
		else
		{
			game->trainCurrentWaitTime += scaledTime;
			if (game->trainCurrentWaitTime > game->trainWaitTimeOnStop)
			{
				game->trainMoveState 		= TrainState_move;
				game->trainCurrentSpeed 	= 0;

				v3 start 	= trainWayPoints[game->trainWayPointIndex];
				v3 end 		= trainWayPoints[(game->trainWayPointIndex + 1) % array_count(trainWayPoints)];

				game->trainWayPointIndex += 1;
				game->trainWayPointIndex %= array_count(trainWayPoints);

				game->trainCurrentTargetPosition 	= end;
				game->trainCurrentDirection 		= v3_normalize(end - start);
			}
		}
	}

	// -----------------------------------------------------------------------------------------------------------
	/// NOBLE PERSON CHARACTER
	{
		CharacterInput nobleCharacterMotorInput = {};

		switch(game->noblePersonMode)
		{
			case NoblePersonMode_wander_around:
			{
				if (game->nobleWanderIsWaiting)
				{
					game->nobleWanderWaitTimer -= scaledTime;
					if (game->nobleWanderWaitTimer < 0)
					{
						game->nobleWanderTargetPosition = { random_range(-99, 99), random_range(-99, 99)};
						game->nobleWanderIsWaiting = false;
					}


					m44 gizmoTransform = make_transform_matrix(	game->noblePersonTransform.position + v3_up * game->noblePersonTransform.scale.z * 2.0f, 
																game->noblePersonTransform.rotation,
																game->nobleWanderWaitTimer);
					FS_DEBUG_NPC(debug_draw_diamond_xz(gizmoTransform, colour_muted_red));
				}
				
				f32 distance = v2_length(game->noblePersonTransform.position.xy - game->nobleWanderTargetPosition.xy);
				if (distance < 1.0f && game->nobleWanderIsWaiting == false)
				{
					game->nobleWanderWaitTimer = 10;
					game->nobleWanderIsWaiting = true;
				}


				v3 input 			= {};
				input.xy	 		= game->nobleWanderTargetPosition.xy - game->noblePersonTransform.position.xy;
				f32 inputMagnitude 	= v3_length(input);
				input 				= input / inputMagnitude;
				inputMagnitude 		= f32_clamp(inputMagnitude, 0.0f, 1.0f);
				input 				= input * inputMagnitude;

				nobleCharacterMotorInput = {input, false, false};

			} break;
		}
	
		update_character_motor(	game->noblePersonCharacterMotor,
								nobleCharacterMotorInput,
								game->collisionSystem,
								scaledTime,
								DEBUG_LEVEL_NPC);
	}

	// -----------------------------------------------------------------------------------------------------------
	/// Update RACCOONS
	{
		// Note(Leo): +1 for player
		s32 maxCarriedRaccoons = game->boxes.count + game->smallPots.count + 1;
		Array<s32> carriedRaccoonIndices = push_array<s32>(*global_transientMemory, maxCarriedRaccoons, ALLOC_GARBAGE);
		{
			if(game->player.carriedEntity.type == EntityType_raccoon)
			{
				carriedRaccoonIndices.push(game->player.carriedEntity.index);
			}
			
			for (s32 i = 0; i < game->boxes.count; ++i)
			{
				if (game->boxes.carriedEntities[i].type == EntityType_raccoon)
				{
					carriedRaccoonIndices.push(game->boxes.carriedEntities[i].index);
				}
			}

			for (s32 i = 0; i < game->smallPots.count; ++i)
			{
				if (game->smallPots.carriedEntities[i].type == EntityType_raccoon)
				{
					f32 escapeChance = 0.001;
					if (random_value() < escapeChance)
					{
						game->smallPots.carriedEntities[i] = {EntityType_none};
					}			
					else
					{
						carriedRaccoonIndices.push(game->smallPots.carriedEntities[i].index);
					}
				}
			}
		}


		for(s32 i = 0; i < game->raccoonCount; ++i)
		{
			CharacterInput raccoonCharacterMotorInput = {};
	
			v3 toTarget 			= game->raccoonTargetPositions[i] - game->raccoonTransforms[i].position;
			f32 distanceToTarget 	= v3_length(toTarget);

			v3 input = {};

			if (distanceToTarget < 1.0f)
			{
				game->raccoonTargetPositions[i] 	= snap_on_ground(random_inside_unit_square() * 100 - v3{50,50,0});
				toTarget 							= game->raccoonTargetPositions[i] - game->raccoonTransforms[i].position;
			}
			else
			{
				input.xy	 		= game->raccoonTargetPositions[i].xy - game->raccoonTransforms[i].position.xy;
				f32 inputMagnitude 	= v3_length(input);
				input 				= input / inputMagnitude;
				inputMagnitude 		= f32_clamp(inputMagnitude, 0.0f, 1.0f);
				input 				= input * inputMagnitude;
			}

			raccoonCharacterMotorInput = {input, false, false};


			bool isCarried = false;
			for (s32 carriedRaccoonIndex : carriedRaccoonIndices)
			{
				if (carriedRaccoonIndex == i)
				{
					isCarried = true;
					break;
				}
			}


			if (isCarried == false)
			{
				update_character_motor( game->raccoonCharacterMotors[i],
										raccoonCharacterMotorInput,
										game->collisionSystem,
										scaledTime,
										DEBUG_LEVEL_NPC);
			}

			// debug_draw_circle_xy(snap_on_ground(game->raccoonTargetPositions[i].xy) + v3{0,0,1}, 1, colour_bright_red, DEBUG_LEVEL_ALWAYS);
		}


	}

	// -----------------------------------------------------------------------------------------------------------


	/// SUBMIT COLLIDERS
	// Note(Leo): These colliders are used mainly in next game loop, since we update them here after everything has moved
	// Make a proper decision whether or not this is something we need
	// Todo(Leo): most colliders are really static, so this is major stupid
	{
		collision_system_reset_submitted_colliders(game->collisionSystem);

		monuments_submit_colliders(game->monuments, game->collisionSystem);

		// Todo(Leo): Maybe make something like that these would have predetermined range, that is only updated when
		// tree has grown a certain amount or somthing. These are kinda semi-permanent by nature.

		auto submit_cylinder_colliders = [&collisionSystem = game->collisionSystem](f32 radius, f32 halfHeight, s32 count, Transform3D * transforms)
		{
			for (s32 i = 0; i < count; ++i)
			{
				submit_cylinder_collider(collisionSystem, {radius, halfHeight, v3{0, 0, halfHeight}}, transforms[i]);
			}
		};

		f32 smallPotColliderRadius = 0.3;
		f32 smallPotColliderHeight = 0.58;
		f32 smallPotHalfHeight = smallPotColliderHeight / 2;
		submit_cylinder_colliders(smallPotColliderRadius, smallPotHalfHeight, game->smallPots.count, game->smallPots.transforms);

		f32 bigPotColliderRadius 	= 0.6;
		f32 bigPotColliderHeight 	= 1.16;
		f32 bigPotHalfHeight 		= bigPotColliderHeight / 2;
		submit_cylinder_colliders(bigPotColliderRadius, bigPotHalfHeight, game->bigPotTransforms.count, game->bigPotTransforms.memory);


		// NEW TREES 3
		for (auto tree : game->trees.array)
		{
			CylinderCollider collider =
			{
				.radius 	= tree.nodes[tree.branches[0].startNodeIndex].radius,
				.halfHeight = 1.0f,
				.center 	= {0,0,1}
			};

			submit_cylinder_collider(game->collisionSystem, collider, {tree.position, quaternion_identity, {1,1,1}});
		}

		submit_box_collider(game->collisionSystem, {{0.5,0.5,0.5}, quaternion_identity, {0,0,0.5}}, game->boxes.transforms[0]);
		submit_box_collider(game->collisionSystem, {{0.5,0.5,0.5}, quaternion_identity, {0,0,0.5}}, game->boxes.transforms[1]);

		/// BUILDING BLOCKS
		for (s32 i = 0; i < game->scene.buildingBlocks.count; ++i)
		{
			submit_box_collider(game->collisionSystem, {{1,1,1}, quaternion_identity, {0,0,0}}, game->scene.buildingBlocks[i]);
		}

		for (s32 i = 0; i < game->scene.buildingPipes.count; ++i)
		{
			// submit_cylinder_collider(game->)
		}


		ImGui::Begin("Test collider");
		{
			ImGui::DragFloat3("corner 0", &game->collisionSystem.testTriangleCollider[0].x);
			ImGui::DragFloat3("corner 1", &game->collisionSystem.testTriangleCollider[1].x);
			ImGui::DragFloat3("corner 2", &game->collisionSystem.testTriangleCollider[2].x);

			ImGui::Spacing();
			ImGui::DragFloat3("ray position", &game->testRayPosition.x, 0.1);
			if (ImGui::DragFloat3("ray direction", &game->testRayDirection.x), 0.1)
			{
				game->testRayDirection = v3_normalize(game->testRayDirection);
			}
			ImGui::DragFloat("ray length", &game->testRayLength, 0.1);
		}
		ImGui::End();

		RaycastResult rayResult;
		bool hit = raycast_3d(&game->collisionSystem, game->testRayPosition, game->testRayDirection, game->testRayLength, &rayResult);


		if (hit)
		{
			FS_DEBUG_ALWAYS(debug_draw_vector(game->testRayPosition, rayResult.hitPosition - game->testRayPosition, colour_bright_red));
			// FS_DEBUG_ALWAYS(debug_draw_line(game->testRayPosition, rayResult.hitPosition, colour_bright_red));
		}
		else
		{
			FS_DEBUG_ALWAYS(debug_draw_vector(game->testRayPosition, game->testRayDirection * game->testRayLength, colour_bright_green));
			// FS_DEBUG_ALWAYS(debug_draw_line(game->testRayPosition, game->testRayPosition + game->testRayDirection * game->testRayLength, colour_bright_green));
		}


		FS_DEBUG_ALWAYS(debug_draw_line(game->collisionSystem.testTriangleCollider[0], game->collisionSystem.testTriangleCollider[1], colour_bright_green));
		FS_DEBUG_ALWAYS(debug_draw_line(game->collisionSystem.testTriangleCollider[1], game->collisionSystem.testTriangleCollider[2], colour_bright_green));
		FS_DEBUG_ALWAYS(debug_draw_line(game->collisionSystem.testTriangleCollider[2], game->collisionSystem.testTriangleCollider[0], colour_bright_green));
	}

	update_skeleton_animator(game->player.skeletonAnimator, scaledTime);
	update_skeleton_animator(game->noblePersonSkeletonAnimator, scaledTime);
	
	/// GENERATE MESH IF ENABLED
	{
		if (game->drawMCStuff)
		{
			v3 position = multiply_point(game->metaballTransform, {0,0,0});

			local_persist f32 testX = 0;
			local_persist f32 testY = 0;
			local_persist f32 testZ = 0;
			local_persist f32 testW = 0;

			testX += scaledTime;
			testY += scaledTime * 1.2;
			testZ += scaledTime * 0.9;
			testW += scaledTime * 1.7;

			v4 positions[] =
			{
				{mathfun_pingpong_f32(testX, 5),2,2,										2},
				{1,mathfun_pingpong_f32(testY, 3),1,										1},
				{4,3,mathfun_pingpong_f32(testZ, 3) + 1,									1.5},
				{mathfun_pingpong_f32(testW, 4) + 1, mathfun_pingpong_f32(testY, 3), 3,		1.2},
			};

			generate_mesh_marching_cubes(	game->metaballVertexCapacity, game->metaballVertices, &game->metaballVertexCount,
											game->metaballIndexCapacity, game->metaballIndices, &game->metaballIndexCount,
											sample_four_sdf_balls, positions, game->metaballGridScale);

			// auto sample_sdf_2 = [](v3 position, void const * data)
			// {
			// 	v3 a = {2,2,2};
			// 	v3 b = {5,3,5};

			// 	f32 rA = 1;
			// 	f32 rB = 1;

			// 	f32 t = f32_min(1, f32_max(0, v3_dot(position - a, b - a) / v3_sqr_length(b-a)));
			// 	f32 d = v3_length(position - a  - t * (b -a));

			// 	return d - f32_lerp(0.5,0.1,t);
			// };

			f32 fieldMemory [] =
			{
				-5,-5,-5,-5,-5,
				-5,-5,-5,-5,-5,
				-5,-5,-5,-5,-5,
				-5,-5,-5,-5,-5,
				
				-5,-5,-5,-5,-5,
				-5,-5,-5,-5,-5,
				-5,-5,-5,-5,-5,
				-5,-5,-5,-5,-5,
				
				5,5,5,5,5,
				5,-2,-2,5,5,
				5,-1,-1,-1,5,
				5,5,5,5,5,
				
				5,5,5,5,5,
				5,5,5,5,5,
				5,5,5,5,5,
				5,5,5,5,5,
			};

			VoxelField field = {5, 4, 4, fieldMemory};

			// Todo(Leo): This can be used to create terrain from actual voxels or something similar generated from heightmap
			generate_mesh_marching_cubes(	game->metaballVertexCapacity2, game->metaballVertices2, &game->metaballVertexCount2,
											game->metaballIndexCapacity2, game->metaballIndices2, &game->metaballIndexCount2,
											sample_heightmap_for_mc, &game->collisionSystem.terrainCollider, game->metaballGridScale);

			FS_DEBUG_ALWAYS(debug_draw_circle_xy(multiply_point(game->metaballTransform2, game->metaballVertices2[0].position), 5.0f, colour_bright_green));
		}
	}

	/// UPDATE TREES
	for (auto & tree : game->trees.array)
	{
		GetWaterFunc get_water = {game->waters, game->player.carriedEntity.index, game->player.carriedEntity.type == EntityType_water };
		update_tree_3(tree, scaledTime, get_water);
		
		tree.leaves.position = tree.position;
		tree.leaves.rotation = tree.rotation;
	}

	for (auto & tree : game->trees.array)
	{
		leaves_update(tree.leaves, scaledTime, tree.settings->leafSize);
	}

	// ---------- PROCESS AUDIO -------------------------

	{
		local_persist f32 timeToSpawnAudioOnOtherGuy = 0;
		timeToSpawnAudioOnOtherGuy -= scaledTime;
		if (timeToSpawnAudioOnOtherGuy < 0)
		{
			game->audioClipsOnPlay.push({game->stepSFX, 0, random_range(0.8, 1.2), 1, game->noblePersonTransform.position});
			timeToSpawnAudioOnOtherGuy = 1.0f;
		}
		
		for (s32 outputIndex = 0; outputIndex < soundOutput->sampleCount; ++outputIndex)
		{
			auto & output = soundOutput->samples[outputIndex];
			output = {};
			get_next_sample(game->backgroundAudioClip, output);

			for (s32 i = 0; i < game->audioClipsOnPlay.count; ++i)
			{
				auto & clip = game->audioClipsOnPlay[i];

				StereoSoundSample sample;
				bool32 hasSamplesLeft = get_next_sample(clip, sample);

				f32 attenuation;
				{
					f32 distanceToPlayer = v3_length(clip.position - game->player.characterTransform.position);
					distanceToPlayer = f32_clamp(distanceToPlayer, 0, 50);
					attenuation = 1 - (distanceToPlayer / 50);
				}

				output.left += sample.left * attenuation;
				output.right += sample.right * attenuation;


				if (hasSamplesLeft == false)
				{
					array_unordered_remove(game->audioClipsOnPlay, i);
					i -= 1;
				}
			}
		}
	}

	// ------------------------------------------------------------------------

	if (input_button_went_down(input, InputButton_keyboard_escape))
	{
		game->guiVisible = !game->guiVisible;
	}

	auto gui_result = game->guiVisible ? do_gui(game, input) : true;

	gui_end_frame();

	return gui_result;
}

internal Game * game_load_game(MemoryArena & persistentMemory, PlatformFileHandle saveFile)
{
	Game * game = push_memory<Game>(persistentMemory, 1, ALLOC_ZERO_MEMORY);
	*game = {};
	game->persistentMemory = &persistentMemory;


	// Todo(Leo): this is stupidly zero initialized here, before we read settings file, so that we don't override its settings
	game->gameCamera 	= {};
	game->editorCamera = {};

	// Note(Leo): We currently only have statically allocated stuff (or rather allocated with game),
	// this can be read here, at the top. If we need to allocate some stuff, we need to reconsider.
	read_settings_file(game_get_serialized_objects(*game));

	game->cameraMode = CameraMode_player;
	platform_window_set(platformWindow, PlatformWindowSetting_cursor_hidden_and_locked, true);

	// ----------------------------------------------------------------------------------

	// Todo(Leo): Think more about implications of storing pointer to persistent memory here
	// Note(Leo): We have not previously used persistent allocation elsewhere than in this load function
	game->assets 			= init_game_assets(&persistentMemory);
	game->collisionSystem 	= init_collision_system(persistentMemory);

	game->gui = {}; // Todo(Leo): remove this we now use ImGui

	initialize_physics_world(game->physicsWorld, persistentMemory);

	log_application(1, "Allocations succesful! :)");

	// ----------------------------------------------------------------------------------

	// Skysphere
	game->skybox = graphics_memory_push_model(	platformGraphics,
												assets_get_mesh(game->assets, MeshAssetId_skysphere),
												assets_get_material(game->assets, MaterialAssetId_sky));

	// Characters
	{
		game->player.carriedEntity.type 			= EntityType_none;
		game->player.characterTransform 	= {.position = {10, 0, 5}};

		auto & motor 	= game->player.characterMotor;
		motor.transform = &game->player.characterTransform;

		{
			motor.animations[CharacterAnimation_walk] 		= assets_get_animation(game->assets, AnimationAssetId_character_walk);
			motor.animations[CharacterAnimation_run] 		= assets_get_animation(game->assets, AnimationAssetId_character_run);
			motor.animations[CharacterAnimation_idle] 		= assets_get_animation(game->assets, AnimationAssetId_character_idle);
			motor.animations[CharacterAnimation_jump]		= assets_get_animation(game->assets, AnimationAssetId_character_jump);
			motor.animations[CharacterAnimation_fall]		= assets_get_animation(game->assets, AnimationAssetId_character_fall);
			motor.animations[CharacterAnimation_crouch] 	= assets_get_animation(game->assets, AnimationAssetId_character_crouch);
			motor.animations[CharacterAnimation_climb] 		= assets_get_animation(game->assets, AnimationAssetId_character_climb);
		}

		game->player.skeletonAnimator = 
		{
			.skeleton 		= assets_get_skeleton(game->assets, SkeletonAssetId_character),
			.animations 	= motor.animations,
			.weights 		= motor.animationWeights,
			.animationCount = CharacterAnimationCount
		};

		game->player.skeletonAnimator.boneBoneSpaceTransforms = push_array<Transform3D>(	persistentMemory,
																							game->player.skeletonAnimator.skeleton->boneCount,
																							ALLOC_ZERO_MEMORY);
		array_fill_with_value(game->player.skeletonAnimator.boneBoneSpaceTransforms, identity_transform);

		auto model = graphics_memory_push_model(platformGraphics,
												assets_get_mesh(game->assets, MeshAssetId_character),
												assets_get_material(game->assets, MaterialAssetId_character));
		game->player.animatedRenderer = make_animated_renderer(&game->player.characterTransform, game->player.skeletonAnimator.skeleton, model);

	}

	// NOBLE PERSON GUY
	{
		v3 position = {random_range(-99, 99), random_range(-99, 99), 0};
		v3 scale 	= make_uniform_v3(random_range(0.8f, 1.5f));

		game->noblePersonTransform.position 	= position;
		game->noblePersonTransform.scale 		= scale;

		game->noblePersonCharacterMotor = {};
		game->noblePersonCharacterMotor.transform = &game->noblePersonTransform;

		{
			game->noblePersonCharacterMotor.animations[CharacterAnimation_walk] 	= assets_get_animation(game->assets, AnimationAssetId_character_walk);
			game->noblePersonCharacterMotor.animations[CharacterAnimation_run] 	= assets_get_animation(game->assets, AnimationAssetId_character_run);
			game->noblePersonCharacterMotor.animations[CharacterAnimation_idle]	= assets_get_animation(game->assets, AnimationAssetId_character_idle);
			game->noblePersonCharacterMotor.animations[CharacterAnimation_jump] 	= assets_get_animation(game->assets, AnimationAssetId_character_jump);
			game->noblePersonCharacterMotor.animations[CharacterAnimation_fall] 	= assets_get_animation(game->assets, AnimationAssetId_character_fall);
			game->noblePersonCharacterMotor.animations[CharacterAnimation_crouch] 	= assets_get_animation(game->assets, AnimationAssetId_character_crouch);
			game->noblePersonCharacterMotor.animations[CharacterAnimation_climb] 	= assets_get_animation(game->assets, AnimationAssetId_character_climb);
		}

		game->noblePersonSkeletonAnimator = 
		{
			.skeleton 		= assets_get_skeleton(game->assets, SkeletonAssetId_character),
			.animations 	= game->noblePersonCharacterMotor.animations,
			.weights 		= game->noblePersonCharacterMotor.animationWeights,
			.animationCount = CharacterAnimationCount
		};
		game->noblePersonSkeletonAnimator.boneBoneSpaceTransforms = push_array<Transform3D>(	persistentMemory,
																								game->noblePersonSkeletonAnimator.skeleton->boneCount,
																								ALLOC_ZERO_MEMORY);
		array_fill_with_value(game->noblePersonSkeletonAnimator.boneBoneSpaceTransforms, identity_transform);

		auto model = graphics_memory_push_model(platformGraphics,
												assets_get_mesh(game->assets, MeshAssetId_character), 
												assets_get_material(game->assets, MaterialAssetId_character)); 

		game->noblePersonAnimatedRenderer = make_animated_renderer(&game->noblePersonTransform, game->noblePersonSkeletonAnimator.skeleton, model);

	}

	game->worldCamera 				= make_camera(60, 0.1f, 1000.0f);

	// Environment
	{
		/// GROUND AND TERRAIN
		{
			constexpr f32 mapSize 		= 1200;
			constexpr f32 minTerrainElevation = -5;
			constexpr f32 maxTerrainElevation = 100;

			// Note(Leo): this is maximum size we support with u16 mesh vertex indices
			s32 chunkResolution			= 128;
			
			s32 chunkCountPerDirection 	= 10;
			f32 chunkSize 				= 1.0f / chunkCountPerDirection;
			f32 chunkWorldSize 			= chunkSize * mapSize;

			HeightMap heightmap;
			{
				auto checkpoint = memory_push_checkpoint(*global_transientMemory);
				
				s32 heightmapResolution = 1024;

				// todo(Leo): put to asset system thing
				TextureAssetData heightmapData = assets_get_texture_data(game->assets, TextureAssetId_heightmap);
				heightmap 				= make_heightmap(&persistentMemory, &heightmapData, heightmapResolution, mapSize, minTerrainElevation, maxTerrainElevation);

				memory_pop_checkpoint(*global_transientMemory, checkpoint);
			}

			s32 terrainCount 			= chunkCountPerDirection * chunkCountPerDirection;
			game->terrainCount 		= terrainCount;
			game->terrainTransforms 	= push_memory<m44>(persistentMemory, terrainCount, ALLOC_GARBAGE);
			game->terrainMeshes 		= push_memory<MeshHandle>(persistentMemory, terrainCount, ALLOC_GARBAGE);
			game->terrainMaterial 		= assets_get_material(game->assets, MaterialAssetId_ground);

			/// GENERATE GROUND MESH
			{
				auto checkpoint = memory_push_checkpoint(*global_transientMemory);
				// push_memory_checkpoint(*global_transientMemory);
			
				for (s32 i = 0; i < terrainCount; ++i)
				{
					s32 x = i % chunkCountPerDirection;
					s32 y = i / chunkCountPerDirection;

					v2 position = { x * chunkSize, y * chunkSize };
					v2 size 	= { chunkSize, chunkSize };

					auto groundMeshAsset 	= generate_terrain(*global_transientMemory, heightmap, position, size, chunkResolution, 20);
					game->terrainMeshes[i] = graphics_memory_push_mesh(platformGraphics, &groundMeshAsset);
				}
			
				memory_pop_checkpoint(*global_transientMemory, checkpoint);
				// pop_memory_checkpoint(*global_transientMemory);
			}

			f32 halfMapSize = mapSize / 2;
			v3 terrainOrigin = {-halfMapSize, -halfMapSize, 0};

			for (s32 i = 0; i < terrainCount; ++i)
			{
				s32 x = i % chunkCountPerDirection;
				s32 y = i / chunkCountPerDirection;

				v3 leftBackCornerPosition = {x * chunkWorldSize - halfMapSize, y * chunkWorldSize - halfMapSize, 0};
				game->terrainTransforms[i] = translation_matrix(leftBackCornerPosition);
			}

			game->collisionSystem.terrainCollider 	= heightmap;
			game->collisionSystem.terrainOffset = {{-mapSize / 2, -mapSize / 2, 0}};

			MeshAssetData seaMeshAsset = {};
			{
				Vertex vertices []
				{
					{-0.5, -0.5, 0, 0, 0, 1, 1,1,1, 0, 0},
					{ 0.5, -0.5, 0, 0, 0, 1, 1,1,1, 1, 0},
					{-0.5,  0.5, 0, 0, 0, 1, 1,1,1, 0, 1},
					{ 0.5,  0.5, 0, 0, 0, 1, 1,1,1, 1, 1},
				};
				seaMeshAsset.vertices 	= push_and_copy_memory(*global_transientMemory, array_count(vertices), vertices); 

				u16 indices [] 			= {0,1,2,2,1,3};
				seaMeshAsset.indexCount = array_count(indices);
				seaMeshAsset.indices 	= push_and_copy_memory(*global_transientMemory, array_count(indices), indices);

				seaMeshAsset.indexType = MeshIndexType_uint16;
			}
			mesh_generate_tangents(seaMeshAsset);
			game->seaMesh 		= graphics_memory_push_mesh(platformGraphics, &seaMeshAsset);
			game->seaMaterial 	= assets_get_material(game->assets, MaterialAssetId_sea);
			game->seaTransform = transform_matrix({0,0,0}, quaternion_identity, {mapSize, mapSize, 1});
		}

		game->sceneries = push_array<Scenery>(persistentMemory, 100, ALLOC_GARBAGE);

		/// TOTEMS
		{
			Scenery & totems = game->sceneries.push({});

			s64 totemCount = 2;
			totems =
			{
				.mesh 		= MeshAssetId_totem,
				.material 	= MaterialAssetId_building_block,
				.count 		= totemCount,
				.transforms = push_memory<m44>(persistentMemory, totemCount, ALLOC_GARBAGE)
			};

			v3 position0 = {0,0,0}; 	position0.z = get_terrain_height(game->collisionSystem, position0.xy);
			v3 position1 = {0,5,0}; 	position1.z = get_terrain_height(game->collisionSystem, position1.xy);

			totems.transforms[0] = transform_matrix({position0, quaternion_identity, {1,1,1}});
			totems.transforms[1] = transform_matrix({position1, quaternion_identity, {0.5, 0.5, 0.5}});

			BoxCollider totemCollider = {v3{1,1,5}, quaternion_identity, v3{0,0,2}};

			push_static_box_collider(game->collisionSystem, totemCollider, totems.transforms[0]);
			push_static_box_collider(game->collisionSystem, totemCollider, totems.transforms[1]);
		}

		/// RACCOONS
		{
			game->raccoonCount = 4;
			push_multiple_memories(	persistentMemory,
									game->raccoonCount,
									ALLOC_ZERO_MEMORY,
									
									&game->raccoonModes,
									&game->raccoonTransforms,
									&game->raccoonTargetPositions,
									&game->raccoonCharacterMotors);

			for (s32 i = 0; i < game->raccoonCount; ++i)
			{
				game->raccoonModes[i]					= RaccoonMode_idle;

				game->raccoonTransforms[i] 			= identity_transform;
				game->raccoonTransforms[i].position 	= random_inside_unit_square() * 100 - v3{50, 50, 0};
				game->raccoonTransforms[i].position.z  = get_terrain_height(game->collisionSystem, game->raccoonTransforms[i].position.xy);

				game->raccoonTargetPositions[i] 		= random_inside_unit_square() * 100 - v3{50,50,0};
				game->raccoonTargetPositions[i].z  	= get_terrain_height(game->collisionSystem, game->raccoonTargetPositions[i].xy);

				// ------------------------------------------------------------------------------------------------
		
				// Note Todo(Leo): Super debug, not at all like this
				game->raccoonCharacterMotors[i] = {};
				game->raccoonCharacterMotors[i].transform = &game->raccoonTransforms[i];
				game->raccoonCharacterMotors[i].walkSpeed = 2;
				game->raccoonCharacterMotors[i].runSpeed = 4;

				{
					game->raccoonCharacterMotors[i].animations[CharacterAnimation_walk] 	= assets_get_animation(game->assets, AnimationAssetId_raccoon_empty);
					game->raccoonCharacterMotors[i].animations[CharacterAnimation_run] 	= assets_get_animation(game->assets, AnimationAssetId_raccoon_empty);
					game->raccoonCharacterMotors[i].animations[CharacterAnimation_idle]	= assets_get_animation(game->assets, AnimationAssetId_raccoon_empty);
					game->raccoonCharacterMotors[i].animations[CharacterAnimation_jump] 	= assets_get_animation(game->assets, AnimationAssetId_raccoon_empty);
					game->raccoonCharacterMotors[i].animations[CharacterAnimation_fall] 	= assets_get_animation(game->assets, AnimationAssetId_raccoon_empty);
					game->raccoonCharacterMotors[i].animations[CharacterAnimation_crouch] 	= assets_get_animation(game->assets, AnimationAssetId_raccoon_empty);
				}
			}

			game->raccoonMesh 		= assets_get_mesh(game->assets, MeshAssetId_raccoon);
			game->raccoonMaterial	= assets_get_material(game->assets, MaterialAssetId_raccoon);
		}

		// /// INVISIBLE TEST COLLIDER, for nostalgia :')
		// {
		// 	Transform3D * transform = game->transforms.push_return_pointer({});
		// 	transform->position = {21, 15};
		// 	transform->position.z = get_terrain_height(game->collisionSystem, {20, 20});

		// 	push_box_collider( 	game->collisionSystem,
		// 						v3{2.0f, 0.05f,5.0f},
		// 						v3{0,0, 2.0f},
		// 						transform);
		// }

		game->monuments = init_monuments(persistentMemory, game->assets, game->collisionSystem);

		// TEST ROBOT
		{
			Scenery & robots = game->sceneries.push({});

			robots =
			{
				.mesh 		= MeshAssetId_robot,
				.material 	= MaterialAssetId_robot,
				.count 		= 1,
				.transforms = push_memory<m44>(persistentMemory, 1, ALLOC_GARBAGE)
			};

			v3 position 				= {21, 10, 0};
			position.z 					= get_terrain_height(game->collisionSystem, position.xy);
			robots.transforms[0] 	= translation_matrix(position);
		}

		/// SMALL SCENERY OBJECTS
		{			
			game->potMesh 		= assets_get_mesh(game->assets, MeshAssetId_small_pot);
			game->potMaterial 	= assets_get_material(game->assets, MaterialAssetId_environment);

			{
				game->smallPots.capacity 			= 10;
				game->smallPots.count 				= game->smallPots.capacity;
				game->smallPots.transforms 			= push_memory<Transform3D>(persistentMemory, game->smallPots.capacity, ALLOC_GARBAGE);
				game->smallPots.waterLevels 		= push_memory<f32>(persistentMemory, game->smallPots.capacity, ALLOC_GARBAGE);
				game->smallPots.carriedEntities 	= push_memory<EntityReference>(persistentMemory, game->smallPots.capacity, ALLOC_ZERO_MEMORY);

				for(s32 i = 0; i < game->smallPots.capacity; ++i)
				{
					v3 position 			= {15, i * 5.0f, 0};
					position.z 				= get_terrain_height(game->collisionSystem, position.xy);
					game->smallPots.transforms[i]	= { .position = position };
				}
			}

			// ----------------------------------------------------------------	

			auto bigPotMesh = assets_get_mesh(game->assets, MeshAssetId_big_pot);

			s32 bigPotCount = 5;

			game->bigPotMesh 		= assets_get_mesh(game->assets, MeshAssetId_big_pot);
			game->bigPotMaterial 	= assets_get_material(game->assets, MaterialAssetId_environment);
			game->bigPotTransforms 	= push_array<Transform3D>(persistentMemory, bigPotCount, ALLOC_GARBAGE);
			game->bigPotTransforms.count = bigPotCount;

			for (s32 i = 0; i < bigPotCount; ++i)
			{
				v3 position = {13, 2.0f + i * 4.0f}; 		position.z = get_terrain_height(game->collisionSystem, position.xy);

				Transform3D * transform = &game->bigPotTransforms[i];
				*transform 				= identity_transform;
				transform->position 	= position;

				// f32 colliderHeight = 1.16;
				// push_cylinder_collider(game->collisionSystem, 0.6, colliderHeight, v3{0,0,colliderHeight / 2}, transform);
			}


			// ----------------------------------------------------------------	

			/// WATERS
			initialize_waters(game->waters, persistentMemory);


		}

		/// TRAIN
		{
			game->trainMesh 		= assets_get_mesh(game->assets, MeshAssetId_train);
			game->trainMaterial 	= assets_get_material(game->assets, MaterialAssetId_environment);

			// ----------------------------------------------------------------------------------

			game->trainStopPosition 	= {50, 0, 0};
			game->trainStopPosition.z 	= get_terrain_height(game->collisionSystem, game->trainStopPosition.xy);
			game->trainStopPosition.z 	= f32_max(0, game->trainStopPosition.z);

			f32 trainFarDistance 		= 2000;

			game->trainFarPositionA 	= {50, trainFarDistance, 0};
			game->trainFarPositionA.z 	= get_terrain_height(game->collisionSystem, game->trainFarPositionA.xy);
			game->trainFarPositionA.z 	= f32_max(0, game->trainFarPositionA.z);

			game->trainFarPositionB 	= {50, -trainFarDistance, 0};
			game->trainFarPositionB.z 	= get_terrain_height(game->collisionSystem, game->trainFarPositionB.xy);
			game->trainFarPositionB.z 	= f32_max(0, game->trainFarPositionB.z);

			game->trainTransform.position 			= game->trainStopPosition;

			game->trainMoveState 					= TrainState_wait;

			game->trainFullSpeed 					= 200;
			game->trainStationMinSpeed 			= 1;
			game->trainAcceleration 				= 20;
			game->trainWaitTimeOnStop 				= 5;

			/*
			v = v0 + a*t
			-> t = (v - v0) / a
			d = d0 + v0*t + 1/2*a*t^2
			d - d0 = v0*t + 1/2*a*t^2
			*/

			f32 timeToBrakeBeforeStation 			= (game->trainFullSpeed - game->trainStationMinSpeed) / game->trainAcceleration;
			game->trainBrakeBeforeStationDistance 	= game->trainFullSpeed * timeToBrakeBeforeStation
													// Note(Leo): we brake, so acceleration term is negative
													- 0.5f * game->trainAcceleration * timeToBrakeBeforeStation * timeToBrakeBeforeStation;

			game->trainCurrentWaitTime = 0;
			game->trainCurrentSpeed 	= 0;

		}
	}

	// ----------------------------------------------------------------------------------

	/// MARCHING CUBES AND METABALLS TESTING
	{
		v3 position = {-10, -30, 0};
		position.z = get_terrain_height(game->collisionSystem, position.xy) + 3;

		s32 vertexCountPerCube 	= 14;
		s32 indexCountPerCube 	= 36;
		s32 cubeCapacity 		= 100000;
		s32 vertexCapacity 		= vertexCountPerCube * cubeCapacity;
		s32 indexCapacity 		= indexCountPerCube * cubeCapacity;

		Vertex * vertices 	= push_memory<Vertex>(persistentMemory, vertexCapacity, ALLOC_GARBAGE);
		u16 * indices 		= push_memory<u16>(persistentMemory, indexCapacity, ALLOC_GARBAGE);

		u32 vertexCount;
		u32 indexCount;

		game->metaballGridScale = 0.3;

		game->metaballVertexCount 	= vertexCount;
		game->metaballVertices 	= vertices;

		game->metaballIndexCount 	= indexCount;
		game->metaballIndices 		= indices;

		game->metaballVertexCapacity 	= vertexCapacity;
		game->metaballIndexCapacity 	= indexCapacity;

		game->metaballTransform 	= translation_matrix(position);

		game->metaballMaterial = assets_get_material(game->assets, MaterialAssetId_tree);

		// ----------------------------------------------------------------------------------

		game->metaballVertexCapacity2 	= vertexCapacity;
		game->metaballVertexCount2 	= 0;
		game->metaballVertices2 		= push_memory<Vertex>(persistentMemory, vertexCapacity, ALLOC_GARBAGE);

		game->metaballIndexCapacity2 	= vertexCapacity;
		game->metaballIndexCount2 		= 0;
		game->metaballIndices2 		= push_memory<u16>(persistentMemory, indexCapacity, ALLOC_GARBAGE);

		v3 position2 				= {0, -30, 0};
		position2.z 				= get_terrain_height(game->collisionSystem, position2.xy) + 3;
		game->metaballTransform2 	= translation_matrix(position2);

	}

	// ----------------------------------------------------------------------------------
	
	{	
		game->trees.array = push_array<Tree>(persistentMemory, 200, ALLOC_ZERO_MEMORY);
		for (s32 i = 0; i < game->trees.array.capacity; ++i)
		{
			allocate_tree_memory(persistentMemory, game->trees.array.memory[i]);
		}
		game->trees.selectedIndex = 0;
	}

	{
		v3 boxPosition0 		= {20, 2, get_terrain_height(game->collisionSystem, {20, 2})};
		v3 boxPosition1 		= {30, 5, get_terrain_height(game->collisionSystem, {30, 5})};
		v3 coverPositionOffset 	= {0, -0.5, 0.85};

		game->boxes.count 					= 2;
		game->boxes.transforms 				= push_memory<Transform3D>(persistentMemory, game->boxes.count, ALLOC_GARBAGE);
		game->boxes.coverLocalTransforms 	= push_memory<Transform3D>(persistentMemory, game->boxes.count, ALLOC_GARBAGE);
		game->boxes.states 					= push_memory<BoxState>(persistentMemory, game->boxes.count, ALLOC_ZERO_MEMORY);
		game->boxes.openStates				= push_memory<f32>(persistentMemory, game->boxes.count, ALLOC_GARBAGE);
		game->boxes.carriedEntities 		= push_memory<EntityReference>(persistentMemory, game->boxes.count, ALLOC_ZERO_MEMORY);

		game->boxes.transforms[0] = {boxPosition0};
		game->boxes.transforms[1] = {boxPosition1};

		game->boxes.coverLocalTransforms[0] = {coverPositionOffset};
		game->boxes.coverLocalTransforms[1] = {coverPositionOffset};
	
		game->boxes.carriedEntities[0] = {EntityType_tree_3, game_spawn_tree(*game, boxPosition0 + v3{0,0,0.1}, 0, false)};
		game->boxes.carriedEntities[1] = {EntityType_tree_3, game_spawn_tree(*game, boxPosition0 + v3{0,0,0.1}, 1, false)};
	}


	// ----------------------------------------------------------------------------------

	initialize_clouds(game->clouds, persistentMemory);

	// ----------------------------------------------------------------------------------

	game->scene.buildingBlocks = push_array<m44>(persistentMemory, 1000, ALLOC_GARBAGE);
	game->scene.buildingPipes = push_array<m44>(persistentMemory, 1000, ALLOC_GARBAGE);
	scene_asset_load(game->scene);

	game->selectedBuildingBlockIndex = 0;
	game->selectedBuildingPipeIndex = 0;

	// ----------------------------------------------------------------------------------

	{
		// struct MeshAssetData
		// {
		// 	s64 				vertexCount;
		// 	Vertex * 			vertices;
		// 	VertexSkinData * 	skinning;

		// 	s64 	indexCount;
		// 	u16 * 	indices;

		// 	MeshIndexType indexType = MeshIndexType_uint16;


		game->castlePosition = {70, 70, 20};

		auto castleMesh 	= assets_get_mesh_asset_data(game->assets, MeshAssetId_castle_main, *global_transientMemory);
		s64 triangleCount 	= castleMesh.indexCount / 3;

		game->collisionSystem.triangleColliders = push_array<TriangleCollider>(persistentMemory, triangleCount, ALLOC_GARBAGE);

		for (s64 i = 0; i < triangleCount; ++i)
		{
			s64 t0 = castleMesh.indices[i * 3 + 0];
			s64 t1 = castleMesh.indices[i * 3 + 1];
			s64 t2 = castleMesh.indices[i * 3 + 2];

			TriangleCollider collider = {};
			collider.vertices[0] = castleMesh.vertices[t0].position + game->castlePosition;
			collider.vertices[1] = castleMesh.vertices[t1].position + game->castlePosition;
			collider.vertices[2] = castleMesh.vertices[t2].position + game->castlePosition;

			game->collisionSystem.triangleColliders.push(collider);
		}

	}
	// ----------------------------------------------------------------------------------

	game->backgroundAudio 	= assets_get_audio(game->assets, SoundAssetId_background);
	game->stepSFX			= assets_get_audio(game->assets, SoundAssetId_step_1);
	game->stepSFX2			= assets_get_audio(game->assets, SoundAssetId_step_2);
	// game->stepSFX3			= assets_get_audio(game->assets, SoundAssetId_birds);

	game->backgroundAudioClip 	= {game->backgroundAudio, 0};

	game->audioClipsOnPlay = push_array<AudioClip>(persistentMemory, 1000, ALLOC_ZERO_MEMORY);

	// ----------------------------------------------------------------------------------

	log_application(0, "Game loaded, ", used_percent(*global_transientMemory) * 100, "% of transient memory used. ",
					reverse_gigabytes(global_transientMemory->used), "/", reverse_gigabytes(global_transientMemory->size), " GB");

	log_application(0, "Game loaded, ", used_percent(persistentMemory) * 100, "% of transient memory used. ",
					reverse_gigabytes(persistentMemory.used), "/", reverse_gigabytes(persistentMemory.size), " GB");

	return game;
}
