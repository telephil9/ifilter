#include <u.h>
#include <libc.h>
#include <draw.h>
#include <memdraw.h>

typedef struct Filter Filter;
int size;

struct Filter
{
	const char *name;
	uchar* (*filter)(uchar *data, int w, int h, int depth);
};

uchar*
box(uchar *data, int w, int h, int depth)
{
	uchar *out;
	int x, y, c, s;

	out = malloc(depth*w*h*sizeof(uchar));
	if(out == nil)
		return nil;
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
pixelate(uchar *data, int w, int h, int depth)
{
	uchar *out;
	int x, y, ox, oy, oi, sr, sg, sb;

	if(size < 0)
		sysfatal("pixelate filter needs a size argument");
	out = malloc(depth*w*h*sizeof(uchar));
	if(out == nil)
		return nil;
	for(y = 0; y < h; y += size){
		for(x = 0; x < w; x += size){
			sr = sg = sb = 0;
			for(oy = y; oy < y+size; oy++){
				for(ox = x; ox < x+size; ox++){
					oi = (ox + w * oy) * depth;
					sr += data[oi + 0]*data[oi + 0];
					sg += data[oi + 1]*data[oi + 0];
					sb += data[oi + 2]*data[oi + 0];
				}
			}
			sr = sqrt(sr/(size*size));
			sg = sqrt(sg/(size*size));
			sb = sqrt(sb/(size*size));
			for(oy = y; oy < y+size; oy++){
				for(ox = x; ox < x+size; ox++){
					oi = (ox + w * oy) * depth;
					out[oi + 0] = sr;
					out[oi + 1] = sg;
					out[oi + 2] = sb;
				}
			}
		}
	}
	return out;
}

void
usage(void)
{
	fprint(2, "usage: %s [-s size] [box|pixelate]\n", argv0);
	exits("usage");
}

static Filter filters[] = {
	{ "box", box },
	{ "pixelate", pixelate },
};

void
main(int argc, char *argv[])
{
	Memimage *o, *i;
	int w, h, n;
	uchar *buf, *out;
	Filter *f;

	size = -1;
	f = nil;
	ARGBEGIN{
	case 's':
		size = atoi(EARGF(usage()));
		break;
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

