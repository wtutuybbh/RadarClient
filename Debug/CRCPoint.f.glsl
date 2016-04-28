varying vec3 n;

uniform vec4 color;

void main(void)
{
    vec3 eyeSpaceLigthDirection = vec3(0.0,0.0,1.0);
	float diffuse = max(0.0,dot(normalize(n),eyeSpaceLigthDirection));
	gl_FragColor = vec4(color.xyz*diffuse,color.w);
	//gl_FragColor = vec4(1, 1, 1, 1);
}