#include "a.h"

int size;

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
	fprint(2, "usage: %s [-s size] [box|gaussian|pixelate]\n", argv0);
	exits("usage");
}

void
main(int argc, char *argv[])
{
	size = 8;
	ARGBEGIN{
	case 's':
		size = atoi(EARGF(usage()));
		break;
	default:
		usage();
		break;
	}ARGEND;
	applyfilter("pixelate", pixelate);
}
