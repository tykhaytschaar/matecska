#!/bin/sh
# Verzio a git tagbol (egyetlen forras: a v* tagek).
#   tagen allva:            0.1.0
#   3 committal a tag utan: 0.1.0+3
#   piszkos working tree:   ...*
#   tag nelkul:             0.0.0+<SHA7>
# A kimenet csak a 3x5 font karaktereit hasznalja (szamok . + * A-F).
cd "$(dirname "$0")/.." || exit 1
desc=$(git describe --tags --long --dirty --match 'v*' 2>/dev/null)
if [ -n "$desc" ]; then
    dirty=""
    case "$desc" in *-dirty) dirty="*"; desc=${desc%-dirty};; esac
    rest=${desc%-g*}          # v0.1.0-3
    n=${rest##*-}             # 3
    tag=${rest%-*}; tag=${tag#v}
    if [ "$n" = "0" ]; then echo "$tag$dirty"; else echo "$tag+$n$dirty"; fi
else
    sha=$(git rev-parse --short=7 HEAD 2>/dev/null | tr a-z A-Z)
    dirty=""; git diff --quiet 2>/dev/null || dirty="*"
    echo "0.0.0+${sha:-UNKNOWN}$dirty"
fi
