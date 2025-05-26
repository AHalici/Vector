#version 430

in vec3 varyingNormal, varyingLightDir, varyingVertPos, varyingHalfVec;
in vec4 shadow_coord;
out vec4 fragColor;
 
struct PositionalLight
{	vec4 ambient, diffuse, specular;
	vec3 position;
	vec3 direction;
	float cutoffAngle;
	float exponent;
};

struct Material
{	vec4 ambient, diffuse, specular;
	float shininess;
};

uniform vec4 globalAmbient;
uniform PositionalLight light;
uniform Material material;
uniform mat4 m_matrix;
uniform mat4 v_matrix; 
uniform mat4 p_matrix;
uniform mat4 norm_matrix;
uniform mat4 shadowMVP;

layout (binding=0) uniform sampler2DShadow spotlightShadowTex;

uniform bool isLine;
uniform bool isRow;
uniform bool isAxes;

float shadowfactor = 0.0;

// Attenuation (constant, linear, quadratic)
float kc = 1.0;
float kl = 0.09;
float kq = 0.032;

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
	

	vec4 shadowColor = globalAmbient * material.ambient
				+ light.ambient * material.ambient * attenuation;
	
	vec4 lightedColor;

	// Light Max Distance 
	if (distance < 8.0f)
	{
		lightedColor = light.diffuse * material.diffuse * max(dot(L,N),0.0)
				+ light.specular * material.specular
				* pow(max(dot(H,N),0.0),material.shininess);

		lightedColor =  ((15 * lightedColor) * (attenuation)); // Multiplying by 15 to increase
	}

	fragColor = vec4((shadowColor.xyz + intensityFactor * shadowfactor * lightedColor.xyz),1.0);
}