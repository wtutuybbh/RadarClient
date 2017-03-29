varying vec2 tc;
varying vec4 c;
varying vec4 c2;
varying vec2 useTex;

uniform sampler2D tex;


void main(void)
{
	gl_FragColor = mix(mix(c, texture2D(tex, tc), useTex.x), c2, useTex.y);
}
