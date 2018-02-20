#ifndef GL_C
#define GL_C

#define KCMD_ESCAPE   (1<<0)
#define KCMD_ENTER    (1<<1)
#define KCMD_TAB    (1<<2)
#define KCMD_BACKSPACE    (1<<3)
#define KCMD_INSERT    (1<<4)
#define KCMD_DELETE    (1<<5)
#define KCMD_RIGHT    (1<<6)
#define KCMD_LEFT    (1<<7)
#define KCMD_DOWN    (1<<8)
#define KCMD_UP    (1<<9)
#define KCMD_PAGE_UP    (1<<10)
#define KCMD_PAGE_DOWN    (1<<11)
#define KCMD_HOME    (1<<12)
#define KCMD_END    (1<<13)
#define KCMD_CAPS_LOCK    (1<<14)
#define KCMD_SCROLL_LOCK    (1<<15)
#define KCMD_NUM_LOCK    (1<<16)
#define KCMD_PRINT_SCREEN    (1<<17)
#define KCMD_PAUSE    (1<<18)
#define KCMD_LEFT_SHIFT   (1<<19)
#define KCMD_LEFT_CONTROL   (1<<20)
#define KCMD_LEFT_ALT   (1<<21)
#define KCMD_LEFT_SUPER   (1<<22)
#define KCMD_RIGHT_SHIFT   (1<<23)
#define KCMD_RIGHT_CONTROL   (1<<24)
#define KCMD_RIGHT_ALT   (1<<25)
#define KCMD_RIGHT_SUPER   (1<<26)
#define KCMD_MENU   (1<<27)



#define KALPHA_A (1<<0)
#define KALPHA_B (1<<1)
#define KALPHA_C (1<<2)
#define KALPHA_D (1<<3)
#define KALPHA_E (1<<4)
#define KALPHA_F (1<<5)
#define KALPHA_G (1<<6)
#define KALPHA_H (1<<7)
#define KALPHA_I (1<<8)
#define KALPHA_J (1<<9)
#define KALPHA_K (1<<10)
#define KALPHA_L (1<<11)
#define KALPHA_M (1<<12)
#define KALPHA_N (1<<13)
#define KALPHA_O (1<<14)
#define KALPHA_P (1<<15)
#define KALPHA_Q (1<<16)
#define KALPHA_R (1<<17)
#define KALPHA_S (1<<18)
#define KALPHA_T (1<<19)
#define KALPHA_U (1<<20)
#define KALPHA_V (1<<21)
#define KALPHA_W (1<<22)
#define KALPHA_X (1<<23)
#define KALPHA_Y (1<<24)
#define KALPHA_Z (1<<25)

#define KNUM_0 (1<<0)
#define KNUM_1 (1<<1)
#define KNUM_2 (1<<2)
#define KNUM_3 (1<<3)
#define KNUM_4 (1<<4)
#define KNUM_5 (1<<5)
#define KNUM_6 (1<<6)
#define KNUM_7 (1<<7)
#define KNUM_8 (1<<8)
#define KNUM_9 (1<<9)



#ifndef KEY_DEFS_ONLY

#define GLFW_INCLUDE_ES2
#include <GLFW/glfw3.h>

#define MAX_KEY_INPUT_BUF 64

typedef struct s_Window Window;
typedef struct s_Shader Shader;
typedef struct s_Texture Texture;
typedef struct s_FrameBuffer FrameBuffer;

struct s_Window {
	GLFWwindow *window;
	GLFWwindow *w_window;
	GLFWwindow *fs_window;
	int frame;
	
// Screen
	int w, h;
	int fs;

// Cursor
	struct {
		int hx, hy;
		int btn;
		int edge;
		int click, release;
		float x,y;
		int sx, sy;
		int sx0, sy0;
	} m;
	int scroll;
	int clicks[MAX_KEY_INPUT_BUF];
	int releases[MAX_KEY_INPUT_BUF];
	int releases_i;
	int clicks_i;

// Keyboard Input
	char input_chars[MAX_KEY_INPUT_BUF];
	int input_i;
	uint32_t alpha;
	uint32_t num;
	uint32_t function;
	uint32_t cmd;	
	uint32_t mod;
	
// Render State
	GLfloat color[4];
	GLfloat camx, camy;
	GLfloat camdx, camdy;
	GLfloat zoomx, zoomy;
	GLfloat vmat[3][3];

	
};

