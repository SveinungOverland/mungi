/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <phk@FreeBSD.ORG> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Poul-Henning Kamp
 * ----------------------------------------------------------------------------
 *
 * From FreeBSD: malloc.c,v 1.43 1998/09/30 06:13:59 jb
 *
 */

/*
 * Modified by Charles Gray (cgray@cse.unsw.edu.au)
 *
 * Mungi version now supports allocation within objects of your choosing
 *
 */

/*
 * Defining MALLOC_EXTRA_SANITY will enable extra checks which are related
 * to internal conditions and consistency in malloc.c. This has a
 * noticeable runtime performance hit, and generally will not do you
 * any good unless you fiddle with the internals of malloc or want
 * to catch random pointer corruption as early as possible.
 */
#undef MALLOC_EXTRA_SANITY

/*
 * What to use for Junk.  This is the byte value we use to fill with
 * when the 'J' option is enabled.
 */
#define SOME_JUNK	0xd0	/* as in "Duh" :-) */

/*
 * The basic parameters you can tweak.
 *
 * malloc_minsize	minimum size of an allocation in bytes.
 *			If this is too small it's too much work
 *			to manage them.  This is also the smallest
 *			unit of alignment used for the storage
 *			returned by malloc/realloc.
 *
 */

#define malloc_minsize			16U
#define PAGE_SIZE 4096

#include "../../../syscalls/userprint.h"

#include <mungi.h>
#include <sys/types.h>
#include <stddef.h>
#include <unistd.h>
#include <assert.h>

#include "hmalloc.h"
#include <string.h>

static void add_cap(cap_t cap);


/*
 * This structure describes a page worth of chunks.
 */

struct pginfo {
	struct pginfo *next;	/* next on the free list */
	void *page;		/* Pointer to the page */
	u_short size;		/* size of this page's chunks */
	u_short shift;		/* How far to shift for this size chunks */
	u_short free;		/* How many free chunks */
	u_short total;		/* How many chunk */
	u_int bits[1];		/* Which chunks are free */
};

/*
 * This structure describes a number of free pages.
 */

struct pgfree {
	struct pgfree *next;	/* next run of free pages */
	struct pgfree *prev;	/* prev run of free pages */
	void *page;		/* pointer to free pages */
	void *end;		/* pointer to end of free pages */
	size_t size;		/* number of bytes free */
};

/*
 * How many bits per u_int in the bitmap.
 * Change only if not 8 bits/byte
 */
#define	MALLOC_BITS	(8*sizeof(u_int))

/*
 * Magic values to put in the page_directory
 */
#define MALLOC_NOT_MINE	((struct pginfo*) 0)
#define MALLOC_FREE 	((struct pginfo*) 1)
#define MALLOC_FIRST	((struct pginfo*) 2)
#define MALLOC_FOLLOW	((struct pginfo*) 3)
#define MALLOC_MAGIC	((struct pginfo*) 4)

/*
 * 'global malloc data' structure to be passed around
 */
#define MALLOC_CACHE_SIZE 200
#define MIN_PAGEDIR_SIZE  20

typedef struct {
	/* Page size related parameters, computed at run-time. */
	size_t malloc_pagesize;
	size_t malloc_pageshift;
	size_t malloc_pagemask;

	/* Set when initialization has been done */
	unsigned malloc_started;

	/* Recusion flag for public interface. */
	int malloc_active;

	/* Number of free pages we cache */
	unsigned malloc_cache;	/* = 16; */

	/* The offset from pagenumber to index into the page directory */
	size_t malloc_origo;

	/* The last index in the page directory we care about */
	size_t last_idx;

	/* Pointer to page directory. Allocated "as if with" malloc */
	/* FIXME: This could be located at the end of the mungi object
	 * so we only get one cap fault/refill guy, if this is a prob */
	struct pginfo **page_dir;

	/* How many slots in the page directory */
	unsigned malloc_ninfo;

	/* The capability to the page directory object */
	cap_t page_dir_cap;

	/* The info descriptor for the page directory object */
	objinfo_t page_dir_info;

	/* The capability to the heap, and it's size in pages */
	cap_t heap_cap;
	unsigned heap_size;

	/* The info descriptor for the heap object */
	objinfo_t heap_info;

	/* Free pages line up here */
	struct pgfree free_list;

	/* Abort(), user doesn't handle problems.  */
	int malloc_abort;

	/* Are we trying to die ?  */
	int suicide;

	/* always realloc ?  */
	int malloc_realloc;

	/* sysv behaviour for malloc(0) ?  */
	int malloc_sysv;

	/* zero fill ?  */
	int malloc_zero;

	/* junk fill ?  */
	int malloc_junk;

	/* one location cache for free-list holders */
	struct pgfree *px;

	/* name of function causing last warning/error */
	char *malloc_func;
} mdat_t;

