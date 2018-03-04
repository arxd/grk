#ifndef TRADES_C
#define TRADES_C

#include <stdlib.h>
#include "function.c"
#include "math.c"

typedef struct s_Trade Trade;
typedef struct s_TradeData TradeData;

struct s_TradeData {
	unsigned long long len;
	unsigned long long last;
	Trade *data;
};

struct s_Trade {
	double t;
	double val;
	double amt;
};


void td_init(TradeData *self);
void td_fini(TradeData *self);
void td_write(TradeData *self, const char *filename);
void td_read(TradeData *self, const char *filename);
TradeData* td_merge(TradeData *self, TradeData *other);
void td_bin(TradeData *self, Function1 *func, V1 sample_rate, KernType ktype, V1 kwidth, int causal, V1 since);

#if __INCLUDE_LEVEL__ == 0 || defined(PIXIE_NOLIB)

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include "logging.c"

void td_init(TradeData *self)
{
	memset(self, 0, sizeof(TradeData));
	
}

void td_fini(TradeData *self)
{
	if (self->data) {
		XINFO("Free data");
		free(self->data);
	}
	memset(self, 0, sizeof(TradeData));
}

void td_write(TradeData *self, const char *filename)
{
	FILE *fout = fopen(filename, "wb");
	ASSERT(fout, "Couldn't open '%s' for writing", filename);	
	ASSERT( fwrite(self, 8, 2, fout) == 2, "Can't write to '%s'",filename);
	INFO("Writing %d records to '%s'", self->len, filename);
	ASSERT( fwrite(self->data, 8*3, self->len, fout) == self->len, "Can't write to '%s'", filename);
	fclose(fout);
}

void td_bin(TradeData *self, Function1 *func, V1 sample_rate, KernType ktype, V1 ksize, int causal, V1 since)
{
	V1 tN = self->data[self->len-1].t;
	V1 t0 = since? tN - since : self->data[0].t;
		
	INFO("From %f s by %f  (%d ... %d)", tN-t0, sample_rate,  self->len, (int)((tN-t0) / sample_rate));
	
	f1_init(func, (tN-t0) / sample_rate + 1);
	func->x0 = 0.0;
	func->dx = sample_rate;
	
	V1 t = t0;
	int a = 0, b;
	while (t <= tN) {
		while (self->data[a].t < t - ksize)
			a++;
		b = a;
		while (b < self->len && self->data[b].t < t + ksize) {
			b++;
		}
		V1 mass = 0.0;
		V1 vol = 0.0;
		for (int i=a; i < b; ++i) {
			if (causal && self->data[i].t > t)
				continue;
			V1 factor = fabs(self->data[i].amt) * window_func(ktype, t - self->data[i].t, ksize);
			mass += self->data[i].val*factor;
			vol += factor;
		}
		if (vol) {
			//~ INFO("v=%f  avg=%f", vol, mass/vol);
			f1_append(func, 10.0*log(mass/vol)/log(1.1));
			//~ f1_append(func, mass/vol);
			
		} else {
			//~ INFO("v=%f  avg=NA", vol);
			f1_append(func, func->ys[func->len-1]);
		}
		t += sample_rate;
	}
	
	INFO("gen %d", func->len);
}

void td_read(TradeData *self, const char *filename)
{
	struct stat sb;
	int fd = open(filename, O_RDONLY);
	ASSERT (fd != -1, "Couldn't open '%s' readonly", filename);
	ASSERT (fstat(fd, &sb) != -1, "Unable to obtain file size");
	INFO("Mapping %d kb", sb.st_size/1024);
	unsigned long long *data = mmap(NULL,  sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	ASSERT( data != MAP_FAILED, "mmap failed");
	
	self->len = data[0];
	self->last = data[1];	
	INFO("Reading %d trades @ %llu", self->len, self->last);
	self->data = realloc(self->data,8*3*self->len);
	ASSERT(self->data, "Realloc failed");
	memcpy(self->data, &data[2], 8*3*self->len);
	
	close(fd);
	
	munmap(data, sb.st_size);
}

TradeData* td_merge(TradeData *self, TradeData *other)
{
	// precondition: Both data sets are sorted
	
	int len = self->len + other->len;
	Trade *merged = malloc(3*8*len);
	
	int si = 0; // self idx
	int oi = 0; // other idx	
	int mi = -1; // merged idx
	int rem = 0, move = 0;
	while (si != self->len || oi != other->len) {
		if (si == self->len) {
			merged[++mi] = other->data[oi++];
		} else if (oi == other->len) {
			merged[++mi] = self->data[si++];
		} else if (other->data[oi].t < self->data[si].t) {
			merged[++mi] = other->data[oi++];
		} else {
			merged[++mi] = self->data[si++];
		}
		// fix duplicates
		if (!mi)
			continue;
		if (merged[mi].t == merged[mi-1].t && merged[mi].val == merged[mi-1].val && merged[mi].amt == merged[mi-1].amt) {
			++rem;
			--mi;
		} else if (merged[mi].t <= merged[mi-1].t) {
			++move;
			WARN("(%.1f, %f[%f]) (%.1f, %f[%f])", merged[mi-1].t, merged[mi-1].val, merged[mi-1].amt, 
				merged[mi].t, merged[mi].val, merged[mi].amt);
			merged[mi].t = merged[mi-1].t + 0.00001;
		}
	}
	INFO("Merged %d + %d-> %d  moved:%d  dups:%d  @ %llu", self->len, other->len, mi+1, move, rem, self->last);
	if (self->data)
		free(self->data);
	self->data = merged;
	self->len = mi+1;
	self->last = (self->last > other->last)? self->last : other->last;
	return self;
}


#ifdef INCLUDE_MAIN


int main(int argc, char *argv[])
{

	ASSERT(argc >= 3, "Usage: ./trades (update|show|merge) OUTFILE.bin [INFILE.bin]");

	TradeData t0;
	td_init(&t0);
	td_read(&t0, argv[2]);
	
	TradeData t1;
	td_init(&t1);
	td_read(&t1, argv[3]);

	//~ INFO("%.1f, %f (%f)", t0.data[0].t, t0.data[0].val, t0.data[0].amt);
	td_merge(&t0, &t1);

	td_write(&t0, argv[2]);
	td_fini(&t1);
	td_fini(&t0);
	return 0;
}


#endif
#endif
#endif

