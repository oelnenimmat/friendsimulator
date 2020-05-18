struct BoxCollider
{
	v3 extents;
	v3 center; 
	quaternion orientation;

	Transform3D * transform;
};

struct CylinderCollider
{
	f32 radius;
	f32 height;
	v3 center;

	Transform3D * transform;
};

struct StaticBoxCollider
{
	m44 transform;
	m44 inverseTransform;
	v3 extents;
};

struct CollisionSystem3D
{
	Array<BoxCollider> 	boxColliders;
	Array<CylinderCollider> cylinderColliders;


	Array<StaticBoxCollider> precomputedColliders;

	// Todo(Leo): include these to above
	HeightMap terrainCollider;
	Transform3D * terrainTransform;
};

internal void precompute_colliders(CollisionSystem3D & system)
{
	s32 boxColliderCount = system.boxColliders.count();
	clear_array(system.precomputedColliders);
	system.precomputedColliders = allocate_array<StaticBoxCollider>(*global_transientMemory, boxColliderCount, ALLOC_NO_CLEAR | ALLOC_FILL);

	for (s32 i = 0; i < boxColliderCount; ++i)
	{
		const BoxCollider & collider = system.boxColliders[i];

		m44 colliderTransform = translation_matrix(collider.transform->position)
								* rotation_matrix(collider.transform->rotation)
								* translation_matrix(collider.center)
								* rotation_matrix(collider.orientation)
								* scale_matrix(collider.extents);

		v3 inverseScale = { 1.0f / collider.extents.x,
							1.0f / collider.extents.y,
							1.0f / collider.extents.z };

		m44 colliderInverseMatrix = scale_matrix(inverseScale)
									* rotation_matrix(collider.orientation.inverse())
									* translation_matrix(-collider.center)
									* rotation_matrix(collider.transform->rotation.inverse())
									* translation_matrix(-collider.transform->position);

		system.precomputedColliders[i].transform 		= colliderTransform;
		system.precomputedColliders[i].inverseTransform = colliderInverseMatrix;
		system.precomputedColliders[i].extents 			= collider.extents;
	}
}


internal void push_box_collider (	CollisionSystem3D & system,
									v3 					extents,
									v3 					center,
									Transform3D * 		transform)
{
	system.boxColliders.push({extents, center, identity_quaternion, transform});
}


internal void push_cylinder_collider ( 	CollisionSystem3D & system,
										f32 radius,
										f32 height,
										v3 center,
										Transform3D * transform)
{
	system.cylinderColliders.push({radius, height, center, transform});
}

internal float
get_terrain_height(CollisionSystem3D * system, v2 position)
{
	position.x -= system->terrainTransform->position.x;
	position.y -= system->terrainTransform->position.y;
	float value = get_height_at(&system->terrainCollider, position);
	return value;
}

struct RaycastResult
{
	v3 hitPosition;
	v3 hitNormal;
	f32 hitDistance;
};

