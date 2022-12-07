#include <u.h>
#include <libc.h>
#include <draw.h>
#include <memdraw.h>

int
clamp(int n)
{
	if(n < 0)
		n = 0;
	else if(n > 255)
		n = 255;
	return n;
}

uchar*
dither(uchar *data, int w, int h, int depth)
{
#define P(X,Y,C) out[((X) + w*(Y))*depth + C]
#define SETP(X,Y,C,E) out[((X) + w*(Y))*depth + C] = clamp(out[((X) + w*(Y))*depth + C] + E)
	uchar *out;
	int x, y, c, o, n, q;

	out = malloc(depth*w*h*sizeof(uchar));
	if(out == nil)
		return nil;
	for(y = 0; y < depth*w*h; y++)
		out[y] = data[y];
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
	Memimage *o, *i;
	int w, h, n;
	uchar *buf, *out;

	ARGBEGIN{
	default:
		usage();
		break;
	}ARGEND;
	if(memimageinit()<0)
		sysfatal("memimageinit: %r");
	o = readmemimage(0);
	if(o==nil)
		sysfatal("readmemimage: %r");
	i = allocmemimage(rectsubpt(o->r, o->r.min), XRGB32);
	memimagedraw(i, i->r, o, o->r.min, nil, ZP, S);
	freememimage(o);
	w = Dx(i->r);
	h = Dy(i->r);
	n = 4*w*h*sizeof(uchar);
	buf = malloc(n);
	if(buf==nil)
		sysfatal("malloc: %r");
	if(unloadmemimage(i, i->r, buf, n)<0)
		sysfatal("unloadmemimage: %r");
	freememimage(i);
	out = dither(buf, w, h, 4);
	if(out == nil)
		sysfatal("dither failed: %r");
	print("   x8r8g8b8 %11d %11d %11d %11d ", 0, 0, w, h);
	write(1, out, n);
	exits(nil);
}

