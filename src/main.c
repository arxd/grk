//~ #include <math.h>
#include <malloc.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "logging.c"
#include "gl.c"
#include "glmath.c"
#include "math.c"
#include "shaders.h"
#include "trades.c"
#include "function.c"

#define MAX_FUNCS 8
const char *gl_name = "GRK";

//~ Function1 avg1s;
//~ Function1 avg1m;

Shader g_dfun_shader = {0};
Texture gph0;

int g_nfuncs;
Function1 tdat[MAX_FUNCS];
FunctionRanged tdat_r[MAX_FUNCS];
V1 min_dx;
//~ Function1 w_rect, w_tri, w_hann, w_hamming, w_blackman, w_blackharris,w_gauss;

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
	
	
	
	for (int i=0;i < g_nfuncs; ++i) {
		V1 logmult = 10.0/log(1.1);
		V1 y0 = logmult*log(tdat[i].ys[tdat[i].len-1]);
		V1 f(V1 y)  {return logmult*log(y) - y0;}
		f1_map(tdat+i, f);
	}
	view_fit(&view, tdat, 1);
	for (int i=0;i < g_nfuncs; ++i) {
		INFO("Range %d: %f", i, view.vps.x);
		fr_init(tdat_r+i, 1024);
		fr_compile(tdat_r+i, tdat+i, view.vps.x);
	}
	min_dx = tdat[0].dx;

	//~ fr_init(&gtdf0r, 1024);
	//~ fr_compile(&gtdf0r, &avg1m, view.vps.x);
}





V1 angle = 0.0;


V1 prev_vps = 0;

Color COLORS[MAX_FUNCS] = {
	{1.0, 0.0, 0.0, 0.5},
	{0.0, 1.0, 0.0, 0.5},
	{0.0, 0.0, 1.0, 0.5},
	{1.0, 1.0, 0.0, 0.5},
	{1.0, 1.0, 1.0, 0.5},
};

int gl_frame(void)
{
	V2 mxy = screen_to_view(&view, v2(GW.m.hx, GW.m.hy));
	view_navigate(&view, GW.m.btn, 1.125);

	glClear(GL_COLOR_BUFFER_BIT);
	color = rgb(1.0, 0.8, 0.2);
	glLineWidth(1.0);
	
	if (view.vps.x != prev_vps) {
		if (view.vps.x < min_dx)
			view_zoom_at(&view, screen_to_view(&view, v2(GW.m.hx, GW.m.hy)), v2(min_dx, view.vps.y));
		for (int i = 0; i<g_nfuncs; ++i) 
			fr_compile(tdat_r+i, tdat+i, view.vps.x);
		prev_vps = view.vps.x;
	}
	
	for (int i = 0; i < g_nfuncs; ++i)
		fr_render(tdat_r+i, &view, COLORS[i], 1);
	
	//~ switch (key_pop()) {
		//~ case 'l':
			//~ linear = !linear; 
			//~ view_fit(&view, tdat, 1);
			//~ break;
	//~ }
	
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
		//~ INFO("%f", mxy.x);
		for (int i=0; i < g_nfuncs; ++i ) {
			V1 y0 = f1_eval_at(tdat+i, mxy.x);
			V1 f(V1 y)  {return y - y0;}
			f1_map(tdat+i, f);
			fr_compile(tdat_r+i, tdat+i, view.vps.x);
		}
		
		
	}
	
	

	return !(GW.cmd & KCMD_ESCAPE);
}

int main_init(int argc, char *argv[])
{
	TradeData td;
	ASSERT(argc >= 3, "grk nweeks somefile.bin");
	double weeks = strtof(argv[1], 0);
	double n = time(0);
	
	g_nfuncs = argc - 2;
	ASSERT(g_nfuncs <= MAX_FUNCS, "%d max funcs", MAX_FUNCS);
	
	INFO("%f weeks  before %f", weeks, n);
	for (int i=0; i < g_nfuncs; ++i) {
		INFO("LOAD %s", argv[i+2]);
		td_init(&td);
		td_read(&td, argv[i+2]);
		td_bin(&td, tdat+i, 2*60.0, W_RECTANGLE , 10*60.0, 0, n - weeks*WEEKS, n-3600*6.0);//W_BLACKHARRIS
		td_fini(&td);
	}
	
	
	return 0;
}


