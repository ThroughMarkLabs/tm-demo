---
description: Get authoritative facts (chip datasheets, engineering standards like DO-178C/MISRA, protocol specs, hardware errata, vendor app notes, internal engineering specs) from the on-box reference knowledge b
---
Call the `vitrevant-devkit` MCP tool `tm_skill_procedure` with skill_name="refdocs" to retrieve the full procedure, then follow it exactly for this repository. $ARGUMENTS

Example call sequence:
  1. Load procedure:  vitrevant-devkit › tm_skill_procedure(skill_name="refdocs")
  2. Search the docs: vitrevant-refdocs › search_docs(query="...", scope="<part-number>", collection="chips")
