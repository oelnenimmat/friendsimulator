struct Rectangle
{
	float2 position;
	float2 size;
};

internal Rectangle
scale_rectangle(Rectangle rect, float2 scale)
{
	float2 center = rect.position + rect.size / 2.0f;
	rect.size.x *= scale.x;
	rect.size.y *= scale.y;
	rect.position = center - rect.size / 2.0f;

	return rect;
}

struct Circle
{
	float2 position;
	real32 radius;
};

internal bool32
CircleCircleCollision(Circle a, Circle b)
{
	DEVELOPMENT_BAD_PATH("vector::get_sqr_distance() is not tested");

	real32 distanceThreshold = a.radius + b.radius;
	real32 sqrDistanceThreshold = distanceThreshold * distanceThreshold;

	real32 sqrDistance = vector::get_sqr_distance(a.position, b.position);

	bool32 result = sqrDistance < sqrDistanceThreshold;
	return result;
}

internal bool32
CircleRectangleCollisionAABB(Circle c, Rectangle r)
{
	bool32 isInsideAABB = 
		(c.position.x + c.radius) > r.position.x
		&& (c.position.y + c.radius) > r.position.y
		&& (c.position.x - c.radius) < (r.position.x + r.size.x)
		&& (c.position.y - c.radius) < (r.position.y + r.size.y);

	float sqrDistanceThreshold = c.radius * c.radius;

	return isInsideAABB;
}

struct CollisionResult
{
	bool32 isCollision;
	s32 otherColliderIndex;
};

internal CollisionResult
GetCollisions(Circle collider, ArenaArray<Circle> * otherColliders)
{
	for (int otherIndex = 0; otherIndex < otherColliders->count(); ++otherIndex)
	{
		if (CircleCircleCollision(collider, (*otherColliders)[otherIndex]))
		{
			return {true, otherIndex};
		}
	}
	return {false};
}

internal CollisionResult
GetCollisions(Circle collider, ArenaArray<Rectangle> * otherColliders)
{
	for (int otherIndex = 0; otherIndex < otherColliders->count(); ++otherIndex)
	{
		if (CircleRectangleCollisionAABB(collider, (*otherColliders)[otherIndex]))
		{
			return { true, otherIndex };
		}
	}

	return {false};
}