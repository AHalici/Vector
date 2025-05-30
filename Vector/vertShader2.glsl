#version 430

layout (location=0) in vec3 position;
layout (location=1) in vec3 vertNormal;

out vec3 varyingNormal, varyingLightDir, varyingVertPos, varyingHalfVec; 
out vec4 shadow_coord;

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
uniform DirectionalLight sunlight;
uniform Material material;
uniform mat4 m_matrix;
uniform mat4 v_matrix;
uniform mat4 p_matrix;
uniform mat4 norm_matrix;
uniform mat4 shadowMVP;

layout (binding=0) uniform sampler2DShadow spotlightShadowTex;
layout (binding=1) uniform sampler2DShadow sunShadowTex;

uniform bool isLine;
uniform bool isRow;
uniform bool isAxes;

// Grid Lines
float offsetAmount = 1.0f;
int row, col;

void main(void)
{
    row = gl_InstanceID % 21;
    col = gl_InstanceID % 21;

    vec3 offsetPosition = position;
    
    //output the vertex position to the rasterizer for interpolation
	varyingVertPos = (m_matrix * vec4(position,1.0)).xyz;

    //get a vector from the vertex to the light and output it to the rasterizer for interpolation
	varyingLightDir = light.position - varyingVertPos;

	//get a vertex normal vector in eye space and output it to the rasterizer for interpolation
	varyingNormal = (norm_matrix * vec4(vertNormal,1.0)).xyz;

	// calculate the half vector (L+V)
	varyingHalfVec = (varyingLightDir-varyingVertPos).xyz;

    if (isLine)
    {
        offsetPosition.x += float(col) * offsetAmount;
        shadow_coord = shadowMVP * vec4(offsetPosition, 1.0);
        gl_Position = p_matrix * v_matrix * m_matrix * vec4(offsetPosition, 1.0);
    }
    else if (isRow)
    {
        offsetPosition.z += float(row) * offsetAmount;
        shadow_coord = shadowMVP * vec4(offsetPosition, 1.0);
        gl_Position = p_matrix * v_matrix * m_matrix * vec4(offsetPosition, 1.0);
    }
    else
    {
        shadow_coord = shadowMVP * vec4(offsetPosition, 1.0);
        gl_Position = p_matrix * v_matrix * m_matrix * vec4(offsetPosition, 1.0);
    }
}

