---
description: Resolve a FLAGGED provenance fence — code the guardrail marked as resembling a known/proprietary source or a patented approach. Walks the developer through describing the concern, rewriting from first
---
Call the `vitrevant-devkit` MCP tool `tm_skill_procedure` with skill_name="flag-triage" to retrieve the full procedure for this skill, then follow it exactly for this repository. $ARGUMENTS

The MCP server name is `vitrevant-devkit` (include the full name with hyphen). Call it as:
  server: vitrevant-devkit  tool: tm_skill_procedure  args: skill_name="flag-triage"
Then use whichever tools the returned procedure specifies (e.g. server: vitrevant-refdocs  tool: search_docs).
