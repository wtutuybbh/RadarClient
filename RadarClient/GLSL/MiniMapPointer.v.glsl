attribute vec4 vertex;
attribute vec2 texcoor;

uniform mat4 mvp;

varying vec2 tc;

void main(void)
{
	gl_Position = mvp * vertex;
	tc = texcoor;
}