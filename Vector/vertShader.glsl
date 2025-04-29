#version 430

layout (location=0) in vec3 position;

uniform mat4 mv_matrix;
uniform mat4 p_matrix;

uniform bool isLine;
uniform bool isRow;

float offsetAmount = 0.1f;
int row, col;

void main(void)
{
    row = gl_InstanceID / 21;
    col = gl_InstanceID % 21;
    
    if (isLine)
    {
        vec3 offsetPosition = position;
        offsetPosition.x += float(col) * offsetAmount;

        
        gl_Position = p_matrix * mv_matrix * vec4(offsetPosition, 1.0);
    }
    else if (isRow)
    {
        vec3 offsetPosition = position;
      
        offsetPosition.x -= float(col) * offsetAmount;

        gl_Position = p_matrix * mv_matrix * vec4(offsetPosition, 1.0);
    }
    else
    {
        gl_Position = p_matrix * mv_matrix * vec4(position, 1.0);
    }
}