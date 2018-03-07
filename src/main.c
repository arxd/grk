//~ #include <math.h>
#include <malloc.h>
#include <string.h>
#include <math.h>

#include "logging.c"
#include "gl.c"
#include "glmath.c"
#include "math.c"
#include "shaders.h"
#include "trades.c"
#include "function.c"

const char *gl_name = "GRK";

Function1 avg1s;
Function1 avg1m;

Shader g_dfun_shader = {0};
Texture gph0;
TradeData gtd;
Function1 gtdf0;
FunctionRanged gtdf0r;

Function1 w_rect, w_tri, w_hann, w_hamming, w_blackman, w_blackharris,w_gauss;

static int linear = 1;


void gl_init(void)
{
	int max_tex;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_tex);
	INFO("GL INIT Max tex %d", max_tex);
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.0, 0.0 , 0.0, 1.0);
	
	geom_init();
	grid_render_init();
	font_init();
	f1_render_init();
	
	
	td_bin(&gtd, &avg1m, 2*60.0, W_BLACKHARRIS, 10*60.0, 0, 50*WEEKS);
	view_fit(&view, &avg1m, linear);

	fr_init(&gtdf0r, 1024);
	fr_compile(&gtdf0r, &avg1m, view.vps.x);
}





V1 angle = 0.0;


V1 prev_vps = 0;

int gl_frame(void)
{
	V2 mxy = screen_to_view(&view, v2(GW.m.hx, GW.m.hy));
	view_navigate(&view, GW.m.btn, 1.125);

	glClear(GL_COLOR_BUFFER_BIT);
	color = rgb(1.0, 0.8, 0.2);
	glLineWidth(1.0);
	
	if (view.vps.x != prev_vps) {
		if (view.vps.x < avg1m.dx)
			view_zoom_at(&view, screen_to_view(&view, v2(GW.m.hx, GW.m.hy)), v2(avg1m.dx, view.vps.y));
		fr_compile(&gtdf0r, &avg1m, view.vps.x);
		prev_vps = view.vps.x;
		
	}
	
	fr_render(&gtdf0r, &view, rgb(0.3, 0.8, 0.0), linear);
	
	switch (key_pop()) {
		case 'l':
			linear = !linear; 
			view_fit(&view, &avg1m, linear);
			break;
	}
	
	char *xfmt = "%f";
	char *yfmt = "%f";
	V2 xmaj = v2(5.0, 5.0);
	V2 xmin = v2(1.0, 1.0);
	if (1.0 / view.vps.x > 20) {
		xfmt = "%.0fs";
		V1 thresh = 20;
		//~ while (1.0/view.vps.x > thresh) {
			//~ xmaj.x *= 5;
			//~ xmin.x *= 5;
			//~ thresh *= 5;
		//~ }
	}
	
	//~ grid_render(&view, xfmt, yfmt, xmaj, xmin, rgb(1.0, 0.0, 0.0));
	grid_time_render(&view,  rgb(1.0, 0.0, 0.0));

	grid_vert_render(&view,  rgb(0.6, 0.5, 0.0));

	color = rgb(1.0, 1.0, 1.0);
	
	if (GW.m.hx >= 0) {
		glLineWidth(1.0);
		draw_line_strip(v2(0.0, 0.0), v2(1.0, 1.0), 0.0, 2, (GLfloat[]){mxy.x,view.ll.y, mxy.x, view.ur.y});
		INFO("%s", v2str(fr_eval(&gtdf0r, mxy.y)));
		
		
	}
	
	

	return !(GW.cmd & KCMD_ESCAPE);
}

int main_init(int argc, char *argv[])
{
	ASSERT(argc >= 2, "grk somefile.bin");
	INFO("LOAD %s", argv[1]);
	td_init(&gtd);
	td_read(&gtd, argv[1]);
	
	return 0;
}


