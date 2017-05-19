varying vec2 tc;

uniform sampler2D tex;
uniform float alpha;

void main(void)
{
	gl_FragColor = texture2D(tex, tc);
	gl_FragColor.a = alpha;
}