/*	$OpenBSD: scheduler_ramqueue.c,v 1.36 2013/12/26 17:25:32 eric Exp $	*/

/*
 * Copyright (c) 2012 Gilles Chehade <gilles@poolp.org>
 * Copyright (c) 2012 Eric Faurot <eric@openbsd.org>
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

#include <sys/types.h>
#include <sys/queue.h>
#include <sys/tree.h>
#include <sys/socket.h>

#include <ctype.h>
#include <err.h>
#include <event.h>
#include <fcntl.h>
#include <imsg.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "smtpd.h"
#include "log.h"

TAILQ_HEAD(evplist, rq_envelope);

struct rq_message {
	uint32_t		 msgid;
	struct tree		 envelopes;
};

struct rq_envelope {
	TAILQ_ENTRY(rq_envelope) entry;
	SPLAY_ENTRY(rq_envelope) t_entry;

	uint64_t		 evpid;
	uint64_t		 holdq;
	enum delivery_type	 type;

#define	RQ_EVPSTATE_PENDING	 0
#define	RQ_EVPSTATE_SCHEDULED	 1
#define	RQ_EVPSTATE_INFLIGHT	 2
#define	RQ_EVPSTATE_HELD	 3
	uint8_t			 state;

#define	RQ_ENVELOPE_EXPIRED	 0x01
#define	RQ_ENVELOPE_REMOVED	 0x02
#define	RQ_ENVELOPE_SUSPEND	 0x04
#define	RQ_ENVELOPE_UPDATE	 0x08
	uint8_t			 flags;

	time_t			 ctime;
	time_t			 sched;
	time_t			 expire;

	struct rq_message	*message;

	time_t			 t_inflight;
	time_t			 t_scheduled;
};

struct rq_holdq {
	struct evplist		 q;
};

struct rq_queue {
	size_t			 evpcount;
	struct tree		 messages;
	SPLAY_HEAD(prioqtree, rq_envelope)	q_priotree;

	struct evplist		 q_pending;
	struct evplist		 q_inflight;

	struct evplist		 q_mta;
	struct evplist		 q_mda;
	struct evplist		 q_bounce;
	struct evplist		 q_update;
	struct evplist		 q_expired;
	struct evplist		 q_removed;
};

static int rq_envelope_cmp(struct rq_envelope *, struct rq_envelope *);

SPLAY_PROTOTYPE(prioqtree, rq_envelope, t_entry, rq_envelope_cmp);
static int scheduler_ram_init(void);
static int scheduler_ram_insert(struct scheduler_info *);
static size_t scheduler_ram_commit(uint32_t);
static size_t scheduler_ram_rollback(uint32_t);
static int scheduler_ram_update(struct scheduler_info *);
static int scheduler_ram_delete(uint64_t);
static int scheduler_ram_hold(uint64_t, uint64_t);
static int scheduler_ram_release(int, uint64_t, int);
static int scheduler_ram_batch(int, struct scheduler_batch *);
static size_t scheduler_ram_messages(uint32_t, uint32_t *, size_t);
static size_t scheduler_ram_envelopes(uint64_t, struct evpstate *, size_t);
static int scheduler_ram_schedule(uint64_t);
static int scheduler_ram_remove(uint64_t);
static int scheduler_ram_suspend(uint64_t);
static int scheduler_ram_resume(uint64_t);

static void sorted_insert(struct rq_queue *, struct rq_envelope *);

static void rq_queue_init(struct rq_queue *);
static void rq_queue_merge(struct rq_queue *, struct rq_queue *);
static void rq_queue_dump(struct rq_queue *, const char *);
static void rq_queue_schedule(struct rq_queue *rq);
static struct evplist *rq_envelope_list(struct rq_queue *, struct rq_envelope *);
static void rq_envelope_schedule(struct rq_queue *, struct rq_envelope *);
static int rq_envelope_remove(struct rq_queue *, struct rq_envelope *);
static int rq_envelope_suspend(struct rq_queue *, struct rq_envelope *);
static int rq_envelope_resume(struct rq_queue *, struct rq_envelope *);
static void rq_envelope_delete(struct rq_queue *, struct rq_envelope *);
static const char *rq_envelope_to_text(struct rq_envelope *);

struct scheduler_backend scheduler_backend_ramqueue = {
	scheduler_ram_init,

	scheduler_ram_insert,
	scheduler_ram_commit,
	scheduler_ram_rollback,

	scheduler_ram_update,
	scheduler_ram_delete,
	scheduler_ram_hold,
	scheduler_ram_release,

	scheduler_ram_batch,

	scheduler_ram_messages,
	scheduler_ram_envelopes,
	scheduler_ram_schedule,
	scheduler_ram_remove,
	scheduler_ram_suspend,
	scheduler_ram_resume,
};

static struct rq_queue	ramqueue;
static struct tree	updates;
static struct tree	holdqs[3]; /* delivery type */

