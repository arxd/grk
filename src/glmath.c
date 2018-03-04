#ifndef GLMATH_C
#define GLMATH_C

#include "math.c"
#include "function.c"
#include "gl.c"

typedef struct s_View View;
typedef union s_Color Color;


struct s_View {
	V2 origin; // origin of this view in screen coordinates
	V2 drag;  // A delta added to origin for dragging
	V2 vps; // view per screen (unit/px)
	V2 ll;
	V2 ur;
};
extern View view;

union s_Color {
	float rgba[4];
	struct {
		float r,g,b,a;
	};
};
extern Color color;

Color rgb(V1 r, V1 g, V1 b);
Color hsv(V1 h, V1 s, V1 v);
Color hue(V1 hue);


void view_drag(View *self, V2 screen_dxy);
void view_drag_stop(View *self);
void view_fit(View *self, Function1 *func);
V2 unit_to_screen(View *self, V2 view_xy);
V2 screen_to_view(View *self, V2 xy_screen);
void view_zoom_at(View *self, V2 xy_view, V2 zoom);
V2 view_LL(View *self);
V2 view_UR(View *self);

void grid_render_init(void);
void grid_render(View *view,  const char* fmtx, const char* fmty, V2 major, V2 minor, Color color);
void grid_time_render(View *view,  Color clr);
void grid_vert_render(View *view, Color clr);

void f1_render_init(void);
void f1_render(Function1 *self, View *view, Color color);

//~ void draw_color(float r, float g, float b, float a);
//~ void draw_line_strip(GLfloat xy[2], GLfloat scale[2], GLfloat angle, int npts, GLfloat *pts);
//~ void draw_line_loop(GLfloat xy[2], GLfloat scale[2], GLfloat angle, int npts, GLfloat *pts);
//~ void draw_lines(GLfloat xy[2], GLfloat scale[2], GLfloat angle, int npts, GLfloat *pts);
//~ void draw_polygon(GLfloat xy[2], GLfloat scale[2], GLfloat angle, int npts, GLfloat *pts);
//~ void draw_circle(GLfloat xy[2], GLfloat scale[2]);
void geom_send(GLenum mode, GLuint aPos, int npts, GLfloat *pts);
void draw_line_strip(V2 xy, V2 scale, V1 angle, int npts, GLfloat *pts);
void geom_init(void);
void bind_geom(GLuint aPos, int npts, GLfloat *pts);

void font_init(void);
void font_render(V2 screen_xy, const char *fmt, ...);
void font_test(void);

#if __INCLUDE_LEVEL__ == 0

#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "logging.c"
#include "shaders.h"

View view;
Color color;
static Shader g_grid_shader = {0};
static GLfloat GEOM_FSRECT[2*4] = {-1.0,-1.0,  -1.0,1.0,  1.0,-1.0,  1.0,1.0};
static GLfloat GEOM_RECT[] = {0.0,0.0,   0.0,1.0,   1.0,0.0,   1.0,1.0};


Color rgb(V1 r, V1 g, V1 b)
{
	return (Color){r, g, b, 1.0};
}

Color hsv(V1 h, V1 s, V1 v)
{
	
}

Color hue(V1 hue)
{
	return hsv(hue, 1.0, 0.5);
}


void view_drag_screen(View *self, V2 screen_dxy)
{
	self->drag = v2mul2(screen_dxy, self->vps);
	self->ll = screen_to_view(self, v2(0.0, 0.0));
	self->ur = screen_to_view(self, v2(GW.w , GW.h ));
}

void view_drag_stop(View *self)
{
	self->origin = v2add(self->origin, self->drag);
	self->drag = v2(0.0, 0.0);
}

V2 view_to_screen(View *self, V2 xy_view)
{
	return v2add(v2div2(xy_view, self->vps), v2add(self->origin, self->drag));
}

V2 screen_to_view(View *self, V2 xy_screen)
{
	return v2mul2(v2sub(xy_screen, v2add(self->origin, self->drag)), self->vps);
}

