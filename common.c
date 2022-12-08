#include "a.h"

int
isalpha(ulong chan)
{
	ulong t;

	for(t = chan; t; t >>= 8)
		if(TYPE(t) == CAlpha)
			return 1;
	return 0;
}

void
applyfilter(const char *fname, filterfn f)
{
	Memimage *o, *i;
	int w, h, n, a;
	uchar *buf, *out;

	if(memimageinit()<0)
		sysfatal("memimageinit: %r");
	o = readmemimage(0);
	if(o==nil)
		sysfatal("readmemimage: %r");
	a = isalpha(o->chan);
	i = allocmemimage(rectsubpt(o->r, o->r.min), a ? ARGB32 : XRGB32);
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
	out = f(buf, w, h, 4);
	if(out == nil)
		sysfatal("%s failed: %r", fname);
	print("   %c8r8g8b8 %11d %11d %11d %11d ", a ? 'a' : 'x', 0, 0, w, h);
	write(1, out, n);
	exits(nil);
}

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
eallocbuf(ulong s)
{
	uchar *p;

	p = malloc(s);
	if(p == nil)
		sysfatal("malloc: %r");
	return p;
}
