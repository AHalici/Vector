#version 430

out vec4 color;

uniform mat4 mv_matrix;
uniform mat4 p_matrix;

uniform bool isLine;
uniform bool isRow;
uniform bool isAxes;

void main(void)
{
	if (isLine)
	{
		color = vec4(0.2, 0.2, 0.2, 1.0);
	}
	else if (isRow)
	{
		color = vec4(0.2, 0.2, 0.2, 1.0);
	}
	else if (isAxes)
	{
		color = vec4(1.0, 0.0, 0.0, 1.0);
	}
	else
	{
		color = vec4(0.3, 0.3, 0.3, 1.0);
	}
}