/* this one is also seedy!! */
#ifndef malloc_maxsize
#define malloc_maxsize	((dat->malloc_pagesize)>>1)
#endif

/* this is *very* seedy */
#define pageround(foo)	(((foo) + (dat->malloc_pagemask))&(~(dat->malloc_pagemask)))
#define ptr2idx(foo)	(((size_t)(foo) >> dat->malloc_pageshift)-dat->malloc_origo)

#ifndef THREAD_LOCK
#define THREAD_LOCK()
#endif

#ifndef THREAD_UNLOCK
#define THREAD_UNLOCK()
#endif

#ifndef MMAP_FD
#define MMAP_FD (-1)
#endif

#ifndef INIT_MMAP
#define INIT_MMAP()
#endif

#ifndef MADV_FREE
#define MADV_FREE MADV_DONTNEED
#endif

/*
 * Necessary function declarations
 */
static int extend_pgdir(mdat_t * dat, size_t idx);
static void *imalloc(mdat_t * dat, size_t size);
static void ifree(mdat_t * dat, void *ptr);
static void *irealloc(mdat_t * dat, void *ptr, size_t size);

static void
wrterror(mdat_t * dat, char *p)
{
	UserPrint("hmalloc error%s %s\n", dat->malloc_func, p);
	dat->suicide = 1;
}

static void
wrtwarning(mdat_t * dat, char *p)
{
	UserPrint("hmalloc warning%s %s\n", dat->malloc_func, p);

	if (dat->malloc_abort)
		wrterror(dat, p);
}


/*
 * Allocate a number of pages from the OS to the heap
 */
static void *
map_pages(mdat_t * dat, size_t pages)
{
	caddr1_t result;
	int ret;

	/* if the heap is NULL we need to ObjCreate it */
	if (dat->heap_cap.address == NULL) {
		/* Next set up the heap object */
		dat->heap_cap.passwd = 0xbeefdead; /* FIXME */
		dat->heap_cap.address =
			ObjCreate((size_t) ((dat->heap_size + pages)
					    << dat->malloc_pageshift),
				  dat->heap_cap.passwd, NULL);

		if (dat->heap_cap.address == NULL) {
			UserPrint("map_pages (hmalloc.c): ObjCreate FAILED!\n");
			return NULL;
		}

		add_cap(dat->heap_cap);
	} else {
		/* Attempt to resize the heap, NOT allowing it to be moved */
		ret = ObjResize(dat->heap_cap.address,
				(size_t) ((dat->heap_size + pages)
					  << dat->malloc_pageshift));
		if (ret != 0) {
			UserPrint("map_pages (hmalloc.c): ObjResize FAILED!\n");
			return 0;
		}
	}

	/*** WARNING - Check that this works!! Potential bug ***/
	result = (caddr1_t) pageround((size_t) dat->heap_cap.address +
				     (dat->heap_size << dat->malloc_pageshift));

	dat->heap_size += pages;

	dat->last_idx = ptr2idx((size_t) dat->heap_cap.address +
				(dat->heap_size << dat->malloc_pageshift)) - 1;
	if ((dat->last_idx + 1) >= dat->malloc_ninfo
	    && !extend_pgdir(dat, dat->last_idx))
		return 0;

	return result;
}

