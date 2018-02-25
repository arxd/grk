#ifndef FUNCTION_C
#define FUNCTION_C

#include "math.c"
typedef struct s_Function1 Function1;
typedef struct s_Function2 Function2;

struct s_Function1 {
	int len, memlen;
	V1 x0;
	V1 dx;
	V1 *ys;
};

struct s_Function2 {
	int len, memlen;
	V2 *pts;
};


void f1_init(Function1 *self, int memlen);
void f1_resize(Function1 *self, int len);
void f1_fini(Function1 *self);
V1 f1_eval_at(Function1 *self, V1 x);
void f1_compile(Function1 *self, char *tex, int w, int h, V2 xrange, V2 yrange);
void f1_append(Function1 *self, V1 y);


void f2_init(Function2 *self, int memlen);
void f2_resize(Function2 *self, int len);
void f2_fini(Function2 *self);
V2 f2_minmax(Function2 *self);
void f2_subfunc1(Function2 *self, Function1 *other, V2 xrange);


#if __INCLUDE_LEVEL__ == 0

#include <string.h>
#include <stdint.h>
#include <math.h>
#include <malloc.h>
#include "logging.c"

void f1_init(Function1 *self, int memlen)
{
	memset(self, 0, sizeof(Function1));
	self->memlen = memlen;
	self->ys = malloc(sizeof(V1) * self->memlen);
	ASSERT(self->ys, "Out of Mem");
}

void f1_fini(Function1 *self)
{
	if(self->ys)
		free(self->ys);
	memset(self, 0, sizeof(Function1));
}

void f1_resize(Function1 *self, int len)
{
	if (len > self->memlen) {
		self->ys = realloc(self->ys, sizeof(V1)*len);
		ASSERT(self->ys, "Out of Mem");
		self->memlen = len;
	}
	self->len = len;
}

void f2_init(Function2 *self, int memlen)
{
	memset(self, 0, sizeof(Function2));
	self->memlen = memlen;
	self->pts = malloc(sizeof(V2) * self->memlen);
	ASSERT(self->pts, "Out of Mem");
}

void f2_fini(Function2 *self)
{
	if(self->pts)
		free(self->pts);
	memset(self, 0, sizeof(Function2));
}

void f2_resize(Function2 *self, int len)
{
	if (len > self->memlen) {
		self->pts = realloc(self->pts, sizeof(V2)*len);
		ASSERT(self->pts, "Out of Mem");
		self->memlen = len;
	}
	self->len = len;
}

void f1_append(Function1 *self, V1 y)
{
	f1_resize(self, self->len+1);
	self->ys[self->len-1] = y;
	
}


V1 f1_eval_at(Function1 *self, V1 x)
{
	double di = (x - self->x0) / self->dx;
	long long i = floor(di);
	double dt = di - i;
	double a = (i < 0)? self->ys[0]: ((i >= self->len)? self->ys[self->len-1]: self->ys[i]);
	++i;
	double b = (i < 0)? self->ys[0]: ((i >= self->len)? self->ys[self->len-1]: self->ys[i]);
	
	return (b-a)*dt + a;
}

V2 f2_minmax(Function2 *self)
{
	if (self->len == 0)
		return v2(0.0, 0.0);
	
	V2 mm = v2(self->pts[0].y, self->pts[0].y);
	for (int i=1; i < self->len; ++i) {
		//~ if (i > 1) INFO("%d %f", i, self->pts[i].y);
		if (self->pts[i].y < mm.x)
			mm.x = self->pts[i].y;
		if (self->pts[i].y > mm.y)
			mm.y = self->pts[i].y;
	}
	return mm;
}

void f2_subfunc2(Function2 *self, Function2 *other, V2 xrange)
{


	
}

void f2_subfunc1(Function2 *self, Function1 *other, V2 xrange)
{
	long long i = nearbyint(ceil((xrange.x - other->x0) / other->dx));
	long long j = nearbyint(ceil((xrange.y - other->x0) / other->dx));
	if (j > other->len)
		j = other->len;
	if (i > j || i < 0)
		i = j;
	f2_resize(self, 2 + j-i);
	self->pts[0] = v2(xrange.x,  f1_eval_at(other, xrange.x));
	int z = 1;
	//~ INFO("%d %d  %d", i, j, other->len);
	while (i < j) {
		self->pts[z++] = v2(other->x0 + i*other->dx ,other->ys[i]);
		++i;
	}
	self->pts[z] = v2(xrange.y, f1_eval_at(other, xrange.y));
}




#endif
#endif
