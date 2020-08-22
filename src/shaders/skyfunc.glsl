layout(set = SKY_GRADIENT_SET, binding = 0) uniform sampler2D skyGradients[2];

layout (set = LIGHTING_SET, binding = 0) uniform Lighting
{
	vec4 direction;
	vec4 IGNORED_color;
	vec4 ambient;
	vec4 cameraPosition;
	float skyColourSelection;
} light;


vec3 compute_sky_color(vec3 normal, vec3 lightDir)
{
	float d = dot(lightDir, normal);

	d = (d + 1) / 2;
	d = max(0, d);

	vec2 uv = vec2(d, 0);

	vec3 colorA = texture(skyGradients[0], uv).rgb;
	vec3 colorB = texture(skyGradients[1], uv).rgb;

	vec3 color = mix(colorA, colorB, light.skyColourSelection);
	return color;
}