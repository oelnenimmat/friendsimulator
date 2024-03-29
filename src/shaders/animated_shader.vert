#version 450

layout (set = 0, binding = 0) uniform CameraProjections 
{
	mat4 view;
	mat4 projection;
	mat4 lightViewProjection;
	float shadowDistance;
	float shadowTransitionDistance;
} camera;

layout(set = 2, binding = 0) uniform ModelData
{
	mat4 localToWorld;
	float isAnimated;
	mat4 bonesToLocal [32];
} model;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inTangent;
layout (location = 3) in vec2 inTexCoord;

layout (location = 4) in uvec4 inBoneIndices;
layout (location = 5) in vec4 inBoneWeights;

layout (location = 0) out vec3 fragColor;
layout (location = 1) out vec2 fragTexCoord;
layout (location = 2) out vec3 fragNormal;
layout (location = 3) out vec3 fragPosition;
layout (location = 4) out vec4 lightCoords;
layout (location = 5) out mat3 tbnMatrix;

// Todo(Leo): Move to uniform lighting section
const float shadowDistance = 90.0;
const float transitionDistance = 10.0;

void main ()
{
	/* Note(Leo): There have been unaddressed suspicions about this
	kind of linear matrix interpolation, but so far everything seems
	to work nicely. */
	mat4 poseMatrix =
		inBoneWeights[0] * model.bonesToLocal[inBoneIndices[0]] +
		inBoneWeights[1] * model.bonesToLocal[inBoneIndices[1]] +
		inBoneWeights[2] * model.bonesToLocal[inBoneIndices[2]] +
		inBoneWeights[3] * model.bonesToLocal[inBoneIndices[3]];

	float assertValue = abs(poseMatrix[3][3] - 1.0);
	if(assertValue > 0.00001)
	{
		// Do not animate if this happens.
		poseMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
	}
	// Note(Leo): there is apparantly w division happening in between vertex and
	// fragment shaders, so we do not need to do it ourselves
	// poseMatrix /= poseMatrix[3][3];



	// MetaNote(Leo): This note may be important, save it for later when we have time to study
	// Note(Leo): position coordinates must be divided with z-component after matrix multiplication
	vec4 posePosition = poseMatrix * vec4(inPosition, 1);
	vec4 poseNormal = poseMatrix * vec4(inNormal, 0);

	// world space tangent, normal and bitangent
	vec3 t = normalize((model.localToWorld * vec4(inTangent, 0)).xyz);
	vec3 n = normalize((model.localToWorld * poseNormal).xyz);
	vec3 b = normalize(cross(n, t));
	tbnMatrix = transpose(mat3(t, b, n));

	gl_Position = camera.projection * camera.view * model.localToWorld * posePosition;
	// vec4 poseNormal = vec4(inNormal, 0);
	fragNormal 		= normalize((transpose(inverse(model.localToWorld)) * poseNormal).xyz);
	
	lightCoords = camera.lightViewProjection * model.localToWorld * posePosition;//vec4(inPosition, 1.0);
	lightCoords.xy *= 0.5;
	lightCoords.xy -= 0.5;

	vec4 worldPosition = model.localToWorld * posePosition;

	fragPosition = worldPosition.xyz;

	float distance = length((camera.view * worldPosition).xyz);
	distance = distance - (shadowDistance - transitionDistance);
	distance = distance / transitionDistance;
	lightCoords.w = clamp (1.0 - distance, 0, 1);

	// fragColor = inColor;
	fragTexCoord = inTexCoord;
}