void view_zoom_at(View *self, V2 xy_view, V2 zoom)
{
	V2 screen = view_to_screen(self, xy_view);
	self->vps = v2mul2(self->vps, zoom);
	V2 delta = v2sub(screen_to_view(self, screen), xy_view);
	self->origin = v2add(self->origin, v2div2(delta, self->vps));
	self->ll = screen_to_view(self, v2(0.0, 0.0));
	self->ur = screen_to_view(self, v2(GW.w , GW.h ));

}

void view_fit(View *self, Function1 *func)
{
	V2 xrange = v2(func->x0, func->len*func->dx);
	V2 yrange = f1_minmax(func);
	
	V1 yvps = (yrange.y - yrange.x) / GW.h;
	V1 xvps = (xrange.y - xrange.x) / GW.w;
	INFO("view_fit   xrange (%f, %f) %f   yrange(%f, %f) %f", xrange.x, xrange.y, xvps, yrange.x, yrange.y, yvps);
	self->origin = v2(-xrange.x / xvps, -yrange.x / yvps);// 0.0);//mm.x/yvps);
	self->vps = v2(xvps, yvps);
	self->drag = v2(0.0, 0.0);
	self->ll = screen_to_view(self, v2(0.0, 0.0));
	self->ur = screen_to_view(self, v2(GW.w , GW.h ));

}


/* ============== Function =============
*/

static Shader g_f1_shader = {0};
static Texture g_f1_texture = {0, 128, 128, GL_ALPHA, GL_UNSIGNED_BYTE};

void f1_render_init(void)
{
	if (g_f1_shader.id)
		return;
	
	ASSERT(shader_init(&g_f1_shader, V_STRAIGHT, F_DFUN, (char*[]){
		"aPos", "uFramebuffer", "uColor", NULL}),  "Couldn't create shader");
	on_exit(shader_on_exit, &g_f1_shader);
	tex_set(&g_f1_texture, 0);
}

void f1_render(Function1 *self, View *view, Color color)
{
	char *tex = alloca(128*128);
	V1 x0 = 0.0 - (view->origin.x + view->drag.x) * view->vps.x;
	V1 y0 = 0.0 - (view->origin.y + view->drag.y) * view->vps.y;
	V2 x, y;
	Function2 subfunc;
	f2_init(&subfunc,1024);
	for(int i=0; i < GW.w; ++i) {
		x = v2(x0 + i*view->vps.x, x0 + (i+1)*view->vps.x);
		f2_subfunc1(&subfunc, self, x);
		//~ INFO("%f =  %f %f", subfunc.pts[0].y, x.x);
		y = v2(subfunc.pts[0].y, subfunc.pts[0].y);//
		y = f2_minmax(&subfunc);
		// scale it
		y = v2div(v2sub(y, v2(y0, y0)), view->vps.y);
		uint16_t ymin = 0xFFFE;
		uint16_t ymax = 0xFFFF;
		if (!(y.y < 0.0 || y.x >= GW.h || x.y <= self->x0 || x.x >= self->x0 + self->dx*self->len)) {
			ymin = (y.x < 0.0)? 0 : nearbyint(y.x);
			ymax = (y.y >= GW.h)? GW.h-1 : nearbyint(y.y);
		}
		uint16_t ym = (ymin < ymax)? ymin: ymax;
		uint16_t yx = (ymin > ymax)? ymin: ((ymin==ymax)? ymin+1: ymax);
		tex[i*4+0] = (ym>>8)&0xFF;
		tex[i*4+1] = ym&0xFF;
		tex[i*4+2] = (yx>>8)&0xFF;
		tex[i*4+3] = yx&0xFF;	
	}
	
	//~ compile_graph(&gph0, fun, v2(x0-dx0, x0-dx0 + GW.w*ppx), v2(-1.0, 1.0));
	
	glUseProgram(g_f1_shader.id);
	tex_set(&g_f1_texture, tex);
	glUniform1i(g_f1_shader.args[1], 0); // uFramebuffer
	glUniform4fv(g_f1_shader.args[2], 1, color.rgba); // uColor
	geom_send(GL_TRIANGLE_STRIP, g_f1_shader.args[0], 4, GEOM_FSRECT);
}


