#!/bin/bash
# Usage: run.sh SECONDS
# Runs flagtest2 in a fixed schedule of thread mixes. N is the CPU count. Output goes to stdout and flags-out/.
set -u
SECS="$1"
N=$(sysctl -n hw.ncpu)
OUT="$PWD/flags-out"
mkdir -p "$OUT"
rep() { local k="$1" n="$2" s=""; for _ in $(seq 1 "$n"); do s="$s $k"; done; echo "$s"; }
alt() { local n="$1" s=""; for _ in $(seq 1 "$n"); do s="$s eq64 int64"; done; echo "$s"; }

step() {
    local name="$1"; shift
    echo "=== $name: $*"
    ./flagtest2 "$SECS" "$@" | tee "$OUT/$name.txt"
}

step eq0 $(rep eq0 "$N")
step eq8 $(rep eq8 "$N")
step eq64 $(rep eq64 "$N")
step eq512 $(rep eq512 "$N")
step oe $(rep oe "$N")
step lt64 $(rep lt64 "$N")
step un64 $(rep un64 "$N")
step int64 $(rep int64 "$N")
step eq64_single eq64
step mix_threads $(alt "$N")
step eq64_oversubscribed $(rep eq64 $((N * 2)))

echo "=== mix_processes: eq64 x$N and int64 x$N in two processes"
./flagtest2 "$SECS" $(rep eq64 "$N") > "$OUT/mix_processes_eq.txt" &
./flagtest2 "$SECS" $(rep int64 "$N") > "$OUT/mix_processes_int.txt" &
wait
cat "$OUT/mix_processes_eq.txt" "$OUT/mix_processes_int.txt"

echo "=== summary"
grep -h "bad=[1-9]" "$OUT"/*.txt || echo "no bad reads"
