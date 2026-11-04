---
description: Cite an approved source for AI-assisted code in a regulated repo. For on-box reference documents (datasheets, standards, specs already ingested) use vitrevant-refdocs › get_citation — it returns a log
---
Call the `vitrevant-devkit` MCP tool `tm_skill_procedure` with skill_name="cite" to retrieve the full procedure for this skill, then follow it exactly for this repository. $ARGUMENTS

The MCP server name is `vitrevant-devkit` (include the full name with hyphen). Call it as:
  server: vitrevant-devkit  tool: tm_skill_procedure  args: skill_name="cite"
Then use whichever tools the returned procedure specifies (e.g. server: vitrevant-refdocs  tool: search_docs).
