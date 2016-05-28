attribute vec4 vertex;
attribute vec4 color;
attribute vec2 texcoor;

uniform mat4 mvp;
uniform int useTexture;
uniform int y0;

varying vec2 tc;
varying vec4 c;
flat varying int useTex;

void main(void)
{	
	useTex = useTexture;
	tc = texcoor;
	c = color;
	if (y0==-1) {
		gl_Position = mvp * vertex;
	}
	else {
		gl_Position = mvp * vec4(vertex.x, y0, vertex.z, 1);
	}
}
