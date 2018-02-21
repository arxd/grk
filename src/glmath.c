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
void view_set(View *self, V2 xrange, V2 yrange, View *screen);
V2 unit_to_screen(View *self, V2 view_xy);
V2 screen_to_view(View *self, V2 xy_screen);
void view_zoom_at(View *self, V2 xy_view, V2 zoom);

void grid_render_init(void);
void grid_render(View *view, V2 major, V2 minor, Color color);

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


#if __INCLUDE_LEVEL__ == 0

#include <math.h>
#include "logging.c"
#include "shaders.h"

View view;
Color color;


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
}

void view_drag_stop(View *self)
{
	self->origin = v2add(self->origin, self->drag);
	self->drag = v2(0.0, 0.0);
}

V2 view_to_screen(View *self, V2 xy_view)
{
	return v2sub(v2div2(xy_view, self->vps), v2add(self->origin, self->drag));
}

V2 screen_to_view(View *self, V2 xy_screen)
{
	return v2mul2(v2sub(xy_screen, v2add(self->origin, self->drag)), self->vps);
}

void view_zoom_at(View *self, V2 xy_view, V2 zoom)
{
	V2 screen = view_to_screen(self, xy_view);
	INFO("screen %s", v2str(screen));
	
	V2 sxy = v2mul2(xy_view, self->vps);
	
	self->vps = v2mul2(self->vps, zoom);
	V2 xy1 = v2mul2(xy_view, self->vps);
	//~ INFO("%s  %s", v2str(xy0), v2str(xy1));
}

void view_set(View *self, V2 xrange, V2 yrange, View *other)
{
	//~ View screen;
	//~ if (!other) {
		//~ other = &screen;
		//~ screen.drag = v2(0.0, 0.0);
		//~ screen.offset = v2(0.0, 0.0);
		//~ screen.scale = v2(GW.w, GW.h);
	//~ }
	
	//~ self->drag = v2(0.0, 0.0);
	//~ self->offset = v2(
}

/* ============== GRID =============
*/
static Shader g_grid_shader = {0};
static GLfloat FS_RECT_PTS[2*4] = {-1.0, -1.0, -1.0, 1.0, 1.0, -1.0, 1.0, 1.0};

void grid_render_init(void)
{
	if (g_grid_shader.id)
		return;
	ASSERT(shader_init(&g_grid_shader, V_STRAIGHT, F_GRID, (char*[]){
		"aPos", "uOrigin", "uVps", "uMajor", "uMinor", "uColor", NULL}), "Couldn't create shader"); // unit
	on_exit(shader_on_exit, &g_grid_shader);
}

void grid_render(View *view, V2 major, V2 minor, Color color)
{
	glUseProgram(g_grid_shader.id);
	glUniform2f(g_grid_shader.args[1], view->origin.x + view->drag.x, view->origin.y + view->drag.y);// uOffset
	glUniform2f(g_grid_shader.args[2], view->vps.x, view->vps.y);// uScale
	glUniform2f(g_grid_shader.args[3], major.x, major.y);// uMajor
	glUniform2f(g_grid_shader.args[4], minor.x, minor.y);// uMinor
	glUniform4fv(g_grid_shader.args[5], 1, color.rgba);// uColor
	geom_send(GL_TRIANGLE_STRIP, g_grid_shader.args[0], 4, FS_RECT_PTS);
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
	//~ V2 xrange = v2(offset.x, offset.x+GW.w*scale.x);
	//~ V2 yrange = v2(offset.y, offset.y + GW.h*scale.y);
	char *tex = alloca(128*128);
	V1 x0 = 0.0 - view->origin.x * view->vps.x;
	V1 y0 = 0.0 - view->origin.y * view->vps.y;
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
		ymin = (ymin < ymax)? ymin: ymax;
		ymax = (ymin > ymax)? ymin: ((ymin==ymax)? ymin+1: ymax);
		tex[i*4+0] = (ymin>>8)&0xFF;
		tex[i*4+1] = ymin&0xFF;
		tex[i*4+2] = (ymax>>8)&0xFF;
		tex[i*4+3] = ymax&0xFF;	
	}
	
	//~ compile_graph(&gph0, fun, v2(x0-dx0, x0-dx0 + GW.w*ppx), v2(-1.0, 1.0));
	
	glUseProgram(g_f1_shader.id);
	tex_set(&g_f1_texture, tex);
	glUniform1i(g_f1_shader.args[1], 0); // uFramebuffer
	glUniform4fv(g_f1_shader.args[2], 1, color.rgba); // uColor
	geom_send(GL_TRIANGLE_STRIP, g_f1_shader.args[0], 4, FS_RECT_PTS);
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

void geom_send(GLenum mode, GLuint aPos, int npts, GLfloat *pts)
{
	//~ GLfloat pts[4*2] = {ll.x, ll.y, ll.x, ur.y, ur.x, ll.y, ur.x, ur.y};
	glBindBuffer(GL_ARRAY_BUFFER, g_geom_vb);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2*npts, pts, GL_STATIC_DRAW);
	glVertexAttribPointer(aPos, 2, GL_FLOAT, 0, 0, 0);
	glEnableVertexAttribArray(aPos);
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


#endif
#endif
