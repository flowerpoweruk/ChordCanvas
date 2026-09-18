# ChordCanvas: technical architecture

**Authority:** Implementation boundaries, build system, threading, state ownership and feasibility gates.  
**Read with:** All domain specifications; [references](13_VERIFIED_TECHNICAL_REFERENCES.md) S01–S09, S15–S19, S22–S24 and S27.

## 1. Fixed target and implementation freedom

[C] C++ with JUCE, VST3 instrument, Windows 11 x64, Ableton Live 12. One self-contained audio instrument, not a MIDI-effect-only device or Max for Live patch. The user does not want to set up a development environment merely to use the installed product.

[D] Use a CMake-based, scriptable native Windows build. Choose a supported stable JUCE revision, a compatible MSVC toolchain, Windows SDK and C++ language level; pin their actual tested versions/commits. Do not treat a remembered JUCE version as current. JUCE's published licensing material has changed over time; the applicable release and rights must be checked.

[D] Prefer C++20 when supported by the pinned stack. Use ordinary JUCE components with a custom design system. Do not add Electron, a browser service, Node/Python at runtime, Docker or a cloud backend. Development-only tools are acceptable when justified and pinned.

[C] Public repository and self-contained installers do not authorise copying third-party code/assets without complying with their licences. Licence selection is a separate permission/release gate, not an implicit consequence of choosing JUCE.

## 2. Suggested repository structure

The exact directory layout is an engineering default, but separation of responsibilities is required.

```text
ChordCanvas/
  AGENTS.md
  00_MASTER_BRIEF.md ... 13_VERIFIED_TECHNICAL_REFERENCES.md
  BUILD_STATUS.md
  CMakeLists.txt
  CMakePresets.json
  Source/
    Plugin/          processor/editor and host adapter
    Theory/          spelling, keys, chords, voicings
    Model/           immutable chord/block/timeline values
    Commands/        edits, transactions, undo/redo
    Audio/           voices, sounds, scheduling, preview ownership
    Export/          Standard MIDI File and native drag support
    Persistence/     explicit progression files; minimal host state
    Diagnostics/     event capture, log service, crash coordination
    UI/              components, layout, interactions, design tokens
  Assets/            only original or redistribution-cleared assets
  Tests/
    Unit/
    Property/
    Integration/
    Fixtures/
  Packaging/
    Windows/
  Scripts/
  release/           structured current-version data, not build outputs
  engineering/       decisions, design review, sanitised test evidence
  .github/workflows/
  .gitignore
```

[D] Keep installers and packaged builds as release artifacts, not repeatedly committed binary blobs. Store matching symbols/build provenance securely where needed for crash diagnosis. Clean-machine reproducibility requires asset sources or deterministic, pinned retrieval with rights, not undocumented local paths.

## 3. Domain modules and interfaces

### Theory engine

Pure functions for key inventory, scale spelling, chord construction, suspension/seventh handling, inversion, octave and labels. No GUI dependency. Store theoretical and display spellings separately. Return explicit MIDI integers. Supply independent fixtures and full combination tests.

### Model and commands

A `TimelineModel` owns an integer bar length and validated non-overlapping blocks. A block owns its origin harmony and resolved notes; it must not query the current pad key later. Selection/viewport state is separate from musical state.

All edits pass through command transactions, including replacement, group paste, destructive length changes and block property edits. A revision counter identifies committed state. Enforce invariants after every command, Undo/Redo and load.

### Audio scheduler

Consumes an immutable, prevalidated playback snapshot. Receives small real-time-safe events for audition/transport/voice changes. Owns voice lifetimes, sample-accurate event offsets within the available processing block and local musical phase. It never traverses GUI components or serializes project files.

### Host adapter

Reads supported tempo/meter/running information inside the audio callback. It converts host transitions into the documented local transport state machine, never directly copies host song position into the progression. Handle missing optional host information explicitly.

### GUI layer

Renders the model and issues commands. It owns pointer capture, hover previews, marquee geometry, keyboard focus, view transforms and popovers. It does not become the authority for chord pitches or timeline duration.

### Export and persistence