static time_t		currtime;

static int
scheduler_ram_init(void)
{
	rq_queue_init(&ramqueue);
	tree_init(&updates);
	tree_init(&holdqs[D_MDA]);
	tree_init(&holdqs[D_MTA]);
	tree_init(&holdqs[D_BOUNCE]);

	return (1);
}

static int
scheduler_ram_insert(struct scheduler_info *si)
{
	uint32_t		 msgid;
	struct rq_queue		*update;
	struct rq_message	*message;
	struct rq_envelope	*envelope;

	currtime = time(NULL);

	msgid = evpid_to_msgid(si->evpid);

	/* find/prepare a ramqueue update */
	if ((update = tree_get(&updates, msgid)) == NULL) {
		update = xcalloc(1, sizeof *update, "scheduler_insert");
		stat_increment("scheduler.ramqueue.update", 1);
		rq_queue_init(update);
		tree_xset(&updates, msgid, update);
	}

	/* find/prepare the msgtree message in ramqueue update */
	if ((message = tree_get(&update->messages, msgid)) == NULL) {
		message = xcalloc(1, sizeof *message, "scheduler_insert");
		message->msgid = msgid;
		tree_init(&message->envelopes);
		tree_xset(&update->messages, msgid, message);
		stat_increment("scheduler.ramqueue.message", 1);
	}

	/* create envelope in ramqueue message */
	envelope = xcalloc(1, sizeof *envelope, "scheduler_insert");
	envelope->evpid = si->evpid;
	envelope->type = si->type;
	envelope->message = message;
	envelope->ctime = si->creation;
	envelope->expire = si->creation + si->expire;
	envelope->sched = scheduler_compute_schedule(si);
	tree_xset(&message->envelopes, envelope->evpid, envelope);

	update->evpcount++;
	stat_increment("scheduler.ramqueue.envelope", 1);

	envelope->state = RQ_EVPSTATE_PENDING;
	TAILQ_INSERT_TAIL(&update->q_pending, envelope, entry);

	si->nexttry = envelope->sched;

	return (1);
}

static size_t
scheduler_ram_commit(uint32_t msgid)
{
	struct rq_queue	*update;
	size_t		 r;

	currtime = time(NULL);

	update = tree_xpop(&updates, msgid);
	r = update->evpcount;

	if (verbose & TRACE_SCHEDULER)
		rq_queue_dump(update, "update to commit");

	rq_queue_merge(&ramqueue, update);

	if (verbose & TRACE_SCHEDULER)
		rq_queue_dump(&ramqueue, "resulting queue");

	rq_queue_schedule(&ramqueue);

	free(update);
	stat_decrement("scheduler.ramqueue.update", 1);

	return (r);
}

static size_t
scheduler_ram_rollback(uint32_t msgid)
{
	struct rq_queue		*update;
	struct rq_envelope	*evp;
	size_t			 r;

	currtime = time(NULL);

	if ((update = tree_pop(&updates, msgid)) == NULL)
		return (0);
	r = update->evpcount;

	while ((evp = TAILQ_FIRST(&update->q_pending))) {
		TAILQ_REMOVE(&update->q_pending, evp, entry);
		rq_envelope_delete(update, evp);
	}

	free(update);
	stat_decrement("scheduler.ramqueue.update", 1);

	return (r);
}

static int
scheduler_ram_update(struct scheduler_info *si)
{
	struct rq_message	*msg;
	struct rq_envelope	*evp;
	uint32_t		 msgid;

	currtime = time(NULL);

	msgid = evpid_to_msgid(si->evpid);
	msg = tree_xget(&ramqueue.messages, msgid);
	evp = tree_xget(&msg->envelopes, si->evpid);

	/* it *must* be in-flight */
	if (evp->state != RQ_EVPSTATE_INFLIGHT)
		errx(1, "evp:%016" PRIx64 " not in-flight", si->evpid);

	TAILQ_REMOVE(&ramqueue.q_inflight, evp, entry);

	/*
	 * If the envelope was removed while inflight,  schedule it for
	 * removal immediatly.
	 */
	if (evp->flags & RQ_ENVELOPE_REMOVED) {
		TAILQ_INSERT_TAIL(&ramqueue.q_removed, evp, entry);
		evp->state = RQ_EVPSTATE_SCHEDULED;
		evp->t_scheduled = currtime;
		return (1);
	}

	while ((evp->sched = scheduler_compute_schedule(si)) <= currtime)
		si->retry += 1;

	evp->state = RQ_EVPSTATE_PENDING;
	if (!(evp->flags & RQ_ENVELOPE_SUSPEND))
		sorted_insert(&ramqueue, evp);

	si->nexttry = evp->sched;

	return (1);
}

