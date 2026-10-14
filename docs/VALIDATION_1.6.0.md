# KB-09 1.6.0 validation

Release includes standalone MIDI mapping, shared dialog typography, compact action spacing, keyboard-centered mapping UI, footer spacing, performance-bay outline and progressive rightmost key crack.

The local audition passed four processor/MIDI/preset suites, 160 mapping/GUI/stress checks with zero audio-thread C++ new allocations, effective typography and keyboard centering at three editor sizes, and 63 actual message/menu states. Kevin approved the standalone workflow and appearance before release.

Release validation and hashes are recorded in artifacts/Release160/validation. Windows VST3/standalone, universal Mac AU/VST3/standalone and Linux x86-64 standalone are rebuilt with test identity disabled. Native CI validates MIDI mapping on each platform. Linux retains its physical-playback UNVERIFIED designation.
