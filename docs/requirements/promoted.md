# SafeLog Requirements (SRS-001 .. SRS-004)

Registry of safety requirements for the SafeLog module. Each requirement is
traceable to a hazard in `docs/risks/promoted.md` where applicable.

| ID | Title | Linked hazard |
|---|---|---|
| SRS-001 | Log entries shall be written atomically | HAZ-1 |
| SRS-002 | Null and empty message inputs shall be rejected | — |
| SRS-003 | Log timestamps shall use UTC ISO-8601 | HAZ-2 |
| SRS-004 | Error-level entries shall flush within 500 ms | — |

> Authoring aid. Requirements are the developer's to define; record any
> derivation decisions in a DDR.
