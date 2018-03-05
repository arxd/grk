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
	
	
	//~ f1_window(&w_rect, W_RECTANGLE, 1024, 1.0, v2(-1.2, 1.2));
	//~ f1_window(&w_tri, W_TRIANGLE, 1024, 1.0, v2(-1.2, 1.2));
	//~ f1_window(&w_hann, W_HANN, 1024, 1.0, v2(-1.2, 1.2));
	//~ f1_window(&w_hamming, W_HAMMING, 1024, 1.0, v2(-1.2, 1.2));
	//~ f1_window(&w_blackman, W_BLACKMAN, 1024, 1.0, v2(-1.2, 1.2));
	//~ f1_window(&w_blackharris, W_BLACKHARRIS, 1024, 1.0, v2(-1.2, 1.2));
	//~ f1_window(&w_gauss, W_APXGAUSS, 1024, 1.0, v2(-1.2, 1.2));
	
	//~ td_bin(&gtd, &gtdf0, 60.0, W_BLACKHARRIS, 60.0, 1, 50*WEEKS);
	
	td_bin(&gtd, &avg1m, 2*60.0, W_BLACKHARRIS, 10*60.0, 0, 50*WEEKS);
	view_fit(&view, &avg1m);

	fr_init(&gtdf0r, 1024);
	fr_eval(&gtdf0r, &avg1m, view.vps.x);
	


}





V1 angle = 0.0;



int gl_frame(void)
{
	V2 mxy = screen_to_view(&view, v2(GW.m.hx, GW.m.hy));
	view_navigate(&view, GW.m.btn, 1.125);

	glClear(GL_COLOR_BUFFER_BIT);
	color = rgb(1.0, 0.8, 0.2);
	glLineWidth(1.0);
	//~ f1_render(&w_rect, &view, rgb(1.0, 1.0, 0.0));
	//~ f1_render(&w_tri, &view, rgb(1.0, 0.0, 0.0));
	//~ f1_render(&w_hamming, &view, rgb(1.0, 1.0, 1.0));
	//~ f1_render(&w_hann, &view, rgb(1.0, 0.0, 1.0));
	//~ f1_render(&w_blackman, &view, rgb(0.0, 1.0, 1.0));
	//~ f1_render(&w_blackharris, &view, rgb(0.0, 1.0, 0.0));
	//~ f1_render(&w_gauss, &view, rgb(0.0, 0.0, 1.0));
	
	//~ f1_render(&gtdf0, &view, rgb(0.8, 0.8, 1.0));
	//~ f1_render(&avg1m, &view, rgb(0.8, 0.8, 0.0));
	fr_render(&gtdf0r, &view, rgb(0.3, 0.8, 0.0));
	
	
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
	} else if (view.vps.x > 60)
	
	//~ grid_render(&view, xfmt, yfmt, xmaj, xmin, rgb(1.0, 0.0, 0.0));
	grid_time_render(&view,  rgb(1.0, 0.0, 0.0));

	grid_vert_render(&view,  rgb(0.6, 0.5, 0.0));

	color = rgb(1.0, 1.0, 1.0);
	
	if (GW.m.hx >= 0)
		draw_line_strip(v2(0.0, 0.0), v2(1.0, 1.0), 0.0, 2, (GLfloat[]){mxy.x,view.ll.y, mxy.x, view.ur.y});
	
	

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


