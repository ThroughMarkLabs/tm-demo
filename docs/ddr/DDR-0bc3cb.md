## DDR-0bc3cb — MPC5534 DSPI driver scaffolding

Date: 2026-11-01   Developer: andrew   Trace: 3ddea93
Options considered: (1) single-file dspi.c only; (2) .c/.h split under src/dspi/; (3) grouped under src/periph/
Chosen: .c/.h split under src/dspi/ (dspi.c + dspi.h)

Rationale: fill in your own words — the .c/.h split under src/dspi/ and the interrupt-driven transfer design: a circular FIFO (DSPI_FIFO_DEPTH) holds queued frames; the TXRDY (TCF) ISR drains one frame per tick into DSPIx_PUSHR with EOQ on the last frame, and the RXRDY (RFDF) ISR pops DSPIx_POPR, records completion via a done flag, and clears RFDF. The public dspi_transfer() pushes one frame, primes PUSHR, enables TCF_RE + RFDF_RE in RSER, then blocks on the done flag before copying the received word out.
Conceived-by: human
Sources:
- [chips] MPC5534RM.pdf p.1039  sha256:8d11dd9382d481f8ba6d8e6ece22440c50a9136961e5824cf289cdeef48912f1  Retrieved:2026-11-01T19:29:02.284972Z
- [chips] MPC5534RM.pdf p.768  sha256:8d11dd9382d481f8ba6d8e6ece22440c50a9136961e5824cf289cdeef48912f1  Retrieved:2026-11-01T19:29:02.284972Z
- [chips] MPC5534RM.pdf p.771  sha256:8d11dd9382d481f8ba6d8e6ece22440c50a9136961e5824cf289cdeef48912f1  Retrieved:2026-11-01T19:29:02.284972Z
- [chips] MPC5534RM.pdf p.773  sha256:8d11dd9382d481f8ba6d8e6ece22440c50a9136961e5824cf289cdeef48912f1  Retrieved:2026-11-01T19:29:02.284972Z
- [chips] MPC5534RM.pdf p.782  sha256:8d11dd9382d481f8ba6d8e6ece22440c50a9136961e5824cf289cdeef48912f1  Retrieved:2026-11-01T19:29:02.284972Z
- [chips] MPC5534RM.pdf p.784  sha256:8d11dd9382d481f8ba6d8e6ece22440c50a9136961e5824cf289cdeef48912f1  Retrieved:2026-11-01T19:29:02.284972Z
- [chips] MPC5534RM.pdf p.778  sha256:8d11dd9382d481f8ba6d8e6ece22440c50a9136961e5824cf289cdeef48912f1  Retrieved:2026-11-01T19:29:02.284972Z
- [chips] MPC5534RM.pdf p.780  sha256:8d11dd9382d481f8ba6d8e6ece22440c50a9136961e5824cf289cdeef48912f1  Retrieved:2026-11-01T19:29:02.284972Z
