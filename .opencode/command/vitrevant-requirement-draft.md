---
description: Propose CANDIDATE requirements from what the project already contains — captured AI sessions, provenance-fenced code, DDRs — and write them to docs/requirements/candidates/ for a human to accept or re
---
Call the `vitrevant-devkit` MCP tool `tm_skill_procedure` with skill_name="requirement-draft" to retrieve the full procedure for this skill, then follow it exactly for this repository. $ARGUMENTS

The MCP server name is `vitrevant-devkit` (include the full name with hyphen). Call it as:
  server: vitrevant-devkit  tool: tm_skill_procedure  args: skill_name="requirement-draft"
Then use whichever tools the returned procedure specifies (e.g. server: vitrevant-refdocs  tool: search_docs).
