# ChordCanvas: master build brief

**Specification version:** 1.0  
**Prepared:** 18 September 2026  
**Audience:** Codex operating in the owner's authorised Windows development environment  
**Status:** Build specification, not a claim that the software has been built or tested

## 1. The assignment

Build **ChordCanvas** completely: a polished, self-contained **VST3 instrument**, written in **C++ with JUCE**, for **Ableton Live 12 on Windows 11 x64**. Deliver a working release, a minimal-interaction `Setup.exe`, a separate self-contained `Updater.exe`, and the complete reproducible source project in a **public GitHub repository named exactly `ChordCanvas` in the owner's account**.

Work autonomously through implementation, debugging, installation, host testing, visual review and packaging. Do not stop at a scaffold, demonstration, screenshot, source-code listing or instructions for the owner to compile. Do not replace the desktop plug-in with a website, Max for Live device, MIDI effect, standalone-only program or another plug-in format.

The owner's central workflow is to choose a song key, audition seven diatonic chords, arrange them as editable blocks on a timeline, hear the progression, and drag the complete progression into Live as one MIDI clip. Vocals remain in Ableton; importing or analysing vocals inside ChordCanvas was explicitly removed.

**GUI, UI and UX are fundamental product architecture.** Design interactions and visual hierarchy while designing the system. Do not finish the implementation and then put a cosmetic skin over it. ChordCanvas needs a coherent identity of its own, not an Ableton copy or a generic AI-generated application.

## 2. Read the entire pack before implementation

Keep these files together in the repository root initially. Read `AGENTS.md`, this file, and every numbered document below. The numbers specify organisation, not an override hierarchy.

| File | Authoritative subject |
|---|---|
| [01_PRODUCT_SPEC.md](01_PRODUCT_SPEC.md) | Product scope, workflow, defaults, lifecycle and exclusions |
| [02_MUSIC_THEORY_SPEC.md](02_MUSIC_THEORY_SPEC.md) | Keys, spelling, diatonic harmony, chord modifications and absolute MIDI pitches |
| [03_GUI_UX_DESIGN_SPEC.md](03_GUI_UX_DESIGN_SPEC.md) | Design system, layout, gesture ownership, visual states and review |
| [04_TIMELINE_INTERACTION_SPEC.md](04_TIMELINE_INTERACTION_SPEC.md) | Editing, time units, collisions, selection, history and boundaries |
| [05_AUDIO_AUDITION_SPEC.md](05_AUDIO_AUDITION_SPEC.md) | Sound generation, pad audition, repeats, transport and tempo |
| [06_MIDI_EXPORT_SPEC.md](06_MIDI_EXPORT_SPEC.md) | Native MIDI drag-out and exact exported content |
| [07_TECHNICAL_ARCHITECTURE.md](07_TECHNICAL_ARCHITECTURE.md) | Modules, threading, state, build system and early feasibility gates |
| [08_INSTALLER_UPDATER_RELEASES.md](08_INSTALLER_UPDATER_RELEASES.md) | Installation, updates, release identity and package verification |
| [09_DIAGNOSTICS_AND_LOGGING.md](09_DIAGNOSTICS_AND_LOGGING.md) | Five session logs, crash diagnostics, privacy and Explorer link |
| [10_GITHUB_AND_AUTONOMY.md](10_GITHUB_AND_AUTONOMY.md) | Repository creation, ongoing commits, permissions and autonomous execution |
| [11_ACCEPTANCE_TESTS.md](11_ACCEPTANCE_TESTS.md) | Required test scenarios and completion evidence |
| [12_DECISIONS_AND_TRACEABILITY.md](12_DECISIONS_AND_TRACEABILITY.md) | Conversation reconciliation, explicit defaults, requirement-to-test mapping |
| [13_VERIFIED_TECHNICAL_REFERENCES.md](13_VERIFIED_TECHNICAL_REFERENCES.md) | Primary technical sources, limitations and verification boundaries |

[BUILD_STATUS.md](BUILD_STATUS.md) is the working ledger, not an alternative specification. Maintain it throughout development. This pack and engineering evidence are development material; the user rejected an end-user README/manual, not engineering specifications, tests or legally necessary licence notices.