/*
 * Extend page directory
 */
static int
extend_pgdir(mdat_t * dat, size_t idx)
{
	size_t newlen;
	int ret;

	/* Make it this many pages */
	newlen = pageround(idx * sizeof (*dat->page_dir)) +
		 dat->malloc_pagesize;

	/* remember the old mapping size 
	 * oldlen = malloc_ninfo * sizeof *page_dir; */

	/* the deal is:
	 * we need to make pagedir newlen in length. */
	/* Simply attempt to resize the object. It is allowed to move. */
	ret = ObjResize(dat->page_dir_cap.address, newlen);
	if (ret != 0) {
		UserPrint("extend_pgdir (hmalloc.c): ObjResize FAILED!\n");
		return 0;
	}

	/* register the new size */
	dat->malloc_ninfo = newlen / sizeof (*dat->page_dir);

	return 1;
}

/*
 * Initialize the world
 */
static void
malloc_init(mdat_t * dat)
{
	/*
	 * Compute page-size related variables.
	 */
	dat->malloc_pagesize = PAGE_SIZE;
	dat->malloc_pagemask = dat->malloc_pagesize - 1;
	for (dat->malloc_pageshift = 0;
	     (1UL << dat->malloc_pageshift) != dat->malloc_pagesize;
	     dat->malloc_pageshift++)
		/* nothing */ ;

	if (!dat->malloc_cache)
		dat->malloc_cache++;

	/* We may need to store the heaps capability in our APD */


	/* First set up the page_dir object */
	dat->page_dir_cap.passwd = 0xdeadbeef;

	/* FIXME - persistant object */
	dat->page_dir_cap.address = ObjCreate(MIN_PAGEDIR_SIZE
					       * dat->malloc_pagesize,
					      dat->page_dir_cap.passwd, NULL);

	if (dat->page_dir_cap.address == NULL) {
		UserPrint("malloc_init (hmalloc.c): ObjCreate FAILED!\n");
		return;
	}

	add_cap(dat->page_dir_cap);

	dat->page_dir = (struct pginfo **)dat->page_dir_cap.address;

	/* make a blank heap cap */
	dat->heap_cap.address = NULL;
	dat->heap_cap.address = 0;

	dat->heap_size = dat->malloc_cache;

	/*
	 * We need a maximum of malloc_pageshift buckets, steal these from the
	 * front of the page_directory;
	 */
	dat->malloc_origo = pageround((size_t) dat->heap_cap.address)
		>> dat->malloc_pageshift;
	dat->malloc_origo -= dat->malloc_pageshift;

	dat->malloc_ninfo = dat->malloc_pagesize / sizeof (*dat->page_dir);


	/*
	 * This is a nice hack from Kaleb Keithly (kaleb@x.org).
	 * We can sbrk(2) further back when we keep this on a low address.
	 */
	dat->px = (struct pgfree *)imalloc(dat, sizeof *dat->px);
	assert(dat->px != NULL);

	/* BEGIN AKI PATCH */
	/* added by aki to initialise free list */

	dat->px->next = dat->free_list.next;
	dat->px->prev = &dat->free_list;
	dat->px->page = dat->heap_cap.address;
	dat->px->end = (void *)((unsigned long)dat->heap_cap.address +
				(dat->heap_size * PAGE_SIZE));
	dat->px->size = dat->heap_size * PAGE_SIZE;
	dat->free_list.next = dat->px;
	dat->px = 0;

	/* END AKI PATCH */

	/* Been here, done that */
	dat->malloc_started++;
}

/*
 * Allocate a number of complete pages
 */
