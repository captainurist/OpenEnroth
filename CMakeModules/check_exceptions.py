#!/usr/bin/env python3
"""Checks that the number of `throw` statements stays within the per-directory budget.

OpenEnroth reports errors with `Result<T>` rather than by throwing – see the "Error handling" section in HACKING.md.
This script is the ratchet that keeps it that way: it counts `throw` statements per source directory and compares
them against `exception_budget.txt`. Going over budget, or throwing from a directory that has no budget at all, is
an error.

Usage: check_exceptions.py <budget-file> <source-root>...
"""

import collections
import os
import re
import sys

# A `throw` that's followed by an identifier. This skips `throw;` re-throws (which can't introduce a new exception
# into an otherwise exception-free call path) and, together with the comment check below, doc comments.
THROW_RE = re.compile(r'(^|[^\w])throw\s+[A-Za-z_:]')
COMMENT_RE = re.compile(r'\s*(//|\*|/\*)')
SOURCE_SUFFIXES = ('.cpp', '.h', '.cc', '.hh', '.c', '.cxx')


def parse_budget(path):
    result = {}
    with open(path, encoding='utf-8') as file:
        for line in file:
            line = line.split('#', 1)[0].strip()
            if not line:
                continue
            prefix, count = line.rsplit(None, 1)
            result[prefix.rstrip('/') + '/'] = int(count)
    return result


def count_throws(roots):
    result = collections.Counter()
    for root in roots:
        for dirpath, _, filenames in os.walk(root):
            for filename in filenames:
                if not filename.endswith(SOURCE_SUFFIXES):
                    continue
                path = os.path.join(dirpath, filename)
                # Budget prefixes are relative to the repo root, and that's where we're expected to be run from.
                key = os.path.relpath(path).replace(os.sep, '/')
                with open(path, encoding='utf-8', errors='replace') as file:
                    for line in file:
                        if THROW_RE.search(line) and not COMMENT_RE.match(line):
                            result[key] += 1
    return result


def main(argv):
    if len(argv) < 3:
        print(__doc__, file=sys.stderr)
        return 2

    budget = parse_budget(argv[1])
    throws = count_throws(argv[2:])

    actual = collections.Counter()
    unbudgeted = collections.Counter()
    for path, count in throws.items():
        # Longest matching prefix wins, so that a subdirectory can have a tighter budget than its parent.
        prefix = max((p for p in budget if path.startswith(p)), key=len, default=None)
        if prefix is None:
            unbudgeted[path] += count
        else:
            actual[prefix] += count

    errors = []
    for path, count in sorted(unbudgeted.items()):
        errors.append(f"{path}: {count} throw statement(s) in code that's supposed to be exception-free. "
                      f"Use `Result<T>` from Utility/Error/Result.h instead.")
    for prefix, allowed in sorted(budget.items()):
        if actual[prefix] > allowed:
            errors.append(f"{prefix}: {actual[prefix]} throw statement(s), but the budget is {allowed}. "
                          f"Use `Result<T>` from Utility/Error/Result.h, or raise the budget and say why.")

    for prefix, allowed in sorted(budget.items()):
        if actual[prefix] < allowed:
            print(f"check_exceptions: {prefix} is now down to {actual[prefix]} throws (budget is {allowed}), "
                  f"please lower the budget in exception_budget.txt.")

    for error in errors:
        print(f"check_exceptions: error: {error}", file=sys.stderr)
    if errors:
        return 1

    print(f"check_exceptions: {sum(actual.values())} throw statements, all within budget.")
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))
