#include "a.h"

enum { Fbox, Fgaussian, };
char* filters[] = { "box", "gaussian" };

double *kernel;
int size;

double boxkernel[] = {
	1./9., 1./9., 1./9.,
	1./9., 1./9., 1./9.,
	1./9., 1./9., 1./9.,
};

double*
computekernel(int radius, double σ)
{
	const double Π = 3.14159265358979323846;
	double *k, d, g, s;
	int kw, x, y;

	kw = 2*radius+1;
	k = malloc(kw*kw*sizeof(double));
	if(k == nil)
		sysfatal("malloc: %r");
	d  = 1.0/(2.0*Π*σ*σ);
	s  = 0.0;
	for(y = -radius; y <= radius; y++){
		for(x = -radius; x <= radius; x++){
			g = d*exp(-(x*x+y*y)/(2*σ*σ));
			k[(x+radius) + kw * (y+radius)] = g;
			s += g;
		}
	}
	for(y = 0; y < kw; y++){
		for(x = 0; x < kw; x++){
			k[x + kw * y] /= s;
		}
	}
	return k;
}

uchar*
convolve(uchar *data, int w, int h, int depth)
{
	uchar *out;
	int kw, n, x, y, kx, ky;
	double v, r, g, b;

	if(size < 0)
		sysfatal("gaussian filter needs a size argument");
	n = depth*w*h*sizeof(uchar);
	out = eallocbuf(n);
	memmove(out, data, n);
	kw = 2*size+1;
	for(y = size; y < (h - size); y++){
		for(x = size; x < (w - size); x++){
			r = g = b = 0.0;
			for(ky = -size; ky <= size; ky++){
				for(kx = -size; kx <= size; kx++){
					v = kernel[(kx + size) + kw * (ky + size)];
#define P(X,Y,C) data[((X) + (w*(Y)))*depth + C]
					r += (double)P(x - kx, y - ky, 0) * v;
					g += (double)P(x - kx, y - ky, 1) * v;
					b += (double)P(x - kx, y - ky, 2) * v;
#undef P
				}
			}
			out[(x + w*y)*depth + 0] = r;
			out[(x + w*y)*depth + 1] = g;
			out[(x + w*y)*depth + 2] = b;
		}
	}
	return out;
}

void
usage(void)
{
	fprint(2, "usage: %s -f box|gaussian [-s size]\n", argv0);
	exits("usage");
}

void
main(int argc, char *argv[])
{
	char *fname;
	int f, i;

	fname = nil;
	size = 2;
	f = -1;
	ARGBEGIN{
	case 'f':
		fname = EARGF(usage());
		for(i=0; i<nelem(filters); i++){
			if(strcmp(fname, filters[i])==0){
				f = i;
				break;
			}
		}
		break;
	case 's':
		size = atoi(EARGF(usage()));
		break;
	default:
		usage();
		break;
	}ARGEND;
	if(f<0)
		usage();
	switch(f){
	case Fbox:
		size   = 1;
		kernel = boxkernel;
		break;
	case Fgaussian:
		kernel = computekernel(size, 1.0);
		break;
	}
	applyfilter(fname, convolve);
}
