# Contributing

Describe the observable problem, the expected behavior and how to reproduce it.
Keep changes small, use descriptive names and preserve the existing license.

Run make against matching kernel headers. Run the integration checker only in an isolated machine or VM where you deliberately loaded the module; it changes the test device state.

Use the checked-in .clang-format for maintained C sources. Keep generated build
products out of source directories.

Report automated results separately from physical-device or live-kernel checks.
Document public API, command, wiring and behavior changes in the relevant guide
and CHANGELOG.md. Never include private captures, credentials or board data.
