---
description: Tag human-written code that has no provenance fence. Identifies human code by the ABSENCE of an AI fence (not by git history), wraps each unfenced region as HUMAN-AUTHORED (or BOILERPLATE for standard
---
Call the `vitrevant-devkit` MCP tool `tm_skill_procedure` with skill_name="tag" to retrieve the full procedure for this skill, then follow it exactly for this repository. $ARGUMENTS

The MCP server name is `vitrevant-devkit` (include the full name with hyphen). Call it as:
  server: vitrevant-devkit  tool: tm_skill_procedure  args: skill_name="tag"
Then use whichever tools the returned procedure specifies (e.g. server: vitrevant-refdocs  tool: search_docs).
