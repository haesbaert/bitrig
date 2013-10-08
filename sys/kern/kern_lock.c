/*	$OpenBSD: kern_lock.c,v 1.39 2013/03/28 16:55:25 deraadt Exp $	*/

/* 
 * Copyright (c) 1995
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code contains ideas from software contributed to Berkeley by
 * Avadis Tevanian, Jr., Michael Wayne Young, and the Mach Operating
 * System project at Carnegie-Mellon University.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)kern_lock.c	8.18 (Berkeley) 5/21/95
 */

#include <sys/param.h>
#include <sys/proc.h>
#include <sys/lock.h>
#include <sys/systm.h>
#include <sys/sched.h>
#include <sys/mutex.h>

#include <machine/atomic.h>

#ifdef MP_LOCKDEBUG
/* CPU-dependent timing, needs this to be settable from ddb. */
int __mp_lock_spinout = 200000000;
#endif

void
lockinit(struct lock *lkp, int prio, char *wmesg, int timo, int flags)
{

	KASSERT(flags == 0);

	bzero(lkp, sizeof(struct lock));
	rrw_init(&lkp->lk_lck, wmesg);
}

int
lockstatus(struct lock *lkp)
{
	int	status;

	status = rrw_status(&lkp->lk_lck);
	if (status == 1)
		return LK_EXCLUSIVE;
	if (status == -1)
		return LK_SHARED;
	return 0;
}

int
lockmgr(struct lock *lkp, u_int flags, void *notused)
{
	int	rwflags;

	rwflags = 0;

	KASSERT(!((flags & (LK_SHARED|LK_EXCLUSIVE)) ==
	    (LK_SHARED|LK_EXCLUSIVE)));
	KASSERT(!((flags & (LK_CANRECURSE|LK_RECURSEFAIL)) ==
	    (LK_CANRECURSE|LK_RECURSEFAIL)));
	KASSERT((flags & LK_RELEASE) ||
	    (flags & (LK_SHARED|LK_EXCLUSIVE|LK_DRAIN)));


	if (flags & LK_RELEASE) {
		rrw_exit(&lkp->lk_lck);
		return (0);
	}

	if (flags & LK_SHARED)
		rwflags |= RW_READ;
	if (flags & (LK_EXCLUSIVE|LK_DRAIN))
		rwflags |= RW_WRITE;

	if (flags & LK_RECURSEFAIL)
		rwflags |= RW_RECURSEFAIL;
	if (flags & LK_NOWAIT)
		rwflags |= RW_NOSLEEP;

	return (rrw_enter(&lkp->lk_lck, rwflags));
}

#if defined(MULTIPROCESSOR)
/*
 * Functions for manipulating the kernel_lock.  We put them here
 * so that they show up in profiles.
 */

struct __mp_lock kernel_lock;

void
_kernel_lock_init(void)
{
	__mp_lock_init(&kernel_lock);
}

/*
 * Acquire/release the kernel lock.  Intended for use in the scheduler
 * and the lower half of the kernel.
 */

void
_kernel_lock(void)
{
	SCHED_ASSERT_UNLOCKED();
	__mp_lock(&kernel_lock);
}

void
_kernel_unlock(void)
{
	__mp_unlock(&kernel_lock);
}

#endif /* MULTIPROCESSOR */

void
__mtx_init(struct mutex *mtx, int ipl)
{
	mtx->mtx_wantipl = ipl;
	mtx->mtx_oldipl	 = IPL_NONE;
	mtx->mtx_owner	 = NULL;
}

int
mtx_enter_try(struct mutex *mtx)
{
	struct cpu_info *ci = curcpu();

#ifdef DIAGNOSTIC
	if (__predict_false(mtx->mtx_owner == ci))
		panic("mtx_enter_try: locking against myself");
#endif
	crit_enter();

#ifdef MULTIPROCESSOR
	if (atomic_cmpset_ptr(&mtx->mtx_owner, NULL, ci) == 0) {
		crit_leave();
		return (0);
	}
#else
	mtx->mtx_owner = ci;
#endif

#ifdef DIAGNOSTIC
	ci->ci_mutex_level++;
#endif

	return (1);
}

void
mtx_enter(struct mutex *mtx)
{
	while (mtx_enter_try(mtx) == 0)
		;
}

void
mtx_leave(struct mutex *mtx)
{
	struct cpu_info *ci = curcpu();

#ifdef DIAGNOSTIC
	if (__predict_false(mtx->mtx_owner != ci))
		panic("mtx_leave: lock not held");
#endif
	/* XXX compiler fence here ? */
	mtx->mtx_owner = NULL;
#ifdef DIAGNOSTIC
	ci->ci_mutex_level--;
#endif

	crit_leave();
}
