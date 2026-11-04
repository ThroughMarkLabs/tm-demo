---
description: Prepare a clean-room PR for a qualified reviewer — assemble the diff, each AI-DRAFTED/FLAGGED block next to its cited source and each HUMAN-AUTHORED block next to its DDR, list what still needs checki
---
Call the `vitrevant-devkit` MCP tool `tm_skill_procedure` with skill_name="review-assist" to retrieve the full procedure for this skill, then follow it exactly for this repository. $ARGUMENTS

The MCP server name is `vitrevant-devkit` (include the full name with hyphen). Call it as:
  server: vitrevant-devkit  tool: tm_skill_procedure  args: skill_name="review-assist"
Then use whichever tools the returned procedure specifies (e.g. server: vitrevant-refdocs  tool: search_docs).
