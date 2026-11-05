#!/usr/bin/env python3
"""Exercise an already loaded proc_counter module; no module is loaded here."""

import argparse
import errno
import os
from pathlib import Path
import re


def require(condition, snapshot):
    if not condition:
        raise AssertionError(f"Unexpected counter snapshot: {snapshot!r}")


def read_snapshot(path, chunk_size):
    with path.open("rb", buffering=0) as stream:
        chunks = []
        while chunk := stream.read(chunk_size):
            chunks.append(chunk)
    text = b"".join(chunks).decode("ascii")
    match = re.fullmatch(
        r"Read count: (\d+)\nWrite count: (\d+)\nStored value: (-?\d+)\n", text
    )
    if match is None:
        raise AssertionError(f"Incomplete or unexpected snapshot: {text!r}")
    return tuple(int(value) for value in match.groups())


def write_command(path, command):
    descriptor = os.open(path, os.O_WRONLY)
    try:
        written = os.write(descriptor, command)
        if written != len(command):
            raise AssertionError("Unexpected partial command write")
    finally:
        os.close(descriptor)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--path", type=Path, default=Path("/proc/proc_counter"))
    args = parser.parse_args()
    if not args.path.exists():
        parser.exit(2, "Load the module first; see README.md.\n")

    write_command(args.path, b"reset\n")
    snapshot = read_snapshot(args.path, 1)
    require(snapshot == (1, 0, 0), snapshot)
    write_command(args.path, b"-2147483648\n")
    snapshot = read_snapshot(args.path, 2)
    require(snapshot[1:] == (1, -2147483648), snapshot)
    write_command(args.path, b"")
    for invalid in [b"abc", b"2147483648", b"42\x00junk", b"\n", b"1" * 128]:
        try:
            write_command(args.path, invalid)
        except OSError as error:
            if error.errno != errno.EINVAL:
                raise
        else:
            raise AssertionError(f"Invalid command accepted: {invalid!r}")
    snapshot = read_snapshot(args.path, 4096)
    require(snapshot[1:] == (1, -2147483648), snapshot)
    write_command(args.path, b"reset\n")
    print("PASS: one-byte reads, EOF, signed range, empty writes and rejected commands")


if __name__ == "__main__":
    main()
