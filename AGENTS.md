# AGENTS.md

<!-- throughmark:begin -->
This project is monitored by ThroughMark compliance tooling. All provenance, clean-room,
and file-write rules are enforced server-side and injected automatically into every session.

## Reference knowledge base

The on-box knowledge base contains ingested datasheets, reference manuals, and engineering
standards. **Before stating any hardware fact from memory** — register names, bit fields,
clock limits, peripheral behaviour, errata — call `tm_skill_procedure` with
`skill_name="refdocs"` to retrieve the lookup procedure, then follow it.

Available documents include: MPC5534 datasheet and reference manual (SPI, eSCI, eQADC,
FlexCAN, clocks, power modes, interrupts, DMA).

## Devkit tools (vitrevant-devkit MCP)

- `tm_skill_procedure(skill_name)` — load a skill procedure by name before acting on it
- `tm_scan_provenance(files, regulated, ddr_dir)` — scan files for tagging/provenance gaps
- `tm_prefill_ddr(turns)` — extract a DDR draft from session turns
- `tm_get_ddr_template()` — fetch the DDR format guide
- `tm_baseline_tag(files, commit, date)` — bulk-tag pre-existing code at a baseline
<!-- throughmark:end -->
# AGENTS.md

<!-- vitrevant:begin -->
This project is monitored by Vitrevant compliance tooling. All provenance, clean-room,
and file-write rules are enforced server-side and injected automatically into every session.

## Reference knowledge base

The on-box knowledge base contains ingested datasheets, reference manuals, and engineering
standards. **Before stating any hardware fact from memory** — register names, bit fields,
clock limits, peripheral behaviour, errata — call `tm_skill_procedure` with
`skill_name="refdocs"` to retrieve the lookup procedure, then follow it.

## Devkit tools (vitrevant-devkit MCP)

- `tm_skill_procedure(skill_name)` — load a skill procedure by name before acting on it
- `tm_scan_provenance(files, regulated, ddr_dir)` — scan files for tagging/provenance gaps
- `tm_prefill_ddr(turns)` — extract a DDR draft from session turns
- `tm_get_ddr_template()` — fetch the DDR format guide
- `tm_baseline_tag(files, commit, date)` — bulk-tag pre-existing code at a baseline
<!-- vitrevant:end -->
