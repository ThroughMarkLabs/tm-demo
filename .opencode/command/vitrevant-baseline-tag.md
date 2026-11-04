---
description: Onboard an existing, untagged repository into the clean-room process by adding a neutral BASELINE provenance header to every pre-existing source file. BASELINE means "provenance not established" — NOT
---
Call the `vitrevant-devkit` MCP tool `tm_skill_procedure` with skill_name="baseline-tag" to retrieve the full procedure for this skill, then follow it exactly for this repository. $ARGUMENTS

The MCP server name is `vitrevant-devkit` (include the full name with hyphen). Call it as:
  server: vitrevant-devkit  tool: tm_skill_procedure  args: skill_name="baseline-tag"
Then use whichever tools the returned procedure specifies (e.g. server: vitrevant-refdocs  tool: search_docs).
