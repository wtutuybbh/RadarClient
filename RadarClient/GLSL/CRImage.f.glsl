varying vec2 tc;
uniform sampler2D tex;
void main(void)
{
	gl_FragColor = texture2D(tex, tc);
}