attribute vec4 vertex;
attribute vec4 color;
attribute vec2 texcoor;

uniform mat4 mvp;
uniform int useTexture;
uniform int useY0;
uniform float y0;
uniform vec4 tracks[32];

varying vec2 tc;
varying vec4 c;
varying vec2 useTex;

void main(void)
{	
	useTex = vec2(useTexture, 0);
	tc = texcoor;
	c = color;
	/*for(int i; i<31; i++) 
	{
		if (tracks[i+1].w != 0) 
		{
			if (abs((vertex.z - tracks[i].z) / (tracks[i+1].z - tracks[i].z)) - abs((vertex.x - tracks[i].x) / (tracks[i+1].x - tracks[i].x)) < 0.0001) 
			{
				c = vec4(1, 1, 1, 1);
			}
		}
	}*/
	gl_Position = mix(mvp * vertex, mvp * vec4(vertex.x, y0, vertex.z, 1), useY0);	
}
