#!/usr/bin/env bash
# ThroughMark advisory pre-commit — surface provenance / DDR gaps BEFORE the commit, left of the server
# gate, so nothing is first discovered as a red check at PR time. NON-BLOCKING by default: it warns and
# lets the commit proceed (a blocking local hook is exactly the friction developers route around).
#   TM_PRECOMMIT_BLOCK=1  -> gaps fail the commit (opt-in, for shops that want local enforcement)
#   TM_PRECOMMIT_SKIP=1   -> silence entirely
set -u
[ "${TM_PRECOMMIT_SKIP:-0}" = 1 ] && exit 0
ROOT="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"
SC="$ROOT/.claude/skills/throughmark-validate/scripts/validate_scan.sh"
[ -f "$SC" ] || exit 0   # kit not installed here — nothing to do
# Findings are KIND<TAB>location<TAB>detail; keep only real finding lines (drop informational parentheticals).
OUT="$( (cd "$ROOT" && bash "$SC" 2>/dev/null) | awk -F'\t' 'NF>=2 && $1 ~ /^[A-Z][A-Z-]+$/' || true )"
[ -z "$OUT" ] && exit 0
{
  echo "──── ThroughMark: provenance gaps on this branch (advisory) ────"
  printf '%s\n' "$OUT" | sed 's/^/  • /'
  echo "  Resolve with the validate / tag / ddr-draft skills — or mark DDR: none / downgrade to BOILERPLATE where apt."
  echo "  (advisory: commit continues. TM_PRECOMMIT_BLOCK=1 to enforce · TM_PRECOMMIT_SKIP=1 to silence)"
} >&2
if [ "${TM_PRECOMMIT_BLOCK:-0}" = 1 ]; then
  echo "  TM_PRECOMMIT_BLOCK=1 → blocking this commit until the gaps above are resolved." >&2
  exit 1
fi
exit 0
