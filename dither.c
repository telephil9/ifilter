#include "a.h"

uchar*
dither(uchar *data, int w, int h, int depth)
{
#define P(X,Y,C) out[((X) + w*(Y))*depth + C]
#define SETP(X,Y,C,E) out[((X) + w*(Y))*depth + C] = clamp(out[((X) + w*(Y))*depth + C] + E)
	uchar *out;
	int x, y, c, o, n, q;

	n = depth*w*h*sizeof(uchar);
	out = eallocbuf(n);
	memmove(out, data, n);
	for(y = 0; y < h; y++){
		for(x = 0; x < w; x++){
			for(c = 0; c < 3; c++){ /* B G R */
				o = P(x, y, c);
				n = 255*floor(o/255.0 + 0.5);
				q = o - n;
				P(x, y, c) = n;
				if(x + 1 < w)               SETP(x + 1, y    , c, q * 7.0/16.0);
				if(x - 1 >= 0 && y + 1 < h) SETP(x - 1, y + 1, c, q * 3.0/16.0);
				if(y + 1 < h)               SETP(x    , y + 1, c, q * 5.0/16.0);
				if(x + 1 < w && y + 1 < h)  SETP(x + 1, y + 1, c, q * 1.0/16.0);
			}
		}
	}
	return out;
#undef SETP
#undef P
}

void
usage(void)
{
	fprint(2, "usage: %s\n", argv0);
	exits("usage");
}

void
main(int argc, char *argv[])
{
	ARGBEGIN{
	default:
		usage();
		break;
	}ARGEND;
	applyfilter("dither", dither);
}
