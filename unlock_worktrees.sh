#!/bin/bash
#
# Undoes lock_worktrees.sh, restoring whatever lock each worktree had before.
#
# Only locks this script's counterpart placed are touched. A lock somebody set by hand is
# left exactly as it is, and so is a worktree that was never locked.
#
# Leaving the worktrees unlocked is the normal state. Lock them only while something on
# the other side of the shared .git is running, because a lock also stops Claude Code from
# recycling its own worktrees: it tears them down with "git worktree remove --force", and
# git refuses that on a locked worktree since it wants "-f -f".
#
# Usage: ./unlock_worktrees.sh

set -euo pipefail

MARKER="locked by lock_worktrees.sh"
RESTORE="$MARKER, restoring: "

unlocked_count=0
restored_count=0
skipped_count=0

process() {
    local path="$1" is_locked="$2" reason="$3"

    if [[ "$is_locked" != "1" || "$reason" != "$MARKER"* ]]; then
        skipped_count=$((skipped_count + 1))
        return
    fi

    git worktree unlock "$path"
    if [[ "$reason" == "$RESTORE"* ]]; then
        git worktree lock "$path" --reason "${reason#"$RESTORE"}"
        echo "  restored $path"
        restored_count=$((restored_count + 1))
    else
        echo "  unlocked $path"
        unlocked_count=$((unlocked_count + 1))
    fi
}

# The main worktree comes first in the porcelain listing and cannot be locked, so drop it.
first=1
path=""
is_locked=0
reason=""

flush() {
    if [[ -n "$path" ]]; then
        if [[ "$first" == "1" ]]; then
            first=0
        else
            process "$path" "$is_locked" "$reason"
        fi
    fi
    path=""
    is_locked=0
    reason=""
}

while IFS= read -r line; do
    case "$line" in
        "worktree "*) flush; path="${line#worktree }" ;;
        "locked")     is_locked=1; reason="" ;;
        "locked "*)   is_locked=1; reason="${line#locked }" ;;
    esac
done < <(git worktree list --porcelain)
flush

echo "unlocked $unlocked_count, restored $restored_count, left alone $skipped_count"
