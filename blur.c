#include <u.h>
#include <libc.h>
#include <draw.h>
#include <memdraw.h>

typedef struct Filter Filter;

struct Filter
{
	const char *name;
	uchar* (*filter)(uchar *data, int w, int h, int depth);
};

enum { R, G, B };

uchar*
box(uchar *data, int w, int h, int depth)
{
	uchar *out;
	int x, y, r, c, s;

	r = w * depth;
	out = malloc(depth*w*h*sizeof(uchar));
	if(out == nil)
		return nil;
	/* box blur is not defined for edge points */
	for(y = 1; y < h - 1; y++){
		for(x = 1; x < w - 1; x++){
			for(c = 0; c < 3; c++){ /* R G B */
				s = data[(x - 1)*depth + r * (y + 1) + c] +
					data[(x + 0)*depth + r * (y + 1) + c] +
					data[(x + 1)*depth + r * (y + 1) + c] +
					data[(x - 1)*depth + r * (y + 0) + c] +
					data[(x + 0)*depth + r * (y + 0) + c] +
					data[(x + 1)*depth + r * (y + 0) + c] +
					data[(x - 1)*depth + r * (y - 1) + c] +
					data[(x + 0)*depth + r * (y - 1) + c] +
					data[(x + 1)*depth + r * (y - 1) + c];
				out[x * depth + r * y + c] = s / 9;
			}
		}
	}
	return out;
}

void
usage(void)
{
	fprint(2, "usage: %s [box]\n", argv0);
	exits("usage");
}

static Filter filters[] = {
	{ "box", box },
};

void
main(int argc, char *argv[])
{
	Memimage *o, *i;
	int w, h, n;
	uchar *buf, *out;
	Filter *f;

	f = nil;
	ARGBEGIN{
	default:
		usage();
		break;
	}ARGEND;
	if(argc!=1)
		usage();
	for(n=0; n<nelem(filters); n++){
		if(strcmp(*argv, filters[n].name)==0){
			f = filters+n;
			break;
		}
	}
	if(f==nil)
		usage();
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
	out = f->filter(buf, w, h, 4);
	if(out == nil)
		sysfatal("filter '%s' failed: %r", f->name);
	print("   x8r8g8b8 %11d %11d %11d %11d ", 0, 0, w, h);
	write(1, out, n);
	exits(nil);
}

