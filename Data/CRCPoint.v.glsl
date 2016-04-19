attribute vec4 vertex;
attribute vec3 normal;

uniform mat4 mvp; // model-view-projection matrix
uniform mat3 norm; // normal matrix

varying vec3 n;

void main(void) {
	// compute position
	gl_Position = mvp * vertex;

	// compute light info
	n = normalize(norm * normal);
}