/* ============== Geometry =============
*/
static Shader g_geom_shader = {0};
static GLuint g_geom_vb=0;
static GLuint g_circle_vb=0;
static const int N_CIRCLE_PTS = 180;

void geom_init(void)
{
	if (!g_geom_shader.id) {	
		ASSERT(shader_init(&g_geom_shader,V_GEOM, F_FLAT, (char*[]){
			"aPos", "uOrigin","uVps", "uScreen", "uTranslate", "uScale",  "uAngle", "uColor",  NULL}
			), "Couldn't create g_line_shader shader");
		on_exit(shader_on_exit, &g_geom_shader);
	}
	glDeleteBuffers(1, &g_geom_vb);
	glGenBuffers(1, &g_geom_vb);
	
	glDeleteBuffers(1, &g_circle_vb);
	glGenBuffers(1, &g_circle_vb);
	GLfloat cdat[N_CIRCLE_PTS*2];
	for (int i=0; i < N_CIRCLE_PTS; ++i) {
		cdat[i*2+0] = cos(2.0*M_PI*i / N_CIRCLE_PTS);
		cdat[i*2+1] = sin(2.0*M_PI*i / N_CIRCLE_PTS);
	}
	glBindBuffer(GL_ARRAY_BUFFER, g_circle_vb);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2*N_CIRCLE_PTS, cdat, GL_STATIC_DRAW);
}



static void geom_render(GLenum mode, V2 xy, V2 scale, V1 angle, int npts, GLfloat *pts)
{
	glUseProgram(g_geom_shader.id);
	//~ glBindBuffer(GL_ARRAY_BUFFER, g_geom_vb);
	//~ glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2*npts, pts, GL_STATIC_DRAW);
	//~ glVertexAttribPointer(g_geom_shader.args[0], 2, GL_FLOAT, 0, 0, 0);
	//~ glEnableVertexAttribArray(g_geom_shader.args[0]);
	//~ glUniformMatrix3fv(g_line_shader.args[1], 1, GL_FALSE, &GW.vmat[0][0]);// uScreen
	glUniform2f(g_geom_shader.args[1], view.origin.x+view.drag.x, view.origin.y+view.drag.y); // uOrigin
	glUniform2f(g_geom_shader.args[2], view.vps.x, view.vps.y); // uVps
	glUniform2f(g_geom_shader.args[3], GW.w, GW.h); // uScreen
	glUniform2f(g_geom_shader.args[4], xy.x, xy.y); // uTranslate
	glUniform2f(g_geom_shader.args[5], scale.x, scale.y); // uScale
	glUniform1f(g_geom_shader.args[6], angle*M_PI/180.0); //uAngle
	glUniform4fv(g_geom_shader.args[7], 1, color.rgba); //uColor
	//~ glDrawArrays(mode, 0, npts);
	geom_send(mode, g_geom_shader.args[0], npts, pts);
}

void bind_geom(GLuint aPos, int npts, GLfloat *pts)
{
	glBindBuffer(GL_ARRAY_BUFFER, g_geom_vb);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2*npts, pts, GL_STATIC_DRAW);
	glVertexAttribPointer(aPos, 2, GL_FLOAT, 0, 0, 0);
	glEnableVertexAttribArray(aPos);	
}

void geom_send(GLenum mode, GLuint aPos, int npts, GLfloat *pts)
{
	bind_geom(aPos, npts, pts);
	glDrawArrays(mode, 0, npts);
}


void circle_render(GLuint aPos, V2 ll, V2 ur)
{
	
	
}

void draw_line_strip(V2 xy, V2 scale, V1 angle, int npts, GLfloat *pts)
{
	geom_render(GL_LINE_STRIP, xy, scale, angle, npts, pts);
}