static int
scheduler_ram_delete(uint64_t evpid)
{
	struct rq_message	*msg;
	struct rq_envelope	*evp;
	uint32_t		 msgid;

	currtime = time(NULL);

	msgid = evpid_to_msgid(evpid);
	msg = tree_xget(&ramqueue.messages, msgid);
	evp = tree_xget(&msg->envelopes, evpid);

	/* it *must* be in-flight */
	if (evp->state != RQ_EVPSTATE_INFLIGHT)
		errx(1, "evp:%016" PRIx64 " not in-flight", evpid);

	TAILQ_REMOVE(&ramqueue.q_inflight, evp, entry);

	rq_envelope_delete(&ramqueue, evp);

	return (1);
}

static int
scheduler_ram_hold(uint64_t evpid, uint64_t holdq)
{
	struct rq_holdq		*hq;
	struct rq_message	*msg;
	struct rq_envelope	*evp;
	uint32_t		 msgid;

	currtime = time(NULL);

	msgid = evpid_to_msgid(evpid);
	msg = tree_xget(&ramqueue.messages, msgid);
	evp = tree_xget(&msg->envelopes, evpid);

	/* it *must* be in-flight */
	if (evp->state != RQ_EVPSTATE_INFLIGHT)
		errx(1, "evp:%016" PRIx64 " not in-flight", evpid);

	TAILQ_REMOVE(&ramqueue.q_inflight, evp, entry);

	/* If the envelope is suspended, just mark it as pending */
	if (evp->flags & RQ_ENVELOPE_SUSPEND) {
		evp->state = RQ_EVPSTATE_PENDING;
		return (0);
	}

	hq = tree_get(&holdqs[evp->type], holdq);
	if (hq == NULL) {
		hq = xcalloc(1, sizeof(*hq), "scheduler_hold");
		TAILQ_INIT(&hq->q);
		tree_xset(&holdqs[evp->type], holdq, hq);
	}

	evp->state = RQ_EVPSTATE_HELD;
	evp->holdq = holdq;
	/* This is an optimization: upon release, the envelopes will be
	 * inserted in the pending queue from the first element to the last.
	 * Since elements already in the queue were received first, they
	 * were scheduled first, so they will be reinserted before the
	 * current element.
	 */
	TAILQ_INSERT_HEAD(&hq->q, evp, entry);
	stat_increment("scheduler.ramqueue.hold", 1);

	return (1);
}

static int
scheduler_ram_release(int type, uint64_t holdq, int n)
{
	struct rq_holdq		*hq;
	struct rq_envelope	*evp;
	int			 i, update;

	currtime = time(NULL);

	hq = tree_get(&holdqs[type], holdq);
	if (hq == NULL)
		return (0);

	if (n == -1) {
		n = 0;
		update = 1;
	}
	else
		update = 0;

	for (i = 0; n == 0 || i < n; i++) {
		evp = TAILQ_FIRST(&hq->q);
		if (evp == NULL)
			break;

		TAILQ_REMOVE(&hq->q, evp, entry);
		evp->holdq = 0;

		/* When released, all envelopes are put in the pending queue
		 * and will be rescheduled immediatly.  As an optimization,
		 * we could just schedule them directly.
		 */
		evp->state = RQ_EVPSTATE_PENDING;
		if (update)
			evp->flags |= RQ_ENVELOPE_UPDATE;
		sorted_insert(&ramqueue, evp);
	}

	if (TAILQ_EMPTY(&hq->q)) {
		tree_xpop(&holdqs[type], holdq);
		free(hq);
	}
	stat_decrement("scheduler.ramqueue.hold", i);

	return (i);
}

