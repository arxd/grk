#ifndef NNET_C
#define NNET_C

#include "math.c"
#include "function.c"
#include "logging.c"

#define MAX_ROWS 8
#define MAX_INPUTS 1024

typedef struct s_Analytic Analytic;
typedef struct s_Term Term;
typedef struct s_NNet NNet;
typedef V2 AnalyticRegularCoeff(int i);

struct s_Term {
	V1 coeff;
	V1 exp;
	Analytic *x;
};

struct s_Analytic {
	int len;
	V1 y;
	V1 x0;
	Term terms[];
};

struct s_NNet {
	int nrows;
	int cols[MAX_ROWS];
	int ninputs;
	Analytic inputs[MAX_INPUTS];
	Analytic *output;
};


Analytic *an_new(int len);
Analytic *an_new_regular(int len, AnalyticRegularCoeff *func);
V1 an_eval(Analytic *self);
void an_eval_regular_f1(Analytic *self, Function1 *func, V1 x0, V2 xrange, int npts);


void nn_init(NNet *self, int ninputs, int nrows, int cols[]);


#if __INCLUDE_LEVEL__ == 0

#include <malloc.h>
#include <math.h>

Analytic *an_new(int len)
{
	Analytic *self = (Analytic*)malloc(sizeof(Analytic) + sizeof(Term)*len);
	self->len = len;
	self->y = 0.0;
	return self;
}

Analytic *an_new_regular(int len, AnalyticRegularCoeff *func)
{
	Analytic *self = an_new(len);
	Analytic *x = an_new(0);
	for (int i=0; i < len; ++i) {
		V2 out = func(i);
		self->terms[i].coeff = out.x;
		self->terms[i].exp = out.y;
		self->terms[i].x = x;
	}
	return self;
}

V1 an_eval(Analytic *self)
{
	V1 x;
	self->y = 0.0;
	for (int i=0; i < self->len; ++i) {
		if (self->terms[i].x->len)
			x = an_eval(self->terms[i].x);
		else
			x = self->terms[i].x->y;
		self->y += self->terms[i].coeff * pow(x - self->x0, self->terms[i].exp);
	}
	return self->y;
}

void an_eval_regular_f1(Analytic *self, Function1 *func, V1 x0, V2 xrange, int npts)
{
	
	f1_init(func, npts);
	self->x0 = x0;
	func->dx = (xrange.y-xrange.x)/ (npts-1);
	for(int i=0; i < npts; ++i) {
		self->terms[0].x->y = xrange.x + func->dx * i;
		f1_append(func, an_eval(self));
	}
}

void nn_init(NNet *self, int ninputs, int nrows, int cols[])
{
	self->nrows = nrows;
	for (int i=0;i < nrows; ++i )
		self->cols[i] = cols[i];
	self->ninputs = ninputs;
	Analytic **prev = 0, **row;
	int prev_num = ninputs;
	for (int r=0; r < nrows; ++r) {
		row = (Analytic **)malloc(sizeof(Analytic*)*self->cols[r]);
		for (int c = 0; c < self->cols[r]; ++c) {
			row[c] = an_new(prev_num);
			for (int j=0; j < prev_num; ++j) 
				row[c]->terms[j].x = prev? prev[j] : &self->inputs[j];
		}
		prev_num = self->cols[r];
		if (prev)
			free(prev);
		prev = row;
	}
	self->output = an_new(prev_num);
	for (int j=0; j < prev_num; ++j) 
		self->output->terms[j].x = prev? prev[j] : &self->inputs[j];
	if (prev)
		free(prev);
	
}


#endif
#endif
