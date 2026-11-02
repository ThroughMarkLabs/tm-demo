---
description: Propose fixes for static-analysis findings (clang-tidy, cppcheck, Semgrep) raised by the Vitrevant gate. Reads the gate's findings, explains what each one actually means for this code, and proposes a 
---
Call the `vitrevant-devkit` MCP tool `tm_skill_procedure` with skill_name="static-fix" to retrieve the full procedure for this skill, then follow it exactly for this repository. $ARGUMENTS

The MCP server name is `vitrevant-devkit` (include the full name with hyphen). Call it as:
  server: vitrevant-devkit  tool: tm_skill_procedure  args: skill_name="static-fix"
Then use whichever tools the returned procedure specifies (e.g. server: vitrevant-refdocs  tool: search_docs).