Exports a revision-frozen MIDI snapshot. Explicit progression Save/Load uses a distinct, versioned data format. Minimal host state deliberately does not restore the progression on project reopening. These paths share validation but are not interchangeable.

### Diagnostics

Receives structured action/error events. A background writer creates bounded logs in the writable per-user directory. The real-time path only posts compact, bounded events and never formats strings or opens files.

## 4. Data representation

[D] Prefer value types and stable IDs. A useful conceptual schema is:

```text
KeyIdentity { tonicLetter, tonicAccidental, mode }
ChordDefinition {
  originKey, originDegree, baseTonicMidi,
  suspension, seventhEnabled, inversion, octavePosition,
  resolvedMidiNotes, theoreticalNames, displayLabel, degreeColourId
}
ChordBlock { id, startTick, endTick, chord }
TimelineState { schemaVersion, revision, lengthBars, blocks }
EditorSession { selectedIds, playheadTick, zoom, scroll, tool, clipboard }
AuditionState { source, activePadOrBlock, heldOrLatched, repeatRate }
```

The names are examples, not required class names. Avoid redundant representations that can silently diverge. Where derived data is cached, validate/rebuild it from the frozen semantic origin and detect a mismatch.

[D] In versioned progression files, use bounded arrays, integer time values, valid enum domains and predictable text encoding. Reject duplicate IDs, overlaps, out-of-range pitches, unknown required schema versions and impossible durations. Load into a temporary validated model before committing one transaction.

## 5. Real-time and concurrency rules

The processing callback must have a bounded workload. Preallocate voice/event storage. Avoid heap allocation/free, file operations, network requests, GUI calls, OS dialogs, mutex acquisition, uncontrolled logging or exceptions escaping the plug-in boundary.

[D] Use a tested lock-free queue or double-buffered handoff for UI-to-audio events and snapshots. Immutable snapshots alone are not sufficient if their reference counting/destruction causes heap work on the audio thread; reclaim storage on a non-real-time thread. Include an explicit overflow/coalescing policy for rapid UI edits.

[D] Provide a safe high-priority path for release/stop/bypass commands so a full ordinary command queue cannot leave a chord stuck. Retain the latest complete musical state when coalescing property updates; never drop a release silently.

[D] Rate-limit audio-to-UI playhead updates and diagnostics. UI timers may draw the playhead but must not schedule notes. A sluggish window or open file dialogue must not slow the audio clock.

Use deterministic event ordering at boundaries. Test tempo/sample-rate/buffer changes, seeking, loop wraps and continuous operation over many loops. Validate denormal/NaN/infinity handling without hiding an engine error behind a silent successful status.

## 6. Plug-in classification and buses

[C] Register as a VST3 instrument with audio output and the ChordCanvas name. Use stable manufacturer/product IDs, component/controller IDs and parameter identities. Once released, do not regenerate them during an update.

[D] Start with a stereo output bus and no external audio input or sidechain. Live MIDI-output routing is not required. The chord engine generates internal note events for its own sound engine; MIDI drag-out is a separate file operation.

[D] Expose only event buses actually needed by the wrapper/host integration. Do not add an external MIDI-pad-mapping feature. If an input event bus is declared for normal instrument compatibility, document its exact treatment and do not allow arbitrary incoming notes to silently change the pad/timeline model. The chosen configuration must pass validation and stopped-transport audition in Live.

[D] Host automation is not a requested v1 feature. Do not automatically expose every UI control as a host-restored parameter, because that would conflict with fresh-start requirements. Use the minimum valid host-facing state/parameters and explicitly test any wrapper-provided recall behaviour.

Provide correct sample-rate/block-size preparation, audio buffer clearing, release-resource behaviour, bypass response and editor lifecycle. Do not open a second audio device from the VST3 or install an audio driver; Live owns the audio device.

## 7. Lifecycle and state ownership

[C] Fresh start when a new instance is constructed or a Live Set is reopened. No automatic save/restore or crash recovery of the composition. The same processor must retain its current session when only its editor closes and reopens.

[D] Return a valid minimal host-state blob containing schema/product compatibility information rather than a serialized progression. Do not crash if Live supplies empty, old or unexpected state. Do not restore musical settings through another route such as a global preferences file.

