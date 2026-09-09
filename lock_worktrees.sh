#!/bin/bash
#
# Locks every linked worktree so that git will not prune it.
#
# The macOS host and the devcontainer share a single .git directory, so every worktree
# registration is visible to both while its path resolves on only one of them. Each side
# therefore reads the other side's worktrees as stale, and Claude Code's stale-worktree
# cleanup deletes their admin directories under .git/worktrees. Every git command in a
# wiped worktree then fails with "fatal: not a git repository".
#
# Locking prevents that, but only with a reason Claude Code did not write itself. Its own
# locks read "claude session <name> (pid N start M)", and it will unlock and prune those
# once the pid looks dead, which a pid from the other side always does. This script takes
# such locks over and remembers the original reason so unlock_worktrees.sh can restore it.
#
# Run this on the host before starting anything that touches the repository, and run
# unlock_worktrees.sh when you are done.
#
# Usage: ./lock_worktrees.sh

set -euo pipefail

MARKER="locked by lock_worktrees.sh"
CLAUDE_LOCK='^claude (agent|session) .+ \(pid [0-9]+( start .+)?\)$'

locked_count=0
kept_count=0
skipped_count=0

# Takes over one worktree's lock. Worktrees locked by hand are left alone, since a reason
# somebody chose is a deliberate one and this script has no business overwriting it.
process() {
    local path="$1" is_locked="$2" reason="$3"

    if [[ "$is_locked" == "1" ]]; then
        if [[ "$reason" == "$MARKER"* ]]; then
            kept_count=$((kept_count + 1))
            return
        fi
        if [[ ! "$reason" =~ $CLAUDE_LOCK ]]; then
            echo "  skipped  $path"
            echo "           locked by hand: $reason"
            skipped_count=$((skipped_count + 1))
            return
        fi
        git worktree unlock "$path"
        git worktree lock "$path" --reason "$MARKER, restoring: $reason"
        echo "  took over $path"
    else
        git worktree lock "$path" --reason "$MARKER"
        echo "  locked   $path"
    fi
    locked_count=$((locked_count + 1))
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

echo "locked $locked_count, already ours $kept_count, left alone $skipped_count"
