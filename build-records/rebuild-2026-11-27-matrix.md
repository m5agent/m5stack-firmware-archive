# Rebuild the board documentation matrix

The English Arduino examples are pinned to docs revision
`7219cc9286c39b463da84053deee40bc234625db`. Each case manifest links to
its docs page and the location within that revision. The original compiler
receipts and defect evidence are from the [integration record](https://github.com/m5agent/agent-work-logs/tree/6806ff46edbe0a5d3c7a28059ca1e6b0adbd6d2e/records/esp32sim/2026-11-27-lx6b-integration).

Use Arduino CLI 1.3.1, M5Stack board package 3.2.5 and the macOS arm64
toolchain from the [original rebuild recipe](rebuild-2026-11-26.md).
The exact FQBN and any library override appear in the case manifest.
Compile errors are recorded alongside the original sketch and compiler
diagnostic. A case with a null firmware hash has no archived image.
