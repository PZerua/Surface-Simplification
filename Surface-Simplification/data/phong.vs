

uniform mat4 mvp;
uniform vec3 light;
uniform vec3 view;
uniform mat4 model;


varying vec4 vertex;
varying vec3 normal;

void main(void)
{
	vertex = model * gl_Vertex;
	normal = (model * vec4(gl_Normal, 0.0)).xyz;
	gl_Position = mvp * gl_Vertex;  

}