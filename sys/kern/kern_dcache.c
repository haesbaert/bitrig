/*
 * Copyright (c) 2013 Christiano F. Haesbaert <haesbaert@haesbaert.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * This is heavily based on Dragonfly's objcache from Jeffrey M. Hsu.
 *
 * dcache is a lock/atomic-ops free object allocator on the fast path and
 * spinlocked on the slow path.
 *
 * Each cpu holds always 2 magazines, each magazine holds MAG_CAPACITY
 * objects. When one cpu requests an object, it looks into it's 'loaded'
 * magazine, if it's empty, it swaps its 'loaded' magazine with the 'prev'
 * magazine, if the new 'loaded' magazine is also empty, it enters the slow
 * path. In slow path the cpu exchanges its empty magazine with a full loaded
 * one from the global depot, to do it, it must acquire the depot lock. It is
 * amortized since one slow path yields another MAG_CAPACITY fast paths.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/malloc.h>
#include <sys/dcache.h>

#include <uvm/uvm.h>

#define MAG_EMPTY(dm)		(dm->dm_rounds == 0)
#define MAG_FULL(dm)		(dm->dm_rounds == MAG_CAPACITY)
#define MAG_POPOBJ(dm)		((dm)->dm_objs[--(dm)->dm_rounds])
#define MAG_PUTOBJ(dm, obj)	((dm)->dm_objs[(dm)->dm_rounds++] = obj)
#define MAG_SWAP(x, y)		({ struct dcache_mag *t = (x); (x) = (y); (y) = (t); })

int	dcache_add_mag(struct dcache *, int, int);
int	dcache_mag_prime(struct dcache *, struct dcache_mag *, int);

void
dcache_init(struct dcache *dc, size_t objsize, u_int maxobjs)
{
	struct dcache_pcpu *dp;
	int i, nemptymags, nstartfullmags;
	int myncpus;

	if (ncpus <= 1) {
		printf("\n\nCreating dcache before ncpu is set ?\n\n");
		myncpus = 6;
	} else
		myncpus = ncpus;

	bzero(dc, sizeof(*dc));

	mtx_init(&dc->dc_mtx, IPL_HIGH);
	dc->dc_objsize = objsize;
	/* Calculate how many full magazines we can have */
	dc->dc_maxmags = howmany(maxobjs, MAG_CAPACITY);
	if (dc->dc_maxmags < (myncpus * 2))
		dc->dc_maxmags = myncpus * 2;

	SLIST_INIT(&dc->dc_emptymags);
	SLIST_INIT(&dc->dc_fullmags);

	/* We need at least these emptymags to avoid worst case */
	nemptymags = (myncpus - 1) * 2; /* may be 0 */
	if (nemptymags == 0)
		nemptymags = 2;

	while (nemptymags--)
		if (dcache_add_mag(dc, 1, 1) != 0)
			panic("dcache_init");

	for (nstartfullmags = myncpus; nstartfullmags > 0; nstartfullmags--)
		if (dcache_add_mag(dc, 0, 1) != 0)
			panic("dcache_init");

	/* Distribute the magazines across cpus */
	mtx_enter(&dc->dc_mtx);
	for (i = 0; i < myncpus; i++) {
		dp = &dc->dc_pcpu[i];

		KASSERT(SLIST_FIRST(&dc->dc_fullmags) != NULL);
		KASSERT(SLIST_FIRST(&dc->dc_emptymags) != NULL);
		dp->dp_loadedmag = SLIST_FIRST(&dc->dc_fullmags);
		SLIST_REMOVE_HEAD(&dc->dc_fullmags, dm_entry);
		dp->dp_prevmag = SLIST_FIRST(&dc->dc_emptymags);
		SLIST_REMOVE_HEAD(&dc->dc_emptymags, dm_entry);
	}
	mtx_leave(&dc->dc_mtx);
}

int
dcache_add_mag(struct dcache *dc, int empty, int waitok)
{
	struct dcache_mag *dm;

	/* KERNEL_LOCK(); */
	dm = malloc(sizeof(*dm), M_DEVBUF, (waitok ? M_WAITOK : 0) | M_ZERO);
	/* KERNEL_UNLOCK(); */

	if (dm == NULL)
		return (ENOMEM);

	if (empty) {
		mtx_enter(&dc->dc_mtx);
		SLIST_INSERT_HEAD(&dc->dc_emptymags, dm, dm_entry);
		mtx_leave(&dc->dc_mtx); 
	} else if (dcache_mag_prime(dc, dm, waitok) != 0) {
		free(dm, M_DEVBUF);
		return (ENOMEM);
	}

	return (0);
}

