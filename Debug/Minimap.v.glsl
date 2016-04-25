attribute vec4 vertex;
attribute vec2 texcoor;

varying vec2 tc;

void main(void)
{
	gl_Position = vertex;
	tc = texcoor;
}