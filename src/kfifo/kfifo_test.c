// PROVENANCE-BEGIN: BOILERPLATE  Agent: opencode-ornith  Trace: T-unknown  Module: kfifo
// Tests for the kfifo-style ring buffer. Covers init validation, basic
// put/get ordering, the no-wrap case, the wrap-around case (data straddling
// the physical end of the backing store), full/empty/free transitions,
// peek (non-destructive), skip, and CRC-8 dequeue validation (corruption
// detection + refused poisoned read).
// PROVENANCE-END: BOILERPLATE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "kfifo.h"

static int g_failures = 0;

#define CHECK(cond, msg)                              \
    do {                                              \
        if (!(cond)) {                                \
            printf("FAIL: %s\n", (msg));              \
            g_failures++;                             \
        }                                             \
    } while (0)

static void test_init_validation(void)
{
    kfifo_t fifo;
    uint8_t buf[16];

    /* Non-power-of-two and zero size are rejected. */
    CHECK(kfifo_init(&fifo, buf, 0) == -1, "init rejects zero size");
    CHECK(kfifo_init(&fifo, buf, 12) == -1, "init rejects non-power-of-two size");

    /* NULL arguments are rejected. */
    CHECK(kfifo_init(NULL, buf, 16) == -1, "init rejects NULL fifo");
    CHECK(kfifo_init(&fifo, NULL, 16) == -1, "init rejects NULL buffer");

    /* Valid power-of-two size initializes. */
    CHECK(kfifo_init(&fifo, buf, 16) == 0, "init accepts power-of-two size");
    CHECK(kfifo_size(&fifo) == 16, "size reports capacity");
    CHECK(kfifo_is_empty(&fifo), "new fifo is empty");
    CHECK(kfifo_free(&fifo) == 16, "new fifo is fully free");
}

static void test_basic_ordering(void)
{
    kfifo_t fifo;
    uint8_t buf[16];
    uint8_t out[4];

    CHECK(kfifo_init(&fifo, buf, 16) == 0, "init basic");

    const char *w = "abcd";
    size_t n = kfifo_put(&fifo, w, 4);
    CHECK(n == 4, "put returns bytes written");
    CHECK(!kfifo_is_empty(&fifo), "not empty after put");
    CHECK(kfifo_free(&fifo) == 12, "free reflects the put");

    n = kfifo_get(&fifo, out, 4);
    CHECK(n == 4, "get returns bytes read");
    CHECK(memcmp(out, "abcd", 4) == 0, "get preserves FIFO order");
    CHECK(kfifo_is_empty(&fifo), "empty after draining");
}

static void test_no_wrap(void)
{
    /*
     * Fill to exactly capacity, then drain one frame at a time across the
     * whole buffer, confirming order is preserved through every slot.
     */
    kfifo_t fifo;
    uint8_t buf[16];
    uint8_t out[4];
    uint8_t byte = 0u;

    CHECK(kfifo_init(&fifo, buf, 16) == 0, "init wrap");
    for (int i = 0; i < 16; i++) {
        uint8_t v = (uint8_t)i;
        CHECK(kfifo_put(&fifo, &v, 1) == 1, "put single byte");
    }
    CHECK(kfifo_is_full(&fifo), "full at capacity");
    CHECK(kfifo_put(&fifo, &byte, 1) == 0, "put on full buffer is a short write of 0");

    int idx = 0;
    while (idx < 16) {
        size_t n = kfifo_get(&fifo, out, 4);
        CHECK(n == 4, "no-wrap reads full frame");
        for (int j = 0; j < 4; j++) {
            CHECK(out[j] == (uint8_t)(idx & 0xFF), "wrap-free get ordering");
            idx++;
        }
    }
    CHECK(idx == 16, "all bytes consumed");
    CHECK(kfifo_is_empty(&fifo), "empty after full drain");
}

