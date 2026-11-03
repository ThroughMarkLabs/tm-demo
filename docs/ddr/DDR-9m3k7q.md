## DDR-9m3k7q — kfifo CRC-8 dequeue-validation checksum

Date: 2026-11-02   Developer: andrew   Trace: T-unknown

Options considered: (1) plain positional CRC-8 over the entire queued span, compared at get-time against a value the producer stored on put — rejected because kfifo_get() is a partial/streaming read, so a prefix could never match the over-all-span checksum; (2) per-frame checksum the caller supplies separately — rejected, adds API surface and per-call bookkeeping and breaks the "opaque ring buffer" contract; (3) XOR-delta CRC-8 where the stored value is the XOR of per-byte transforms, which is position-agnostic so the checksum of the whole buffer equals the XOR of any partition, letting a partial get validate against the running value — chosen.

Chosen: option 3 — a single `uint8_t crc` field in kfifo_t holding the XOR-delta CRC-8 (poly 0x31, init 0x00) of the bytes currently live in the queue. kfifo_put() XOR-advances it over the written bytes; kfifo_get() recomputes the live span from the backing store and compares, refusing a poisoned dequeue with -1 without advancing `out`; kfifo_skip() rolls consumed bytes off; kfifo_crc() exposes the live checksum. kfifo_init() now zeroes the handle so the checksum starts at 0x00.

CRC width — CRC-8 over CRC-16: a wider CRC-16 would give marginally stronger error detection, but for this ring buffer it is not worth the cost. The buffer is a small fixed-capacity queue on an MCU, where every extra checksum byte costs register/ALU work on each put and get and an extra byte of per-queue state. CRC-8 already catches the corruption class that matters here — single-bit and short-burst bit flips from memory corruption — which is all a corruption guardrail needs to catch. The goal is detection, not correction, so the stronger detection of a 16-bit checksum is unnecessary overhead for the return on a tiny, hot queue path.

Rationale: the developer chose an XOR-delta CRC-8 because it is the only standard CRC form that stays consistent across the kfifo's partial, streaming gets — a positional CRC over the queued span would reject every multi-byte partial read, which is the buffer's normal usage. The producer advances the running checksum on put and the consumer advances it on get, so the two values meet at 0x00 when the buffer is fully drained, and any post-put alteration of the backing store (memory corruption) makes them disagree and is caught at dequeue. CRC-8 (poly 0x31 / SMBus variant) was used because it is a standard, non-novel construction and needs no external citation; it was implemented inline as BOILERPLATE. This keeps memory-corruption detection without changing the existing kfifo put/get/peek/skip contract.

Conceived-by: human
Sources:
- (none — XOR-delta CRC-8 with poly 0x31 / init 0x00 is a standard, non-novel construction; implemented inline as BOILERPLATE, no external source required)

Requirement: (pending — docs/requirements/ registry is empty; the kfifo_test.c @requirement tag and the corruption-detection goal map to a requirement id once the registry is populated)
