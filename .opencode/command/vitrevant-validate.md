---
description: Pre-PR readiness check for the clean-room process. Scans the branch's changed code for provenance GAPS — unfenced regions, HUMAN-AUTHORED sections needing a DDR decision, DDR references that don't res
---
Call the `vitrevant-devkit` MCP tool `tm_skill_procedure` with skill_name="validate" to retrieve the full procedure for this skill, then follow it exactly for this repository. $ARGUMENTS

The MCP server name is `vitrevant-devkit` (include the full name with hyphen). Call it as:
  server: vitrevant-devkit  tool: tm_skill_procedure  args: skill_name="validate"
Then use whichever tools the returned procedure specifies (e.g. server: vitrevant-refdocs  tool: search_docs).