extern Window GW;


/** ========== Shaders =============
 */
#define MAX_SHADER_ARGS 32

struct s_Shader {
	GLuint id;
	GLuint argc;
	GLuint args[MAX_SHADER_ARGS];
};

int shader_init(Shader *self, const char *vert_src, const char *frag_src, char *args[]);
void shader_fini(Shader *self);
void shader_on_exit(int status, void *arg);


/** ========== Textures =============
 */
struct s_Texture {
	GLuint id;
	int w, h;
	GLenum format;
	GLenum type;
};

void tex_set(Texture *self, void * pixels);
void tex_on_exit(int status, void *arg);
void tex_fini(Texture *self);

/** ========== Frame Buffers =============
 */

struct s_FrameBuffer {
	GLuint id;
	Texture tex;
};

int framebuffer_init(FrameBuffer *self, int w, int h);
void framebuffer_fini(FrameBuffer *self);
void framebuffer_on_exit(int status, void *arg);

/** ==========Misc =============
 */

void gl_error_check(void);
double time(void);
char key_pop(void);

/** ========== Drawing =============
 */
#ifdef GL_DRAWING
void draw_color(float r, float g, float b, float a);
void draw_line_strip(GLfloat xy[2], GLfloat scale[2], GLfloat angle, int npts, GLfloat *pts);
void draw_line_loop(GLfloat xy[2], GLfloat scale[2], GLfloat angle, int npts, GLfloat *pts);
void draw_lines(GLfloat xy[2], GLfloat scale[2], GLfloat angle, int npts, GLfloat *pts);
void draw_polygon(GLfloat xy[2], GLfloat scale[2], GLfloat angle, int npts, GLfloat *pts);
void draw_circle(GLfloat xy[2], GLfloat scale[2]);
void draw_rect(GLuint aPos, GLfloat xy_ll[2], GLfloat xy_ur[2]);


#endif

#if __INCLUDE_LEVEL__ == 0

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <math.h>
#include <string.h>

#include "logging.c"
//~ #include "shaders.h"

Window GW;

extern int gl_frame(void);
extern void gl_init(void);
extern int main_init(int argc, char *argv[]);

extern const char *gl_name;

void gl_error_check(void)
{
	int err;
	ASSERT( !(err=glGetError()),  "GL Error %d", err);
}

void tex_set(Texture *self, void * pixels )
{
	if (!self->id) {// create a new texture
		glGenTextures(1, &self->id);
		XINFO("Create texture %d", self->id);
		glBindTexture(GL_TEXTURE_2D, self->id);
		glTexImage2D(GL_TEXTURE_2D, 0, self->format, self->w, self->h, 0, self->format, self->type,  pixels);
		gl_error_check();

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	} else {
		glBindTexture(GL_TEXTURE_2D, self->id);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, self->w, self->h, self->format, self->type,  pixels);
		
		
	}//~ glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // only power of 2 textures can be wrapped 
		//~ glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // only power of 2 textures can be wrapped 

	
}

void tex_fini(Texture *self)
{
	if (self->id) {
		XINFO("Unbind texture %d", self->id);
		glDeleteTextures(1, &self->id);
	}
	self->id = 0;
}

void tex_on_exit(int status, void *arg)
{
	tex_fini((Texture*)arg);
}


int glsl_check(
	void (*get_param)(GLuint,  GLenum,  GLint *),
	void (*get_log)(GLuint,  GLsizei,  GLsizei *,  GLchar *),
	GLuint prog,
	GLuint param)
{
	GLint status;
	get_param(prog, param, &status);
	if (!status) {
		GLint len = 0;
		get_param(prog, GL_INFO_LOG_LENGTH, &len);
		if ( len > 0 ) {
			char* log = alloca(sizeof(char) * len);
			get_log(prog, len, NULL, log);
			fprintf(stderr, "Compile Error : %d : %s\n", prog, log);
		}
		return -1;
	}
	return 0;
}

void shader_fini(Shader *self)
{
	if (self->id){
		XINFO("Program deleted %d", self->id);
		glDeleteProgram(self->id);
	}
	self->id = 0;
}

