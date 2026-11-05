# Changelog

## Unreleased

- Use seq_file for partial reads, EOF and seeking; count produced snapshots rather than individual read calls.
- Parse commands before changing synchronized state. Reject embedded NUL bytes, oversized input and invalid or out-of-range decimal values.
- Use one canonical implementation with a Portuguese walkthrough, configurable kernel headers and an explicit integration checker.
