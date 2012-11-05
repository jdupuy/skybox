#version 420


// --------------------------------------------------
// Uniforms
// --------------------------------------------------
uniform samplerCube sSky;
uniform mat4 uModelViewProjection;


// --------------------------------------------------
// Vertex shader
// --------------------------------------------------
#ifdef _VERTEX_
layout(location=0) out vec3 oTexCoord;

void main() {
	// extract vertices
	int r = int(gl_VertexID > 6);
	int i = r==1 ? 13-gl_VertexID : gl_VertexID;
	int x = int(i<3 || i==4);
	int y = r ^ int(i>0 && i<4);
	int z = r ^ int(i<2 || i>5);

	// compute world pos and project
	const float SKY_SIZE = 100.0;
	oTexCoord = vec3(x,y,z)*2.0-1.0;
	gl_Position = uModelViewProjection *
	              vec4(oTexCoord*SKY_SIZE,1);
}
#endif


// --------------------------------------------------
// Fragment shader
// --------------------------------------------------
#ifdef _FRAGMENT_
layout(location=0) in vec3 iTexCoord;
layout(location=0) out vec4 oColour;

void main() {
	oColour = texture(sSky, normalize(iTexCoord));
}
#endif


