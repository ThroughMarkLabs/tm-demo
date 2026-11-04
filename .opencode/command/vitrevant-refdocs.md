---
description: Get authoritative facts (chip datasheets, engineering standards like DO-178C/MISRA, protocol specs, hardware errata, vendor app notes, internal engineering specs) from the on-box reference knowledge b
---
Call the `vitrevant-devkit` MCP tool `tm_skill_procedure` with skill_name="refdocs" to retrieve the full procedure for this skill, then follow it exactly for this repository. $ARGUMENTS

The MCP server name is `vitrevant-devkit` (include the full name with hyphen). Call it as:
  server: vitrevant-devkit  tool: tm_skill_procedure  args: skill_name="refdocs"
Then use whichever tools the returned procedure specifies (e.g. server: vitrevant-refdocs  tool: search_docs).
