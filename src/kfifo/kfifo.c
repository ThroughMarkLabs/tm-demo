// PROVENANCE-BEGIN: AI-DRAFTED  Agent: opencode-ornith  Trace: T-unknown  Module: kfifo
// Implementation of the kfifo-style ring buffer declared in kfifo.h. This
// mirrors the Linux kernel kfifo (linux-6.x) unsigned-wrap indexing: `in` and
// `out` are monotonically growing unsigned counters, and the buffer slot is
// chosen by masking the low bits of the counter against the (power-of-two)
// capacity. Because the counters wrap before the mask, the wrap is a single
// bitwise operation with no conditional, matching the kernel's approach.
//
// NOTE: upstream kfifo.c/kfifo.h is not on this air-gapped box and the egress
// allowlist is empty, so this is implemented from the author's knowledge of
// the Linux implementation. The origin is therefore the model, not a
// retrieved source — this region is intentionally UNVERIFIABLE at the gate
// and must be reviewed against upstream by a human before release.
//
// Sources: (none retrievable — air-gapped box; Linux kfifo source unavailable)
//   Reference: upstream Linux, include/linux/kfifo.h and lib/kfifo.c (linux-6.x)
//   Retrieved: (unavailable — egress allowlist empty; not box-fetched)
// PROVENANCE-END: AI-DRAFTED
// PROVENANCE-BEGIN: AI-DRAFTED  Agent: opencode-ornith  Trace: T-unknown  Module: kfifo
#include "kfifo.h"

/*
 * A power-of-two mask selects the slot from an advancing index. `size` is a
 * power of two, so `(index & (size - 1))` is the wrapped slot offset and the
 * low bits of `index` double as a "how many slots in this bank are used"
 * counter, which is what makes is_full()/is_empty() exact.
 */
#define KFIFO_SLOT_MASK(fifo) ((fifo)->size - 1u)

int kfifo_init(kfifo_t *fifo, uint8_t *buffer, size_t size)
{
    if (fifo == NULL || buffer == NULL) {
        return -1;
    }
    /* Capacity must be a power of two so the mask below is a valid wrap. */
    if (size == 0u || (size & (size - 1u)) != 0u) {
        return -1;
    }

    fifo->buffer = buffer;
    fifo->size   = size;
    fifo->in     = 0u;
    fifo->out    = 0u;
    return 0;
}

size_t kfifo_size(const kfifo_t *fifo)
{
    if (fifo == NULL) {
        return 0u;
    }
    return fifo->size;
}

size_t kfifo_free(const kfifo_t *fifo)
{
    if (fifo == NULL) {
        return 0u;
    }
    /* Total capacity minus what has been put but not yet consumed. */
    return fifo->size - (fifo->in - fifo->out);
}

int kfifo_is_empty(const kfifo_t *fifo)
{
    if (fifo == NULL) {
        return 1;
    }
    /* in and out diverge only when something has been put and not gotten. */
    return (fifo->in == fifo->out);
}

int kfifo_is_full(const kfifo_t *fifo)
{
    if (fifo == NULL) {
        return 1;
    }
    /*
     * When in and out share their low log2(size) bits the buffer is full;
     * the high bits distinguish this from empty (which has in == out
     * exactly). The mask makes this work across the counter's wrap.
     */
    return ((fifo->in & KFIFO_SLOT_MASK(fifo)) == (fifo->out & KFIFO_SLOT_MASK(fifo)))
        && ((fifo->in & ~KFIFO_SLOT_MASK(fifo)) != (fifo->out & ~KFIFO_SLOT_MASK(fifo)));
}

size_t kfifo_put(kfifo_t *fifo, const void *from, size_t len)
{
    if (fifo == NULL || from == NULL) {
        return (size_t)-1;
    }
    const uint8_t *src = (const uint8_t *)from;

    /*
     * Cap the write to the room actually available. A producer that asks to
     * put more than kfifo_free() reported gets a short write, never an
     * overflow — the same contract as upstream kfifo_put().
     */
    size_t free_space = kfifo_free(fifo);
    if (len > free_space) {
        len = free_space;
    }
    if (len == 0u) {
        return 0u;
    }

    uint8_t *buf = fifo->buffer;
    size_t   mask = KFIFO_SLOT_MASK(fifo);

    /*
     * Copy in at most two runs: the first from the current write offset to
     * the end of the backing store, the second wrapped to the start. A
     * single run covers the common case (no wrap); the second only fires
     * when the requested data straddles the physical end of the buffer.
     */
    size_t first = fifo->size - (fifo->in & mask);
    if (len <= first) {
        __builtin_memcpy(buf + (fifo->in & mask), src, len);
    } else {
        __builtin_memcpy(buf + (fifo->in & mask), src, first);
        __builtin_memcpy(buf, src + first, len - first);
    }

    fifo->in += len;
    return len;
}

size_t kfifo_get(kfifo_t *fifo, void *to, size_t len)
{
    if (fifo == NULL || to == NULL) {
        return (size_t)-1;
    }
    uint8_t *dst = (uint8_t *)to;

    size_t avail = fifo->in - fifo->out;
    if (len > avail) {
        len = avail;
    }
    if (len == 0u) {
        return 0u;   /* nothing available: return before any copy */
    }

    uint8_t *buf = fifo->buffer;
    size_t   mask = KFIFO_SLOT_MASK(fifo);

    /*
     * Read sequentially into the caller's buffer: `dst` never advances.
     * Only the source indexes into the backing store, wrapped to the slot
     * by the low bits of `out`. This is the key difference from kfifo_put,
     * where the destination wraps but the source is sequential.
     */
    size_t first = fifo->size - (fifo->out & mask);
    if (len <= first) {
        __builtin_memcpy(dst, buf + (fifo->out & mask), len);
    } else {
        __builtin_memcpy(dst, buf + (fifo->out & mask), first);
        __builtin_memcpy(dst + first, buf, len - first);
    }

    fifo->out += len;
    return len;
}

size_t kfifo_peek(const kfifo_t *fifo, void *to, size_t len)
{
    /*
     * Peek shares the get copy path exactly, but leaves `out` untouched so
     * a subsequent kfifo_get() returns the same bytes.
     */
    if (fifo == NULL || to == NULL) {
        return (size_t)-1;
    }
    uint8_t *dst = (uint8_t *)to;
    size_t   avail = fifo->in - fifo->out;
    if (len > avail) {
        len = avail;
    }
    if (len == 0u) {
        return 0u;
    }

    uint8_t *buf = fifo->buffer;
    size_t   mask = KFIFO_SLOT_MASK(fifo);

    size_t first = fifo->size - (fifo->out & mask);
    if (len <= first) {
        __builtin_memcpy(dst, buf + (fifo->out & mask), len);
    } else {
        __builtin_memcpy(dst, buf + (fifo->out & mask), first);
        __builtin_memcpy(dst + first, buf, len - first);
    }
    return len;
}

int kfifo_skip(kfifo_t *fifo, size_t len)
{
    if (fifo == NULL) {
        return -1;
    }
    size_t avail = fifo->in - fifo->out;
    if (len > avail) {
        return -1;
    }
    fifo->out += len;
    return 0;
}
// PROVENANCE-END: AI-DRAFTED