## 3. How to interpret requirements

The pack distinguishes four categories:

- **[C] Confirmed:** an explicit final user decision or a direct consequence of it. Implement it. Do not reopen it as a preference question.
- **[D] Engineering default:** a necessary implementation decision not explicitly answered by the user. It is supplied transparently so work can proceed. Do not describe it as a user quotation. A documented alternative is acceptable only where the relevant section permits one and all confirmed requirements remain satisfied.
- **[T] Technical correction:** an earlier conversational statement was incomplete or technically misleading. Implement the technically valid interpretation while preserving the user's outcome.
- **[G] Verification/permission gate:** evidence, installed software or authorisation must actually exist. A written instruction cannot manufacture it. Record a blocked gate honestly rather than asserting success.

Detailed domain specifications control their own subject. The decision register identifies superseded decisions. Acceptance tests verify requirements; they do not authorise adding features. A later file number is never permission to override another document.

For an ordinary missing implementation detail, inspect the related specifications, choose the least surprising compatible behaviour, record it, implement it and test it. Do not ask the owner another round of obvious questions. Escalate only a genuine product contradiction or a permission/licensing/access barrier that cannot be resolved safely from available evidence.

## 4. Non-negotiable product boundaries

### Platform and delivery

[C] Windows 11 x64; Ableton Live 12 is the sole required host and major version. Test the exact installed Live 12 build and record its version. Do not promise support for other DAWs, operating systems, earlier Live versions or Windows ARM.

[C] VST3 **instrument** with its own four audition sounds: **Piano, Guitar, Strings, Pad**. All four ship in the first release. They are not Ableton stock devices secretly hosted inside the plug-in.

[C] Stable product name, VST3 identifiers and installer identity across updates. `Setup.exe` and `Updater.exe` contain their required payloads locally. There is no online updater service or required runtime download.

### Musical workflow

[C] Default C major; seven chord pads in ascending scale-degree order. Include every natural, single-sharp and single-flat spelling of C, D, E, F, G, A and B in Major and Minor. That is **42 selectable key names**, not 42 distinct pitch-class collections. Enharmonic entries remain separate.

[C] Minor means natural minor for this product. Keep correct theoretical spelling internally. Display a simple enharmonic substitute where a double accidental would otherwise be needed; do not remove the selected key or globally respell legitimate single accidentals.

[C] Independent octave and inversion controls on every pad and every timeline block. Five octave positions; position 3 is the default. Optional Seventh, Sus2 and Sus4 controls are separately enabled in Settings and hidden by default.

### Timeline and audition

[C] One timeline lane, initially 8 bars, maximum 32, only 4/4. New blocks normally last one bar. Minimum block duration is one quarter note. The displayed length can be typed or stepped by one bar; adding content can auto-expand it to the maximum.

[C] Move a block by its body; resize from either edge; use a razor tool to slice at a snapped click position. No overlapping blocks. Incoming blocks replace intersecting blocks. Deletion leaves gaps. Shortening the timeline destroys material beyond the new endpoint; extending it does not resurrect that material. Undo remains available.

[C] Grid snapping is strict and configurable in Settings; slicing has its own grid setting. Both default to one quarter note, which is a quarter bar in this 4/4-only product. There is no snap-off toggle or temporary bypass.

[C] Local playback always loops the full timeline, including empty space. Host tempo governs playback and repeats. Sync follows host start/stop, **not host song position**; each host start begins at ChordCanvas's own bar 1.

[C] Repeats is for pad audition only. Its five rates are **1/8 note, 1/4 note, 1/2 note, 1 bar, 2 bars**. It defaults off. A latched repeat continues until changed/stopped as specified. It does not change blocks or MIDI export.

### State, diagnostics and design

[C] New instances and reopened Live projects start fresh. Do not add automatic restoration or crash autosave. Closing and reopening the editor of the same live instance is not a new instance; see the explicit lifecycle default in the product specification.

