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

#ifndef _SYS_DCACHE_H_
#define _SYS_DCACHE_H_

#define MAG_CAPACITY		256

struct dcache_pcpu {
	struct dcache_mag *		dp_loadedmag;
	struct dcache_mag *		dp_prevmag;
	int				dp_sleeper;
};

struct dcache_mag {
	SLIST_ENTRY(dcache_mag)		dm_entry;
	int				dm_rounds;
	void *				dm_objs[MAG_CAPACITY];
};

struct dcache {
	u_int				dc_maxmags;
	u_int				dc_curmags;
	size_t				dc_objsize;
	struct mutex			dc_mtx; /* Protects emptymags and fullmags */
	int				dc_sleeper;
	SLIST_HEAD(, dcache_mag)	dc_emptymags;
	SLIST_HEAD(, dcache_mag)	dc_fullmags;
	struct dcache_pcpu		dc_pcpu[MAXCPUS];
};


void	dcache_init(struct dcache *, size_t, u_int);
void *	dcache_get(struct dcache *, int);
void	dcache_put(struct dcache *, void *);

#endif	/* _SYS_DCACHE_H_ */
