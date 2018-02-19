//~ #include <math.h>
#include <malloc.h>
#include <string.h>
#include <math.h>

#include "logging.c"
#include "gl.c"
#include "math.c"
#include "shaders.h"
#include "trades.c"

const char *gl_name = "GRK";


typedef V2 FMinMax(V2);

Shader g_dfun_shader = {0};
Texture gph0;
uint8_t gph0pts[128*128];

Texture gph1;

Texture gph2;

//~ struct s_Texture {
	//~ GLuint id;
	//~ int w, h;
	//~ GLenum format;
	//~ GLenum type;
//~ };


void gl_init(void)
{
	int max_tex;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_tex);
	INFO("GL INIT Max tex %d", max_tex);
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.498, 0.624 , 0.682, 1.0);	
	if (g_dfun_shader.id == 0) {
		ASSERT(shader_init(&g_dfun_shader, V_STRAIGHT, F_DFUN, (char*[]){
			"aPos", "uFramebuffer", "uYrange", "uColor", NULL}),  "Couldn't create shader");
		on_exit(shader_on_exit, &g_dfun_shader);

		gph0 = (Texture){0, 128, 128, GL_ALPHA, GL_UNSIGNED_BYTE};
		tex_set(&gph0, 0);
	//~ gph1 = (Texture){0, 4096, 2, GL_ALPHA, GL_UNSIGNED_BYTE};
	}
	
	
}

V2 uYrange;


void compile_graph(Texture *tex, FMinMax *fun, V2 xrange, V2 yrange)
{
	V1 dx = (xrange.y - xrange.x) / GW.w;
	V1 dy = yrange.y - yrange.x;
	static uint8_t pts[128*128];
	//~ INFO("DX: %s %f",v2str(xrange), dx);
	for (int i=0; i < GW.w; ++i) {
		V1 x0 = xrange.x + i*dx;
		V2 x = v2(x0,x0+dx);
		V2 y = fun(x);
		if (y.x > y.y) 
			y = v2(y.y, y.x);
		y = v2sub(y, v2(yrange.x, yrange.x));
		
		y = v2mult(y, 1.0/dy);
		//~ INFO("%s -> %s %f", v2str(x),  v2str(y), yrange.x);
		y = v2clamp(y);
		uint16_t um = y.x*GW.h;
		pts[i*4+0] = (um>>8)&0xFF;
		pts[i*4+1] = um&0xFF;
		
		uint16_t ux =  y.y*GW.h;
		if (ux == um)
			ux = um+1;
		pts[i*4+2] = (ux>>8)&0xFF;
		pts[i*4+3] = ux&0xFF;	
	}
	
	glActiveTexture(GL_TEXTURE0);
	tex_set(tex, pts);

	
}


V2 sinfunc(V2 x)
{
	V2 y = v2(sin(x.x), sin(x.y));
		
	
}

int gl_frame(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	draw_color(0.2, 0.3, 0.5, 1.0);
	glLineWidth(2.0);

	compile_graph(&gph0, sinfunc, v2(0.0, 2*M_PI), v2(-1.0, 1.0));
	
	glUseProgram(g_dfun_shader.id);
	//~ glActiveTexture(GL_TEXTURE0);
	//~ glBindTexture(GL_TEXTURE_2D, gph0.id);
	//~ tex_set(&gph0, gph0pts);
	glUniform1i(g_dfun_shader.args[1], 0);
	glUniform2f(g_dfun_shader.args[2], -1.0, 1.0);//uTileSize
	glUniform4f(g_dfun_shader.args[3], 1.0, 0.0, 0.0, 1.0);//uTileSize
	
	draw_rect(g_dfun_shader.args[0], (GLfloat[]){-1.0, -1.0}, (GLfloat[]){1.0, 1.0});
	
	if (GW.m.btn) {
		int dx = GW.m.sx - GW.m.sx0;
		int dy = GW.m.sy - GW.m.sy0;
		//~ INFO("%d, %d", dx, dy);
		GW.camdx = 2.0*dx;
		GW.camdy = -2.0*dy;
	} else {
		GW.camx += GW.camdx;
		GW.camy += GW.camdy;
		GW.camdx = 0.0;
		GW.camdy = 0.0;
	}
		
	if (GW.scroll < 0) {
		GW.zoomx *= 1.1;
		
	} else if (GW.scroll > 0) {
		GW.zoomx /= 1.1;
	}
	GW.zoomy = GW.zoomx;

	return !(GW.cmd & KCMD_ESCAPE);
}

int main_init(int argc, char *argv[])
{
	return 0;
}


