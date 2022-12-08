#include "a.h"

typedef struct Filter Filter;

struct Filter
{
	const char *name;
	filterfn f;
};

float ratio;

uchar*
grayscale(uchar* data, int w, int h, int depth)
{
	uchar v;
	int i;

	for(i = 0; i < w*h*depth; i += 4){
		v = 0.2126*data[i+Cred] + 0.7152*data[i+Cgreen] + 0.0722*data[i+Cblue];
		data[i+Cred] = data[i+Cgreen] = data[i+Cblue] = v;
	}
	return data;
}

uchar*
sepia(uchar* data, int w, int h, int depth)
{
	float r, g, b;
	int i;

	for(i = 0; i < w*h*depth; i += 4){
		r = 0.393*data[i+Cred] + 0.769*data[i+Cgreen] + 0.189*data[i+Cblue];
		g = 0.349*data[i+Cred] + 0.686*data[i+Cgreen] + 0.168*data[i+Cblue];
		b = 0.272*data[i+Cred] + 0.534*data[i+Cgreen] + 0.131*data[i+Cblue];
		data[i+Cred]   = clamp(r);
		data[i+Cgreen] = clamp(g);
		data[i+Cblue]  = clamp(b);
	}
	return data;
}

uchar*
invert(uchar* data, int w, int h, int depth)
{
	int i;

	for(i = 0; i < w*h*depth; i += 4){
		data[i+Cred]   = 255 - data[i+Cred];
		data[i+Cgreen] = 255 - data[i+Cgreen];
		data[i+Cblue]  = 255 - data[i+Cblue];
	}
	return data;
}

uchar*
shade(uchar* data, int w, int h, int depth)
{
	int i;

	for(i = 0; i < w*h*depth; i += 4){
		data[i+Cred]   *= (1 - ratio);
		data[i+Cgreen] *= (1 - ratio);
		data[i+Cblue]  *= (1 - ratio);
	}
	return data;
}

uchar*
tint(uchar* data, int w, int h, int depth)
{
	int i;

	for(i = 0; i < w*h*depth; i += 4){
		data[i+Cred]   += (255 - data[i+Cred]) * ratio;
		data[i+Cgreen] += (255 - data[i+Cgreen]) * ratio;
		data[i+Cblue]  += (255 - data[i+Cblue]) * ratio;
	}
	return data;
}

void
usage(void)
{
	fprint(2, "usage: %s -f grayscale|sepia|invert|shade|tint [-r ratio]\n", argv0);
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
main(int argc, char *argv[])
{
	Filter *f;
	char *fname;
	int n;

	f = nil;
	ratio = 0.0;
	ARGBEGIN{
	case 'f':
		fname = EARGF(usage());
		for(n=0; n<nelem(filters); n++){
			if(strcmp(fname, filters[n].name)==0){
				f = filters+n;
				break;
			}
		}
		break;
	case 'r':
		ratio = (float)atof(EARGF(usage()));
		break;
	default:
		usage();
		break;
	}ARGEND;
	if(f==nil)
		usage();
	if(ratio<0.0 || ratio>1.0){
		fprint(2, "ratio should be between 0.0 and 1.0\n");
		exits("invalid ratio");
	}
	applyfilter(f->name, f->f);
}
