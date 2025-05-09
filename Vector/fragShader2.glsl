#version 430

//out vec4 fragColor;

in vec3 varyingNormal, varyingLightDir, varyingVertPos, varyingHalfVec;
in vec4 shadow_coord;
out vec4 fragColor;
 
struct PositionalLight
{	vec4 ambient, diffuse, specular;
	vec3 position;
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

layout (binding=0) uniform sampler2DShadow shadowTex;

//uniform mat4 mv_matrix;
//uniform mat4 p_matrix;

uniform bool isLine;
uniform bool isRow;
uniform bool isAxes;

float lookup(float x, float y)
{  	float t = textureProj(shadowTex, shadow_coord + vec4(x * 0.001 * shadow_coord.w,
                                                         y * 0.001 * shadow_coord.w,
                                                         -0.01, 0.0));
	return t;
}

void main(void)
{

	//float shadowFactor = 0.0;

	vec3 L = normalize(varyingLightDir);
	vec3 N = normalize(varyingNormal);
	vec3 V = normalize(-v_matrix[3].xyz - varyingVertPos);
	vec3 H = normalize(varyingHalfVec);


	/*float swidth = 2.5;
	vec2 o = mod(floor(gl_FragCoord.xy), 2.0) * swidth;
	shadowFactor += lookup(-1.5*swidth + o.x,  1.5*swidth - o.y);
	shadowFactor += lookup(-1.5*swidth + o.x, -0.5*swidth - o.y);
	shadowFactor += lookup( 0.5*swidth + o.x,  1.5*swidth - o.y);
	shadowFactor += lookup( 0.5*swidth + o.x, -0.5*swidth - o.y);
	shadowFactor = shadowFactor / 4.0;*/


	vec4 shadowColor = globalAmbient * material.ambient
				+ light.ambient * material.ambient;

	vec4 lightedColor = light.diffuse * material.diffuse * max(dot(L,N),0.0)
				+ light.specular * material.specular
				* pow(max(dot(H,N),0.0),material.shininess*3.0);

	fragColor = vec4((shadowColor.xyz + /*shadowFactor**/(lightedColor.xyz)),1.0);

	/*if (isLine)
	{
		fragColor = vec4(0.0, 0.0, 1.0, 1.0);
	}
	else if (isRow)
	{
		fragColor = vec4(0.0, 1.0, 0.0, 1.0);
	}
	else if (isAxes)
	{
		fragColor = vec4(1.0, 0.0, 0.0, 1.0);
	}
	else
	{
		fragColor = vec4(0.3, 0.3, 0.3, 1.0);
	}*/
}