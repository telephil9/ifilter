#include <u.h>
#include <libc.h>
#include <draw.h>
#include <memdraw.h>

#define MIN(x,y) ((x)<(y)?(x):(y))

typedef void(*filterfun)(uchar[3]);

void
grayscale(uchar p[3])
{
	uchar v = 0.2126*p[0] + 0.7152*p[1] + 0.0722*p[2];
	p[0] = p[1] = p[2] = v;
}

void
sepia(uchar p[3])
{
	float r, g, b;

	r = 0.393*p[2] + 0.769*p[1] + 0.189*p[0];
	g = 0.349*p[2] + 0.686*p[1] + 0.168*p[0];
	b = 0.272*p[2] + 0.534*p[1] + 0.131*p[0];
	p[2] = MIN(r, 255);
	p[1] = MIN(g, 255);
	p[0] = MIN(b, 255);
}

void
usage(void)
{
	fprint(2, "usage: %s [grayscale|sepia]\n", argv0);
	exits("usage");
}

void
main(int argc, char *argv[])
{
	Memimage *i;
	int w, h, p, n;
	uchar *buf;
	filterfun filter;

	filter = nil;
	ARGBEGIN{
	}ARGEND;
	if(argc!=1)
		usage();
	if(strcmp(*argv, "grayscale")==0)
		filter = grayscale;
	else if(strcmp(*argv, "sepia")==0)
		filter = sepia;
	else
		usage();
	if(memimageinit()<0)
		sysfatal("memimageinit: %r");
	i = readmemimage(0);
	if(i==nil)
		sysfatal("readmemimage: %r");
	w = Dx(i->r);
	h = Dy(i->r);
	n = 4*w*h*sizeof(uchar);
	buf = malloc(n);
	if(buf==nil)
		sysfatal("malloc: %r");
	if(unloadmemimage(i, i->r, buf, n)<0)
		sysfatal("unloadmemimage: %r");
	for(p = 0; p < n; p+=4){
		filter(buf+p);
	}
	if(loadmemimage(i, i->r, buf, n)<0)
		sysfatal("unloadmemimage: %r");
	writememimage(1, i);
	freememimage(i);
	exits(nil);
}

