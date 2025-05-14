#version 430

layout (location=0) in vec3 position;

uniform mat4 mv_matrix;
uniform mat4 p_matrix;

uniform bool isLine;
uniform bool isRow;
uniform bool isAxes;

uniform mat4 shadowMVP;

float offsetAmount = 1.0f;
int row, col;

void main(void)
{
    row = gl_InstanceID % 21;
    col = gl_InstanceID % 21;

    vec3 offsetPosition = position;
    
    if (isLine)
    {
        offsetPosition.x -= float(col) * offsetAmount;

        gl_Position = shadowMVP * vec4(offsetPosition, 1.0);
    }
    else if (isRow)
    {
        offsetPosition.z += float(row) * offsetAmount;

        gl_Position = shadowMVP * vec4(offsetPosition, 1.0);
    }
    else
    {
        gl_Position = shadowMVP * vec4(position, 1.0);
    }
}