static int
scheduler_ram_batch(int typemask, struct scheduler_batch *ret)
{
	struct evplist		*q;
	struct rq_envelope	*evp;
	size_t			 n;
	int			 retry;

	currtime = time(NULL);

	rq_queue_schedule(&ramqueue);
	if (verbose & TRACE_SCHEDULER)
		rq_queue_dump(&ramqueue, "scheduler_ram_batch()");

	if (typemask & SCHED_REMOVE && TAILQ_FIRST(&ramqueue.q_removed)) {
		q = &ramqueue.q_removed;
		ret->type = SCHED_REMOVE;
	}
	else if (typemask & SCHED_EXPIRE && TAILQ_FIRST(&ramqueue.q_expired)) {
		q = &ramqueue.q_expired;
		ret->type = SCHED_EXPIRE;
	}
	else if (typemask & SCHED_UPDATE && TAILQ_FIRST(&ramqueue.q_update)) {
		q = &ramqueue.q_update;
		ret->type = SCHED_UPDATE;
	}
	else if (typemask & SCHED_BOUNCE && TAILQ_FIRST(&ramqueue.q_bounce)) {
		q = &ramqueue.q_bounce;
		ret->type = SCHED_BOUNCE;
	}
	else if (typemask & SCHED_MDA && TAILQ_FIRST(&ramqueue.q_mda)) {
		q = &ramqueue.q_mda;
		ret->type = SCHED_MDA;
	}
	else if (typemask & SCHED_MTA && TAILQ_FIRST(&ramqueue.q_mta)) {
		q = &ramqueue.q_mta;
		ret->type = SCHED_MTA;
	}
	else if ((evp = TAILQ_FIRST(&ramqueue.q_pending))) {
		ret->type = SCHED_DELAY;
		ret->evpcount = 0;
		if (evp->sched < evp->expire)
			ret->delay = evp->sched - currtime;
		else
			ret->delay = evp->expire - currtime;
		return (1);
	}
	else {
		ret->type = SCHED_NONE;
		ret->evpcount = 0;
		return (0);
	}

	for (n = 0; (evp = TAILQ_FIRST(q)) && n < ret->evpcount; n++) {

		TAILQ_REMOVE(q, evp, entry);

		/* consistency check */
		if (evp->state != RQ_EVPSTATE_SCHEDULED)
			errx(1, "evp:%016" PRIx64 " not scheduled", evp->evpid);

		ret->evpids[n] = evp->evpid;

		if (ret->type == SCHED_REMOVE || ret->type == SCHED_EXPIRE)
			rq_envelope_delete(&ramqueue, evp);
		else if (ret->type == SCHED_UPDATE) {

			evp->flags &= ~RQ_ENVELOPE_UPDATE;

			/* XXX we can't really use scheduler_compute_schedule */
			retry = 0;
			while ((evp->sched = evp->ctime + 800 * retry * retry / 2) <= currtime)
				retry += 1;

			evp->state = RQ_EVPSTATE_PENDING;
			if (!(evp->flags & RQ_ENVELOPE_SUSPEND))
				sorted_insert(&ramqueue, evp);
		}
		else {
			TAILQ_INSERT_TAIL(&ramqueue.q_inflight, evp, entry);
			evp->state = RQ_EVPSTATE_INFLIGHT;
			evp->t_inflight = currtime;
		}
	}

	ret->evpcount = n;

	return (1);
}

static size_t
scheduler_ram_messages(uint32_t from, uint32_t *dst, size_t size)
{
	uint64_t id;
	size_t	 n;
	void	*i;

	for (n = 0, i = NULL; n < size; n++) {
		if (tree_iterfrom(&ramqueue.messages, &i, from, &id, NULL) == 0)
			break;
		dst[n] = id;
	}

	return (n);
}

static size_t
scheduler_ram_envelopes(uint64_t from, struct evpstate *dst, size_t size)
{
	struct rq_message	*msg;
	struct rq_envelope	*evp;
	void			*i;
	size_t			 n;

	if ((msg = tree_get(&ramqueue.messages, evpid_to_msgid(from))) == NULL)
		return (0);

	for (n = 0, i = NULL; n < size; ) {

		if (tree_iterfrom(&msg->envelopes, &i, from, NULL,
		    (void**)&evp) == 0)
			break;

		if (evp->flags & (RQ_ENVELOPE_REMOVED | RQ_ENVELOPE_EXPIRED))
			continue;

		dst[n].evpid = evp->evpid;
		dst[n].flags = 0;
		dst[n].retry = 0;
		dst[n].time = 0;

		if (evp->state == RQ_EVPSTATE_PENDING) {
			dst[n].time = evp->sched;
			dst[n].flags = EF_PENDING;
		}
		else if (evp->state == RQ_EVPSTATE_SCHEDULED) {
			dst[n].time = evp->t_scheduled;
			dst[n].flags = EF_PENDING;
		}
		else if (evp->state == RQ_EVPSTATE_INFLIGHT) {
			dst[n].time = evp->t_inflight;
			dst[n].flags = EF_INFLIGHT;
		}
		else if (evp->state == RQ_EVPSTATE_HELD) {
			/* same as scheduled */
			dst[n].time = evp->t_scheduled;
			dst[n].flags = EF_PENDING;
			dst[n].flags |= EF_HOLD;
		}
		if (evp->flags & RQ_ENVELOPE_SUSPEND)
			dst[n].flags |= EF_SUSPEND;

		n++;
	}

	return (n);
}

