// PROVENANCE-BEGIN: BOILERPLATE  Agent: opencode-ornith  Trace: T-unknown  Module: dspi
// Standard peripheral driver scaffolding: register-mapped access types, error
// enum, and the configuration struct passed to dspi_init().
#ifndef DSPI_H
#define DSPI_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * DSPI register-block view for the MPC5534 (base + 0x0000 offsets).
 * Only the fields the scaffolding touches are named; the transfer
 * queue/interrupt logic (HUMAN-AUTHORED placeholder) references the rest.
 */
typedef struct {
    volatile uint32_t MCR;  /* 0x0000  Module Configuration Register   */
    volatile uint32_t RESERVED0[7];
    volatile uint32_t TCR;  /* 0x0008  Transfer Count Register         */
    volatile uint32_t CTAR[8]; /* 0x000C Clock/Transfer Attribute Regs */
    volatile uint32_t SR;   /* 0x0028  Status Register                 */
    volatile uint32_t RSER; /* 0x002C  DMA/Interrupt Request Sel/Enable*/
    volatile uint32_t PUSHR; /* 0x0030  Push TX FIFO Register          */
    volatile uint32_t POPR;  /* 0x0034  Pop RX FIFO Register           */
    volatile uint32_t TXFR[4]; /* 0x0038  Transmit FIFO Registers      */
    volatile uint32_t RESERVED1[11];
    volatile uint32_t RXFR[4]; /* 0x007C  Receive FIFO Registers       */
} dspi_regs_t;

/* Error codes returned by the DSPI driver (see dspi.h / DDR for origin). */
typedef enum {
    DSPI_OK = 0,          /* Operation completed successfully            */
    DSPI_ERR_PARAM,       /* NULL/invalid argument passed to a function  */
    DSPI_ERR_BUSY,        /* A transfer is already in progress           */
    DSPI_ERR_TIMEOUT,     /* Transfer did not complete within the budget */
    DSPI_ERR_OVERFLOW,    /* RX FIFO overflow (RFOF) during a transfer   */
    DSPI_ERR_UNDERFLOW,   /* TX FIFO underflow (TFUF) in slave mode      */
    DSPI_ERR_MODULE       /* Module was not initialized before use       */
} dspi_status_t;

/*
 * Configuration for a single DSPI instance. Populated by the caller and
 * handed to dspi_init(); the driver maps it onto the CTAR/MCR registers.
 */
typedef struct {
    dspi_regs_t *regs;     /* Register block base address for this instance */
    uint8_t      mode;     /* Master or slave mode (see dspi_mode_t)        */
    uint8_t      cpol;     /* Clock polarity (see dspi_cpol_t)              */
    uint8_t      cpha;     /* Clock phase (see dspi_cpha_t)                 */
    uint8_t      lsb_first;/* 0 = MSB first, 1 = LSB first                  */
    uint16_t     bits;     /* Frame size in bits (4..16)                    */
    uint32_t     baud_hz;  /* Desired SCK frequency in Hz (0 = driver default) */
} dspi_config_t;

/* Operational mode (maps to DSPIx_MCR.MSTR). */
typedef enum {
    DSPI_MODE_SLAVE = 0,
    DSPI_MODE_MASTER = 1
} dspi_mode_t;

/* Clock polarity (maps to DSPIx_CTARn.CPOL). */
typedef enum {
    DSPI_CPOL_ACTIVE_LOW = 0,
    DSPI_CPOL_ACTIVE_HIGH = 1
} dspi_cpol_t;

/* Clock phase (maps to DSPIx_CTARn.CPHA). */
typedef enum {
    DSPI_CPHA_EDGE_1 = 0,
    DSPI_CPHA_EDGE_2 = 1
} dspi_cpha_t;

/*
 * Initialize the DSPI instance described by `config`.
 * Returns DSPI_OK on success, or a dspi_status_t error code.
 */
dspi_status_t dspi_init(const dspi_config_t *config);

/*
 * Deinitialize the instance: stop transfers, disable the module.
 * Returns DSPI_OK or DSPI_ERR_MODULE if never initialized.
 */
dspi_status_t dspi_deinit(void);

/*
 * Blocking single-frame SPI transfer. Writes tx_bytes and reads rx_bytes
 * for one frame of `frame_bits` length. Returns DSPI_OK on success.
 * The frame-queueing and interrupt servicing that drive the actual shift
 * are the caller's (see the HUMAN-AUTHORED placeholder in dspi.c).
 */
dspi_status_t dspi_transfer(dspi_regs_t *regs,
                            const uint8_t *tx_bytes,
                            uint8_t *rx_bytes,
                            uint16_t frame_bits);

#ifdef __cplusplus
}
#endif

#endif /* DSPI_H */
// PROVENANCE-END: BOILERPLATE
