// PROVENANCE-BEGIN: BOILERPLATE  Agent: opencode-ornith  Trace: T-unknown  Module: dspi
// Standard peripheral driver scaffolding for the MPC5534 DSPI: register
// access, init/deinit register plumbing, and error handling around the
// blocking transfer. The transfer's frame-queueing and interrupt servicing
// are the developer's invention (see the HUMAN-AUTHORED placeholder below).
#include "dspi.h"

/* Track the initialized instance so deinit/transfer can find its registers. */
static dspi_regs_t *g_dspi_regs = NULL;

/* Register bit masks pulled from MPC5534RM (see Sources below). */
#define DSPI_MCR_MSTR_MASK  0x00000001u
#define DSPI_MCR_MSTR_SHIFT 0u
#define DSPI_MCR_MDIS_MASK  0x00020000u
#define DSPI_MCR_MDIS_SHIFT 16u
#define DSPI_MCR_FRZ_MASK   0x00001000u
#define DSPI_MCR_FRZ_SHIFT  12u
#define DSPI_SR_TCF_MASK    0x00000001u
#define DSPI_SR_TCF_SHIFT   0u
#define DSPI_SR_TXRXS_MASK  0x00000002u
#define DSPI_SR_TXRXS_SHIFT 1u
#define DSPI_SR_EOF_MASK    0x00000004u
#define DSPI_SR_EOF_SHIFT   2u

// PROVENANCE-END: BOILERPLATE

// PROVENANCE-BEGIN: BOILERPLATE  Agent: opencode-ornith  Trace: T-unknown  Module: dspi
// Register accessors and the init/deinit register plumbing.
static inline void dspi_set_mcr(dspi_regs_t *regs, uint32_t value)
{
    regs->MCR = value;
}

static inline uint32_t dspi_get_mcr(dspi_regs_t *regs)
{
    return regs->MCR;
}

static inline uint32_t dspi_get_sr(dspi_regs_t *regs)
{
    return regs->SR;
}

/* Enable the DSPI module clock and put it in master/running mode. */
static dspi_status_t dspi_module_enable(dspi_regs_t *regs, uint8_t mode)
{
    uint32_t mcr = dspi_get_mcr(regs);

    /* MDIS=0: enable the DSPI clock. */
    mcr &= ~DSPI_MCR_MDIS_MASK;
    /* Clear FRZ so the module runs outside debug. */
    mcr &= ~DSPI_MCR_FRZ_MASK;
    /* Set/clear MSTR for the requested master/slave mode. */
    if (mode == DSPI_MODE_MASTER) {
        mcr |= (DSPI_MCR_MSTR_MASK << DSPI_MCR_MSTR_SHIFT);
    } else {
        mcr &= ~DSPI_MCR_MSTR_MASK;
    }

    dspi_set_mcr(regs, mcr);
    return DSPI_OK;
}

/* Disable the DSPI module clock and stop TX/RX. */
static dspi_status_t dspi_module_disable(dspi_regs_t *regs)
{
    uint32_t mcr = dspi_get_mcr(regs);

    /* MDIS=1: allow the clock to stop; MSTR=0 stops TX/RX. */
    mcr |= (DSPI_MCR_MDIS_MASK << DSPI_MCR_MDIS_SHIFT);
    mcr &= ~DSPI_MCR_MSTR_MASK;

    dspi_set_mcr(regs, mcr);
    g_dspi_regs = NULL;
    return DSPI_OK;
}

// PROVENANCE-END: BOILERPLATE

// PROVENANCE-BEGIN: BOILERPLATE  Agent: opencode-ornith  Trace: T-unknown  Module: dspi
// Public API: init, deinit. Init maps the caller's config struct onto the
// CTAR and MCR registers and records the instance; deinit stops and disables it.
dspi_status_t dspi_init(const dspi_config_t *config)
{
    if (config == NULL || config->regs == NULL) {
        return DSPI_ERR_PARAM;
    }

    dspi_regs_t *regs = config->regs;

    /* Configure the clock/transfer attribute register for the transfer. */
    uint32_t ctar = 0u;
    ctar |= ((uint32_t)config->cpol & 0x1u) << 3u;  /* CPOL */
    ctar |= ((uint32_t)config->cpha & 0x1u) << 4u;  /* CPHA */
    ctar |= ((uint32_t)config->lsb_first & 0x1u) << 9u; /* LSB */
    /* Frame size in bits, field [13:10] (FRF). */
    if (config->bits > 16u) {
        return DSPI_ERR_PARAM;
    }
    ctar |= ((uint32_t)(config->bits - 1u) & 0xFu) << 10u;
    regs->CTAR[0] = ctar;

    /* Bring the module up in the requested mode. */
    dspi_status_t rc = dspi_module_enable(regs, config->mode);
    if (rc != DSPI_OK) {
        return rc;
    }

    /* Record the live instance for deinit/transfer. */
    g_dspi_regs = regs;
    return DSPI_OK;
}

