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
	int halfId = 5 - gl_InstanceID >> 1; // substraction inverts x and z
//	float s    = (gl_InstanceID & 1)*2.0-1.0;
	float sx = (gl_InstanceID & 3) > 0 ? 1 : -1;
	float sy = gl_InstanceID == 2 ? -1 : 1;
	float sz = (gl_InstanceID==1 || 
	            gl_InstanceID==3 || 
	            gl_InstanceID==4) ? -1 : 1;
	vec3 pos = vec3(oTexCoord.st*2.0-1.0, -1);
	vec3 k; // "normal"
	k[0] = sx*pos[halfId & 2];
	k[1] = sy*pos[1 << (halfId & 1)];
	k[2] = sz*pos[2 >> halfId];
	gl_Position = uModelViewProjection * vec4(k,1);
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
//	oColour.rg = iTexCoord.st;
	oColour.b  = iTexCoord.p/6.0;
}
#endif


