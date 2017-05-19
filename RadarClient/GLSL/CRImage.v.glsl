attribute vec4 vertex;
attribute vec4 color;
attribute vec2 texcoor;

uniform mat4 mvp;
uniform float pointSize;


varying vec4 c;
varying vec2 tc;

void main(void) {
	gl_Position = mvp * vertex;
	gl_PointSize = pointSize;
	c = color;
	tc = texcoor;
}