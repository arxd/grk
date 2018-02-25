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

V2 g_xy;
V2 g_dxy;
V2 g_pix;

Function1 f_sin;

void f_sin_init(V1 freq, int cycles)
{
	int len = 1024*cycles;
	f1_init(&f_sin, len);
	f1_resize(&f_sin, len);
	f_sin.x0 = 0.0;
	f_sin.dx = M_PI*2.0/1024;//freq/1024 * /freq
	for (int i=0; i < len; ++i)
		f_sin.ys[i] = sin(i*f_sin.dx);
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
	
	V2 mm = v2(1e100, -1e100);
	for (int i=0; i < gtdf0.len; ++i) {
		if (gtdf0.ys[i] < mm.x)
			mm.x = gtdf0.ys[i];
		if (gtdf0.ys[i] > mm.y)
			mm.y = gtdf0.ys[i];
		
		
	}
	INFO("n=%d  min=%f  max=%f", gtdf0.len, mm.x, mm.y);
	V1 yvps = (mm.y-mm.x)/GW.h;
	view.origin = v2(60.0, 0.0);//mm.x/yvps);
	view.vps = v2(356.0*24*3600.0/GW.w, mm.y/GW.h);
	view.drag = v2(0.0, 0.0);
	
	f_sin_init(5.0, 3);
	
	
	
	
	//~ f1_init(&avg1s, ceil(gtd.data[gtd.len-1].t - gtd.data[0].t) + 1000);
	//~ avg1s.x0 = floor(gtd.data[0].t);
	//~ avg1s.dx = 1.0;
	//~ long long t = 0, i = -1;
	//~ double mass = 0.0;
	//~ double vol = 0.0;
	//~ V2 mm = v2(1e100, -1e100);
	//~ V1 prevt = 0.0;
	//~ while (++i < gtd.len) {
		//~ if (gtd.data[i].t >= avg1s.x0 + t + 1.0) {
			//~ f1_resize(&avg1s, avg1s.len+1);
			//~ if (vol == 0.0) {
				//~ avg1s.ys[avg1s.len-1] = avg1s.ys[avg1s.len-2];
			//~ } else {
				//~ avg1s.ys[avg1s.len-1] = mass / vol;
			//~ }
			//~ mass = 0.0;
			//~ vol = 0.0;
		//~ }
		//~ INFO("%f", gtd.data[i].t - prevt);
		//~ prevt = gtd.data[i].t;
		//~ V1 val = gtd.data[i].val;
		//~ if (val > mm.y)
			//~ mm.y = val;
		//~ if (val < mm.x)
			//~ mm.x = val;
		//~ mass += val * fabs(gtd.data[i].amt);
		//~ vol += fabs(gtd.data[i].amt);
	//~ }
	
	//~ INFO("LEN: %d seconds   %f  ... %f",avg1s.len, mm.x, mm.y);
	// setup camer
	//~ g_xy = v2(avg1s.x0, -mm.x);
	//~ g_dxy = v2 (0.0, 0.0);
	//~ g_pix = v2(avg1s.dx, (mm.y-mm.x)/GW.h);
	
	//~ INFO("(%f %f)  (%f %f)", g_xy.x, g_xy.y,  g_pix.x, g_pix.y);
	
	//~ V2 tmm = v2(1e100, -1e100);
	
	//~ for (int i=0; i < gfn.len; ++i) {
		//~ if (gfn.data[i].t < tmm.x)
			//~ tmm.x = gfn.data[i].t;
		//~ if (gfn.data[i].t > tmm.y)
			//~ tmm.y = gfn.data[i].t;
		
	//~ }
	//~ INFO("%f  -  %f", tmm.x, tmm.y);
	
	//~ x0 = tmm.x;
	//~ ppx = 60*60*24;
	
}


//~ void compile_graph(Texture *tex, FMinMax *fun, V2 xrange, V2 yrange)
//~ {
	//~ V1 dx = (xrange.y - xrange.x) / GW.w;
	//~ V1 dy = yrange.y - yrange.x;
	//~ static uint8_t pts[128*128];
	//~ INFO("DX: %s %f",v2str(xrange), dx);
	//~ V2 *ys = alloca(sizeof(V2) * GW.w);
	//~ V1 maxy = -1e100;
	//~ V1 miny = 1e100;
	//~ for (int i=0; i < GW.w; ++i) {
		//~ V1 x0 = xrange.x + i*dx;
		//~ V2 x = v2(x0,x0+dx);
		//~ V2 y = fun(x);
		//~ if (y.x > y.y) 
			//~ y = v2(y.y, y.x);
		//~ ys[i] = y;
		//~ if (y.x < miny)
			//~ miny = y.x;
		//~ if (y.y > maxy)
			//~ maxy = y.y;
	//~ }
	
	//~ miny = yrange.x;
	//~ maxy = yrange.y;
	
	//~ for (int i=0; i < GW.w; ++i) {
		
		//~ V2 y = v2sub(ys[i], v2(miny,miny));
		//~ y = v2mult(y, 1.0/ (maxy-miny));
		//~ INFO("%s -> %s %f", v2str(x),  v2str(y), yrange.x);
		//~ y = v2clamp(y);
		//~ uint16_t um = y.x*GW.h;
		//~ pts[i*4+0] = (um>>8)&0xFF;
		//~ pts[i*4+1] = um&0xFF;
		
		//~ uint16_t ux =  y.y*GW.h;
		//~ if (ux == um)
			//~ ux = um+1;
		//~ pts[i*4+2] = (ux>>8)&0xFF;
		//~ pts[i*4+3] = ux&0xFF;	
	//~ }
	
	//~ glActiveTexture(GL_TEXTURE0);
	//~ tex_set(tex, pts);

	
