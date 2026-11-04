---
description: Produce the end-of-session clean-room summary the interaction protocol calls for — items generated (with provenance tags), decisions deferred to the human, DDRs recorded, concerns flagged, and items t
---
Call the `vitrevant-devkit` MCP tool `tm_skill_procedure` with skill_name="session-summary" to retrieve the full procedure for this skill, then follow it exactly for this repository. $ARGUMENTS

The MCP server name is `vitrevant-devkit` (include the full name with hyphen). Call it as:
  server: vitrevant-devkit  tool: tm_skill_procedure  args: skill_name="session-summary"
Then use whichever tools the returned procedure specifies (e.g. server: vitrevant-refdocs  tool: search_docs).
