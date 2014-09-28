#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include "xmalloc.h"


void *
xmalloc (size_t size)
{
	void *p;

	p = malloc(size);
	return p;
}

void *
xrealloc (void *ptr, size_t size)
{
	void *p;

	p = ptr ? realloc(ptr, size) : malloc(size);

	return p;
}

void
xfree (void *ptr)
{
	if (ptr)
		free(ptr);
}

char *
xstrdup (const char *str)
{
	char *p;

	if (!str)
		return 0;
	strcpy((p = (char*)xmalloc(strlen(str) + 1)), str);

	return p;
}