dspi_status_t dspi_deinit(void)
{
    if (g_dspi_regs == NULL) {
        return DSPI_ERR_MODULE;
    }
    return dspi_module_disable(g_dspi_regs);
}

// PROVENANCE-END: BOILERPLATE

// PROVENANCE-BEGIN: HUMAN-AUTHORED  Developer: andrew  Trace: 3ddea93  DDR: DDR-0bc3cb
// Interrupt-driven transfer body (developer's invention): a circular FIFO
// holds queued frames; the TXRDY (TCF) ISR drains one frame per tick into
// DSPIx_PUSHR, and the RXRDY (RFDF) ISR pops the received frame via
// DSPIx_POPR and signals completion via a done flag. See DDR-0bc3cb.
// Sources (from MPC5534RM, retrieved 2026-11-01):
//   [chips] MPC5534RM.pdf p.778  sha256:8d11dd9382d481f8ba6d8e6ece22440c50a9136961e5824cf289cdeef48912f1
//   [chips] MPC5534RM.pdf p.781  sha256:8d11dd9382d481f8ba6d8e6ece22440c50a9136961e5824cf289cdeef48912f1
#include <string.h>
#include "dspi.h"

/* Maximum queued frames for the circular FIFO. */
#define DSPI_FIFO_DEPTH 8u

/* One queued SPI frame: a transmit word and its matched receive word. */
typedef struct {
    uint32_t tx_data;
    uint32_t rx_data;
} dspi_frame_t;

/*
 * Per-instance driver state. A single live instance is tracked to keep the
 * ISR-driven state co-located with the module it drives.
 */
typedef struct {
    dspi_regs_t *regs;
    volatile uint8_t initialized;

    /* Circular FIFO of queued frames. */
    dspi_frame_t queue[DSPI_FIFO_DEPTH];
    uint16_t head;   /* push index */
    uint16_t tail;   /* pop index    */
    uint16_t count;

    /* Completion signalling for the current transfer. */
    volatile uint8_t done;
    dspi_status_t rc;
} dspi_instance_t;

/* The single tracked instance. */
static dspi_instance_t g_dspi;

/* Interrupt numbers wired to the DSPI module (board/INTC-specific). */
#define DSPI_IRQ_TX   0u   /* TXRDY / transfer-complete request */
#define DSPI_IRQ_RX   0u   /* RXRDY / RX FIFO drain request     */

/*
 * Queue helpers. The FIFO is the developer's design; these operate on the
 * circular buffer without touching the hardware.
 */
static uint8_t dspi_queue_is_empty(void)
{
    return (g_dspi.count == 0u);
}

static uint8_t dspi_queue_is_full(void)
{
    return (g_dspi.count >= DSPI_FIFO_DEPTH);
}

static void dspi_queue_push(const dspi_frame_t *frame)
{
    if (dspi_queue_is_full()) {
        return; /* caller should have checked; drop overflow defensively */
    }
    g_dspi.queue[g_dspi.head] = *frame;
    g_dspi.head = (uint16_t)((g_dspi.head + 1u) % DSPI_FIFO_DEPTH);
    g_dspi.count++;
}

/* Pop the next queued frame, or NULL when empty. */
static dspi_frame_t *dspi_queue_pop(void)
{
    if (dspi_queue_is_empty()) {
        return NULL;
    }
    dspi_frame_t *frame = &g_dspi.queue[g_dspi.tail];
    g_dspi.tail = (uint16_t)((g_dspi.tail + 1u) % DSPI_FIFO_DEPTH);
    g_dspi.count--;
    return frame;
}

/*
 * Push the next queued frame into DSPIx_PUSHR.
 * PUSHR field TXDATA [31:16] is the transmit word; for a single-frame
 * transfer the EOQ bit terminates the queue. See MPC5534RM p.782.
 */
static void dspi_isr_tx_fill(void)
{
    dspi_frame_t *frame = dspi_queue_pop();
    if (frame == NULL) {
        return; /* nothing queued; leave TX FIFO empty */
    }

    uint32_t pusr = frame->tx_data << 16u;
    /* EOQ set on the final queued frame to end the transfer. */
    if (dspi_queue_is_empty()) {
        pusr |= 0x00000008u; /* EOQ bit */
    }
    g_dspi.regs->PUSHR = pusr;
}

/*
 * Pop the received frame from DSPIx_POPR and record completion.
 * RFDF is write-1-to-clear (see MPC5534RM p.778).
 */
static void dspi_isr_rx_pop(void)
{
    uint32_t rx = g_dspi.regs->POPR;
    /*
     * Match the received word against the queued frame. The queue maps
     * each tx to its rx; here we take the most recently pushed frame's
     * rx slot for the current transfer.
     */
    g_dspi.done = 1u;
    /* Write 1 to clear the RFDF flag. */
    g_dspi.regs->SR |= 0x00004000u; /* RFDF */
}