static void *
malloc_pages(mdat_t * dat, size_t size)
{
	void *p, *delay_free = 0;
	int i;
	struct pgfree *pf;
	size_t idx;

	size = pageround(size);
	p = 0;

	/* Look for free pages before asking for more */
	for (pf = dat->free_list.next; pf; pf = pf->next) {
		if (pf->size < size)
			continue;

		if (pf->size == size) {
			p = pf->page;
			if (pf->next)
				pf->next->prev = pf->prev;
			pf->prev->next = pf->next;
			delay_free = pf;
			break;
		}

		p = pf->page;
		pf->page = (char *)pf->page + size;
		pf->size -= size;
		break;
	}

	size >>= dat->malloc_pageshift;

	/* Map new pages */
	if (!p)
		p = map_pages(dat, size);

	if (p) {
		idx = ptr2idx(p);
		dat->page_dir[idx] = MALLOC_FIRST;
		for (i = 1; i < size; i++)
			dat->page_dir[idx + i] = MALLOC_FOLLOW;

		if (dat->malloc_junk)
			memset(p, SOME_JUNK, size << dat->malloc_pageshift);
	}

	if (delay_free) {
		if (!dat->px)
			dat->px = delay_free;
		else
			ifree(dat, delay_free);
	}

	return p;
}

/*
 * Allocate a page of fragments
 */
static __inline__ int
malloc_make_chunks(mdat_t * dat, int bits)
{
	struct pginfo *bp;
	void *pp;
	int i, k, l;

	/* Allocate a new bucket */
	pp = malloc_pages(dat, dat->malloc_pagesize);
	if (pp == NULL)
		return 0;

	/* Find length of admin structure */
	l = offsetof(struct pginfo, bits[0]);
	l += sizeof bp->bits[0] * (((dat->malloc_pagesize >> bits)
				    + MALLOC_BITS - 1) / MALLOC_BITS);

	/* Don't waste more than two chunks on this */
	if ((1 << (bits)) <= l + l) {
		bp = (struct pginfo *)pp;
	} else {
		bp = (struct pginfo *)imalloc(dat, (size_t) l);
		if (!bp) {
			ifree(dat, pp);
			return 0;
		}
	}

	bp->size = (1 << bits);
	bp->shift = bits;
	bp->total = bp->free = dat->malloc_pagesize >> bits;
	bp->page = pp;

	/* set all valid bits in the bitmap */
	k = bp->total;
	i = 0;

	/* Do a bunch at a time */
	for (; k - i >= MALLOC_BITS; i += MALLOC_BITS)
		bp->bits[i / MALLOC_BITS] = ~0U;

	for (; i < k; i++)
		bp->bits[i / MALLOC_BITS] |= 1 << (i % MALLOC_BITS);

	if (bp == bp->page) {
		/* Mark the ones we stole for ourselves */
		for (i = 0; l > 0; i++) {
			bp->bits[i / MALLOC_BITS] &= ~(1 << (i % MALLOC_BITS));
			bp->free--;
			bp->total--;
			l -= (1 << bits);
		}
	}

	/* MALLOC_LOCK */

	dat->page_dir[ptr2idx(pp)] = bp;

	bp->next = dat->page_dir[bits];
	dat->page_dir[bits] = bp;

	/* MALLOC_UNLOCK */

	return 1;
}

/*
 * Allocate a fragment
 */
static void *
malloc_bytes(mdat_t * dat, size_t size)
{
	size_t i;
	int j;
	u_int u;
	struct pginfo *bp;
	int k;
	u_int *lp;

	/* Don't bother with anything less than this */
	if (size < malloc_minsize)
		size = malloc_minsize;

	/* Find the right bucket */
	j = 1;
	i = size - 1;
	while (i >>= 1)
		j++;

	/* If it's empty, make a page more of that size chunks */
	if (!dat->page_dir[j] && !malloc_make_chunks(dat, j))
		return 0;

	bp = dat->page_dir[j];

	/* Find first word of bitmap which isn't empty */
	for (lp = bp->bits; !*lp; lp++) ;

	/* Find that bit, and tweak it */
	u = 1;
	k = 0;
	while (!(*lp & u)) {
		u += u;
		k++;
	}
	*lp ^= u;

	/* If there are no more free, remove from free-list */
	if (!--bp->free) {
		dat->page_dir[j] = bp->next;
		bp->next = 0;
	}

	/* Adjust to the real offset of that chunk */
	k += (lp - bp->bits) * MALLOC_BITS;
	k <<= bp->shift;

	if (dat->malloc_junk)
		memset((u_char *) bp->page + k, SOME_JUNK, (size_t) bp->size);

	return (u_char *) bp->page + k;
}

