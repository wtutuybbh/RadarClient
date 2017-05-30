attribute vec4 vertex;
attribute vec4 color;
attribute vec3 norm;

uniform mat4 mvp;

uniform vec4 uniColor;
uniform float useUniColor;

varying vec4 c;

void main(void)
{
	gl_Position = mvp * vertex;
	c = mix(color, uniColor, useUniColor);
}