[C] Keep five rotating diagnostic session `.txt` logs in `%LOCALAPPDATA%\ChordCanvas\Logs\`. Settings contains a direct **Open Logs Folder** control opening Windows Explorer. Capture detailed ChordCanvas actions and best-effort crash information without monitoring unrelated computer activity.

[C] One carefully designed theme, no theme switcher. A distinctive, internally consistent interaction and visual language, seven distinct degree colours and an interface designed for repeated real work are release requirements.

## 5. Important corrections that must not be lost

1. **MIDI is not a sound bank.** Include an actual local sound engine and cleared assets or original synthesis.
2. **MIDI drag-out is not live MIDI output routing.** A native file drag supplies a Standard MIDI File to Live. Cross-host MIDI-output compatibility is not part of this product.
3. **A valid MIDI file needs structural information.** “Notes only, no metadata” excludes musical/control extras, not the file header, timing division and required End-of-Track marker. Preserve rests without adding fake notes.
4. **Octave slot 3 is not a universal scientific-pitch label.** Use explicit MIDI integers and the notation policy in the theory spec. Verify displayed naming against Live.
5. **VST3 does not expose a universal separate pause command.** Preserve start-at-bar-one semantics and validate the actual running/stopped transitions Live reports. Do not claim a pause/resume distinction that the host does not provide.
6. **Program Files is not the runtime log directory.** Install the bundle to the standard VST3 location; write mutable files per user.
7. **One-click does not bypass Windows security.** An administrator consent prompt, a file-in-use condition or an authentication boundary is real. Never disable protection, force-close an unsaved Live Set or invent credentials.
8. **A public repository is not a licence.** Resolve dependency and distribution rights; do not silently assign the owner's code a copyleft or commercial licence on the strength of “public”.

Technical sources and exact verification boundaries are in [13_VERIFIED_TECHNICAL_REFERENCES.md](13_VERIFIED_TECHNICAL_REFERENCES.md).

## 6. Execute in this order

### Phase A: establish a working, testable foundation

Read the pack. Inspect the authorised environment and GitHub identity. Record the tools and exact versions actually available. Create the repository when authorised access is available. Establish the design system and interaction model, then build an early VST3 vertical slice proving sound while the host is stopped, host-tempo access, start-at-bar-one sync, keyboard focus and native MIDI drag-out.

Specifically test whether Live 12 retains a dragged MIDI file's **trailing empty bars** using an explicit End-of-Track event. Documentation of MIDI import alone is not evidence of that behaviour. Do this before assuming the final clip-length requirement has been solved.

### Phase B: implement shared musical and editing behaviour

Build and independently verify the theory engine. Implement the timeline as deterministic commands with transaction-based undo/redo. Integrate the four sounds, audition state machine, repeats and playback scheduler. Connect the actual UI to production model code, not to a separate mock-only model.

### Phase C: complete the product, not just the feature list

Implement diagnostics, manual progression save/load, MIDI export, About, current-version changelog and packaging. Repeatedly inspect and exercise the rendered plug-in inside Live. Fix gesture conflicts, focus bugs, illegible blocks, poor hit areas, clipping and visual inconsistency as functional defects.

### Phase D: release evidence

Build Release x64 packages, validate clean installation and an in-place update from a prior test version, run the acceptance suite, retain evidence, commit and push the final source, and publish the matching release assets where rights and authorisation allow.

These phases are dependencies, not invitations to stop after each one for approval. Continue until completion or a real external gate blocks further work.

## 7. Definition of done

Do not label the job complete until all required gates in the acceptance document are met. Completion requires:

- A release VST3 that scans, loads and behaves correctly in the recorded Ableton Live 12/Windows 11 x64 environment.
- All confirmed functions implemented, all excluded functions absent, and no placeholder instruments or controls.
- Automated theory, timeline, MIDI, state and scheduling tests, plus actual host and GUI evidence.
- A fresh-install `Setup.exe` and separately named, self-contained `Updater.exe`, both tested with the same release payload and stable identity.
- Five-session logs, Explorer access, current version/changelog and safe failed-update handling.
- Public source, build scripts, assets or reproducible licensed asset acquisition, tests, installer source, release metadata and version tags in `ChordCanvas`.
- A candid final status distinguishing passed, failed, blocked and not-run checks. Never convert “not tested in Ableton” into “should work in Ableton”.

A successful build command is not sufficient evidence of a successful instrument.