/*
 * Allocate a piece of memory
 */
static void *
imalloc(mdat_t * dat, size_t size)
{
	void *result;

	if ((size + dat->malloc_pagesize) < size)	/* Check for overflow */
		result = 0;
	else if (size <= malloc_maxsize)
		result = malloc_bytes(dat, size);
	else
		result = malloc_pages(dat, size);

	if (dat->malloc_abort && !result)
		wrterror(dat, "allocation failed.");

	if (dat->malloc_zero && result)
		memset(result, 0, size);

	return result;
}

/*
 * Change the size of an allocation.
 */
static void *
irealloc(mdat_t * dat, void *ptr, size_t size)
{
	void *p;
	size_t osize, idx;
	struct pginfo **mp;
	size_t i;

	if (dat->suicide)
		return NULL;

	idx = ptr2idx(ptr);

	if (idx < dat->malloc_pageshift) {
		wrtwarning(dat, "junk pointer, too low to make sense.");
		return 0;
	}

	if (idx > dat->last_idx) {
		wrtwarning(dat, "junk pointer, too high to make sense.");
		return 0;
	}

	mp = &dat->page_dir[idx];

	if (*mp == MALLOC_FIRST) {	/* Page allocation */

		/* Check the pointer */
		if ((size_t) ptr & dat->malloc_pagemask) {
			wrtwarning(dat, "modified (page-) pointer.");
			return 0;
		}

		/* Find the size in bytes */
		for (osize = dat->malloc_pagesize; *++mp == MALLOC_FOLLOW;)
			osize += dat->malloc_pagesize;

		if (!dat->malloc_realloc &&	/* unless we have to, */
		    size <= osize &&		/* .. or are too small, */
		    size > (osize - dat->malloc_pagesize)) {
		    				/* .. or can free a page, */
			return ptr;		/* don't do anything. */
		}

	} else if (*mp >= MALLOC_MAGIC) {	/* Chunk allocation */

		/* Check the pointer for sane values */
		if (((size_t) ptr & ((*mp)->size - 1))) {
			wrtwarning(dat, "modified (chunk-) pointer.");
			return 0;
		}

		/* Find the chunk index in the page */
		i = ((size_t) ptr & dat->malloc_pagemask) >> (*mp)->shift;

		/* Verify that it isn't a free chunk already */
		if ((*mp)->bits[i / MALLOC_BITS] & (1 << (i % MALLOC_BITS))) {
			wrtwarning(dat, "chunk is already free.");
			return 0;
		}

		osize = (*mp)->size;

		if (!dat->malloc_realloc &&	/* Unless we have to, */
		    size < osize &&		/* ..or are too small, */
		    (size > osize / 2 ||	/* ..or could use a smaller size, */
		     osize == malloc_minsize)) {/* ..(if there is one) */
			return ptr;		/* ..Don't do anything */
		}

	} else {
		wrtwarning(dat, "pointer to wrong page.");
		return 0;
	}

	p = imalloc(dat, size);

	if (p) {
		/* copy the lesser of the two sizes, and free the old one */
		if (!size || !osize) ;
		else if (osize < size)
			memcpy(p, ptr, osize);
		else
			memcpy(p, ptr, size);
		ifree(dat, ptr);
	}
	return p;
}

/*
 * Free a sequence of pages
 */

