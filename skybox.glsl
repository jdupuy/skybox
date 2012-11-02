#version 420


// --------------------------------------------------
// Uniforms
// --------------------------------------------------
uniform samplerCube uEnvironment;
uniform mat4 uModelViewProjection;


// --------------------------------------------------
// Vertex shader
// --------------------------------------------------
#ifdef _VERTEX_
layout(location=0) out vec3 oTexCoord;

void main() {
	// texture coordinates
	oTexCoord = vec3(gl_VertexID & 1,
	                 gl_VertexID >> 1 & 1,
	                 gl_InstanceID);

	// position
	int halfId = (gl_InstanceID >> 1) % 2; // mod 2 inverts x and z
	float s    = (gl_InstanceID & 1)*2.0-1.0;
	vec3 pos = vec3(oTexCoord.st*2.0-1.0, 1).xzy;
	pos[0] = s*pos[halfId & 2];
	pos[1] = s*pos[1 << (halfId & 1)];
	pos[2] = s*pos[2 >> halfId & 2];
	gl_Position = uModelViewProjection * vec4(pos,1);
}
#endif


// --------------------------------------------------
// Fragment shader
// --------------------------------------------------
#ifdef _FRAGMENT_
layout(location=0) in vec3 iTexCoord;
layout(location=0) out vec4 oColour;

void main() {
	oColour = texture(uEnvironment, iTexCoord);
	oColour.rg = iTexCoord.st;
}
#endif


