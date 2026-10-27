# KB-09 1.10.0 release validation

Release candidate: validation in progress. Do not advance the public update feed until the final platform checks and packages pass.

The shared preset-load completion path, update-summary parser, approved power behavior and startup-dialog serialization passed the local candidate checks. The approved shadow is implemented in the production renderer. Cross-platform release builds and final packaging checks are pending.

Linux standalone operation was reported on Arch/KDE with XWayland and PipeWire. Confirmation of the startup-dialog fix on that physical desktop is still pending. Mac Intel execution is not established by an arm64 runner; both slices must be built and checked. Mac packages remain ad-hoc signed, not notarized.
