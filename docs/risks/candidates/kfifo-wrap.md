## HAZC-1 — Buffer-wrap misread once the in/out counters cross a power-of-two boundary

Description: kfifo_size() is a power of two, and is_empty()/is_full() distinguish empty from full by comparing the low log2(size) bits of the `in` and `out` counters while relying on their high bits to differ. If a caller constructs a kfifo whose `size` is not an exact power of two, or writes past the backing store, the mask no longer selects a valid slot and the buffer reads stale or out-of-range bytes — indistinguishable from a correctly filled buffer.

Evidence: src/kfifo/kfifo.h:30-42 — the kfifo struct documents capacity must be a power of two; kfifo_init() rejects non-power-of-two size but the caller passes the backing-store pointer, so a too-small buffer is not caught.
Evidence: src/kfifo/kfifo.c:82-84 — is_full() compares low bits of in and out and requires the high bits to differ; this is only correct if size is a power of two.
Evidence: src/kfifo/kfifo.c:116,152 — `first = fifo->size - (index & mask)` assumes size == 2^n so that `index & mask` is a valid slot offset.
Unknown: severity — depends on whether a too-small backing store or non-power-of-two size can occur in the DSPI queue path; not stated anywhere in the repo.
Unknown: probability — the DSPI driver currently uses its own pointer queue (DSPI_FIFO_DEPTH), not this kfifo; whether this kfifo is wired into a live producer/consumer path is not established.
Unknown: controllability — kfifo_init rejects non-power-of-two size, but does not validate that the provided backing store is at least `size` bytes; a too-small allocation is uncaught.
Drafted-by: vitrevant-hazard-draft
