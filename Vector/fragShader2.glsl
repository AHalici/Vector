#version 430

in vec3 varyingNormal, varyingLightDir, varyingVertPos, varyingHalfVec;
in vec4 shadow_coord;
out vec4 fragColor;
in vec2 tc;
 
struct PositionalLight
{	vec4 ambient, diffuse, specular;
	vec3 position;
	vec3 direction;
	float cutoffAngle;
	float exponent;
};

struct DirectionalLight
{	vec4 ambient, diffuse, specular;
	vec3 direction;
};

struct Material
{	vec4 ambient, diffuse, specular;
	float shininess;
};

uniform vec4 globalAmbient;
uniform PositionalLight light;
uniform DirectionalLight sunLight;
uniform Material material;
uniform mat4 m_matrix;
uniform mat4 v_matrix; 
uniform mat4 p_matrix;
uniform mat4 norm_matrix;
uniform mat4 shadowMVP;

layout (binding=0) uniform sampler2DShadow spotlightShadowTex;
layout (binding=1) uniform sampler2DShadow sunShadowTex;
layout (binding=2) uniform sampler2D samp;

uniform bool isLine;
uniform bool isRow;
uniform bool isAxes;
uniform bool isCube;

float shadowfactor = 0.0;
vec4 lightedColor;

float currentDepth = shadow_coord.z / shadow_coord.w;  // Current pixel depth
// We use textureProj instead of lookup because lookup makes our x and y values very small and adds a small offset for PCF
float shadowMapDepth = textureProj(spotlightShadowTex, shadow_coord);

// Attenuation (constant, linear, quadratic)
float kc = 1.0;
float kl = 0.009;
float kq = 0.00032;

//layout(std430, binding=2) buffer BooleanBuffer 
//{
//	int booleanValue;
//};

// texture
vec4 texColor = texture(samp, tc);

float lookup(float x, float y)
{  	float t = textureProj(spotlightShadowTex, shadow_coord + vec4(x * 0.001 * shadow_coord.w,
														 y * 0.001 * shadow_coord.w,
														 0.00, 0.0));
	return t;
}

// Visualize different ranges with color coding
void debugValueWithColor(float value) {
    if (value < 0.0) {
        fragColor = vec4(1.0, 0.0, 0.0, 1.0); // Red for negative
    } else if (value < 0.5) {
        fragColor = vec4(0.0, 1.0, 0.0, 1.0); // Green for 0-0.5
    } else if (value < 1.0) {
        fragColor = vec4(0.0, 0.0, 1.0, 1.0); // Blue for 0.5-1.0
    } else if (value > 8.0) {
        fragColor = vec4(1.0, 1.0, 0.0, 1.0); // Yellow for > 1.0
    }
    return;
}

void main(void)
{
	//booleanValue = 0;

	vec3 L = normalize(varyingLightDir);
	vec3 N = normalize(varyingNormal);
	// Accesses the 4th row of the view matrix which is the translation column which has our negated camera position in world space
	vec3 V = normalize(-v_matrix[3].xyz - varyingVertPos);
	vec3 H = normalize(varyingHalfVec);
	vec3 D = normalize(-light.direction);

	// Spotlight Variables
	float theta = cos(radians(light.cutoffAngle));
	float phi = dot(D, L); // This gives the cos between D and L, not the angle itself, so we don't need to get the cos in intensity factor

	float intensityFactor = 0.0;

	if (phi >= theta)
		intensityFactor = pow(phi, light.exponent);
	else
		intensityFactor = 0.0;

	// Attenuation
	vec3 distanceVector = varyingVertPos - light.position;
	// Have to get the scalar value from the vector because that is all we want.
	// We couldn't use it in our attenuation formula because we would be adding a scalar (eg. kc) to a vector
	float distance = length(distanceVector);
	float attenuation = 1.0 / (kc + kl * distance + kq * distance * distance);
	

	// PCF
	float swidth = 2.5;
	vec2 o = mod(floor(gl_FragCoord.xy), 2.0) * swidth;
	shadowfactor += lookup(-1.5*swidth + o.x,  1.5*swidth - o.y);
	shadowfactor += lookup(-1.5*swidth + o.x, -0.5*swidth - o.y);
	shadowfactor += lookup( 0.5*swidth + o.x,  1.5*swidth - o.y);
	shadowfactor += lookup( 0.5*swidth + o.x, -0.5*swidth - o.y);
	shadowfactor = shadowfactor / 4.0;

	// Precise PCF
	float width = 2.5;
	float endp = width * 3.0 + width/2.0;
	for (float m=-endp ; m<=endp ; m=m+width)
	{	for (float n=-endp ; n<=endp ; n=n+width)
		{	shadowfactor += lookup(m,n);
	}	}
	shadowfactor = shadowfactor / 64.0;
	
	vec4 shadowColor;
	if (isCube)
	{
		shadowColor = texColor * (globalAmbient + light.ambient * attenuation);
	}
	else
	{
		shadowColor = globalAmbient * material.ambient + light.ambient * material.ambient * attenuation;
	}


	// Sunlight (Directional)
	float sunShadowFactor = textureProj(sunShadowTex, shadow_coord);

	vec3 sunDir = normalize(-sunLight.direction);
	vec4 sunContribution = sunLight.diffuse * material.diffuse * max(dot(sunDir, N), 0.0);

	// Light Max Distance 
	if (distance < 8.0f)
	{
		if (isCube)
		{
			lightedColor = light.diffuse * max(dot(L,N),0.0) // Removed to remove the circle of light at center
			+ light.specular;

			lightedColor =  texColor * ((20 * lightedColor) * (attenuation)); // Multiplying by 15 to increase
		}
		else
		{
			lightedColor = light.diffuse * material.diffuse * max(dot(L,N),0.0) // Removed to remove the circle of light at center
			+ light.specular * material.specular
			* pow(max(dot(H,N),0.0),material.shininess);

			lightedColor =  ((20 * lightedColor) * (attenuation)); // Multiplying by 15 to increase
		}
	}

	
	fragColor = vec4((shadowColor.xyz + intensityFactor * shadowfactor * lightedColor.xyz + sunContribution.xyz),1.0);

//	if (isCube)
//	{
//		// If the object is in shadow or the intensity factor <= 0 (not in the cutoff angle of spotlight) or distance is > 8, vertex is in shadow
//		if (currentDepth > shadowMapDepth + 0.005 || intensityFactor <= 0.0 || distance > 8.0f) 
//		{
//			booleanValue = 1;
//
//		}
//		else 
//		{
//			booleanValue = 0; 
//		}
//	}

}