void shader_on_exit(int status, void *arg)
{
	shader_fini((Shader*)arg);
}

int shader_init(Shader *self, const char *vert_src, const char *frag_src, char *args[])
{
	self->id = 0;
	self->argc = -1;
	while (args[++self->argc]);
	
	//~ Shader *prog = malloc(sizeof(Shader) + sizeof(GLuint)*argc);
	//~ prog->argc = argc;
	DBG_ASSERT (self->argc <= MAX_SHADER_ARGS, "%d args MAX (%d given)",MAX_SHADER_ARGS, self->argc);

	GLuint vert_shader, frag_shader;
	// Compile Vertex Shader
	vert_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vert_shader, 1, &vert_src, NULL);
	glCompileShader(vert_shader);
	if(glsl_check(glGetShaderiv,  glGetShaderInfoLog, vert_shader, GL_COMPILE_STATUS))
		return 0;
	
	// Compile Fragment Shader
	frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(frag_shader, 1, &frag_src, NULL);
	glCompileShader(frag_shader);
	if(glsl_check(glGetShaderiv,  glGetShaderInfoLog, frag_shader, GL_COMPILE_STATUS)) {
		glDeleteShader(vert_shader);
		return 0;
	}
	
	// Link the program
	self->id = glCreateProgram();
	glAttachShader(self->id, vert_shader);
	glAttachShader(self->id, frag_shader);
	glLinkProgram(self->id);
	if(glsl_check(glGetProgramiv,  glGetProgramInfoLog, self->id, GL_LINK_STATUS)) {
		glDeleteShader(vert_shader);
		glDeleteShader(frag_shader);
		shader_fini(self);
		return 0;
	}	
	// figure out program locations from arguments
	for (int a =0; a < self->argc; ++a) {
		switch (args[a][0]) {
			case 'a':
				self->args[a] = glGetAttribLocation(self->id, args[a]);
				//~ glEnableVertexAttribArray(prog->args[a]);
			break;
			case 'u': self->args[a] = glGetUniformLocation(self->id, args[a]); break;
			default: ABORT("Must be attribute or uniform: %s",args[a]); break;
		}
	}
	
	glDeleteShader(vert_shader); // flagged for deletion when program is deleted
	glDeleteShader(frag_shader); // flagged for deletion when program is deleted
	//~ glValidateProgram(prog->id);
	//~ if(glsl_check(glGetProgramiv,  glGetProgramInfoLog, prog->id, GL_VALIDATE_STATUS))
		//~ return NULL;

	XINFO("Program compiled %d", self->id);
	return 1;
}

int framebuffer_init(FrameBuffer *self, int w, int h)
{
	if (self->id) {// releaase the old framebuffer
		glDeleteFramebuffers(1, &self->id);
	} else { // this is the first time we are called
		XINFO("Create New Texture for framebuffer");
		self->tex.w = w;
		self->tex.h = h;
		self->tex.format = GL_RGB;
		self->tex.type = GL_UNSIGNED_BYTE;
		tex_set(&self->tex, NULL);
	}
	glGenFramebuffers(1, &self->id);
	glBindFramebuffer(GL_FRAMEBUFFER, self->id);
	XINFO("New FrameBuffer %d", self->id);
	//~ glGenTextures(1, &self->txid);
	//~ glBindTexture(GL_TEXTURE_2D, self->txid);
	//~ glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, self->w, self->h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		//~ printf("glTexImage2D : %d\n", glGetError());

	//~ glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//~ glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//~ glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // only power of 2 textures can be wrapped 
	//~ glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // only power of 2 textures can be wrapped 

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, self->tex.id, 0);
	ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer not ready %d", glCheckFramebufferStatus(GL_FRAMEBUFFER));
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // put the default framebuffer back
	return 1;
}

void framebuffer_fini(FrameBuffer *self)
{
	tex_fini(&self->tex);
	if (self->id) {
		XINFO("Delete fb %d", self->id);
		glDeleteFramebuffers(1, &self->id);
	}
	memset(self, 0, sizeof(FrameBuffer));
}

void framebuffer_on_exit(int status, void *arg)
{
	framebuffer_fini((FrameBuffer*)arg);
}



double time(void)
{
	return glfwGetTime ();
}


