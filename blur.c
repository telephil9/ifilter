#include "a.h"
	
typedef struct Filter Filter;

struct Filter
{
	const char *name;
	filterfn filter;
};

int size;

uchar*
box(uchar *data, int w, int h, int depth)
{
	uchar *out;
	int n, x, y, c, s;

	n = depth*w*h*sizeof(uchar);
	out = eallocbuf(n);
	memmove(out, data, n);
	/* box blur is not defined for edge points */
	for(y = 1; y < h - 1; y++){
		for(x = 1; x < w - 1; x++){
			for(c = 0; c < 3; c++){ /* R G B */
#define P²(X,Y,C) (data[((X) + w*(Y))*depth + C] * data[((X) + w*(Y))*depth + C])
				s = P²(x - 1, y + 1, c) +
					P²(x + 0, y + 1, c) +
					P²(x + 1, y + 1, c) +
					P²(x - 1, y + 0, c) +
					P²(x + 0, y + 0, c) +
					P²(x + 1, y + 0, c) +
					P²(x - 1, y - 1, c) +
					P²(x + 0, y - 1, c) +
					P²(x + 1, y - 1, c);
				out[(x + w * y) * depth + c] = sqrt(s / 9);
#undef P²
			}
		}
	}
	return out;
}

uchar*
gaussian(uchar *data, int w, int h, int depth)
{
	const double E = 2.7182818284590452354;
	const double Π = 3.14159265358979323846;
	uchar *out;
	int kw, x, y, kx, ky;
	double *k, σ, s, e, n, d, v, r, g, b;

	if(size < 0)
		sysfatal("gaussian filter needs a size argument");
	n = depth*w*h*sizeof(uchar);
	out = eallocbuf(n);
	memmove(out, data, n);
	σ = (double)size / 2.0;
	kw = 2 * size + 1;
	k = malloc(kw*kw*sizeof(double));
	if(k == nil)
		return nil;
	s = 0.0f;
	for(y = -size; y <= size; y++){
		for(x = -size; x <= size; x++){
			n = (double)(-(x * x + y * y));
			d = 2 * σ * σ;
			e = pow(E, n / d);
			v = e / (2 * Π * σ * σ);
			k[(x + size) + kw * (y + size)] = v;
			s += v;
		}
	}
	for(y = 0; y < kw; y++){
		for(x = 0; x < kw; x++){
			k[x + kw * y] /= s;
		}
	}
	for(y = size; y < (h - size); y++){
		for(x = size; x < (w - size); x++){
			r = g = b = 0.0;
			for(ky = -size; ky <= size; ky++){
				for(kx = -size; kx <= size; kx++){
					v = k[(kx + size) + kw * (ky + size)];
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
	free(k);
	return out;
}

void
usage(void)
{
	fprint(2, "usage: %s -f box|gaussian [-s size]\n", argv0);
	exits("usage");
}

static Filter filters[] = {
	{ "box", box },
	{ "gaussian", gaussian },
};

void
main(int argc, char *argv[])
{
	Filter *f;
	char *fname;
	int i;

	size = 2;
	f = nil;
	ARGBEGIN{
	case 'f':
		fname = EARGF(usage());
		for(i=0; i<nelem(filters); i++){
			if(strcmp(fname, filters[i].name)==0){
				f = filters+i;
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
	if(f==nil)
		usage();
	applyfilter(f->name, f->filter);
}
