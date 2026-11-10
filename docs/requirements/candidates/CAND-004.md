<!-- Promoted to SRS-002. Retained as the evidence/rationale behind SRS-002; not a live candidate. -->

## CAND-004 — dspi_transfer SHALL bound completion by a configured time budget and report DSPI_ERR_TIMEOUT
<!--
Promoted to SRS-002 at 2026-11-10T04:04:27Z (see docs/requirements/promoted.md).
-->

Text: When invoked, dspi_transfer() SHALL complete — or return a status — within a bounded time
budget. If the transfer does not complete within that budget, dspi_transfer() SHALL return DSPI_ERR_TIMEOUT
rather than continuing to wait indefinitely.

Evidence: src/dspi/dspi.h:38 (DSPI_ERR_TIMEOUT in the dspi_status_t enum, documented "Transfer did not
complete within the budget" — the budget is named in the enum's own contract but no function is shown to
return it). src/dspi/dspi.c:269-334 (dspi_transfer(), the HUMAN-AUTHORED transfer body: after priming
PUSHR and enabling the TCF_RE/RFDF_RE interrupts it blocks in `while (g_dspi.done == 0u) { /* spin */ }`
at lines 308-315, with no expiry check — the wait is currently unbounded; the done flag is set only from
the RXRDY ISR at dspi.c:238). src/dspi/dspi_test.c:129-166 (the test acknowledges the blocking while(done)
loop "requires the ISR to set done" and cannot reach a timeout path — confirming none is exercised today).

Confidence: Partial — the enum entry and its "within the budget" documentation, plus the unbounded spin,
are directly observable. But the *budget itself* does not exist in the checked-in code: there is no
configured deadline, no clock or tick reference feeding one, and no code path that ever returns
DSPI_ERR_TIMEOUT. The candidate therefore states the contract the enum name and the spin-loop both imply,
but leaves every quantitative detail (see Unknowns) to the developer.

Standard:

Unknowns:
- The "configured time budget" has no value, unit, or source in the current code. Whether it is measured
  in wall-clock seconds, CPU cycles, or a number of driver ticks (and which clock feeds it) is not stated
  and must be supplied by the developer.
- The current loop is an unconditional spin `while (g_dspi.done == 0u)`. On timeout the driver currently
  leaves interrupts (TCF_RE/RFDF_RE) enabled and does not stop the module or clear the queue — the
  recovery / cleanup contract on timeout (disable interrupts, reset per-transfer state, leave the module in
  a usable state) is not defined.
- It is not stated whether DSPI_ERR_TIMEOUT is the only failure the budget guards, or whether an
  underflow/overflow (DSPI_ERR_UNDERFLOW / DSPI_ERR_OVERFLOW) can also surface as a terminal error before
  the budget elapses, and how the two relate.

Notes: The enum DSPI_ERR_TIMEOUT is declared but unreturned in the checked-in driver, and the transfer path
spins without any expiry. This candidate pins the "SHAIL return DSPI_ERR_TIMEOUT if the transfer does not
complete within the configured time budget" contract; it deliberately states no threshold, unit, or
tolerance, since none appears anywhere in the evidence and a plausible value would be read as a
specification.
