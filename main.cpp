////////////////////////////////////////////////////////////////////////////////
// \author   Jonathan Dupuy
//
////////////////////////////////////////////////////////////////////////////////

// gui
#define _ANT_ENABLE

// GL libraries
#include "glew.hpp"
#include "GL/freeglut.h"

#ifdef _ANT_ENABLE
#	include "AntTweakBar.h"
#endif // _ANT_ENABLE

// Custom libraries
#include "Algebra.hpp"      // Basic algebra library
#include "Transform.hpp"    // Basic transformations
#include "Framework.hpp"    // utility classes/functions

// Standard librabries
#include <iostream>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <cmath>


////////////////////////////////////////////////////////////////////////////////
// Global variables
//
////////////////////////////////////////////////////////////////////////////////

// Constants
const float PI   = 3.14159265;
const float FOVY = PI*0.5f;
const std::string cubemaps[6] = {"interstellar_ft.tga",
                                 "interstellar_bk.tga",
                                 "interstellar_up.tga",
                                 "interstellar_dn.tga",
                                 "interstellar_rt.tga",
                                 "interstellar_lf.tga"};

enum {
	// buffers
	BUFFER1 = 0,
	BUFFER_COUNT,

	// vertex arrays
	VERTEX_ARRAY_EMPTY = 0,
	VERTEX_ARRAY_COUNT,

	// samplers
	SAMPLER_ONE = 0,
	SAMPLER_COUNT,

	// textures
	TEXTURE_SKYBOX = 0,
	TEXTURE_COUNT,

	// programs
	PROGRAM_SKYBOX = 0,
	PROGRAM_COUNT
};

// OpenGL objects
GLuint *buffers      = NULL;
GLuint *vertexArrays = NULL;
GLuint *textures     = NULL;
GLuint *samplers     = NULL;
GLuint *programs     = NULL;

bool mouseLeft  = false;
bool mouseRight = false;
GLfloat deltaTicks = 0.0f;
GLfloat theta = 90.00f; // camera polar angle
GLfloat phi   = 0; // camera azimuthal angle
GLfloat radius     = 2;

#ifdef _ANT_ENABLE
GLfloat speed = 0.0f; // app speed (in ms)
#endif

////////////////////////////////////////////////////////////////////////////////
// Functions
//
////////////////////////////////////////////////////////////////////////////////

#ifdef _ANT_ENABLE

static void TW_CALL toggle_fullscreen(void *data) {
	glutFullScreenToggle();
}

#endif


////////////////////////////////////////////////////////////////////////////////
// on init cb
void on_init() {
	// alloc names
	buffers      = new GLuint[BUFFER_COUNT];
	vertexArrays = new GLuint[VERTEX_ARRAY_COUNT];
	textures     = new GLuint[TEXTURE_COUNT];
	samplers     = new GLuint[SAMPLER_COUNT];
	programs     = new GLuint[PROGRAM_COUNT];

	fw::init_debug_output(std::cout);

	// gen names
	glGenBuffers(BUFFER_COUNT, buffers);
	glGenVertexArrays(VERTEX_ARRAY_COUNT, vertexArrays);
	glGenTextures(TEXTURE_COUNT, textures);
	glGenSamplers(SAMPLER_COUNT, samplers);
	for(GLuint i=0; i<PROGRAM_COUNT;++i)
		programs[i] = glCreateProgram();

	glActiveTexture(GL_TEXTURE0+TEXTURE_SKYBOX);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textures[TEXTURE_SKYBOX]);
		fw::tex_tga_cube_map(cubemaps, GL_TRUE, GL_FALSE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP,
		                GL_TEXTURE_MIN_FILTER,
		                GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindVertexArray(vertexArrays[VERTEX_ARRAY_EMPTY]);
	glBindVertexArray(0);


	fw::build_glsl_program(programs[PROGRAM_SKYBOX],
	                       "skybox.glsl",
	                       "",
	                       GL_TRUE);

	glProgramUniform1i(programs[PROGRAM_SKYBOX],
	                   glGetUniformLocation(programs[PROGRAM_SKYBOX],
	                                        "sSky"),
	                   TEXTURE_SKYBOX);

	glClearColor(0.1,0.1,0.1,1);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

#ifdef _ANT_ENABLE
	// start ant
	TwInit(TW_OPENGL_CORE, NULL);
	// send the ''glutGetModifers'' function pointer to AntTweakBar
	TwGLUTModifiersFunc(glutGetModifiers);

	// Create a new bar
	TwBar* menuBar = TwNewBar("menu");
	TwDefine("menu size='220 170'");
	TwDefine("menu position='0 0'");
	TwDefine("menu alpha='255'");
	TwDefine("menu valueswidth=85");

	TwAddVarRO(menuBar,
	           "speed (ms)",
	           TW_TYPE_FLOAT,
	           &speed,
	           "");

	TwAddButton( menuBar,
	             "fullscreen",
	             &toggle_fullscreen,
	             NULL,
	             "label='toggle fullscreen'");

#endif // _ANT_ENABLE
	fw::check_gl_error();
}