internal bool32
raycast_3d(	CollisionSystem3D * system,
			v3 rayStart,
			v3 normalizedRayDirection,
			float rayLength,
			RaycastResult * outResult = nullptr)
{
	// https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection

	f32 rayHitSquareDistance = highest_f32;
	bool32 hit = false;

	for (auto const & collider : system->precomputedColliders)
	{
		v3 colliderSpaceRayStart 		= multiply_point(collider.inverseTransform, rayStart);
		v3 colliderSpaceRayDirection 	= multiply_direction(collider.inverseTransform, normalizedRayDirection);  

		// Study(Leo): this seems to work, but I still have concerns
		f32 colliderSpaceRayLength = rayLength;

		v3 min = {-1,-1,-1};
		v3 max = {1,1,1};

		v3 inverseDirection = 
		{
			1.0f / colliderSpaceRayDirection.x,
			1.0f / colliderSpaceRayDirection.y,
			1.0f / colliderSpaceRayDirection.z,
		};

		float xDistanceToMin = (min.x - colliderSpaceRayStart.x) * inverseDirection.x;
		float xDistanceToMax = (max.x - colliderSpaceRayStart.x) * inverseDirection.x;
		if (inverseDirection.x < 0)
		{
			swap(xDistanceToMin, xDistanceToMax);
		}

		float yDistanceToMin = (min.y - colliderSpaceRayStart.y) * inverseDirection.y;
		float yDistanceToMax = (max.y - colliderSpaceRayStart.y) * inverseDirection.y;
		if (inverseDirection.y < 0)
		{
			swap(yDistanceToMin, yDistanceToMax);
		}
		
		float zDistanceToMin = (min.z - colliderSpaceRayStart.z) * inverseDirection.z;
		float zDistanceToMax = (max.z - colliderSpaceRayStart.z) * inverseDirection.z;
		if (inverseDirection.z < 0)
		{
			swap(zDistanceToMin, zDistanceToMax);
		}

		// Note(Leo): if any min distance is more than any max distance
		if ((xDistanceToMin > yDistanceToMax) || (yDistanceToMin > xDistanceToMax))
		{
			// no collision
			continue;
		}			

		float distanceToMin = math::max(xDistanceToMin, yDistanceToMin);
		float distanceToMax = math::min(xDistanceToMax, yDistanceToMax);
		

		if ((distanceToMin > zDistanceToMax) || (zDistanceToMin > distanceToMax))
		{
			// no collision
			continue;
		}			

		distanceToMin = math::max(distanceToMin, zDistanceToMin);
		distanceToMax = math::min(distanceToMax, zDistanceToMax);

		if (0.00001f < distanceToMin && distanceToMin < colliderSpaceRayLength)
		{
			hit = true;

			v3 colliderSpaceHitPosition = colliderSpaceRayStart + colliderSpaceRayDirection * distanceToMin;
			v3 hitPosition = multiply_point(collider.transform, colliderSpaceHitPosition);
			f32 sqrDistance = square_magnitude_v3(hitPosition - rayStart);

			if (sqrDistance < rayHitSquareDistance)
			{
				rayHitSquareDistance = sqrDistance;

				// Note(Leo): This is only supporting axis-aligned colliders
				if (outResult != nullptr)
				{
					// Note(Leo): Find maximum since biggest af smallest is the distance of hit
					if (xDistanceToMin > yDistanceToMin && xDistanceToMin > zDistanceToMin)
					{
						outResult->hitNormal = {-Sign(colliderSpaceRayDirection.x), 0, 0};
					}
					else if (yDistanceToMin > zDistanceToMin)
					{
						outResult->hitNormal = {0, -Sign(colliderSpaceRayDirection.y), 0};
					}
					else
					{
						outResult->hitNormal = {0, 0, -Sign(colliderSpaceRayDirection.z)};
					}

					outResult->hitPosition 	= 	colliderSpaceRayStart
												+ colliderSpaceRayDirection
												* distanceToMin;

					outResult->hitPosition = multiply_point(collider.transform, outResult->hitPosition);

					outResult->hitNormal = multiply_direction(collider.transform, outResult->hitNormal);
					outResult->hitNormal = normalize_v3(outResult->hitNormal);//*= colliderSpaceRayLengthMultiplier;
				}

			}

			// f32 sqrDistance = square_magnitude_v3(outResult->hitPosition, rayStart);
			// if (sqr)
			// rayHitSquareDistance = 
			// // return true;
		}
	}

	for (auto const & collider : system->cylinderColliders)
	{
		/*
			test z position against height
			test xy distance againts radius
		*/

		v2 p = rayStart.xy;
		v2 d = normalizedRayDirection.xy;
		f32 dMagnitude = magnitude_v2(d);
		d = normalize_v2(d);

		v3 cPosition = collider.transform->position + collider.center;

		v2 toCircleCenter = cPosition.xy - p;

		if (dot_v2(d, toCircleCenter) < 0.0f)
		{
			continue;
		}

		v2 projectionToCircleCenter = d * dot_v2(d, toCircleCenter);
		f32 projectionLength		= dot_v2(d, toCircleCenter);

		v2 rejectionToCircleCenter 	= toCircleCenter - projectionToCircleCenter;
		f32 rejectionLength 		= magnitude_v2(rejectionToCircleCenter);

		// Note(Leo): sqrBDistance < 0 and xyFar < xyNear both rule that tangent is
		// counted as hit, we'd rathrer not, but it does not matter since it is so thin

		using namespace math;

		f32 sqrBDistance = pow2(collider.radius) - pow2(rejectionLength);
		if (sqrBDistance < 0)
		{
			continue;
		}	

		f32 bDistance = square_root(sqrBDistance);
		// f32 bDistance = square_root(pow2(collider.radius) - pow2(rejectionLength));

		f32 xyNear = (projectionLength - bDistance) / dMagnitude;
		f32 xyFar = (projectionLength + bDistance) / dMagnitude;

		if (xyFar < xyNear)
		{
			continue;
		}

		if(xyNear > rayLength)
		{
			continue;
		}

		// ---------------------------------

		f32 zMin = (cPosition.z - collider.height / 2 - rayStart.z) / normalizedRayDirection.z;
		f32 zMax = (cPosition.z + collider.height / 2 - rayStart.z) / normalizedRayDirection.z;

		f32 zNear, zFar;
		if(zMin < zMax)
		{
			zNear = zMin;
			zFar = zMax;
		}
		else
		{
			zNear = zMax;
			zFar = zMin;
		}

		// ----------------------------------

		f32 near = max(xyNear, zNear);
		f32 far  = min(xyFar, zFar);

		if (far < near)
		{
			continue;
		}

		if (near > rayLength)
		{
			continue;
		}


		v2 hitNormal2 	= -rejectionToCircleCenter + (d * -bDistance);
		v3 hitNormal 	= {hitNormal2.x, hitNormal2.y, 0};

		v3 hitPosition = cPosition + hitNormal;
		f32 sqrDistance = square_magnitude_v3(hitPosition - rayStart);
		if (sqrDistance < rayHitSquareDistance)
		{
			rayHitSquareDistance = sqrDistance;
			
			outResult->hitPosition = cPosition + hitNormal;
			outResult->hitNormal = normalize_v3(hitNormal);
		}

		hit = true;
		// return true;
	}

	return hit;
	// return false;
}