char key_pop(void)
{
	if (GW.input_i)
		return GW.input_chars[--GW.input_i];
	return 0;
}


void set_window(GLFWwindow *w);

GLFWmonitor* cur_monitor(void)
{
	int count, xpos, ypos;
	GLFWmonitor** monitors = glfwGetMonitors(&count);
	GLFWmonitor *monitor =glfwGetPrimaryMonitor() ;
	glfwGetWindowPos(GW.window, &xpos, &ypos);
	for (int m=0; m < count; ++m) {
		int mx, my;
		glfwGetMonitorPos(monitors[m], &mx, &my);
		const GLFWvidmode* mode = glfwGetVideoMode(monitors[m]);
		XINFO("MONITOR %s: %dx%d (%d, %d) %d", glfwGetMonitorName(monitors[m]), 
			mode->width,mode->height, mx,my, mode->refreshRate);
		int nmodes;
		const GLFWvidmode* modes = glfwGetVideoModes(monitors[m], &nmodes);
		for (int d=0; d < nmodes; ++d)
			XINFO ("\t%dx%d %d", modes[d].width, modes[d].height, modes[d].refreshRate);
		
		if (xpos > mx && ypos > my && xpos < mx+mode->width && ypos < my+mode->height)
			monitor = monitors[m];
	}
	return monitor;
}

void do_fullscreen(void)
{
	if (!GW.fs_window) { // we need to make a fullscreen window
		GLFWmonitor *monitor =cur_monitor();
		int count;
		//~ GLFWmonitor** monitors = glfwGetMonitors(&count);
		//~ for (int m=0; m < count; ++m)
			//~ DEBUG("MONITOR %d: %s ", m, glfwGetMonitorName(monitors[m]));
		
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		glfwWindowHint(GLFW_RED_BITS, mode->redBits);
		glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
		glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate); 
		XINFO("Make fs window %dx%d %d", mode->width, mode->height, mode->refreshRate);
		GW.fs_window = glfwCreateWindow(mode->width, mode->height, gl_name, monitor, GW.w_window);
		if (!GW.fs_window) {
			WARN("Failed to create a fs window!");
			return;
		}
	}
	if (GW.fs) {
		XINFO("go Windowed");
		set_window(GW.w_window);
		glfwDestroyWindow(GW.fs_window);
		GW.fs_window = 0;
	} else {
		XINFO("go FS");
		set_window(GW.fs_window);
		//glfwHideWindow(gw.w_window);
	}
	glfwShowWindow(GW.window);
	
	GW.fs = !GW.fs;
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	GW.m.hx = xpos;
	GW.m.hy = ypos;
	GW.m.sx = (xpos<0)?0 : (xpos >GW.w?GW.w:xpos);
	GW.m.sy = (ypos<0)?0 : (ypos>GW.h?GW.h:ypos);
	GW.m.x = (((xpos)-(GW.w/2.0))- (GW.camx+GW.camdx)/2.0)/GW.zoomx ;
	GW.m.y = -(ypos-(GW.h/2.0) + (GW.camy+GW.camdy)/2.0)/GW.zoomy ;	
}
void cursor_enter_callback(GLFWwindow* window, int entered)
{
	if (!entered) {
		GW.m.hx = -1;
		GW.m.hy = -1;
	}
}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (action == GLFW_PRESS) {
		if (!GW.m.btn) { //first press
			GW.m.sx0 = GW.m.sx;
			GW.m.sy0 = GW.m.sy;
		}
	} 
	if (action == GLFW_PRESS) {
		GW.clicks[GW.clicks_i++] = button+1;
		GW.m.btn |= (1<<button);
	}else {
		GW.releases[GW.releases_i++] = button+1;
		GW.m.btn &= ~(1<<button);
	}
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	GW.scroll = yoffset;
}
static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	float monitor_aspect = (16.0/9.0);
	INFO("Resize %d %d (%.3f)",width, height, (double)width/height - monitor_aspect);
	if (fabs((double)width /height - monitor_aspect) > 0.1) {
		//~ printf("BAD");
		if ((double) width/height > monitor_aspect)
			width = height*monitor_aspect;
		else
			height = width/monitor_aspect;
		if (width < 256) {
			width = 256;
			height = 144;
		}
		glfwSetWindowSize(window, width, height);
	} else {
		GW.w = width;
		GW.h = height;
		glViewport(0, 0, width, height);
	}
}