//~ void draw_line_loop(GLfloat xy[2], GLfloat scale[2], GLfloat angle, int npts, GLfloat *pts)
//~ {
	//~ geom_render(GL_LINE_LOOP, xy, scale, angle, npts, pts);
//~ }

//~ void draw_lines(GLfloat xy[2], GLfloat scale[2], GLfloat angle, int npts, GLfloat *pts)
//~ {
	//~ geom_render(GL_LINES, xy, scale, angle, npts, pts);
//~ }

//~ void draw_polygon(GLfloat xy[2], GLfloat scale[2], GLfloat angle, int npts, GLfloat *pts)
//~ {
	//~ geom_render(GL_TRIANGLE_FAN, xy, scale, angle, npts, pts);
//~ }
/* ============== GRID =============
*/
typedef struct s_Range Range;
struct s_Range {
	V1 vpg, scale;
	int lab;
	const char *fmt;
};


Range time_base[] = {
	{100*31557600.0, 1.0/31557600.0, 5, "%.0fy"},
	{50*31557600.0, 1.0/31557600.0, 2, "%.0fy"},
	{25*31557600.0, 1.0/31557600.0, 4, "%.0fy"},
	{10*31557600.0, 1.0/31557600.0, 5, "%.0fy"},
	{5*31557600.0, 1.0/31557600.0, 2, "%.0fy"},
	{31557600.0, 1.0/31557600.0, 5, "%.0fy"},
	{31557600.0/3, 1.0/31557600.0, 4, "%.0fy"},
	{31557600.0/12, 1.0/(31557600.0/12), 3, "%.0fmth"},
	{7*86400.0, 1.0/(7*86400.0), 4, "%.0fw"},
	{86400.0, 1.0/(7*86400.0), 7, "%.0fw"},
	{12*3600.0, 1.0/86400.0, 2, "%.0fd"},
	{4*3600.0, 1.0/3600.0, 3, "%.0fh"},
	{3600.0, 1.0/3600.0, 5, "%.0fh"},
	{30*60.0, 1.0/3600.0, 2, "%.0fh"},
	{10*60.0, 1.0/60.0, 3, "%.0fm"},
	{5*60.0, 1.0/60.0, 2, "%.0fm"},
	{60.0, 1.0/60.0, 5, "%.0fm"},
	{30.0, 1.0/60.0, 2, "%.0fm"},
	{10.0, 1.0, 3, "%.0fs"},
	{5.0, 1.0, 2, "%.0fs"},
	{1.0, 1.0, 5, "%.0fs"},
	{500.0e-3, 1.0, 2, "%.0fs"},
	{250.0e-3, 1.0, 4, "%.0fs"},
	{100.0e-3, 1.0e3, 5, "%.0fms"},
	{50.0e-3, 1.0e3, 2, "%.0fms"},
	{25.0e-3, 1.0e3, 4, "%.0fms"},
	{10e-3, 1.0e3, 5, "%.0fms"},
	{5.0e-3, 1.0e3, 2, "%.0fms"},
	{1.0e-3, 1.0e3, 5, "%.0fms"},
	{500.0e-6, 1.0e3, 2, "%.0fms"},
	{250.0e-6, 1.0e3, 4, "%.0fms"},
	{100.0e-6, 1.0e6, 5, "%.0fus"},
	{50.0e-6, 1.0e6, 2, "%.0fus"},
	{25.0e-6, 1.0e6, 4, "%.0fus"},
	{10e-6, 1.0e6, 5, "%.0fus"},
	{5.0e-6, 1.0e6, 2, "%.0fus"},
	{1.0e-6, 1.0e6, 5, "%.0fus"},
	{500.0e-9, 1.0e6, 2, "%.0fus"},
	{250.0e-9, 1.0e6, 4, "%.0fus"},
	{100.0e-9, 1.0e9, 5, "%.0fns"},
	{50.0e-9, 1.0e9, 2, "%.0fns"},
	{25.0e-9, 1.0e9, 4, "%.0fns"},
	{10e-9, 1.0e9, 5, "%.0fns"},
	{5.0e-9, 1.0e9, 2, "%.0fns"},
	{1.0e-9, 1.0e9, 5, "%.0fns"},
	{500.0e-12, 1.0e9, 2, "%.0fns"},
	{250.0e-12, 1.0e9, 4, "%.0fns"},
};
 

