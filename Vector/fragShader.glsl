#version 430

out vec4 color;

uniform mat4 mv_matrix;
uniform mat4 p_matrix;

uniform bool isLine;
uniform bool isRow;

void main(void)
{
	if (isLine || isRow)
	{
		color = vec4(0.0, 0.0, 1.0, 1.0);
	}
	else
	{
		color = vec4(1.0, 0.0, 0.0, 1.0);
	}
}