#ifndef FUNCTION_C
#define FUNCTION_C

#include <stdint.h>
#include "math.c"

typedef struct s_Function1 Function1;
typedef struct s_Function2 Function2;
typedef struct s_Function11 Function11;
typedef struct s_FunctionRanged FunctionRanged;


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

struct s_FunctionRanged {
	int len, memlen;
	int64_t i0;
	V1 dx;
	V2 *mm;
};


void f1_init(Function1 *self, int memlen);
void f1_resize(Function1 *self, int len);
void f1_fini(Function1 *self);
V1 f1_eval_at(Function1 *self, V1 x);
void f1_compile(Function1 *self, char *tex, int w, int h, V2 xrange, V2 yrange);
void f1_append(Function1 *self, V1 y);
V2 f1_minmax(Function1 *self);


void fr_init(FunctionRanged *self, int memlen);
void fr_resize(FunctionRanged *self, int len);
void fr_fini(FunctionRanged *self);
void fr_eval(FunctionRanged *self, Function1 *func, V1 dx);


void f2_init(Function2 *self, int memlen);
void f2_resize(Function2 *self, int len);
void f2_fini(Function2 *self);
V2 f2_minmax(Function2 *self);
void f2_subfunc1(Function2 *self, Function1 *other, V2 xrange);

typedef enum {W_RECTANGLE, W_TRIANGLE, W_HANN, W_HAMMING, W_BLACKMAN, W_BLACKHARRIS, W_APXGAUSS} KernType;
V1 window_func(KernType type, V1 x, V1 kw);
void f1_window(Function1 *self, KernType type, int npts, V1 kw, V2 xrange);

#if __INCLUDE_LEVEL__ == 0

#include <string.h>
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

void fr_init(FunctionRanged *self, int memlen)
{
	memset(self, 0, sizeof(FunctionRanged));
	self->memlen = memlen;
	self->mm = malloc(sizeof(V2) * self->memlen);
	ASSERT(self->mm, "Out of Mem");
}

void fr_fini(FunctionRanged *self)
{
	if(self->mm)
		free(self->mm);
	memset(self, 0, sizeof(FunctionRanged));
}

void fr_resize(FunctionRanged *self, int len)
{
	if (len > self->memlen) {
		self->mm = realloc(self->mm, sizeof(V2)*len);
		ASSERT(self->mm, "Out of Mem");
		self->memlen = len;
	}
	self->len = len;
}

void fr_eval(FunctionRanged *self, Function1 *func, V1 dx)
{
	self->dx = dx;
	self->i0 =  floor(func->x0 / dx);
	fr_resize(self,  floor((func->x0 + func->len * func->dx) / dx) - self->i0 + 1);

	int64_t i=-1, j=0;
	for (j=0; j < func->len; ++j) {
		V1 x = func->x0 + j * func->dx;
		if (x >= (self->i0 + i + 1) * dx) {
			++i;
			ASSERT(i < self->len, "too big %d > %d", i, self->len);
			self->mm[i].x = self->mm[i].y = func->ys[j];
		}
		
		if (func->ys[j] < self->mm[i].x)
			self->mm[i].x = func->ys[j];
		if (func->ys[j] > self->mm[i].y)
			self->mm[i].y = func->ys[j];
	}
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

V2 f1_minmax(Function1 *self)
{
	if (self->len ==0)
		return v2(0.0, 0.0);
	
	V2 mm = v2(self->ys[0], self->ys[0]);
	for (int i=1; i < self->len; ++i) {
		if (self->ys[i] < mm.x)
			mm.x = self->ys[i];
		if (self->ys[i] > mm.y)
			mm.y = self->ys[i];
	}
	return mm;
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



void f1_window(Function1 *self, KernType type, int npts, V1 kw, V2 xrange)
{
	f1_init(self, npts);
	self->x0 = xrange.x;
	self->dx = (xrange.y - xrange.x)/npts;
	for (int i=0; i < npts; ++i) {
		V1 x = self->x0 + i*self->dx;
		f1_append(self, window_func(type, x, kw));
	}
	
}


static V1 window_rectangle(V1 x, V1 kw)
{
	if (x < -kw || x >= kw)
		return 0.0;
	return 1.0;
}

static V1 window_triangle(V1 x, V1 kw)
{
	if (x < -kw || x >= kw)
		return 0.0;
	return 1.0 - (fabs (x) / kw);
}

static V1 window_hann(V1 x, V1 kw)
{
	if (x < -kw || x >= kw)
		return 0.0;
	V1 s = sin(M_PI*(x+kw)/(2.0*kw));
	return s*s;
}

static V1 window_hamming(V1 x, V1 kw)
{
	if (x < -kw || x >= kw)
		return 0.0;
	V1 beta = 21.0/46.0;
	return 1.0 - beta - beta * cos(2.0*M_PI*(x+kw)/(2.0*kw));
}

static V1 window_bk(V1 x, V1 kw, V1 a0, V1 a1, V1 a2, V1 a3)
{
	if (x < -kw || x >= kw)
		return 0.0;
	V1 c = M_PI*(x+kw)/(2.0*kw);
	return a0 - a1*cos(2.0*c) + a2*cos(4.0*c) - a3*cos(6.0*c);
}

static V1 window_blackman(V1 x, V1 kw)
{
	return window_bk(x, kw, 7938.0/18608.0 , 9240.0/18608.0, 1430.0/18608.0, 0.0);
}

static V1 window_blackharris(V1 x, V1 kw)
{
	return window_bk(x, kw, 0.35875 , 0.48829, 0.14128, 0.01168);
}

static V1 gaussian(V1 x, V1 ot, V1 kw)
{
	V1 p = (x - kw) / (2.0*ot);
	return exp(-p*p);
}

static V1 window_approximate_gauss(V1 x, V1 kw)
{
	if (x < -kw || x >= kw)
		return 0.0;
	V1 G(V1 x) { return gaussian(x, 0.1*kw*2.0, kw); }
	V1 num = G(-0.5) * (G(x+kw+2.0*kw) + G(x+kw-2.0*kw));
	V1 denom = G(-0.5 + 2.0*kw) + G(-0.5 - 2.0*kw);
	return G(x+kw) - num/denom;
}

V1 window_func(KernType type, V1 x, V1 kw)
{
	switch(type) {
		case W_RECTANGLE: return window_rectangle(x, kw);
		case W_TRIANGLE: return window_triangle(x, kw);
		case W_HANN: return window_hann(x, kw);
		case W_HAMMING: return window_hamming(x, kw);
		case W_BLACKMAN: return window_blackman(x, kw);
		case W_BLACKHARRIS: return window_blackharris(x, kw);
		case W_APXGAUSS: return window_approximate_gauss(x, kw);
		default: return window_rectangle(x, kw);
	}
}

#endif
#endif