/*
 * Transfer-complete / TXRDY interrupt service routine. Drains one frame
 * from the circular FIFO into DSPIx_PUSHR and clears TCF.
 */
void DSPI_IRQHandler(void)
{
    /* TCF (bit 0, w1c) is the TXRDY/transfer-complete flag. */
    if ((g_dspi.regs->SR & 0x00000001u) != 0u) {
        g_dspi.regs->SR |= 0x00000001u; /* clear TCF */
        dspi_isr_tx_fill();
    }

    /* RFDF (bit 14) is the RXRDY / RX FIFO drain flag. */
    if ((g_dspi.regs->SR & 0x00004000u) != 0u) {
        dspi_isr_rx_pop();
    }
}

/*
 * Blocking single-frame transfer (developer's design):
 *   1. Push the frame onto the circular FIFO.
 *   2. Prime the TX FIFO with the first entry via PUSHR.
 *   3. Enable TXRDY (TCF_RE) and RXRDY (RFDF_RE) interrupts.
 *   4. Block on the done flag; the ISRs drain the queue and signal completion.
 * The RX data is copied out through `rx_bytes` on completion.
 */
dspi_status_t dspi_transfer(dspi_regs_t *regs,
                            const uint8_t *tx_bytes,
                            uint8_t *rx_bytes,
                            uint16_t frame_bits)
{
    if (regs == NULL || tx_bytes == NULL || rx_bytes == NULL) {
        return DSPI_ERR_PARAM;
    }
    if (g_dspi.initialized == 0u || regs != g_dspi.regs) {
        return DSPI_ERR_MODULE;
    }
    if (frame_bits == 0u || frame_bits > 16u) {
        return DSPI_ERR_PARAM;
    }

    /* Reset per-transfer state. */
    g_dspi.head = 0u;
    g_dspi.tail = 0u;
    g_dspi.count = 0u;
    g_dspi.done = 0u;
    g_dspi.rc = DSPI_OK;

    /* Load the transmit word into the FIFO (TXDATA lives in [31:16]). */
    uint32_t tx_data = 0u;
    for (uint16_t i = 0u; i < (frame_bits + 7u) / 8u; i++) {
        tx_data |= (uint32_t)tx_bytes[i] << (8u * i);
    }
    dspi_frame_t frame = { .tx_data = tx_data, .rx_data = 0u };
    dspi_queue_push(&frame);

    /* Prime the TX FIFO: push the first frame into DSPIx_PUSHR. */
    g_dspi.regs->PUSHR = (tx_data << 16u) | 0x00000008u; /* EOQ */

    /* Enable TXRDY (TCF_RE, bit 0) and RXRDY (RFDF_RE, bit 14) interrupts. */
    uint32_t rser = g_dspi.regs->RSER;
    rser |= 0x00000001u;  /* TCF_RE */
    rser |= 0x00000004u;  /* RFDF_RE */
    g_dspi.regs->RSER = rser;

    /*
     * Block until the RXRDY ISR signals completion. In a real system the
     * caller disables interrupts (or uses a semaphore) around this wait;
     * the done flag is set from the RXRDY ISR (see dspi_isr_rx_pop).
     */
    while (g_dspi.done == 0u) {
        /* spin; replace with an OS sleep/yield if one is available */
    }

    /* Disable the transfer interrupts now that this frame is complete. */
    rser = g_dspi.regs->RSER;
    rser &= ~0x00000001u; /* TCF_RE */
    rser &= ~0x00000004u; /* RFDF_RE */
    g_dspi.regs->RSER = rser;

    /*
     * Copy the received frame out. The RX data was populated by the hardware
     * into DSPIx_POPR[31:16] on the transfer; read it back here.
     */
    uint32_t rx = g_dspi.regs->POPR;
    uint16_t nbytes = (frame_bits + 7u) / 8u;
    for (uint16_t i = 0u; i < nbytes && i < 4u; i++) {
        rx_bytes[i] = (uint8_t)((rx >> (8u * i)) & 0xFFu);
    }

    return g_dspi.rc;
}
// PROVENANCE-END: HUMAN-AUTHORED

/*
 * Clock configuration (developer's choice) — 25 MHz bus, ~1.5 MHz SPI target:
 *   CTAR[0] = DSPI_CTAR_FMSZ(7)   8-bit frame
 *           | DSPI_CTAR_CPOL(0)   Mode 0
 *           | DSPI_CTAR_CPHA(0)
 *           | DSPI_CTAR_BR(0x04)  PBR=1, BR=16 -> 25MHz/16 ≈ 1.5MHz
 * Applied by dspi_init() via the CTAR mapping above; not live code.
 */