static int
scheduler_ram_schedule(uint64_t evpid)
{
	struct rq_message	*msg;
	struct rq_envelope	*evp;
	uint32_t		 msgid;
	void			*i;
	int			 r;

	currtime = time(NULL);

	if (evpid > 0xffffffff) {
		msgid = evpid_to_msgid(evpid);
		if ((msg = tree_get(&ramqueue.messages, msgid)) == NULL)
			return (0);
		if ((evp = tree_get(&msg->envelopes, evpid)) == NULL)
			return (0);
		if (evp->state == RQ_EVPSTATE_INFLIGHT)
			return (0);
		rq_envelope_schedule(&ramqueue, evp);
		return (1);
	}
	else {
		msgid = evpid;
		if ((msg = tree_get(&ramqueue.messages, msgid)) == NULL)
			return (0);
		i = NULL;
		r = 0;
		while (tree_iter(&msg->envelopes, &i, NULL, (void*)(&evp))) {
			if (evp->state == RQ_EVPSTATE_INFLIGHT)
				continue;
			rq_envelope_schedule(&ramqueue, evp);
			r++;
		}
		return (r);
	}
}

static int
scheduler_ram_remove(uint64_t evpid)
{
	struct rq_message	*msg;
	struct rq_envelope	*evp;
	uint32_t		 msgid;
	void			*i;
	int			 r;

	currtime = time(NULL);

	if (evpid > 0xffffffff) {
		msgid = evpid_to_msgid(evpid);
		if ((msg = tree_get(&ramqueue.messages, msgid)) == NULL)
			return (0);
		if ((evp = tree_get(&msg->envelopes, evpid)) == NULL)
			return (0);
		if (rq_envelope_remove(&ramqueue, evp))
			return (1);
		return (0);
	}
	else {
		msgid = evpid;
		if ((msg = tree_get(&ramqueue.messages, msgid)) == NULL)
			return (0);
		i = NULL;
		r = 0;
		while (tree_iter(&msg->envelopes, &i, NULL, (void*)(&evp)))
			if (rq_envelope_remove(&ramqueue, evp))
				r++;
		return (r);
	}
}

static int
scheduler_ram_suspend(uint64_t evpid)
{
	struct rq_message	*msg;
	struct rq_envelope	*evp;
	uint32_t		 msgid;
	void			*i;
	int			 r;

	currtime = time(NULL);

	if (evpid > 0xffffffff) {
		msgid = evpid_to_msgid(evpid);
		if ((msg = tree_get(&ramqueue.messages, msgid)) == NULL)
			return (0);
		if ((evp = tree_get(&msg->envelopes, evpid)) == NULL)
			return (0);
		if (rq_envelope_suspend(&ramqueue, evp))
			return (1);
		return (0);
	}
	else {
		msgid = evpid;
		if ((msg = tree_get(&ramqueue.messages, msgid)) == NULL)
			return (0);
		i = NULL;
		r = 0;
		while (tree_iter(&msg->envelopes, &i, NULL, (void*)(&evp)))
			if (rq_envelope_suspend(&ramqueue, evp))
				r++;
		return (r);
	}
}

static int
scheduler_ram_resume(uint64_t evpid)
{
	struct rq_message	*msg;
	struct rq_envelope	*evp;
	uint32_t		 msgid;
	void			*i;
	int			 r;

	currtime = time(NULL);

	if (evpid > 0xffffffff) {
		msgid = evpid_to_msgid(evpid);
		if ((msg = tree_get(&ramqueue.messages, msgid)) == NULL)
			return (0);
		if ((evp = tree_get(&msg->envelopes, evpid)) == NULL)
			return (0);
		if (rq_envelope_resume(&ramqueue, evp))
			return (1);
		return (0);
	}
	else {
		msgid = evpid;
		if ((msg = tree_get(&ramqueue.messages, msgid)) == NULL)
			return (0);
		i = NULL;
		r = 0;
		while (tree_iter(&msg->envelopes, &i, NULL, (void*)(&evp)))
			if (rq_envelope_resume(&ramqueue, evp))
				r++;
		return (r);
	}
}

