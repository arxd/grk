#ifndef GLMATH_C
#define GLMATH_C

#include "math.c"
#include "function.c"

typedef struct s_View View;
struct s_View {
	V2 origin; // origin of this view in screen coordinates
	V2 drag;  // A delta added to origin for dragging
	V2 vps; // view per screen (unit/px)
};

typedef union s_Color Color;

union s_Color {
	float rgba[4];
	struct {
		float r,g,b,a;
	};
};

Color rgb(V1 r, V1 g, V1 b);
Color hsv(V1 h, V1 s, V1 v);
Color hue(V1 hue);


void view_drag(View *self, V2 screen_dxy);
void view_drag_stop(View *self);
void view_set(View *self, V2 xrange, V2 yrange, View *screen);
V2 unit_to_screen(View *self, V2 view_xy);
V2 screen_to_unit(View *self, V2 screen_xy);

void grid_render_init(void);
void grid_render(View *view, V2 major, V2 minor, Color color);

void f1_render_init(void);
void f1_render(Function1 *self, View *view, Color color);


#if __INCLUDE_LEVEL__ == 0

#include <math.h>
#include "gl.c"
#include "logging.c"
#include "shaders.h"

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
	return v2div2(v2sub(xy_view, v2add(self->origin, self->drag)), self->vps);
}

V2 screen_to_view(View *self, V2 xy_screen)
{
	return v2sub(v2mul2(xy_screen, self->vps), v2add(self->origin, self->drag));
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
	draw_rect(g_grid_shader.args[0], (GLfloat[]){-1.0, -1.0}, (GLfloat[]){1.0, 1.0});	
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
	draw_rect(g_f1_shader.args[0], (GLfloat[]){-1.0, -1.0}, (GLfloat[]){1.0, 1.0});
}


#endif
#endif
