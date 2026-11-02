---
description: Show the clean-room compliance status of the current branch before a PR — which Vitrevant checks will likely pass or fail, which sections still need a DDR or a source, whether a validity sign-off is p
---
Call the `vitrevant-devkit` MCP tool `tm_skill_procedure` with skill_name="status" to retrieve the full procedure for this skill, then follow it exactly for this repository. $ARGUMENTS

The MCP server name is `vitrevant-devkit` (include the full name with hyphen). Call it as:
  server: vitrevant-devkit  tool: tm_skill_procedure  args: skill_name="status"
Then use whichever tools the returned procedure specifies (e.g. server: vitrevant-refdocs  tool: search_docs).