static void
sorted_insert(struct rq_queue *rq, struct rq_envelope *evp)
{
	struct rq_envelope	*evp2;

	SPLAY_INSERT(prioqtree, &rq->q_priotree, evp);
	evp2 = SPLAY_NEXT(prioqtree, &rq->q_priotree, evp);
	if (evp2)
		TAILQ_INSERT_BEFORE(evp2, evp, entry);
	else
		TAILQ_INSERT_TAIL(&rq->q_pending, evp, entry);
}

static void
rq_queue_init(struct rq_queue *rq)
{
	memset(rq, 0, sizeof *rq);
	tree_init(&rq->messages);
	TAILQ_INIT(&rq->q_pending);
	TAILQ_INIT(&rq->q_inflight);
	TAILQ_INIT(&rq->q_mta);
	TAILQ_INIT(&rq->q_mda);
	TAILQ_INIT(&rq->q_bounce);
	TAILQ_INIT(&rq->q_update);
	TAILQ_INIT(&rq->q_expired);
	TAILQ_INIT(&rq->q_removed);
	SPLAY_INIT(&rq->q_priotree);
}

static void
rq_queue_merge(struct rq_queue *rq, struct rq_queue *update)
{
	struct rq_message	*message, *tomessage;
	struct rq_envelope	*envelope;
	uint64_t		 id;
	void			*i;

	while (tree_poproot(&update->messages, &id, (void*)&message)) {
		if ((tomessage = tree_get(&rq->messages, id)) == NULL) {
			/* message does not exist. re-use structure */
			tree_xset(&rq->messages, id, message);
			continue;
		}
		/* need to re-link all envelopes before merging them */
		i = NULL;
		while ((tree_iter(&message->envelopes, &i, &id,
		    (void*)&envelope)))
			envelope->message = tomessage;
		tree_merge(&tomessage->envelopes, &message->envelopes);
		free(message);
		stat_decrement("scheduler.ramqueue.message", 1);
	}

	/* Sorted insert in the pending queue */
	while ((envelope = TAILQ_FIRST(&update->q_pending))) {
		TAILQ_REMOVE(&update->q_pending, envelope, entry);
		sorted_insert(rq, envelope);
	}

	rq->evpcount += update->evpcount;
}

static void
rq_queue_schedule(struct rq_queue *rq)
{
	struct rq_envelope	*evp;

	while ((evp = TAILQ_FIRST(&rq->q_pending))) {
		if (evp->sched > currtime && evp->expire > currtime)
			break;

		if (evp->state != RQ_EVPSTATE_PENDING)
			errx(1, "evp:%016" PRIx64 " flags=0x%x", evp->evpid,
			    evp->flags);

		if (evp->expire <= currtime) {
			TAILQ_REMOVE(&rq->q_pending, evp, entry);
			SPLAY_REMOVE(prioqtree, &rq->q_priotree, evp);
			TAILQ_INSERT_TAIL(&rq->q_expired, evp, entry);
			evp->state = RQ_EVPSTATE_SCHEDULED;
			evp->flags |= RQ_ENVELOPE_EXPIRED;
			evp->t_scheduled = currtime;
			continue;
		}
		rq_envelope_schedule(rq, evp);
	}
}

static struct evplist *
rq_envelope_list(struct rq_queue *rq, struct rq_envelope *evp)
{
	switch (evp->state) {
	case RQ_EVPSTATE_PENDING:
		return &rq->q_pending;

	case RQ_EVPSTATE_SCHEDULED:
		if (evp->flags & RQ_ENVELOPE_EXPIRED)
			return &rq->q_expired;
		if (evp->flags & RQ_ENVELOPE_REMOVED)
			return &rq->q_removed;
		if (evp->flags & RQ_ENVELOPE_UPDATE)
			return &rq->q_update;
		if (evp->type == D_MTA)
			return &rq->q_mta;
		if (evp->type == D_MDA)
			return &rq->q_mda;
		if (evp->type == D_BOUNCE)
			return &rq->q_bounce;
		errx(1, "%016" PRIx64 " bad evp type %d", evp->evpid, evp->type);

	case RQ_EVPSTATE_INFLIGHT:
		return &rq->q_inflight;

	case RQ_EVPSTATE_HELD:
		return (NULL);
	}

	errx(1, "%016" PRIx64 " bad state %d", evp->evpid, evp->state);
	return (NULL);
}

