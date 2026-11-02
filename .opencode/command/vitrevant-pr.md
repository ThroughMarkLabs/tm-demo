---
description: Open a pull request for clean-room work with a provenance-aware description — what was generated (by tag), which decisions were deferred to the human and their DDRs, sources cited (regulated), and any
---
Call the `vitrevant-devkit` MCP tool `tm_skill_procedure` with skill_name="pr" to retrieve the full procedure for this skill, then follow it exactly for this repository. $ARGUMENTS

The MCP server name is `vitrevant-devkit` (include the full name with hyphen). Call it as:
  server: vitrevant-devkit  tool: tm_skill_procedure  args: skill_name="pr"
Then use whichever tools the returned procedure specifies (e.g. server: vitrevant-refdocs  tool: search_docs).
