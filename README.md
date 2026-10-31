# tm-demo — SafeLog Demo Covered Repo

This repository is the **covered repo** for the ThroughMark standalone box demo.
It contains a minimal safety-critical Python logging library ("SafeLog") used to
exercise every feature of the ThroughMark audit system end-to-end.

## What's here

| Path | Purpose |
|---|---|
| `docs/requirements/promoted.md` | 4 IEC 62304 §5.5.3 requirements |
| `docs/risks/promoted.md` | 2 hazards with severity and mitigation |
| `docs/ddr/` | Design decision records |
| `src/logger.py` | SafeLog implementation (added via PR) |
| `tests/test_logger.py` | Pytest tests with `@requirement:` markers |
| `.github/workflows/throughmark-checks.yml` | ThroughMark gate workflow |

## Requirements

See `docs/requirements/promoted.md` for the full registry.

| ID | Title |
|---|---|
| SRS-001 | Log entries shall be written atomically |
| SRS-002 | Null and empty message inputs shall be rejected |
| SRS-003 | Log timestamps shall use UTC ISO-8601 |
| SRS-004 | Error-level entries shall flush within 500 ms |

## Hazards

See `docs/risks/promoted.md` for the full registry.

| ID | Title | Severity |
|---|---|---|
| HAZ-1 | Silent data loss on write failure | critical |
| HAZ-2 | Stale or incorrect timestamp from local timezone | moderate |

## Runner setup

The ThroughMark self-hosted runner is registered on the box.
Set `TM_HOME` as a GitHub repository variable pointing to the 14_standalone checkout:
```
Settings → Variables → TM_HOME = /home/ubuntu/ec2
```

See `docs/ops/RUNBOOK.md` in the 14_standalone repo for the full walkthrough.
