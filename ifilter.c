#include <u.h>
#include <libc.h>
#include <draw.h>
#include <memdraw.h>

#define MIN(x,y) ((x)<(y)?(x):(y))

typedef struct Filter Filter;

struct Filter
{
	const char *name;
	void (*filter)(int *r, int *g, int *b, float);
};

void
grayscale(int *r, int *g, int *b, float)
{
	uchar v = 0.2126*(*r) + 0.7152*(*g) + 0.0722*(*b);
	*r = *g = *b = v;
}

void
sepia(int *r, int *g, int *b, float)
{
	float fr, fg, fb;

	fr = 0.393*(*r) + 0.769*(*g) + 0.189*(*b);
	fg = 0.349*(*r) + 0.686*(*g) + 0.168*(*b);
	fb = 0.272*(*r) + 0.534*(*g) + 0.131*(*b);
	*r = MIN(fr, 255);
	*g = MIN(fg, 255);
	*b = MIN(fb, 255);
}

void
invert(int *r, int *g, int *b, float)
{
	*b = 255 - *b;
	*g = 255 - *g;
	*r = 255 - *r;
}

void
shade(int *r, int *g, int *b, float factor)
{
	*b = *b * (1 - factor);
	*g = *g * (1 - factor);
	*r = *r * (1 - factor);
}

void
tint(int *r, int *g, int *b, float factor)
{
	*b = *b + (255.0 - *b) * factor;
	*g = *g + (255.0 - *g) * factor;
	*r = *r + (255.0 - *r) * factor;
}

void
usage(void)
{
	fprint(2, "usage: %s [-f factor] [grayscale|sepia|invert|shade|tint]\n", argv0);
	exits("usage");
}

static Filter filters[] = {
	{ "grayscale", grayscale },
	{ "sepia", sepia },
	{ "invert", invert },
	{ "shade", shade },
	{ "tint", tint },
};

void
getcolor(int depth, uchar *buf, int *r, int *g, int *b)
{
	int c;

	switch(depth){
	case 8:
		c = cmap2rgb(*buf);
		*r = (c >> 8 * 2) & 0xFF;
		*g = (c >> 8 * 1) & 0xFF;
		*b = (c >> 8 * 0) & 0xFF;
		break;
	case 24:
	case 32:
		*r = buf[2];
		*g = buf[1];
		*b = buf[0];
		break;
	}
}

void
setcolor(int depth, uchar *buf, int r, int g, int b)
{
	switch(depth){
	case 8:
		*buf = rgb2cmap(r, g, b);
		break;
	case 32:
		buf[2] = r;
		buf[1] = g;
		buf[0] = b;
		break;
	}

}

void
main(int argc, char *argv[])
{
	Memimage *i;
	int w, h, p, n, s, r, g, b;
	uchar *buf;
	Filter *f;
	float factor;

	f = nil;
	factor = 0.0;
	ARGBEGIN{
	case 'f':
		factor = (float)atof(EARGF(usage()));
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
	if(factor<0.0 || factor>1.0){
		fprint(2, "factor should be between 0.0 and 1.0\n");
		exits("invalid factor");
	}
	if(memimageinit()<0)
		sysfatal("memimageinit: %r");
	i = readmemimage(0);
	if(i==nil)
		sysfatal("readmemimage: %r");
	s = i->depth/8;
	w = Dx(i->r);
	h = Dy(i->r);
	n = s*w*h*sizeof(uchar);
	buf = malloc(n);
	if(buf==nil)
		sysfatal("malloc: %r");
	if(unloadmemimage(i, i->r, buf, n)<0)
		sysfatal("unloadmemimage: %r");
	for(p = 0; p < n; p+=s){
		getcolor(i->depth, buf+p, &r, &g, &b);
		f->filter(&r, &g, &b, factor);
		setcolor(i->depth, buf+p, r, g, b);
	}
	if(loadmemimage(i, i->r, buf, n)<0)
		sysfatal("unloadmemimage: %r");
	writememimage(1, i);
	freememimage(i);
	exits(nil);
}

