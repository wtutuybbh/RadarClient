attribute vec4 vertex;
attribute vec4 color;

uniform mat4 mvp;
uniform float pointSize;

varying vec4 c;

void main(void) {
	gl_Position = mvp * vertex;
	gl_PointSize = pointSize;
	c = color;
}