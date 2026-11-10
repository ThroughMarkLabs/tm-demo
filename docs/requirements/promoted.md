## SRS-002 — dspi_transfer SHALL bound completion by a configured time budget and report DSPI_ERR_TIMEOUT
When invoked, dspi_transfer() SHALL complete — or return a status — within a bounded time budget. If the transfer does not complete within that budget, dspi_transfer() SHALL return DSPI_ERR_TIMEOUT rather than continuing to wait indefinitely.
Promoted-from: CAND-004
Promoted-by: Andrew
Promoted-at: 2026-11-10T04:04:27.389057Z