void character_callback(GLFWwindow* window, unsigned int codepoint)
{	
	if (codepoint > 0 && codepoint <='~' && GW.input_i < MAX_KEY_INPUT_BUF)
		GW.input_chars[GW.input_i++] = codepoint;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS && key == GLFW_KEY_ENTER && mods |GLFW_MOD_ALT ) {
		do_fullscreen();
		return;
	}

	if ((action == GLFW_PRESS || action == GLFW_REPEAT) && GW.input_i < MAX_KEY_INPUT_BUF) {
		switch (key) {
			case GLFW_KEY_ENTER: GW.input_chars[GW.input_i++] = '\n'; break;
			case GLFW_KEY_TAB: GW.input_chars[GW.input_i++] = '\t'; break;
			case GLFW_KEY_BACKSPACE: GW.input_chars[GW.input_i++] = '\b'; break;
			default:break;
		}
	}
	uint32_t *bitfield = 0;
	int bit = 0;
	if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z) {
		bit = key - GLFW_KEY_A;
		bitfield = &GW.alpha;
	} else if (key >= GLFW_KEY_F1 && key <= GLFW_KEY_F16) {
		bit = key - GLFW_KEY_F1;
		bitfield = &GW.function;
	} else if (key >= GLFW_KEY_0   && key <= GLFW_KEY_9) {
		bit = key - GLFW_KEY_0;
		bitfield = &GW.num;
	} else if (key >= GLFW_KEY_KP_0   && key <= GLFW_KEY_KP_9) {
		bit = key - GLFW_KEY_KP_0 + 16;
		bitfield = &GW.num;
	} else if (key >= GLFW_KEY_ESCAPE && key <= GLFW_KEY_PAUSE) {
		bit = key - GLFW_KEY_ESCAPE;
		bitfield = &GW.cmd;
	} else if (key >= GLFW_KEY_LEFT_SHIFT && key <= GLFW_KEY_MENU ) {
		bit = key - GLFW_KEY_LEFT_SHIFT + 19;
		bitfield = &GW.cmd;
	}
	if (bitfield) {
		if (action == GLFW_PRESS)
			*bitfield |= 1<<bit;
		else if (action == GLFW_RELEASE)
			*bitfield &= ~(1<<bit);
	}	
}

#ifdef GL_DRAWING
Shader g_line_shader = {0};
GLuint g_line_vb=0;
#endif

void helper_gl_init(void)
{
#ifdef GL_DRAWING
	if (!g_line_shader.id) {
		ASSERT(shader_init(&g_line_shader, "#version 100\n\
precision mediump float;\n\
attribute vec2 aPos;\n\
uniform vec2 uScale;\n\
uniform vec2 uTranslate;\n\
uniform mat3 uScreen;\n\
uniform float uAngle;\n\
void main()\n\
{\n\
	float c = cos(uAngle);\n\
	float s = sin(uAngle);\n\
	vec2 pos2 = mat2(c, s, -s, c)*(aPos*uScale)+uTranslate;\n\
	vec3 pos = uScreen*vec3(pos2, 1.0);\n\
	gl_Position = vec4(pos.xy, 0.0, 1.0);\n\
}"
		, "#version 100\n\
			precision mediump float;\n\
			uniform vec4 uColor;\n\
			void main(){gl_FragColor = uColor;}",
			(char*[]){"aPos", "uScreen","uTranslate", "uScale",  "uAngle", "uColor",  NULL}
		), "Couldn't create g_line_shader shader");
		on_exit(shader_on_exit, &g_line_shader);
	}
	glDeleteBuffers(1, &g_line_vb);
	glGenBuffers(1, &g_line_vb);
	
#endif

}

#ifdef GL_DRAWING

void draw_rect(GLuint aPos, GLfloat xy_ll[2], GLfloat xy_ur[2])
{
	GLfloat pts[4*2] = {
		xy_ll[0], xy_ll[1], 
		xy_ll[0], xy_ur[1], 
		xy_ur[0], xy_ll[1],
		xy_ur[0], xy_ur[1]};
	glBindBuffer(GL_ARRAY_BUFFER, g_line_vb);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2*4, pts, GL_STATIC_DRAW);
	glVertexAttribPointer(aPos, 2, GL_FLOAT, 0, 0, 0);
	glEnableVertexAttribArray(aPos);
	
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

}