void grid_render_init(void)
{
	if (g_grid_shader.id)
		return;
	ASSERT(shader_init(&g_grid_shader, V_STRAIGHT, F_GRID, (char*[]){
		"aPos", "uOrigin", "uVps", "uMajor", "uMinor", "uColor", NULL}), "Couldn't create shader"); // unit
	on_exit(shader_on_exit, &g_grid_shader);
}

void screen_line(V2 p1, V2 p2, Color c)
{
	bind_geom(g_geom_shader.args[0], 2, (GLfloat[]){p1.x, p1.y, p2.x, p2.y});
	glUniform2f(g_geom_shader.args[1], 0.0, 0.0); // uOrigin
	glUniform2f(g_geom_shader.args[2], 1.0, 1.0); // uVps
	glUniform2f(g_geom_shader.args[3], GW.w, GW.h); // uScreen
	glUniform2f(g_geom_shader.args[4], 0.0, 0.0); // uTranslate
	glUniform2f(g_geom_shader.args[5], 1.0,  1.0); // uScale
	glUniform1f(g_geom_shader.args[6], 0.0); //uAngle
	glUniform4fv(g_geom_shader.args[7], 1, c.rgba); //uColor
	glDrawArrays(GL_LINES, 0, 2);
}

void grid_vert_render(View *view, Color clr)
{
	
	V2 ll = screen_to_view(view, v2(0.0, 0.0));
	V2 ur = screen_to_view(view, v2(GW.w, GW.h));
	V1 dy = ur.y - ll.y;
	V1 grid = dy *20.0 / GW.h;
	V1 range = floor(log(grid) / log(10.0));
	V1 multipliers[] = {1.0, 2.0, 5.0, 10.0};
	V1 mdist = fabs( pow(10, range) - grid);
	int mi = 0;
	for( int i=1; i < 4; ++i) {
		V1 dist = fabs( pow(10, range)*multipliers[i] - grid);
		if (dist < mdist) {
			mdist = dist;
			mi = i;
		}
	}
	range = pow(10, range)*multipliers[mi];//  /  view->vps.y;
	int num_lines = ceil(dy / range);
	for (int i=0; i < num_lines; ++i) {
		if (i%5==0) {
			clr.a = 0.8;
		} else {
			clr.a = 0.3;
		}
		screen_line(v2(0.0, i*range / view->vps.y), v2(GW.w, i*range/ view->vps.y), clr);
	}
	color = rgb(1.0, 1.0, 1.0);
	font_render(v2(10.0,GW.h-20.0), "%f", range);
	//~ font_render(v2(100.0,100.0), "%f  %f", 1.0/view.vps.x);

	//~ dy = 
	//~ V1 half = GW.h / 2.0;
	//~ half/dy = pixels/
	
}

void grid_time_render(View *view,  Color clr)
{
	V2 ll = screen_to_view(view, v2(0.0, 0.0));
	V2 ur = screen_to_view(view, v2(GW.w, GW.h));
	int i=47;
	while (--i > 0 && time_base[i].vpg / view->vps.x < 20.0);
	long long dx = floor(ll.x / time_base[i].vpg);
	while (dx * time_base[i].vpg < ceil(ur.x)) {
		V2 fxy = view_to_screen(view, v2(dx*time_base[i].vpg, 0.0));
		fxy.y = 0.0;
		clr.a = 0.3;
		if (dx%time_base[i].lab) {
			glLineWidth(1.0);
		} else {
			glLineWidth(2.0);
			color = clr;
			font_render(v2(fxy.x+2, fxy.y), time_base[i].fmt, dx*time_base[i].vpg*time_base[i].scale);
		}
		glUseProgram(g_geom_shader.id);
		screen_line(fxy, v2(fxy.x, GW.h), clr);
		dx ++;
	}
}

