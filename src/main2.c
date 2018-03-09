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
#include "nnet.c"


const char *gl_name = "TMP";


Shader g_dfun_shader = {0};
Texture gph0;

Analytic *ANA;
Function1 F1;
FunctionRanged F1R;


V1 factorial(int x)
{
	if (x == 1)
		return 1.0;
	return x*factorial(x-1);
}

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
	
	V2 coefs(int i) {return v2(((i%2)*-2.0 + 1.0) / factorial( i*2+1),  i*2+1);}
	ANA = an_new_regular(17, coefs);
	an_eval_regular_f1(ANA, &F1, 0.0, v2(-M_PI*3.0, M_PI*3.0), 1024*4);
	view_fit(&view, &F1, 1);
	fr_compile(&F1R, &F1, view.vps.x);
}

V1 prev_vps = 0;

int gl_frame(void)
{
	V2 mxy = screen_to_view(&view, v2(GW.m.hx, GW.m.hy));
	view_navigate(&view, GW.m.btn, 1.125);

	glClear(GL_COLOR_BUFFER_BIT);
	color = rgb(1.0, 0.8, 0.2);
	glLineWidth(1.0);
	
	if (view.vps.x != prev_vps) {
		if (view.vps.x < F1.dx)
			view_zoom_at(&view, screen_to_view(&view, v2(GW.m.hx, GW.m.hy)), v2(F1.dx, view.vps.y));
		fr_compile(&F1R, &F1, view.vps.x);
		prev_vps = view.vps.x;
	}
	
	fr_render(&F1R, &view, rgb(1.0, 1.0, 0.0), 1);
	
	//~ char *xfmt = "%f";
	//~ char *yfmt = "%f";
	//~ V2 xmaj = v2(5.0, 5.0);
	//~ V2 xmin = v2(1.0, 1.0);
	//~ if (1.0 / view.vps.x > 20) {
		//~ xfmt = "%.0fs";
		//~ V1 thresh = 20;
		//~ while (1.0/view.vps.x > thresh) {
			//~ xmaj.x *= 5;
			//~ xmin.x *= 5;
			//~ thresh *= 5;
		//~ }
	//~ }
	
	//~ grid_render(&view, xfmt, yfmt, xmaj, xmin, rgb(1.0, 0.0, 0.0));
	grid_time_render(&view,  rgb(0.4, 0.4, 0.4));

	grid_vert_render(&view,  rgb(0.6, 0.5, 0.0));

	color = rgb(1.0, 1.0, 1.0);
	
	if (GW.m.hx >= 0) {
		glLineWidth(1.0);
		draw_line_strip(v2(0.0, 0.0), v2(1.0, 1.0), 0.0, 2, (GLfloat[]){mxy.x,view.ll.y, mxy.x, view.ur.y});
		//~ INFO("%f", mxy.x);
		//~ for (int i=0; i < g_nfuncs; ++i ) {
			//~ tdat_r[i].yoff = f1_eval_at(tdat+i, mxy.x);
			//~ V1 f(V1 y)  {return y - y0;}
			//~ f1_map(tdat+i, f);
			//~ fr_compile(tdat_r+i, tdat+i, view.vps.x);
		//~ }
		
		
	}
	
	

	return !(GW.cmd & KCMD_ESCAPE);
}

int main_init(int argc, char *argv[])
{
	//~ TradeData td;
	//~ ASSERT(argc >= 3, "grk nweeks somefile.bin");
	//~ double weeks = strtof(argv[1], 0);
	//~ double n = time(0);
	
	//~ g_nfuncs = argc - 2;
	//~ ASSERT(g_nfuncs <= MAX_FUNCS, "%d max funcs", MAX_FUNCS);
	
	//~ INFO("%f weeks  before %f", weeks, n);
	//~ for (int i=0; i < g_nfuncs; ++i) {
		//~ INFO("LOAD %s", argv[i+2]);
		//~ td_init(&td);
		//~ td_read(&td, argv[i+2]);
		//~ td_bin(&td, tdat+i, 2*60.0, W_RECTANGLE , 10*60.0, 0, n - weeks*WEEKS, n - 3600*0.0);//W_BLACKHARRIS
		//~ td_fini(&td);
	//~ }
	
	
	//~ printf("%s %s %s", v1str(0.0031312), v1str(10.0235), v1str(32352313.0));
	
	return 0;
}