static void
rq_envelope_schedule(struct rq_queue *rq, struct rq_envelope *evp)
{
	struct rq_holdq	*hq;
	struct evplist	*q = NULL;

	switch (evp->type) {
	case D_MTA:
		q = &rq->q_mta;
		break;
	case D_MDA:
		q = &rq->q_mda;
		break;
	case D_BOUNCE:
		q = &rq->q_bounce;
		break;
	}

	if (evp->flags & RQ_ENVELOPE_UPDATE)
		q = &rq->q_update;

	if (evp->state == RQ_EVPSTATE_HELD) {
		hq = tree_xget(&holdqs[evp->type], evp->holdq);
		TAILQ_REMOVE(&hq->q, evp, entry);
		if (TAILQ_EMPTY(&hq->q)) {
			tree_xpop(&holdqs[evp->type], evp->holdq);
			free(hq);
		}
		evp->holdq = 0;
		stat_decrement("scheduler.ramqueue.hold", 1);
	}
	else if (!(evp->flags & RQ_ENVELOPE_SUSPEND)) {
		TAILQ_REMOVE(&rq->q_pending, evp, entry);
		SPLAY_REMOVE(prioqtree, &rq->q_priotree, evp);
	}

	TAILQ_INSERT_TAIL(q, evp, entry);
	evp->state = RQ_EVPSTATE_SCHEDULED;
	evp->t_scheduled = currtime;
}

static int
rq_envelope_remove(struct rq_queue *rq, struct rq_envelope *evp)
{
	struct rq_holdq	*hq;
	struct evplist	*evl;

	if (evp->flags & (RQ_ENVELOPE_REMOVED | RQ_ENVELOPE_EXPIRED))
		return (0);
	/*
	 * If envelope is inflight, mark it envelope for removal.
	 */
	if (evp->state == RQ_EVPSTATE_INFLIGHT) {
		evp->flags |= RQ_ENVELOPE_REMOVED;
		return (1);
	}

	if (evp->state == RQ_EVPSTATE_HELD) {
		hq = tree_xget(&holdqs[evp->type], evp->holdq);
		TAILQ_REMOVE(&hq->q, evp, entry);
		if (TAILQ_EMPTY(&hq->q)) {
			tree_xpop(&holdqs[evp->type], evp->holdq);
			free(hq);
		}
		evp->holdq = 0;
		stat_decrement("scheduler.ramqueue.hold", 1);
	}
	else if (!(evp->flags & RQ_ENVELOPE_SUSPEND)) {
		evl = rq_envelope_list(rq, evp);
		TAILQ_REMOVE(evl, evp, entry);
		if (evl == &rq->q_pending)
			SPLAY_REMOVE(prioqtree, &rq->q_priotree, evp);
	}

	TAILQ_INSERT_TAIL(&rq->q_removed, evp, entry);
	evp->state = RQ_EVPSTATE_SCHEDULED;
	evp->flags |= RQ_ENVELOPE_REMOVED;
	evp->t_scheduled = currtime;

	return (1);
}

static int
rq_envelope_suspend(struct rq_queue *rq, struct rq_envelope *evp)
{
	struct rq_holdq	*hq;
	struct evplist	*evl;

	if (evp->flags & RQ_ENVELOPE_SUSPEND)
		return (0);

	if (evp->state == RQ_EVPSTATE_HELD) {
		hq = tree_xget(&holdqs[evp->type], evp->holdq);
		TAILQ_REMOVE(&hq->q, evp, entry);
		if (TAILQ_EMPTY(&hq->q)) {
			tree_xpop(&holdqs[evp->type], evp->holdq);
			free(hq);
		}
		evp->holdq = 0;
		evp->state = RQ_EVPSTATE_PENDING;
		stat_decrement("scheduler.ramqueue.hold", 1);
	}
	else if (evp->state != RQ_EVPSTATE_INFLIGHT) {
		evl = rq_envelope_list(rq, evp);
		TAILQ_REMOVE(evl, evp, entry);
		if (evl == &rq->q_pending)
			SPLAY_REMOVE(prioqtree, &rq->q_priotree, evp);
	}

	evp->flags |= RQ_ENVELOPE_SUSPEND;

	return (1);
}