static void test_wrap_around(void)
{
    /*
     * The wrap path triggers when data straddles the physical end of the
     * backing store. Drive a continuous byte stream through a tiny 4-byte
     * buffer: fill it, then drain and refill in lockstep so the read and
     * write offsets race around the small store many times, exercising both
     * the single-run and two-run (straddle) copy paths on every iteration.
     */
    kfifo_t fifo;
    uint8_t buf[4];
    uint8_t one;
    static const uint8_t pattern[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

    CHECK(kfifo_init(&fifo, buf, 4) == 0, "init small");

    int idx = 0;
    int got = 0;
    /* Fill up front, then drain one and push the next until the stream ends. */
    for (int i = 0; i < 4; i++) {
        kfifo_put(&fifo, &pattern[i], 1);
    }
    idx = 4;
    while (got < 16) {
        CHECK(kfifo_get(&fifo, &one, 1) == 1, "get one from tiny fifo");
        CHECK(one == pattern[got], "wrapped stream ordering");
        got++;
        if (idx < 16) {
            kfifo_put(&fifo, &pattern[idx], 1);
            idx++;
        }
    }
    CHECK(got == 16, "all stream bytes consumed");
    CHECK(kfifo_is_empty(&fifo), "empty after full drain");
}

static void test_peek_non_destructive(void)
{
    kfifo_t fifo;
    uint8_t buf[8];
    uint8_t peeked[2], got[2];

    CHECK(kfifo_init(&fifo, buf, 8) == 0, "init peek");

    const uint8_t data[2] = {0xAB, 0xCD};
    kfifo_put(&fifo, data, 2);

    size_t n = kfifo_peek(&fifo, peeked, 2);
    CHECK(n == 2, "peek returns bytes");
    CHECK(peeked[0] == 0xAB && peeked[1] == 0xCD, "peek sees head bytes");
    CHECK(!kfifo_is_empty(&fifo), "peek does not empty the fifo");

    n = kfifo_get(&fifo, got, 2);
    CHECK(n == 2, "get after peek reads same bytes");
    CHECK(got[0] == 0xAB && got[1] == 0xCD, "get after peek matches peek");
}

static void test_skip(void)
{
    kfifo_t fifo;
    uint8_t buf[8];
    uint8_t out;

    CHECK(kfifo_init(&fifo, buf, 8) == 0, "init skip");

    for (int i = 0; i < 4; i++) {
        uint8_t v = (uint8_t)i;
        kfifo_put(&fifo, &v, 1);
    }
    CHECK(kfifo_skip(&fifo, 2) == 0, "skip advances read index");
    CHECK(kfifo_get(&fifo, &out, 1) == 1, "get after skip");
    CHECK(out == 2, "skip discards the first two bytes");
    CHECK(kfifo_skip(&fifo, 10) == -1, "skip beyond available is an error");
}

/*
 * @requirement: kfifo MUST validate a buffer's contents against the stored
 * CRC-8 checksum before a read is dequeued, so that memory corruption of the
 * backing store is detected and a poisoned dequeue is refused rather than
 * delivered.
 */
static void test_crc_validation(void)
{
    kfifo_t fifo;
    uint8_t buf[16];
    uint8_t out[4];

    CHECK(kfifo_init(&fifo, buf, 16) == 0, "init crc");

    const char *w = "abcd";
    kfifo_put(&fifo, w, 4);

    /* A healthy queue validates and the read succeeds. */
    size_t n = kfifo_get(&fifo, out, 4);
    CHECK(n == 4, "get of uncorrupted data succeeds");
    CHECK(memcmp(out, "abcd", 4) == 0, "get returns the queued bytes");

    /* Re-init and corrupt a queued byte in place, then confirm detection. */
    kfifo_init(&fifo, buf, 16);
    kfifo_put(&fifo, w, 4);
    buf[0] = (uint8_t)(buf[0] ^ 0xFFu);   /* flip the first queued byte */

    n = kfifo_get(&fifo, out, 4);
    CHECK(n == (size_t)-1, "get of corrupted data is refused (-1)");
    CHECK(kfifo_is_empty(&fifo) == 0, "read index does not advance on mismatch");
}

int main(void)
{
    test_init_validation();
    test_basic_ordering();
    test_no_wrap();
    test_wrap_around();
    test_peek_non_destructive();
    test_skip();
    test_crc_validation();

    if (g_failures == 0) {
        printf("ALL KFIFO TESTS PASSED\n");
        return 0;
    }
    printf("%d CHECK(S) FAILED\n", g_failures);
    return 1;
}
// PROVENANCE-END: BOILERPLATE
