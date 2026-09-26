#!/bin/bash
# Usage: probe2.sh GAME_TEST_BINARY WORKERS MINUTES
# Launches the game test binary with a filter that matches no test, so every process runs the engine startup and
# exits. OE_PALETTE_REPEAT from the environment multiplies the palette conversion work inside each process.
# Every non-zero exit is kept in probe-out/hits.
set -u
BIN="$1"; WORKERS="$2"; MINUTES="$3"
OUT="$PWD/probe-out"
mkdir -p "$OUT/hits" "$OUT/counts"
deadline=$(( $(date +%s) + MINUTES * 60 ))

worker() {
    local id="$1" n=0 fails=0 out rc
    while [ "$(date +%s)" -lt "$deadline" ]; do
        out=$("$BIN" --headless --test-path "$PWD" --gtest_filter=NoSuch.Test 2>&1); rc=$?
        n=$((n + 1))
        if [ "$rc" -ne 0 ]; then
            fails=$((fails + 1))
            { echo "worker=$id run=$n rc=$rc date=$(date -u +%FT%TZ)"; printf '%s\n' "$out"; } > "$OUT/hits/w${id}_${n}.txt"
        fi
    done
    echo "$n $fails" > "$OUT/counts/w$id"
}

echo "=== repeat=${OE_PALETTE_REPEAT:-1} workers=$WORKERS minutes=$MINUTES started $(date -u +%TZ)"
start=$(date +%s)
for id in $(seq 1 "$WORKERS"); do
    worker "$id" &
done
wait
total=0; fails=0
for f in "$OUT"/counts/w*; do
    read -r n f2 < "$f"; total=$((total + n)); fails=$((fails + f2))
done
echo "=== RESULT repeat=${OE_PALETTE_REPEAT:-1} processes=$total nonzero=$fails seconds=$(( $(date +%s) - start ))"
for f in "$OUT"/hits/*; do
    [ -f "$f" ] || continue
    echo "----- $f"
    grep -v "^\[20" "$f" | head -80
done
