---
description: Update the content of an existing Design Decision Record (DDR) — its rationale, options, or chosen approach. Invoke with a FILE (iterate every DDR referenced by that file's provenance fences, offering
---
Call the `vitrevant-devkit` MCP tool `tm_skill_procedure` with skill_name="update-ddr" to retrieve the full procedure for this skill, then follow it exactly for this repository. $ARGUMENTS

The MCP server name is `vitrevant-devkit` (include the full name with hyphen). Call it as:
  server: vitrevant-devkit  tool: tm_skill_procedure  args: skill_name="update-ddr"
Then use whichever tools the returned procedure specifies (e.g. server: vitrevant-refdocs  tool: search_docs).
