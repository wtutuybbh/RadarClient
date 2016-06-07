attribute vec4 vertex;
attribute vec4 color;
attribute vec2 texcoor;

uniform mat4 mvp;
uniform int useTexture;
uniform int useY0;
uniform float y0;

varying vec2 tc;
varying vec4 c;
varying vec2 useTex;

void main(void)
{	
	useTex = vec2(useTexture, 0);
	tc = texcoor;
	c = color;
	gl_Position = mix(mvp * vertex, mvp * vec4(vertex.x, y0, vertex.z, 1), useY0);	
}
