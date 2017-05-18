attribute vec4 vertex;
attribute vec4 color;

uniform mat4 mvp;
uniform float pointSize;
uniform vec4 uniColor;
uniform float useUniColor;
uniform float alpha;
varying vec4 c;

void main(void) {
	gl_Position = mvp * vertex;
	gl_PointSize = pointSize;
	c = mix(color, uniColor, useUniColor);
	c.a = alpha;
}