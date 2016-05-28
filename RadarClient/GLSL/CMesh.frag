varying vec2 tc;
varying vec4 c;
flat varying int useTex;

uniform sampler2D tex;


void main(void)
{
	vec4 tmp;
	if (useTex == 1) {
		tmp = texture2D(tex, tc);
	}
	if (useTex == 0) {
		tmp = c;
	}
	gl_FragColor = tmp;
}
