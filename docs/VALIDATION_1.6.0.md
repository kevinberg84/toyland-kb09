# KB-09 1.6.0 validation

Kevin approved the standalone MIDI mapping workflow, dialog typography, compact action spacing, keyboard-centered mapping window and balanced footer before release. The extra outer bottom performance-bay line is removed, and the rightmost key crack begins before maximum age.

Windows: nine CTest suites; 811 real editor checks; 160 standalone mapping, persistence, geometry and stress checks with zero audio-thread C++ new allocations; 63 dialog/menu states; 47 Steinberg VST3 checks; 77 actual VST3 host assertions; pluginval strictness 10 passed. The final MIDI race harness also passed three repeated real event-loop runs.

Mac: native universal Intel/Apple Silicon standalone, AU and VST3 passed nine suites, three additional MIDI race runs, 811 editor checks, 63 dialog/menu states, architecture checks, ad-hoc signature verification and Apple AU validation. Native run: https://github.com/kevinberg84/toyland-kb09/actions/runs/38514901572

Linux: native x86-64 standalone passed nine suites, 811 editor checks, 63 dialog/menu states and shared-library dependency checks. Physical Arch playback/MIDI verification remains outstanding, so UNVERIFIED is retained. Native run: https://github.com/kevinberg84/toyland-kb09/actions/runs/38512562673

Linux CI starts Openbox before dialog-bearing tests. The Mac MIDI tests assert immediate pending audio values separately from asynchronous GUI timer publication, waiting for the observable value with a bounded deadline instead of a fixed short sleep. These CI/test corrections do not change product source. DSP, parameter and factory-preset source remains unchanged from the preceding release; reported latency remains 128 samples.

All three permanent Drive file IDs were updated in place and downloaded again. SHA256, ZIP CRC, 24 presets, manual edition and links passed. The Desktop Host folder retains exactly three platform folders; their package file hashes match staging. Installed Windows VST3 is updated. Prior released versions and personal presets are preserved. The standalone audition, shortcut, isolated settings, temporary build and rollback packages are deleted. Two older locked DSP test modules have one-time cleanup queued for the next Windows sign-in.

Detailed logs, native commit identities, package/download hashes and cleanup records are in artifacts/Release160/validation.
