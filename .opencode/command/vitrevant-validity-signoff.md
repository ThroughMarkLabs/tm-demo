---
description: Record a regulated validity sign-off on a pull request — a qualified reviewer's attestation that the AI-assisted change was checked against its cited source and is valid (or invalid / valid-with-condi
---
Call the `vitrevant-devkit` MCP tool `tm_skill_procedure` with skill_name="validity-signoff" to retrieve the full procedure for this skill, then follow it exactly for this repository. $ARGUMENTS

The MCP server name is `vitrevant-devkit` (include the full name with hyphen). Call it as:
  server: vitrevant-devkit  tool: tm_skill_procedure  args: skill_name="validity-signoff"
Then use whichever tools the returned procedure specifies (e.g. server: vitrevant-refdocs  tool: search_docs).
