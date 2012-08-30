varying vec2 texCoords;

void main(void)
{
	// Transforming the vertex
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	// Passing the texture coordinate of texture unit 0 to the fragment shader
	texCoords = vec2(gl_MultiTexCoord0);
}