void draw_color(float r, float g, float b, float a)
{
	GW.color[0] = r;
	GW.color[1] = g;
	GW.color[2] =b;
	GW.color[3] =a;
	
}

void drawl(GLenum mode, GLfloat xy[2], GLfloat scale[2], GLfloat angle, int npts, GLfloat *pts)
{

//~ DEBUG("%d %d %f\n", g_line_shader.id, npts, pts[0]);
	glUseProgram(g_line_shader.id);
	glBindBuffer(GL_ARRAY_BUFFER, g_line_vb);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2*npts, pts, GL_STATIC_DRAW);
	glVertexAttribPointer(g_line_shader.args[0], 2, GL_FLOAT, 0, 0, 0);
	glEnableVertexAttribArray(g_line_shader.args[0]);
	
	//~ GW.vmat = (GLfloat[3][3]){1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0};
	//~ GW.color = (GLfloat[4]){1.0, 1.0, 0.0, 1.0};
	
	glUniformMatrix3fv(g_line_shader.args[1], 1, GL_FALSE, &GW.vmat[0][0]);// uScreen
	glUniform2fv(g_line_shader.args[2],1, xy); // uTranslate
	glUniform2fv(g_line_shader.args[3],1, scale); //uScale
	glUniform1f(g_line_shader.args[4], angle*M_PI/180.0); //uScale
	glUniform4fv(g_line_shader.args[5], 1, GW.color); //uColor
	glDrawArrays(mode, 0, npts);
}
void draw_line_strip(GLfloat xy[2], GLfloat scale[2], GLfloat angle, int npts, GLfloat *pts)
{
	drawl(GL_LINE_STRIP, xy, scale, angle, npts, pts);
}
void draw_line_loop(GLfloat xy[2], GLfloat scale[2], GLfloat angle, int npts, GLfloat *pts)
{
	drawl(GL_LINE_LOOP, xy, scale, angle, npts, pts);
}
void draw_lines(GLfloat xy[2], GLfloat scale[2], GLfloat angle, int npts, GLfloat *pts)
{
	drawl(GL_LINES, xy, scale, angle, npts, pts);
}
void draw_polygon(GLfloat xy[2], GLfloat scale[2], GLfloat angle, int npts, GLfloat *pts)
{
	drawl(GL_TRIANGLE_FAN, xy, scale, angle, npts, pts);
}
void draw_circle(GLfloat xy[2], GLfloat scale[2])
{
	static GLfloat cpts[36*2] = {
	1.0000000000000000000e+00, 0.0000000000000000000e+00, 
	9.8480775301220802032e-01, 1.7364817766693033119e-01, 
	9.3969262078590842791e-01, 3.4202014332566871291e-01, 
	8.6602540378443870761e-01, 4.9999999999999994449e-01, 
	7.6604444311897801345e-01, 6.4278760968653925190e-01, 
	6.4278760968653936292e-01, 7.6604444311897801345e-01, 
	5.0000000000000011102e-01, 8.6602540378443859659e-01, 
	3.4202014332566882393e-01, 9.3969262078590831688e-01, 
	1.7364817766693041445e-01, 9.8480775301220802032e-01, 
	0.0, 1.0000000000000000000e+00, 
	-1.7364817766693030343e-01, 9.8480775301220802032e-01, 
	-3.4202014332566849086e-01, 9.3969262078590842791e-01, 
	-4.9999999999999977796e-01, 8.6602540378443870761e-01, 
	-6.4278760968653936292e-01, 7.6604444311897801345e-01, 
	-7.6604444311897790243e-01, 6.4278760968653947394e-01, 
	-8.6602540378443848557e-01, 5.0000000000000033307e-01, 
	-9.3969262078590831688e-01, 3.4202014332566887944e-01, 
	-9.8480775301220802032e-01, 1.7364817766693027568e-01, 
	-1.0000000000000000000e+00, 0.0, 
	-9.8480775301220813134e-01, -1.7364817766693002588e-01, 
	-9.3969262078590842791e-01, -3.4202014332566865740e-01, 
	-8.6602540378443859659e-01, -5.0000000000000011102e-01, 
	-7.6604444311897834652e-01, -6.4278760968653891883e-01, 
	-6.4278760968653947394e-01, -7.6604444311897790243e-01, 
	-5.0000000000000044409e-01, -8.6602540378443837454e-01, 
	-3.4202014332566854637e-01, -9.3969262078590842791e-01, 
	-1.7364817766693033119e-01, -9.8480775301220802032e-01, 
	0.0, -1.0000000000000000000e+00, 
	1.7364817766692997036e-01, -9.8480775301220813134e-01, 
	3.4202014332566899046e-01, -9.3969262078590831688e-01, 
	4.9999999999999933387e-01, -8.6602540378443904068e-01, 
	6.4278760968653925190e-01, -7.6604444311897812447e-01, 
	7.6604444311897779141e-01, -6.4278760968653958496e-01, 
	8.6602540378443881863e-01, -4.9999999999999966693e-01, 
	9.3969262078590842791e-01, -3.4202014332566860189e-01, 
	9.8480775301220802032e-01, -1.7364817766693038670e-01, };
	drawl(GL_LINE_LOOP, xy, scale, 0.0, 36, cpts);
}