static int
rq_envelope_resume(struct rq_queue *rq, struct rq_envelope *evp)
{
	struct evplist	*evl;

	if (!(evp->flags & RQ_ENVELOPE_SUSPEND))
		return (0);

	if (evp->state != RQ_EVPSTATE_INFLIGHT) {
		evl = rq_envelope_list(rq, evp);
		if (evl == &rq->q_pending)
			sorted_insert(rq, evp);
		else
			TAILQ_INSERT_TAIL(evl, evp, entry);
	}

	evp->flags &= ~RQ_ENVELOPE_SUSPEND;

	return (1);
}

static void
rq_envelope_delete(struct rq_queue *rq, struct rq_envelope *evp)
{
	tree_xpop(&evp->message->envelopes, evp->evpid);
	if (tree_empty(&evp->message->envelopes)) {
		tree_xpop(&rq->messages, evp->message->msgid);
		free(evp->message);
		stat_decrement("scheduler.ramqueue.message", 1);
	}

	free(evp);
	rq->evpcount--;
	stat_decrement("scheduler.ramqueue.envelope", 1);
}

static const char *
rq_envelope_to_text(struct rq_envelope *e)
{
	static char	buf[256];
	char		t[64];

	snprintf(buf, sizeof buf, "evp:%016" PRIx64 " [", e->evpid);

	if (e->type == D_BOUNCE)
		strlcat(buf, "bounce", sizeof buf);
	else if (e->type == D_MDA)
		strlcat(buf, "mda", sizeof buf);
	else if (e->type == D_MTA)
		strlcat(buf, "mta", sizeof buf);

	snprintf(t, sizeof t, ",expire=%s",
	    duration_to_text(e->expire - currtime));
	strlcat(buf, t, sizeof buf);


	switch (e->state) {
	case RQ_EVPSTATE_PENDING:
		snprintf(t, sizeof t, ",pending=%s",
		    duration_to_text(e->sched - currtime));
		strlcat(buf, t, sizeof buf);
		break;

	case RQ_EVPSTATE_SCHEDULED:
		snprintf(t, sizeof t, ",scheduled=%s",
		    duration_to_text(currtime - e->t_scheduled));
		strlcat(buf, t, sizeof buf);
		break;

	case RQ_EVPSTATE_INFLIGHT:
		snprintf(t, sizeof t, ",inflight=%s",
		    duration_to_text(currtime - e->t_inflight));
		strlcat(buf, t, sizeof buf);
		break;

	case RQ_EVPSTATE_HELD:
		snprintf(t, sizeof t, ",held=%s",
		    duration_to_text(currtime - e->t_inflight));
		strlcat(buf, t, sizeof buf);
		break;
	default:
		errx(1, "%016" PRIx64 " bad state %d", e->evpid, e->state);
	}

	if (e->flags & RQ_ENVELOPE_REMOVED)
		strlcat(buf, ",removed", sizeof buf);
	if (e->flags & RQ_ENVELOPE_EXPIRED)
		strlcat(buf, ",expired", sizeof buf);
	if (e->flags & RQ_ENVELOPE_SUSPEND)
		strlcat(buf, ",suspended", sizeof buf);

	strlcat(buf, "]", sizeof buf);

	return (buf);
}

static void
rq_queue_dump(struct rq_queue *rq, const char * name)
{
	struct rq_message	*message;
	struct rq_envelope	*envelope;
	void			*i, *j;
	uint64_t		 id;

	log_debug("debug: /--- ramqueue: %s", name);

	i = NULL;
	while ((tree_iter(&rq->messages, &i, &id, (void*)&message))) {
		log_debug("debug: | msg:%08" PRIx32, message->msgid);
		j = NULL;
		while ((tree_iter(&message->envelopes, &j, &id,
		    (void*)&envelope)))
			log_debug("debug: |   %s",
			    rq_envelope_to_text(envelope));
	}
	log_debug("debug: \\---");
}

static int
rq_envelope_cmp(struct rq_envelope *e1, struct rq_envelope *e2)
{
	time_t	ref1, ref2;

	ref1 = (e1->sched < e1->expire) ? e1->sched : e1->expire;
	ref2 = (e2->sched < e2->expire) ? e2->sched : e2->expire;
	if (ref1 != ref2)
		return (ref1 < ref2) ? -1 : 1;

	if (e1->evpid != e2->evpid)
		return (e1->evpid < e2->evpid) ? -1 : 1;

	return 0;
}

SPLAY_GENERATE(prioqtree, rq_envelope, t_entry, rq_envelope_cmp);
