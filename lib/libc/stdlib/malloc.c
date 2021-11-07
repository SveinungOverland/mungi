#include <stdlib.h>
#include <assert.h>
#include "lib/naming/hmalloc.h"

#define HEAP_SIZE 1000 /* no. of pages */

static void *vdat = NULL;

static void
malloc_init(void)
{
	if (vdat == NULL) {
		vdat = hmalloc_init(HEAP_SIZE, NULL);
		assert(vdat != NULL);
	}
}

void *
malloc(size_t size)
{
	malloc_init();
	return hmalloc(vdat, size);
}

void
free(void *ptr)
{
	assert(vdat != NULL);
	return hfree(vdat, ptr);
}

void *
realloc(void *ptr, size_t size)
{
	malloc_init();
	return hrealloc(vdat, ptr, size);
}
