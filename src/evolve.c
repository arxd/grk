#ifndef EVOLVE_C
#define EVOLVE_C

#include "math.c"
#include "function.c"
#include "logging.c"
#include "nnet.c"

typedef struct s_DNA DNA;

struct s_DNA {
	int len;
	Analytic *func;
	V1 vars[];
};

DNA *dna_random(int len);

#if __INCLUDE_LEVEL__ == 0

#include <malloc.h>
#include <math.h>
#include <stdlib.h>

DNA *dna_random(int len)
{
	DNA *self = (DNA*)malloc(sizeof(DNA) + sizeof(V1)*len);
	self->len = len;
	for (int i=0; i < len; ++i)
		self->vars[i] = rand()/RAND_MAX;
	
	return self;
}






#endif
#endif
