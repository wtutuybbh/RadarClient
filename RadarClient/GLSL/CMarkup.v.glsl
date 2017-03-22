attribute vec4 vertex;
attribute vec4 color;
attribute vec3 norm;

uniform mat4 mvp;
uniform vec4 unicolor;
uniform vec4 unicolor2;

uniform float uniweight;

varying vec4 c;

void main(void)
{
	gl_Position = mvp * vertex;
	c = mix(unicolor, unicolor2, norm.z);
}