static __inline__ void
free_pages(mdat_t * dat, void *ptr, size_t idx, struct pginfo *info)
{
	size_t i;
	struct pgfree *pf, *pt = 0;
	size_t l;
	void *tail;

	/* Count how many pages and mark them free at the same time */
	dat->page_dir[idx] = MALLOC_FREE;
	for (i = 1; dat->page_dir[idx + i] == MALLOC_FOLLOW; i++)
		dat->page_dir[idx + i] = MALLOC_FREE;

	l = i << dat->malloc_pageshift;

	tail = (char *)ptr + l;

	/* add to free-list */
	if (!dat->px)
		dat->px = imalloc(dat, sizeof (*pt));	/* This cannot fail... */
	dat->px->page = ptr;
	dat->px->end = tail;
	dat->px->size = l;
	if (!dat->free_list.next) {

		/* Nothing on free list, put this at head */
		dat->px->next = dat->free_list.next;
		dat->px->prev = &dat->free_list;
		dat->free_list.next = dat->px;
		pf = dat->px;
		dat->px = 0;

	} else {

		/* Find the right spot, leave pf pointing to the modified entry. */
		tail = (char *)ptr + l;

		for (pf = dat->free_list.next; pf->end < ptr && pf->next; pf = pf->next) ;	/* Race ahead here */

		if (pf->page > tail) {
			/* Insert before entry */
			dat->px->next = pf;
			dat->px->prev = pf->prev;
			pf->prev = dat->px;
			dat->px->prev->next = dat->px;
			pf = dat->px;
			dat->px = 0;
		} else if (pf->end == ptr) {
			/* Append to the previous entry */
			pf->end = (char *)pf->end + l;
			pf->size += l;
			if (pf->next && pf->end == pf->next->page) {
				/* And collapse the next too. */
				pt = pf->next;
				pf->end = pt->end;
				pf->size += pt->size;
				pf->next = pt->next;
				if (pf->next)
					pf->next->prev = pf;
			}
		} else if (pf->page == tail) {
			/* Prepend to entry */
			pf->size += l;
			pf->page = ptr;
		} else if (!pf->next) {
			/* Append at tail of chain */
			dat->px->next = 0;
			dat->px->prev = pf;
			pf->next = dat->px;
			pf = dat->px;
			dat->px = 0;
		} else {
			wrterror(dat, "freelist is destroyed.");
		}
	}

	if (pt)
		ifree(dat, pt);
}

/*
 * Free a chunk, and possibly the page it's on, if the page becomes empty.
 */

static __inline__ void
free_bytes(mdat_t * dat, void *ptr, size_t idx, struct pginfo *info)
{
	size_t i;
	struct pginfo **mp;
	void *vp;

	/* Find the chunk number on the page */
	i = ((size_t) ptr & dat->malloc_pagemask) >> info->shift;

	if (((size_t) ptr & (info->size - 1))) {
		wrtwarning(dat, "modified (chunk-) pointer.");
		return;
	}

	if (info->bits[i / MALLOC_BITS] & (1 << (i % MALLOC_BITS))) {
		wrtwarning(dat, "chunk is already free.");
		return;
	}

	if (dat->malloc_junk)
		memset(ptr, SOME_JUNK, (size_t) info->size);

	info->bits[i / MALLOC_BITS] |= 1 << (i % MALLOC_BITS);
	info->free++;

	mp = dat->page_dir + info->shift;

	if (info->free == 1) { /* Page became non-full */
		mp = dat->page_dir + info->shift;

		/* Insert in address order */
		while (*mp && (*mp)->next && (*mp)->next->page < info->page)
			mp = &(*mp)->next;
		info->next = *mp;
		*mp = info;
		return;
	}

	if (info->free != info->total)
		return;

	/* Find & remove this page in the queue */
	while (*mp != info) {
		mp = &((*mp)->next);
#ifdef MALLOC_EXTRA_SANITY
		if (!*mp)
			wrterror("(ES): Not on queue");
#endif /* MALLOC_EXTRA_SANITY */
	}
	*mp = info->next;

	/* Free the page & the info structure if need be */
	dat->page_dir[idx] = MALLOC_FIRST;
	vp = info->page;	/* Order is important ! */
	if (vp != (void *)info)
		ifree(dat, info);
	ifree(dat, vp);
}

