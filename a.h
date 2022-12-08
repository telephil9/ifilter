#include <u.h>
#include <libc.h>
#include <draw.h>
#include <memdraw.h>

typedef uchar*(*filterfn)(uchar*, int, int, int);

void	applyfilter(const char*, filterfn);
int		clamp(int);
uchar*	eallocbuf(ulong);

enum
{
	Cblue,
	Cgreen,
	Cred,
	Calpha,
};