////////////////////////////////////////////////////////////////////////////////
// on clean cb
void on_clean() {
	// delete objects
	glDeleteBuffers(BUFFER_COUNT, buffers);
	glDeleteVertexArrays(VERTEX_ARRAY_COUNT, vertexArrays);
	glDeleteTextures(TEXTURE_COUNT, textures);
	glDeleteSamplers(SAMPLER_COUNT, samplers);
	for(GLuint i=0; i<PROGRAM_COUNT;++i)
		glDeleteProgram(programs[i]);

	// release memory
	delete[] buffers;
	delete[] vertexArrays;
	delete[] textures;
	delete[] samplers;
	delete[] programs;

#ifdef _ANT_ENABLE
	TwTerminate();
#endif // _ANT_ENABLE

	fw::check_gl_error();
}


////////////////////////////////////////////////////////////////////////////////
// on update cb
void on_update() {
	// Variables
	static fw::Timer deltaTimer;
	GLint windowWidth  = glutGet(GLUT_WINDOW_WIDTH);
	GLint windowHeight = glutGet(GLUT_WINDOW_HEIGHT);
	GLfloat aspect = GLfloat(windowWidth)/GLfloat(windowHeight);

	// stop timing and set delta
	deltaTimer.Stop();
	deltaTicks = deltaTimer.Ticks();
#ifdef _ANT_ENABLE
	speed = deltaTicks*1000.0f;
#endif

	float thetaR = PI*0.5f - theta * PI / 180.0f;
	float phiR   = phi   * PI / 180.0f;
	Affine objectAxis;
	objectAxis.RotateAboutWorldY(phiR);
	objectAxis.RotateAboutWorldX(thetaR);
	objectAxis.TranslateWorld(Vector3(0,0,-radius));
	Matrix4x4 mvp = Matrix4x4::Perspective(FOVY,aspect*0.5f,0.05f,1000.0f)
	              * objectAxis.ExtractTransformMatrix();

	glProgramUniformMatrix4fv(programs[PROGRAM_SKYBOX],
		glGetUniformLocation(programs[PROGRAM_SKYBOX],
		                     "uModelViewProjection"),
		                      1,
		                      GL_FALSE,
		                      &mvp[0][0]);

	// set viewport
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0,0,windowWidth, windowHeight);

	glBindVertexArray(vertexArrays[VERTEX_ARRAY_EMPTY]);
	glUseProgram(programs[PROGRAM_SKYBOX]);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 14);

	// start ticking
	deltaTimer.Start();

#ifdef _ANT_ENABLE
//	TwDraw();
#endif // _ANT_ENABLE

	fw::check_gl_error();

	glutSwapBuffers();
	glutPostRedisplay();
}


////////////////////////////////////////////////////////////////////////////////
// on resize cb
void on_resize(GLint w, GLint h) {
#ifdef _ANT_ENABLE
	TwWindowSize(w, h);
#endif
}


