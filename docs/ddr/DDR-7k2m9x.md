## DDR-7k2m9x — kfifo-style ring buffer for DSPI frame queueing

Date: 2026-11-01   Developer: andrew   Trace: T-qo7wqr

Options considered: (1) pointer-arithmetic circular queue (head/tail pointers into a fixed array, as already drafted in src/dspi/dspi.c); (2) unsigned-wrap counter ring buffer modeled on the Linux kernel kfifo (include/linux/kfifo.h, lib/kfifo.c, linux-6.x) with kfifo_put/kfifo_get/kfifo_peek/kfifo_skip; (3) linked-list queue.

Chosen: option 2 — a portable, power-of-two-capacity ring buffer (src/kfifo/kfifo.{c,h}) mirroring the Linux kfifo API and unsigned-wrap indexing.

Rationale: the developer chose the Linux kfifo model so the DSPI driver gets a reusable, well-understood producer/consumer buffer with a documented contract (power-of-two capacity, short-write semantics, non-destructive peek) rather than a one-off pointer queue. The unsigned-wrap design advances `in`/`out` as monotonically growing counters and selects the slot by masking the low bits, so the wrap is a branch-free bitwise op. Not a retrieved-source decision: the box is air-gapped, so upstream kfifo was not box-fetched; implementation is from knowledge of the Linux reference and is flagged UNVERIFIABLE at the gate for human review against upstream.

Conceived-by: human
Sources:
- (none retrievable — air-gapped box; Linux kfifo source not box-fetched)
