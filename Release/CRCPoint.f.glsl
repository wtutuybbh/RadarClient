varying vec3 n;

void main(void)
{
    vec3 eyeSpaceLigthDirection = vec3(0.0,0.0,1.0);
	float diffuse = max(0.0,dot(normalize(n),eyeSpaceLigthDirection));
	gl_FragColor = vec4(vec3(1, 0, 0)*diffuse,1.0);
	//gl_FragColor = vec4(1, 1, 1, 1);
}