[D] Keep pad/key/repeat/feature-visibility settings instance-local. Shared data is limited to installed assets, release metadata, explicit user-saved files and coordinated diagnostics. Two open instances must not share timelines, audition latches, clipboard content or Undo stacks accidentally.

[D] Preserve file identity and safe minimal state handling across updater releases. Opening a project saved with the preceding version must find the same plug-in and start fresh as required, not report a missing instrument because the plug-in ID changed.

## 8. Early feasibility gates

Implement these proof slices before expensive polishing of the complete interface:

| Gate | Required evidence |
|---|---|
| VST3 loading | Native Release x64 bundle scans and opens in the installed Live 12 |
| Sound while stopped | A pad plays with host transport stopped using the local engine |
| Tempo | Current host BPM read while stopped/playing; tempo change followed |
| Sync | Host starts at a nonzero song position; ChordCanvas starts at its own bar 1 |
| Keyboard ownership | Keys 1–7 work when appropriate, numeric editing does not trigger pads |
| Native MIDI drag | A real file drag creates one correctly timed clip in Live |
| Trailing rests | Imported clip bounds retain the complete requested length |
| New/open editor lifecycle | Editor close/reopen retains work; reopened project starts fresh |
| Packaging | Standard VST3 install and clean in-place replacement are practical |

[G] The research verifies API capabilities, not these host outcomes. Record exactly which gates were executed. Where native desktop automation is unavailable, use an available authorised method or record the host/UI evidence gate as blocked; do not substitute a browser mockup or inferred pass.

## 9. Dependencies, assets and licensing

Use the smallest justified dependency set. Pin versions and hashes. Maintain a dependency/asset manifest with origin, version, licence, redistribution obligations and where notices are shipped. Verify original instrument samples, icons and fonts as carefully as source code.

[T] The inspected JUCE framework licence file documents a JUCE-licence route and AGPLv3 route (S27); verify the licence at the chosen pinned revision. A public GitHub repository does not automatically select either. Do not apply AGPL/GPL/MIT or a commercial licence to the owner's original code merely because it would be convenient. Do not assume the owner qualifies for a particular paid/free commercial tier.

[G] Inspect any already authorised licence or owner instruction in the environment. If a valid route is established, implement its obligations and continue. If not, mark distribution/licensing as a specific unresolved gate, continue independent engineering work where permitted, and obtain the minimum necessary owner authority before a rights-changing public release. Do not buy a licence or accept an unauthorised financial commitment.

For native synthesized sounds written for this project, retain the source. For third-party samples, obtain explicit redistribution permission and required notices before they enter the public repository. “Stock MIDI instruments” is not evidence of asset rights.

## 10. Build, CI and test tooling

[D] Provide non-interactive scripts for environment inspection, configure/build, unit tests, plug-in validation, packaging and release checks. Resolve the native toolchain from actual installation rather than hard-coding an assumed Visual Studio path. End users do not run these scripts.

[D] CI can compile and run automated tests on Windows, but it cannot be described as Ableton-host verification unless an appropriately licensed actual host test ran. Do not upload proprietary Live binaries or credentials to a public repository/runner.

[D] Use a supported VST3 validator and, where appropriate, a separate plug-in test harness. Pin the validator and preserve failures. A development-only standalone target is acceptable for instrumentation/design iteration, but it is not a shipping deliverable and does not replace Live testing.

[D] Build with useful warning levels, treat actionable project warnings seriously, use sanitizers/static analysis where supported, and separate Debug diagnostics from the final Release binary. Test in Release as well as Debug. Package only files in an explicit release manifest.

## 11. Engineering records and honesty

Record architecture choices, chosen defaults, actual dependencies, test commands, native host versions and evidence paths in `BUILD_STATUS.md` and concise engineering records. Record implementation defaults without pretending the owner answered them in the conversation.

Do not create a second simplified specification to work around difficult requirements. Do not report successful shipping, licence clearance, GUI testing or GitHub upload merely because the corresponding script exists. The deliverable includes evidence of execution, not just potential capability.
