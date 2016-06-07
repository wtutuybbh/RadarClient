varying vec2 tc;
varying vec4 c;
varying vec2 useTex;

uniform sampler2D tex;


void main(void)
{
	vec4 tmp;
	gl_FragColor = mix(c, texture2D(tex, tc), useTex.x);
}