int
dcache_mag_prime(struct dcache *dc, struct dcache_mag *dm, int waitok)
{
	char *p, *end;
	size_t sz;
	struct kmem_dyn_mode kd = KMEM_DYN_INITIALIZER;
	int s;

	kd.kd_waitok = waitok;

	sz = round_page(dc->dc_objsize * MAG_CAPACITY);
	s = splvm();
	/* KERNEL_LOCK() */
	p = km_alloc(sz, &kv_intrsafe, &kp_dma_zero, &kd);
	/* KERNEL_UNLOCK() */
	splx(s);

	if (p == NULL)
		return (ENOMEM);

	end = p + sz;

	while (p < end) {
		dm->dm_objs[dm->dm_rounds++] = p;
		p += dc->dc_objsize;

		if (dm->dm_rounds > MAG_CAPACITY)
			panic("whoops dm_rounds overflow");
	}

	KASSERT(dm->dm_rounds == MAG_CAPACITY);

	mtx_enter(&dc->dc_mtx);
	SLIST_INSERT_HEAD(&dc->dc_fullmags, dm, dm_entry);
	dc->dc_curmags++;
	mtx_leave(&dc->dc_mtx);

	return (0);
}

void *
dcache_get(struct dcache *dc, int waitok)
{
	void *obj;
	struct dcache_pcpu *dp = &dc->dc_pcpu[curcpu()->ci_cpuid];
	int s;

again:
	s = splhigh();

	/* 1st Fast path, we have an object in the loaded magazine */
	if (!MAG_EMPTY(dp->dp_loadedmag)) {
		obj = MAG_POPOBJ(dp->dp_loadedmag);
		splx(s);
		return (obj);
	}

	/*
	 * 2nd Fast path, we have an object in the previous magazine,
	 * swap and retry
	 */
	if (!MAG_EMPTY(dp->dp_prevmag)) {
		MAG_SWAP(dp->dp_loadedmag, dp->dp_prevmag);
		splx(s);
		goto again;
	}

	/*
	 * Slow path, there are no objects for us to grab, switch our empty
	 * loaded magazine with a full one from the depot. If there isn't one,
	 * see if we can add a new magazine.
	 */

	/* Can we allocate more ? The comparison may race, it's ok to go over
	 * it a little bit */
	if (SLIST_EMPTY(&dc->dc_fullmags) && dc->dc_curmags < dc->dc_maxmags)
		(void)dcache_add_mag(dc, 1, waitok);

	/* Ok to race here, we recover below */
	if (!waitok && SLIST_EMPTY(&dc->dc_fullmags)) {
		splx(s);
		return (NULL);
	}

	mtx_enter(&dc->dc_mtx);
	if (SLIST_EMPTY(&dc->dc_fullmags)) {
		if (!waitok) {
			mtx_leave(&dc->dc_mtx);
			splx(s);
			return (NULL);
		}
		dp->dp_sleeper++;
		dc->dc_sleeper++;
		msleep(dc, &dc->dc_mtx, PVM, "dacache magazine", 0);
		dc->dc_sleeper--;
		dp->dp_sleeper--;
		mtx_leave(&dc->dc_mtx);
		splx(s);
		goto again;
	}
	SLIST_INSERT_HEAD(&dc->dc_emptymags, dp->dp_loadedmag, dm_entry);
	dp->dp_loadedmag = SLIST_FIRST(&dc->dc_fullmags);
	KASSERT(dp->dp_loadedmag != NULL);
	SLIST_REMOVE_HEAD(&dc->dc_fullmags, dm_entry);
	mtx_leave(&dc->dc_mtx);

	splx(s);
	goto again;
}

void
dcache_put(struct dcache *dc, void *obj)
{
	struct dcache_pcpu *dp = &dc->dc_pcpu[curcpu()->ci_cpuid];
	int s;

again:

	s = splhigh();

	/* 1st Fast path, we have a slot in the loaded mag */
	if (!MAG_FULL(dp->dp_loadedmag)) {
		MAG_PUTOBJ(dp->dp_loadedmag, obj);
		if (dp->dp_sleeper)
			wakeup(dc);
		splx(s);
		return;
	}

	/* 2nd Fast path, we have a slot in the prev mag */
	if (!MAG_FULL(dp->dp_prevmag)) {
		MAG_PUTOBJ(dp->dp_prevmag, obj);
		MAG_SWAP(dp->dp_loadedmag, dp->dp_prevmag);
		if (dp->dp_sleeper)
			wakeup(dc);
		splx(s);
		return;
	}

	mtx_enter(&dc->dc_mtx);
	if (SLIST_EMPTY(&dc->dc_emptymags))
		panic("No more emptymags, this can never happen, wrong math!");
	SLIST_INSERT_HEAD(&dc->dc_fullmags, dp->dp_loadedmag, dm_entry);
	dp->dp_loadedmag = SLIST_FIRST(&dc->dc_emptymags);
	SLIST_REMOVE_HEAD(&dc->dc_emptymags, dm_entry);
	if (dc->dc_sleeper)
		wakeup(dc);
	mtx_leave(&dc->dc_mtx);

	splx(s);
	goto again;
}