//~ }


//~ int find_min(V1 t, int a, int b) {
	//~ if (b == a)
		//~ return a;
	//~ int mid = (b-a)/2 + a;
	//~ if (gfn.data[mid].t < t) 
		//~ return find_min(t, mid+1, b);
	//~ return find_min(t, a, mid);
	
//~ }

//~ V2 financefun_inout(V2 x)
//~ {
	//~ int i = find_min(x.x, 0, gfn.len);
	//~ if (i == gfn.len)
		//~ return v2(0.0, 0.0);

	//~ int j = i;
	//~ while (j < gfn.len && gfn.data[j].t < x.y)
		//~ ++j;
	//~ if (i == j)
		//~ return v2(0.0, 0.0);
	//~ V2 mm = v2(gfn.data[i].val, 0.0);
	//~ while (i < j) {
		//~ mm.y = gfn.data[i].val;
		//~ i++;
	//~ }
	//~ return mm;
//~ }

//~ V2 financefun_minmax(V2 x)
//~ {
	//~ int i = find_min(x.x, 0, gfn.len);
	//~ if (i == gfn.len)
		//~ return v2(0.0, 0.0);

	//~ int j = i;
	//~ while (j < gfn.len && gfn.data[j].t < x.y)
		//~ ++j;
	//~ if (i == j)
		//~ return v2(0.0, 0.0);
	//~ V2 mm = v2(gfn.data[i].val, gfn.data[i].val);
	//~ while (i < j) {
		//~ if (gfn.data[i].val < mm.y)
			//~ mm.y = gfn.data[i].val;
		//~ if (gfn.data[i].val > mm.x)
			//~ mm.x = gfn.data[i].val;
		//~ i++;
	//~ }
	//~ return mm;
//~ }




V1 angle = 0.0;



int gl_frame(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	color = rgb(1.0, 0.8, 0.2);
	glLineWidth(2.0);
	V2 mxy = screen_to_view(&view, v2(GW.m.hx, GW.m.hy));

	//~ f1_render(&f_sin, &view, rgb(1.0, 1.0, 0.0));
	f1_render(&gtdf0, &view, rgb(0.8, 0.8, 1.0));
	
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
	color = rgb(0.0, 0.0, 0.0);
	//~ grid_render(&view, xfmt, yfmt, xmaj, xmin, rgb(1.0, 0.0, 0.0));
	grid_time_render(&view,  rgb(1.0, 0.0, 0.0));

	grid_vert_render(&view,  rgb(0.6, 0.5, 0.0));

	
	
	//~ draw_line_strip(v2(0.0, 0.0), v2(1.0, 1.0), angle, 3, (GLfloat[]){0.0, 0.0, 1.0, 1.0, 0.0, -1.0});
	
	//~ if (GW.m.hx >= 0)
		//~ draw_line_strip(v2(mxy.x, 0.0), v2(1.0, view.vps.y*GW.h), 0.0, 2, (GLfloat[]){0.0, -1.0, 0.0, 1.0});
	

	angle += 2;
	
	
	
	
	
	if (GW.m.btn) {
		view.drag = v2sub(v2(GW.m.sx, GW.m.sy), v2(GW.m.sx0, GW.m.sy0));
	} else if (view.drag.x || view.drag.y) {
		view.origin = v2add(view.origin, view.drag);
		view.drag = v2(0.0, 0.0);
	}
	
	if (GW.scroll != 0) {
		V1 dir = GW.scroll < 0 ? 1.1 : 1.0/1.1;
		V2 z = v2(GW.cmd&KCMD_LEFT_SHIFT?1.0:dir, GW.cmd&KCMD_LEFT_SHIFT?dir:1.0);
		view_zoom_at(&view, mxy, z);
	}

	return !(GW.cmd & KCMD_ESCAPE);
}

int main_init(int argc, char *argv[])
{
	ASSERT(argc >= 2, "grk somefile.bin");
	INFO("LOAD %s", argv[1]);
	td_init(&gtd);
	td_read(&gtd, argv[1]);
	td_bin(&gtd, &gtdf0, 60.0, KERN_RECT, 120.0);
	
	return 0;
}


