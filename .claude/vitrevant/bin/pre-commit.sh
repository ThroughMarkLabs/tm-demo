#!/usr/bin/env bash
# Vitrevant advisory pre-commit — surface provenance / DDR gaps BEFORE the commit, left of the server
# gate, so nothing is first discovered as a red check at PR time. NON-BLOCKING by default: it warns and
# lets the commit proceed (a blocking local hook is exactly the friction developers route around).
#   VLI_PRECOMMIT_BLOCK=1  -> gaps fail the commit (opt-in, for shops that want local enforcement)
#   VLI_PRECOMMIT_SKIP=1   -> silence entirely
set -u
[ "${VLI_PRECOMMIT_SKIP:-0}" = 1 ] && exit 0
ROOT="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"
SC="$ROOT/.claude/skills/vitrevant-validate/scripts/validate_scan.sh"
[ -f "$SC" ] || exit 0   # kit not installed here — nothing to do
# Findings are KIND<TAB>location<TAB>detail; keep only real finding lines (drop informational parentheticals).
OUT="$( (cd "$ROOT" && bash "$SC" 2>/dev/null) | awk -F'\t' 'NF>=2 && $1 ~ /^[A-Z][A-Z-]+$/' || true )"
[ -z "$OUT" ] && exit 0
{
  echo "──── Vitrevant: provenance gaps on this branch (advisory) ────"
  printf '%s\n' "$OUT" | sed 's/^/  • /'
  echo "  Resolve with the validate / tag / ddr-draft skills — or mark DDR: none / downgrade to BOILERPLATE where apt."
  echo "  (advisory: commit continues. VLI_PRECOMMIT_BLOCK=1 to enforce · VLI_PRECOMMIT_SKIP=1 to silence)"
} >&2
if [ "${VLI_PRECOMMIT_BLOCK:-0}" = 1 ]; then
  echo "  VLI_PRECOMMIT_BLOCK=1 → blocking this commit until the gaps above are resolved." >&2
  exit 1
fi
exit 0