/** ========== FONT ==============
*/
uint32_t font_data[325] = {0xfc0, 0x0, 0xfc000000, 0x0, 0xfc0, 0x0, 0xfff00000, 0x0, 0xfc0, 0x0, 
	0xffffc000, 0x0, 0xfc0, 0x0, 0xffffff00, 0x0, 0xfc0, 0x0, 0xfffffffc, 0x104104, 0xfc0, 0xf0000000, 
	0xffffffff, 0x8420e, 0xfc0, 0xffc00000, 0xffffffff, 0x7d57d5, 0xfc0, 0xffff0000, 0xffffffff, 0x8e204, 
	0xfc0, 0xfffffc00, 0xffffffff, 0x104104, 0xfc0, 0xfffffff0, 0xffffffff, 0x0, 0xc0000fc0, 0xffffffff, 0xffffffff, 
	0x0, 0xff000fc0, 0xffffffff, 0xffffffff, 0x0, 0xfffc0fc0, 0xffffffff, 0xffffffff, 0x0, 0xc30c00c, 0x0, 0x0, 
	0x2200000, 0xc30c00c, 0x0, 0x10028400, 0x4104012, 0xc30c00c, 0x0, 0x78a28400, 0x4104095, 
	0xc30c00c, 0x0, 0x14a28400, 0x808414a, 0xc30c00c, 0xe1042800, 0x15f00400, 0x8080148, 
	0x3c3fffcc, 0x538e7cff, 0x38a00401, 0x8080084, 0x3c3fffcc, 0xf7df7cff, 0x51f00401, 0x8080142, 
	0xc00c, 0x410e38c3, 0x50a00400, 0x808064a, 0xc00c, 0xe38410c3, 0x3ca00000, 0x4100255, 
	0xc00c, 0xc3, 0x10000400, 0x4100589, 0xc00c, 0xc3, 0x0, 0x2200000, 0xc00c, 0xc3, 0x0, 0x0, 
	0xc00c, 0xc3, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4, 0x87ce1044, 0x38e7ce7c, 0x1010000, 0x15, 
	0x841118a4, 0x45141104, 0x2008000, 0x10e, 0xc2111512, 0x45120104, 0x4004104, 0x115, 
	0xa1101112, 0x45120134, 0x87c238e, 0x7c07c4, 0xa3881111, 0x78e10f4c, 0x10001104, 
	0x80000100, 0x94041110, 0x41111140, 0x8002000, 0x80000100, 0xf4021110, 0x41109141, 
	0x47c4000, 0x4400c000, 0x844110a0, 0x45109144, 0x2008304, 0x4e004000, 0x839f7c40, 
	0x38e08e38, 0x101010e, 0x4002000, 0x0, 0x0, 0x84, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
	0x0, 0x0, 0x0, 0x0, 0x0, 0xce3c438e, 0xe44e7df3, 0x45105170, 0xe3ce3ce, 0x9148a451, 
	0x44510414, 0x4d105120, 0x11451451, 0x81491451, 0x44410414, 0x4db04920, 0x1451451, 
	0x81491650, 0x44410414, 0x55504520, 0x1451451, 0x81391548, 0x47c13cf4, 0x55504320, 
	0xe3d13d1, 0x8149f544, 0x44590414, 0x65104520, 0x10151051, 0x81491344, 0x44510414, 
	0x65104920, 0x10251051, 0x91491040, 0x44510414, 0x45105124, 0x11455051, 0xce3d1784, 
	0xe44e05f3, 0x4517d118, 0xe44e04e, 0x0, 0x0, 0x0, 0x10000, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
	0x0, 0x0, 0x0, 0x3803800, 0x400, 0x0, 0x5145145f, 0x420109f4, 0x100800, 0x100c010, 
	0x51451444, 0xa2010904, 0x100000, 0x1012010, 0x8a451444, 0x12020882, 0x100001, 
	0x1002010, 0x8a451444, 0x2020882, 0x38f38000, 0xd38239e, 0x454a444, 0x2040841, 0x45140000, 
	0x1344f451, 0xa54a444, 0x2080821, 0x5178000, 0x114427d1, 0xa54a444, 0x2080821, 0x5144000, 
	0x11442051, 0x11544444, 0x2100811, 0x45164000, 0x11782451, 0x11284384, 0x21009f1, 0x38f58000, 
	0x1140239e, 0x0, 0x3803800, 0x7c, 0x440000, 0x0, 0x0, 0x0, 0x380000, 0x0, 0x0, 0x0, 0x0, 
	0x0, 0x0, 0x0, 0x3018000, 0x181000, 0x0, 0x0, 0x4104000, 0x101204, 0x0, 0x8, 0x4104000, 
	0x101000, 0x0, 0x8, 0x4104000, 0x4b109306, 0xe35e3ce3, 0x4514513c, 0x41047d1, 0xd5105204, 
	0x14d14514, 0x29145109, 0x18103211, 0x55103204, 0x60514514, 0x11545108, 0x4104111, 
	0x55105204, 0x80514514, 0x11529108, 0x4104099, 0x55109204, 0x105e3d14, 0x29529949, 
	0x4104056, 0x5139124e, 0xe05004e4, 0x44a11630, 0x41047d0, 0x240, 0x100400, 0x0, 0x3018011, 
	0x180, 0x100400, 0x0, 0xe, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x12, 0x0, 0x0, 0x0, 0x15, 
	0x0, 0x0, 0x0, 0x9};

