varying vec4 c;

void main(void)
{
    gl_FragColor = c;
	gl_FragColor.a = 0.5;
}