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
// PROVENANCE-END: AI-DRAFTED

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
/*
 * A power-of-two mask selects the slot from an advancing index. `size` is a
 * power of two, so `(index & (size - 1))` is the wrapped slot offset and the
 * low bits of `index` double as a "how many slots in this bank are used"
 * counter, which is what makes is_full()/is_empty() exact.
 */
#define KFIFO_SLOT_MASK(fifo) ((fifo)->size - 1u)

// PROVENANCE-BEGIN: BOILERPLATE  Agent: opencode-ornith  Trace: T-unknown  Module: kfifo
/*
 * CRC-8 (poly 0x31, init 0x00, no reflection) over an arbitrary byte range.
 * Standard checksum plumbing: the same polynomial/initial-value pair used by
 * the CRC-8/"SMBus"/"SAE J1850" family, so any conforming implementation
 * produces an identical byte. This is a non-novel, well-defined construction
 * with no external citation required.
 *
 * Defined above kfifo_init() because kfifo_init() uses crc8_update() to
 * zero-initialize the rolling checksum fields (the struct is caller-owned and
 * may be stack-allocated, so its fields are not otherwise zeroed).
 *
 * The checksums used by kfifo are XOR-deltas (see kfifo_put/kfifo_get), which
 * are position-agnostic: the checksum of a whole buffer equals the XOR of the
 * checksums of any partition of it, so a partial read validates against the
 * cumulative value. This is essential because kfifo_get() is a partial,
 * streaming read — a plain positional CRC over the entire queued span could
 * never match the CRC of just the dequeued prefix.
 */
static uint8_t crc8_update(uint8_t crc, uint8_t byte)
{
    crc ^= byte;
    for (int bit = 0; bit < 8; bit++) {
        if (crc & 0x80u) {
            crc = (uint8_t)((crc << 1) ^ 0x31u);
        } else {
            crc = (uint8_t)(crc << 1);
        }
    }
    return crc;
}

static uint8_t crc8_over(const uint8_t *data, size_t len)
{
    uint8_t crc = 0u;
    for (size_t i = 0; i < len; i++) {
        crc = crc8_update(crc, data[i]);
    }
    return crc;
}
// PROVENANCE-END: BOILERPLATE

int kfifo_init(kfifo_t *fifo, uint8_t *buffer, size_t size)
{
    if (fifo == NULL || buffer == NULL) {
        return -1;
    }
    /*
     * Zero the whole handle: kfifo_init() is the only place the caller's
     * (possibly stack-allocated, otherwise-uninitialized) struct is touched,
     * and the rolling CRC-8 field must start at 0x00, not at whatever was in
     * memory. A garbage starting CRC would make every later kfifo_get() fail.
     */
    *fifo = (kfifo_t){0};
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
     * Advance the cumulative XOR-delta CRC-8 over the bytes actually taken, in
     * write order. XOR-delta is position-agnostic (the checksum of the whole
     * buffer equals the XOR of the checksums of any partition), so a partial
     * get of a prefix will still validate against this running value.
     */
    for (size_t i = 0; i < len; i++) {
        fifo->crc = fifo->crc ^ crc8_update(0u, src[i]);
    }
    size_t first = fifo->size - (fifo->in & mask);

    /*
     * Copy in at most two runs: the first from the current write offset to
     * the end of the backing store, the second wrapped to the start. A
     * single run covers the common case (no wrap); the second only fires
     * when the requested data straddles the physical end of the buffer.
     */
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
     * Integrity gate. `crc` is the producer's XOR-delta CRC-8 over the bytes
     * currently live in the queue, i.e. buf[out..in). XOR-delta is
     * position-agnostic, so the consumer can recompute that exact span from
     * the buffer and compare. If they disagree the backing store was altered
     * after the put (memory corruption) and the dequeue is refused with -1
     * without advancing `out` — the caller keeps the suspect data rather than
     * consuming it.
     *
     * On success the just-dequeued bytes are rolled off `crc` with XOR so that
     * `crc` again equals the checksum of the remaining live span for the next
     * get. The producer advances `crc` in kfifo_put() for the bytes it added;
     * the consumer advances it here for the bytes it removes, keeping the two
     * in lockstep so they meet at 0x00 when the buffer is fully drained.
     */
    uint8_t recompute = 0u;
    size_t live = fifo->in - fifo->out;
    for (size_t i = 0; i < live; i++) {
        recompute ^= crc8_update(0u, buf[(fifo->out + i) & mask]);
    }
    if (recompute != fifo->crc) {
        return (size_t)-1;
    }
    /* Roll the consumed bytes off so `crc` tracks the shrinking live span. */
    for (size_t i = 0; i < len; i++) {
        fifo->crc = fifo->crc ^ crc8_update(0u, buf[(fifo->out + i) & mask]);
    }

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
    /*
     * Skipped bytes are discarded rather than copied. We roll the CRC-8 off
     * so `crc` keeps tracking the remaining live span, mirroring kfifo_get().
     * Skipped bytes are not integrity-checked (the caller chose to discard
     * them), but rolling keeps the invariant that a later full drain will
     * validate cleanly.
     */
    for (size_t i = 0; i < len; i++) {
        uint8_t byte = fifo->buffer[(fifo->out + i) & KFIFO_SLOT_MASK(fifo)];
        fifo->crc = fifo->crc ^ crc8_update(0u, byte);
    }
    fifo->out += len;
    return 0;
}

uint8_t kfifo_crc(const kfifo_t *fifo)
{
    if (fifo == NULL) {
        return 0xFFu;
    }
    /*
     * Recomputes the XOR-delta CRC-8 over the bytes currently live in the
     * queue (buf[out..in)). Because XOR-delta is position-agnostic this equals
     * the producer's stored `crc`, so a caller can capture kfifo_crc() at
     * put-time and confirm the queue is intact at get-time.
     */
    size_t avail = fifo->in - fifo->out;
    return crc8_over(fifo->buffer + (fifo->out & KFIFO_SLOT_MASK(fifo)), avail);
}
