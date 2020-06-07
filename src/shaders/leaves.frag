#version 450

layout (set = 3, binding = 0) uniform Lighting
{
	vec4 direction;
	vec4 color;
	vec4 ambient;
} light;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec4 lightCoords;

layout(location = 0) out vec4 outColor;

layout(binding = 0, set = 4) uniform sampler2D lightMap;

const vec3 putple = vec3(0.62, 0.3, 0.8);
const vec3 bluish = vec3(0.3, 0.62, 0.8);
const vec3 green = vec3(0.52, 0.7, 0.40);
const vec3 pinkt = vec3(1.0, 0.7, 0.8);

const vec3 darkerGreen = vec3(0.42, 0.60, 0.35);
const vec3 lighterGreen = vec3(0.52, 0.75, 0.45);

void main()
{
	float distanceFromCenter 	= length(fragTexCoord.xy * 2);
	float opacity 				= step (distanceFromCenter, 1.0);

	const float a = 0.2;

	float x = fragTexCoord.x * 2;
	float y = fragTexCoord.y * 2;
	// float op2 = step (x, a * y + a);
	// float op3 = step (-a * y -a, x);

	float op2 = a * y + a;
	float op3 = -a * y -a;

	if (x > op3 && x < op2)
	{
		opacity = 0;
	}

	if (opacity < 0.5)
	{
		discard;
	}

	vec3 lightDir 	= -light.direction.xyz;
	vec3 normal 	= normalize(fragNormal);

	// Todo(Leo): Add translucency
	// float ldotn = dot(lightDir, normal);
	float ldotn = (dot(lightDir, normal));

	// ldotn = abs(ldotn);
	ldotn = max(0, ldotn);
	vec3 albedo = green;

	// albedo = mix(vec3(0,0,0), vec3(0,1,1), fragTexCoord.y + 0.5);
	albedo = mix(darkerGreen, lighterGreen, fragTexCoord.y + 0.5);

	float lightIntensity = ldotn;


	float lightDepthFromTexture = texture(lightMap, lightCoords.xy).r;

	const float shadowBias = 0.0001;
	float inLight = 1.0 - step(lightDepthFromTexture + shadowBias, lightCoords.z) * 0.8;

	// SHADOWS
	lightIntensity = lightIntensity * inLight;	

	// Note(Leo): try these two
	vec3 lightColor = mix(light.ambient.rgb, light.color.rgb, lightIntensity);
	// vec3 lightColor = light.ambient.rgb + light.color.rgb * lightIntensity;
	
	vec3 color = lightColor * albedo;

	// color *= step(distanceFromCenter, 1.0);
	outColor.rgb = color;



	outColor.a 					= 1;
}