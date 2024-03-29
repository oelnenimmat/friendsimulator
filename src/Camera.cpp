struct Camera
{
	v3 position;
	// Todo(Leo): change to quaternion, maybe
	v3 direction = v3_forward;

	float verticalFieldOfView;
	float nearClipPlane;
	float farClipPlane;
	float aspectRatio;
};

internal v3 get_forward(Camera const*);
internal v3 get_up(Camera const *);
internal v3 get_right(Camera const *);

internal Camera
make_camera(float verticalFieldOfView, float nearClipPlane, float farClipPlane)
{
	return {
		.verticalFieldOfView 	= verticalFieldOfView,
		.nearClipPlane 			= nearClipPlane,
		.farClipPlane 			= farClipPlane,
	};
}

internal m44 camera_projection_matrix(Camera const * camera)
{
	/*
	Todo(Leo): Do studies, this really affects many things
	Study: https://www.scratchapixel.com/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix/building-basic-perspective-projection-matrix
	*/

	float canvasSize = Tan(to_radians(camera->verticalFieldOfView / 2.0f)) * camera->nearClipPlane;

	// Note(Leo): Vulkan NDC goes left-top = (-1, -1) to right-bottom = (1, 1)
	float bottom = canvasSize;
	float top = -bottom;

	float right = bottom * camera->aspectRatio;
	float left = -right;

    float
    	b = bottom,
    	t = top,
    	r = right,
    	l = left,

    	n = camera->nearClipPlane,
    	f = camera->farClipPlane;



    /* Study(Leo): why does this seem to use y-up and z-forward,
    when actual world coordinates are z-up and y-forward 
	
	update: this is projection matrix and operates in screen space (canonical space??) where xy are screen coordinates and z is depth
		this is achieved with view matrix, where z and y axises are flipped
    */
    // Todo(Leo): change axises here and in get_view_transform()
   	// Todo(Leo): minus sign on [1,1] may invalidly correlate to b - t being the wrong way
	m44 result = {
		2*n / (r - l), 		0, 					0, 							0,
		0, 					-2 * n / (b - t), 	0, 							0,
		(r + l)/(r - l), 	(b + t)/(b - t),	-((f + n)/(f -n)),			-1,
		0, 					0, 					-(2 * f * n / (f - n)), 	0
	};

	return result;	
}

internal m44 camera_view_matrix(v3 position, v3 direction)
{
	// Study: https://www.3dgep.com/understanding-the-view-matrix/
	// Todo(Leo): try to undeerstand why this is negative
	v3 yAxis 	= -direction;
	v3 xAxis 	= v3_normalize(v3_cross(v3_up, yAxis));

	/* Note(Leo): this is not normalized because both components are unit length,
	AND they are orthogonal so they produce a unit length vector anyway.
	They are surely orthogonal because xAxis was a cross product from yAxis(with v3_up) */
	v3 zAxis 	= v3_cross(yAxis, xAxis);

	m44 orientation = {
		xAxis.x, zAxis.x, yAxis.x, 0,
		xAxis.y, zAxis.y, yAxis.y, 0,
		xAxis.z, zAxis.z, yAxis.z, 0,
		0, 0, 0, 1
	};
	
	/* Todo(Leo): Translation can be done inline with matrix initialization
	instead of as separate function call */
	m44 translation = translation_matrix(-position);
	m44 result = orientation * translation;

	return result;	
}

v3 get_forward(Camera const * camera)
{
	Assert(abs_f32(v3_sqr_length(camera->direction) - 1.0f) < 0.00001f);
	return camera->direction;
}

v3 get_up(Camera const * camera)
{
	return v3_cross(get_right(camera), camera->direction);
}

v3 get_right(Camera const * camera)
{
	return v3_normalize(v3_cross(camera->direction, v3_up));
}