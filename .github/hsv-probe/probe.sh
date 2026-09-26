#!/bin/bash
# Usage: probe.sh GAME_TEST_BINARY HUE_MICRO WORKERS STARTUP_MIN MICRO_STARTUP_MIN MICRO_FULL_PASSES
# Phase 1 launches the game test binary with a filter that matches no test, so every process runs the engine
# startup, palette conversion included, and exits. Phase 2 launches hue_micro in its startup shape the same way.
# Phase 3 runs hue_micro's full sweep in one process per worker. Every non-zero exit is kept in hits/.
set -u
BIN="$1"; MICRO="$2"; WORKERS="$3"; STARTUP_MIN="$4"; MICRO_STARTUP_MIN="$5"; MICRO_FULL_PASSES="$6"
OUT="$PWD/probe-out"
mkdir -p "$OUT/hits" "$OUT/counts"

loop() {
    local phase="$1" id="$2" deadline="$3"; shift 3
    local n=0 fails=0 out rc
    while [ "$(date +%s)" -lt "$deadline" ]; do
        out=$("$@" 2>&1); rc=$?
        n=$((n + 1))
        if [ "$rc" -ne 0 ]; then
            fails=$((fails + 1))
            { echo "phase=$phase worker=$id run=$n rc=$rc date=$(date -u +%FT%TZ)"; printf '%s\n' "$out"; } > "$OUT/hits/${phase}_w${id}_${n}.txt"
        fi
    done
    echo "$n $fails" > "$OUT/counts/${phase}_w${id}"
}

phase() {
    local phase="$1" minutes="$2"; shift 2
    local deadline=$(( $(date +%s) + minutes * 60 ))
    echo "=== $phase: $WORKERS workers for $minutes min, started $(date -u +%TZ)"
    for id in $(seq 1 "$WORKERS"); do
        loop "$phase" "$id" "$deadline" "$@" &
    done
    wait
    local total=0 fails=0 n f
    for f in "$OUT"/counts/"${phase}"_w*; do
        read -r n f2 < "$f"; total=$((total + n)); fails=$((fails + f2))
    done
    echo "=== $phase: $total processes, $fails non-zero exits"
}

phase startup "$STARTUP_MIN" "$BIN" --headless --test-path "$PWD" --gtest_filter=NoSuch.Test
phase micro_startup "$MICRO_STARTUP_MIN" "$MICRO" startup

echo "=== micro_full: $WORKERS processes x $MICRO_FULL_PASSES passes, started $(date -u +%TZ)"
for id in $(seq 1 "$WORKERS"); do
    ( "$MICRO" full "$MICRO_FULL_PASSES" > "$OUT/counts/micro_full_w$id.txt" 2>&1; echo "rc=$?" >> "$OUT/counts/micro_full_w$id.txt" ) &
done
wait
cat "$OUT"/counts/micro_full_w*.txt
grep -l "HIT\|rc=[1-9]" "$OUT"/counts/micro_full_w*.txt | while read -r f; do cp "$f" "$OUT/hits/"; done

echo "=== hits ==="
ls "$OUT/hits" | head -50
for f in $(ls "$OUT"/hits/* 2>/dev/null | head -5); do echo "--- $f"; head -40 "$f"; done
