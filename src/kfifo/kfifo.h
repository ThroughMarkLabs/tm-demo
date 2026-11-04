// PROVENANCE-BEGIN: AI-DRAFTED  Agent: opencode-ornith  Trace: T-unknown  Module: kfifo
// Portable fixed-capacity ring buffer modeled on the Linux kernel kfifo.
// The API mirrors include/linux/kfifo.h (linux-6.x) as closely as is sane
// for a bare-metal firmware build: an opaque handle, power-of-two capacity,
// and the in/out copy primitives. The design is a standard, non-novel data
// structure; implementation details (unsigned-wrap indexing, __builtin_ctz
// advance) follow the kernel reference.
//
// NOTE: upstream kfifo.c/kfifo.h is not on this air-gapped box and the
// egress allowlist is empty, so this is implemented from the author's
// knowledge of the Linux implementation. The origin is therefore the model,
// not a retrieved source — this region is intentionally UNVERIFIABLE at the
// gate and must be reviewed against upstream by a human before release.
//
// Sources: (none retrievable — air-gapped box; Linux kfifo source unavailable)
//   Reference: upstream Linux, include/linux/kfifo.h and lib/kfifo.c (linux-6.x)
//   Retrieved: (unavailable — egress allowlist empty; not box-fetched)
// PROVENANCE-END: AI-DRAFTED
// PROVENANCE-BEGIN: AI-DRAFTED  Agent: opencode-ornith  Trace: T-unknown  Module: kfifo
#include <stddef.h>
#include <stdint.h>
// PROVENANCE-END: AI-DRAFTED

// PROVENANCE-BEGIN: AI-DRAFTED  Agent: opencode-ornith  Trace: T-unknown  Module: kfifo
#ifndef KFIFO_H
#define KFIFO_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * A fixed-capacity first-in/first-out ring buffer. Capacity must be a power
 * of two; the buffer wraps by masking the advance index rather than modulo,
 * exactly as the kernel kfifo does, so pop/push are branch-free on the wrap.
 *
 * Single-producer / single-consumer only, like upstream kfifo. Do not share
 * one kfifo between a producer and consumer without external locking.
 */
typedef struct kfifo {
    uint8_t  *buffer;  /* Backing storage, allocated by kfifo_init()      */
    size_t    size;    /* Capacity in bytes, a power of two               */
    size_t    in;      /* Total bytes written (unsigned; wraps on overflow) */
    size_t    out;     /* Total bytes read   (unsigned; wraps on overflow)  */
    uint8_t   crc;     /* XOR-delta CRC-8 of the bytes currently live in queue */
} kfifo_t;

/*
 * Attach a kfifo to `buffer` with `size` bytes of capacity. `size` must be a
 * power of two greater than zero. Returns 0 on success, -1 if the arguments
 * are invalid. `buffer` must remain valid for the lifetime of the kfifo.
 */
int kfifo_init(kfifo_t *fifo, uint8_t *buffer, size_t size);

/* Number of bytes currently available to read. */
size_t kfifo_size(const kfifo_t *fifo);

/* Number of bytes free for writing (total capacity minus used). */
size_t kfifo_free(const kfifo_t *fifo);

/* True when no bytes are available to read. */
int kfifo_is_empty(const kfifo_t *fifo);

/* True when no bytes can be written. */
int kfifo_is_full(const kfifo_t *fifo);

/*
 * Copy `len` bytes from `from` into the kfifo. Returns the number of bytes
 * actually written (<= len); a short write means the buffer had room for
 * fewer than `len` bytes. Returns -1 if `fifo` or `from` is NULL.
 */
size_t kfifo_put(kfifo_t *fifo, const void *from, size_t len);

/*
 * Copy up to `len` bytes out of the kfifo into `to`. Returns the number of
 * bytes actually read (<= len); a short read means fewer than `len` bytes
 * were available. Returns -1 if `fifo` or `to` is NULL.
 */
size_t kfifo_get(kfifo_t *fifo, void *to, size_t len);

/*
 * Copy `len` bytes out without advancing the read index (peek). The bytes
 * remain in the buffer and a subsequent kfifo_get() returns them again.
 * Returns bytes available, -1 on NULL, or 0 if empty.
 */
size_t kfifo_peek(const kfifo_t *fifo, void *to, size_t len);

/*
 * Advance the read index by `len` bytes, discarding them without copying.
 * Returns 0 on success, -1 if `len` exceeds the available bytes.
 *
 * The stored CRC-8 is advanced over the skipped bytes (they are discarded, not
 * validated) and the read index moves.
 */
int kfifo_skip(kfifo_t *fifo, size_t len);

/*
 * Compute the current CRC-8 over the bytes queued but not yet dequeued. The
 * value is the same one kfifo_get() compares the dequeue against, so a caller
 * can capture it at put-time and confirm the queue has not been corrupted at
 * get-time. Returns 0xFF if `fifo` is NULL.
 */
uint8_t kfifo_crc(const kfifo_t *fifo);

#ifdef __cplusplus
}
#endif

#endif /* KFIFO_H */
// PROVENANCE-END: AI-DRAFTED