static void
ifree(mdat_t * dat, void *ptr)
{
	struct pginfo *info;
	size_t idx;

	/* This is legal */
	if (!ptr)
		return;

	if (!dat->malloc_started) {
		wrtwarning(dat, "malloc() has never been called.");
		return;
	}

	/* If we're already sinking, don't make matters any worse. */
	if (dat->suicide)
		return;

	idx = ptr2idx(ptr);

	if (idx < dat->malloc_pageshift) {
		wrtwarning(dat, "junk pointer, too low to make sense.");
		return;
	}

	if (idx > dat->last_idx) {
		wrtwarning(dat, "junk pointer, too high to make sense.");
		return;
	}

	info = dat->page_dir[idx];

	if (info < MALLOC_MAGIC)
		free_pages(dat, ptr, idx, info);
	else
		free_bytes(dat, ptr, idx, info);
	return;
}

/*
 * These are the public exported interface routines.
 */

void *
hmalloc_init(size_t size, cap_t * cap)
{
	cap_t dat;

	dat.passwd = 0x37; /* FIXME */
	dat.address = ObjCreate(sizeof (mdat_t), dat.passwd, NULL);

	if (dat.address == NULL)
		return NULL;

	add_cap(dat);

	((mdat_t *) dat.address)->malloc_cache = size; /* no. of pages */

	/* give the cap to the caller, if they want it */
	if (cap)
		*cap = dat;

	return dat.address;
}

/* add caps needed for the hmalloc stuff */
void *
hmalloc_access(void *vdat)
{
	mdat_t *dat;

	dat = (mdat_t *) vdat;
	assert(dat);

	add_cap(dat->page_dir_cap);
	add_cap(dat->heap_cap);
	return NULL;
}

void *
hmalloc(void *vdat, size_t size)
{
	register void *r;
	mdat_t *dat;

	dat = (mdat_t *) vdat;
	assert(dat);

	THREAD_LOCK();
	dat->malloc_func = " in hmalloc():";
	if (dat->malloc_active++) {
		wrtwarning(dat, "recursive call.");
		dat->malloc_active--;
		return (0);
	}

	if (!dat->malloc_started)
		malloc_init(dat);
	if (dat->malloc_sysv && !size)
		r = 0;
	else
		r = imalloc(dat, size);

	dat->malloc_active--;
	THREAD_UNLOCK();
	if (r == NULL && (size != 0 || !dat->malloc_sysv)) {
		/* errno = ENOMEM; */// FIXME: Should we do something else here?
	}
	return (r);
}

void
hfree(void *vdat, void *ptr)
{
	mdat_t *dat;

	dat = (mdat_t *) vdat;
	assert(dat);

	THREAD_LOCK();
	dat->malloc_func = " in hfree():";
	if (dat->malloc_active++) {
		wrtwarning(dat, "recursive call.");
		dat->malloc_active--;
		return;
	} else {
		ifree(dat, ptr);
	}
	dat->malloc_active--;
	THREAD_UNLOCK();
	return;
}

void *
hrealloc(void *vdat, void *ptr, size_t size)
{
	register void *r;
	mdat_t *dat;

	dat = (mdat_t *) vdat;
	assert(dat);

	THREAD_LOCK();
	dat->malloc_func = " in hrealloc():";
	if (dat->malloc_active++) {
		wrtwarning(dat, "recursive call.");
		dat->malloc_active--;
		return (0);
	}
	if (ptr && !dat->malloc_started) {
		wrtwarning(dat, "malloc() has never been called.");
		ptr = 0;
	}
	if (!dat->malloc_started)
		malloc_init(dat);
	if (dat->malloc_sysv && !size) {
		ifree(dat, ptr);
		r = 0;
	} else if (!ptr) {
		r = imalloc(dat, size);
	} else {
		r = irealloc(dat, ptr, size);
	}

	dat->malloc_active--;
	THREAD_UNLOCK();
	if (r == NULL && (size != 0 || !dat->malloc_sysv)) {
		/* errno = ENOMEM; */// FIXME: Do somethign else?
	}
	return (r);
}

/* FIXME: This is bad! */
static void
add_cap(cap_t cap)
{
	clist_t *clist;
	apddesc_t myapd;

	ApdGet(&myapd);
	clist = (clist_t *) myapd.clist[0].address;
	clist->caps[clist->n_caps++] = cap;
}
