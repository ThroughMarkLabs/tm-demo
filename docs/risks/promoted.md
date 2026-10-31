## HAZ-1 — Silent data loss on write failure
Description: A log write fails (disk full, I/O error, permission change) but the failure is swallowed and no exception is raised. The caller believes the entry was recorded; it was not. During incident analysis the entry is missing with no indication it was ever attempted.
Severity: critical
Probability: remote
Mitigation: SRS-001 requires every write to propagate OSError on failure. The logger uses fsync() to confirm durable storage; any failure raises before returning to the caller. Callers are required to handle the exception.
Residual-severity: low
Residual-probability: remote

## HAZ-2 — Stale or incorrect timestamp from local timezone
Description: The logger uses a non-UTC clock source (e.g. datetime.now() without tzinfo, or a system with a misconfigured timezone). Log entries carry incorrect timestamps. When correlated with UTC-timestamped entries from other systems, events appear out of order or at the wrong time, producing a misleading incident timeline.
Severity: moderate
Probability: low
Mitigation: SRS-003 mandates datetime.now(timezone.utc) throughout. The "Z" suffix is checked in the acceptance test. datetime.now() (without timezone.utc) is prohibited by code review.
Residual-severity: negligible
Residual-probability: remote