////////////////////////////////////////////////////////////////////////////////
// on key down cb
void on_key_down(GLubyte key, GLint x, GLint y) {
#ifdef _ANT_ENABLE
	if(1==TwEventKeyboardGLUT(key, x, y))
		return;
#endif
	if (key==27) // escape
		glutLeaveMainLoop();
	if(key=='f')
		glutFullScreenToggle();
	if(key=='p')
		fw::save_gl_front_buffer(0,
		                         0,
		                         glutGet(GLUT_WINDOW_WIDTH),
		                         glutGet(GLUT_WINDOW_HEIGHT));

}


////////////////////////////////////////////////////////////////////////////////
// on mouse button cb
void on_mouse_button(GLint button, GLint state, GLint x, GLint y) {
#ifdef _ANT_ENABLE
	if(1 == TwEventMouseButtonGLUT(button, state, x, y))
		return;
#endif // _ANT_ENABLE
	if(state==GLUT_DOWN) {
		mouseLeft  |= button == GLUT_LEFT_BUTTON;
		mouseRight |= button == GLUT_RIGHT_BUTTON;
	}
	else {
		mouseLeft  &= button == GLUT_LEFT_BUTTON ? false : mouseLeft;
		mouseRight  &= button == GLUT_RIGHT_BUTTON ? false : mouseRight;
	}
	if(button == 4) {
		radius += 0.5f;
	}
	if(button == 3) {
		radius = std::max(0.5f, radius-0.5f);
	}
}


////////////////////////////////////////////////////////////////////////////////
// on mouse motion cb
void on_mouse_motion(GLint x, GLint y) {
#ifdef _ANT_ENABLE
	TwEventMouseMotionGLUT(x,y);
#endif // _ANT_ENABLE

	static GLint sMousePreviousX = 0;
	static GLint sMousePreviousY = 0;
	const GLint MOUSE_XREL = x-sMousePreviousX;
	const GLint MOUSE_YREL = y-sMousePreviousY;
	sMousePreviousX = x;
	sMousePreviousY = y;

	if(mouseLeft) {
		phi   = fmod(phi+deltaTicks*MOUSE_XREL*400.0f, 360.0f);
		theta = std::min(std::max(0.001f,theta-deltaTicks*MOUSE_YREL*400.0f), 179.999f);
	}
}


////////////////////////////////////////////////////////////////////////////////
// on mouse wheel cb
void on_mouse_wheel(GLint wheel, GLint direction, GLint x, GLint y) {
#ifdef _ANT_ENABLE
	if(1 == TwMouseWheel(wheel))
		return;
#endif // _ANT_ENABLE
}


////////////////////////////////////////////////////////////////////////////////
// Main
//
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv) {
	const GLuint CONTEXT_MAJOR = 3;
	const GLuint CONTEXT_MINOR = 3;

	// init glut
	glutInit(&argc, argv);
	glutInitContextVersion(CONTEXT_MAJOR ,CONTEXT_MINOR);
	glutInitContextFlags(GLUT_DEBUG | GLUT_FORWARD_COMPATIBLE);
	glutInitContextProfile(GLUT_CORE_PROFILE);


	// build window
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("OpenGL");

	// init glew
	glewExperimental = GL_TRUE; // segfault on GenVertexArrays on Nvidia otherwise
	GLenum err = glewInit();
	if(GLEW_OK != err) {
		std::stringstream ss;
		ss << err;
		std::cerr << "glewInit() gave error " << ss.str() << std::endl;
		return 1;
	}

	// glewInit generates an INVALID_ENUM error for some reason...
	glGetError();

	// set callbacks
	glutCloseFunc(&on_clean);
	glutReshapeFunc(&on_resize);
	glutDisplayFunc(&on_update);
	glutKeyboardFunc(&on_key_down);
	glutMouseFunc(&on_mouse_button);
	glutPassiveMotionFunc(&on_mouse_motion);
	glutMotionFunc(&on_mouse_motion);
	glutMouseWheelFunc(&on_mouse_wheel);

	// run
	try {
		// run demo
		on_init();
		glutMainLoop();
	}
	catch(std::exception& e) {
		std::cerr << "Fatal exception: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}