#endif

void set_window(GLFWwindow *w)
{
	GW.window = w;
	glfwMakeContextCurrent(GW.window);

	glfwSetFramebufferSizeCallback(GW.window, framebuffer_size_callback);
	glfwSetKeyCallback(GW.window, key_callback);
	glfwSetCharCallback(GW.window, character_callback);
	glfwSetCursorPosCallback(GW.window, cursor_position_callback);
	glfwSetCursorEnterCallback(GW.window, cursor_enter_callback);
	glfwSetMouseButtonCallback(GW.window, mouse_button_callback);
	glfwSetScrollCallback(GW.window, scroll_callback);
	
	int width, height;
	glfwGetFramebufferSize(GW.window, &width, &height);
	framebuffer_size_callback(GW.window, width, height);
	helper_gl_init();
	gl_init();
	XINFO("Initialization Complete");
}

static void error_callback(int error, const char* description)
{
	ABORT("GLFW:%d: %s", error, description);
}

void win_on_exit(int status, void *arg)
{
	if (GW.fs_window) {
		XINFO("Destroy FS window");
		glfwDestroyWindow(GW.fs_window);
	}
	if (GW.w_window) {
		XINFO("Destroy Window");
		glfwDestroyWindow(GW.w_window);
	}
	XINFO("GLFW Terminate");
	glfwTerminate();
	memset(&GW, 0, sizeof(Window));
}

int main(int argc, char *argv[])
{
	memset(&GW, 0, sizeof(Window));
	GW.zoomx = GW.zoomy = 1.0;	
	ASSERT(main_init(argc, argv) == 0, "main_init");
	
	glfwSetErrorCallback(error_callback);
	ASSERT(glfwInit(), "glfwinit");
	on_exit(win_on_exit, 0);
	
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	
	GW.w_window = glfwCreateWindow(16*32, 9*32, gl_name, NULL, NULL);
	ASSERT(GW.w_window, "create window");

	set_window(GW.w_window);
	XINFO("GL_VERSION  : %s", glGetString(GL_VERSION) );
	XINFO("GL_RENDERER : %s", glGetString(GL_RENDERER) );

	glfwSetTime (0.0);
	while(!glfwWindowShouldClose(GW.window)) {
		GW.scroll = 0;
		glfwPollEvents();
		GW.vmat[0][0] = 2.0*GW.zoomx / GW.w;
		GW.vmat[1][1] = 2.0*GW.zoomy / GW.h;
		GW.vmat[2][2] = 1.0;
		GW.vmat[2][0] = (GW.camx+GW.camdx) / GW.w;
		GW.vmat[2][1] = (GW.camy+GW.camdy) / GW.h;
		
		GW.m.click = GW.clicks_i? GW.clicks[--GW.clicks_i]: 0;
		GW.m.release = GW.releases_i? GW.releases[--GW.releases_i]: 0;
		
		if (!gl_frame())
			break;
		++GW.frame;
		glfwSwapBuffers(GW.window);
	}
	XINFO("Goodbye");
	return 0;
}

#endif
#endif
#endif