static Shader g_font_shader = {0};
static Texture g_font_texture = {0, 128, 128, GL_ALPHA, GL_UNSIGNED_BYTE};

void font_init(void)
{
	if (g_font_shader.id)
		return;
	
	ASSERT(shader_init(&g_font_shader,V_GEOM, F_FONT, (char*[]){
		"aPos", "uOrigin","uVps", "uScreen", "uTranslate", "uScale",  "uAngle", "uFrameBuffer", "uChar", "uColor",  NULL}
		), "Couldn't create g_font_shader");
	on_exit(shader_on_exit, &g_font_shader);
		
	uint8_t *tex = alloca(128*128);
	memset(tex, 0, 128*128);
	for (int b=0; b < 325*32; ++b)
		tex[128*(127 - b/128) + (b%128)] = 255 * ((font_data[b / 32] >> (b%32))&1);
	tex_set(&g_font_texture, tex);
	
}

void font_render(V2 screen_xy, const char *fmt, ...)
{
	char buffer[1024];
	va_list args;
	va_start (args, fmt);
	vsnprintf (buffer, 1024, fmt, args);	
	va_end (args);
	
	glUseProgram(g_font_shader.id);
	glBindTexture(GL_TEXTURE_2D, g_font_texture.id);
	bind_geom(g_font_shader.args[0], 4, GEOM_RECT);
	glUniform2f(g_font_shader.args[1], 0.0, 0.0); // uOrigin
	glUniform2f(g_font_shader.args[2], 1.0, 1.0); // uVps
	glUniform2f(g_font_shader.args[3], GW.w, GW.h); // uScreen
	// 4 uTranslate
	glUniform2f(g_font_shader.args[5], 6.0, 13.0); // uScale
	glUniform1f(g_font_shader.args[6], 0.0); //uAngle
	glUniform1i(g_font_shader.args[7], 0); // uFramebuffer
	// 8 charid
	glUniform4fv(g_font_shader.args[9], 1, color.rgba); //uColor
	
	int i = -1;
	V1 x0 = screen_xy.x;
	while (buffer[++i]) {
		if (buffer[i] == '\n') {
			screen_xy.x = x0;
			screen_xy.y += 13.0;
		} else {
			glUniform2f(g_font_shader.args[4], screen_xy.x, screen_xy.y); // uTranslate
			glUniform1f(g_font_shader.args[8], buffer[i]);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			screen_xy.x += 6.0;
		}
	}
}

#endif
#endif
