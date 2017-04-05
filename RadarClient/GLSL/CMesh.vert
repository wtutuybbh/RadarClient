attribute vec4 vertex;
attribute vec4 color;
attribute vec4 color2;
attribute vec2 texcoor;

uniform mat4 mvp;
uniform int useTexture;
uniform int useBlind;
uniform int useY0;
uniform float y_0;
uniform vec4 blindColor0;
uniform vec4 blindColor1;
uniform float blind_zone_height;

varying vec2 tc;
varying vec4 c;
varying vec4 c2;
varying vec2 useTex;

void main(void)
{	
	useTex = vec2(useTexture, useBlind);
	tc = texcoor;
	c = color;
	c2 = color2;
	gl_Position = mix(mvp * vertex, mvp * vec4(vertex.x, y_0, vertex.z, 1), useY0);	
}
