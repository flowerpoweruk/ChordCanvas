# ChordCanvas: decision reconciliation and requirement traceability

**Authority:** which conversational decisions are current, which were superseded, and which details are explicit engineering resolutions.  
**Basis:** the supplied written conversation, including its transcribed voice-chat turns. This is not a claim to have separately listened to unavailable recordings.  
**Read with:** [00_MASTER_BRIEF.md](00_MASTER_BRIEF.md), [01_PRODUCT_SPEC.md](01_PRODUCT_SPEC.md) and [11_ACCEPTANCE_TESTS.md](11_ACCEPTANCE_TESTS.md).

## 1. Interpretation rules

The conversation was exploratory and contained many repetitions and reversals. An earlier “yes” is not binding when the user later clearly changed the decision. An assistant's suggestion, reassurance or statement that something had been checked is not evidence that the user approved it or that a host test actually occurred.

Apply the **latest explicit user decision on the same subject**. Preserve unrelated earlier accepted requirements. Do not infer a new requirement from a transcription fragment, an unanswered suggestion or the assistant's “obviously” question. Only the accepted final scope belongs in the product.

The labels throughout this pack are:

- **[C] Confirmed:** explicit final user decision or direct consequence.
- **[D] Engineering default:** a transparent proposed implementation resolution, not a claimed user instruction.
- **[T] Technical correction:** an earlier conversational explanation was inaccurate or incomplete.
- **[G] Verification/permission gate:** an outcome or authority must be established in the real environment.

Numbers in filenames do not establish precedence. The subject-owning document controls implementation detail; this register resolves conversation history. A test cannot silently enlarge scope. Record a genuine unresolved contradiction before building around it.

## 2. Superseded decisions: do not resurrect these

| Earlier discussion | Final controlling decision | Owner |
|---|---|---|
| Max for Live preferred; VST versus Max undecided | VST3 **instrument**, JUCE/C++, Windows 11 x64, Ableton Live 12 only | 00, 07 |
| Four-bar new chord, then one bar | **One-bar** normal new chord | 04 |
| Sixteen-bar initial timeline | **Eight bars** initially | 04 |
| Potential unlimited timeline | **32-bar maximum**, auto-expand up to that limit | 04 |
| Drag to set total bar count | Editable integer with up/down stepper, one bar per step | 03, 04 |
| Snap toggle/free placement suggested and accepted early | Latest explicit decision: **strict grid**, configurable in Settings, no bypass | 04 |
| Split at playhead | **Razor tool**, click the block at a separately configurable snapped position | 04 |
| Keep destination duration when replacing a block | Incoming block **keeps its original duration**, replaces multiple intersected blocks if needed | 04 |
| Automatically insert/reorder and push later blocks | Later collision decisions use **replacement**, no ripple; deletion leaves gaps | 04 |
| Clear Timeline initially accepted | Later explicitly rejected: **no Clear Timeline button** | 01, 03 |
| Loop toggle accepted | Later **always loop** full timeline; no toggle | 05 |
| Timeline ruler/bar numbers rejected early | Later explicitly accepted: **bar numbers visible** | 03, 04 |
| Host sync at matching song position | Sync starts the plug-in at **its own bar 1**, not the host position | 05 |
| General pause/follow wording | Standard host stopped/playing edges; no invented universal pause event | 05 |
| Manual vocal BPM | Removed with vocal import; playback/repeats **always follow host tempo** | 01, 05 |
| Vocal import, waveform, key detection, open-source analysis stack | **All removed**, including the associated future technical-stack decision | 01 |
| Generic likely-next-chord highlighting | Replaced by vocal compatibility hints, then removed with vocal-dependent features; **neither hint system remains** | 01 |
| Velocity, metronome, performance patterns/humanisation | Rejected; do not add through a sound-engine control panel | 01, 05 |
| Four “MIDI stock instruments” | User accepted an **included local sound engine** with Piano/Guitar/Strings/Pad | 05, 07 |
| Volume slider | **Dial** | 03, 05 |
| Global octave control suggested | **Per-pad and per-block octave**, five positions, middle slot 3 | 02 |
| Seventh/Sus2/Sus4 briefly described as removed | **Not removed**; each hidden unless separately enabled in Settings | 02, 03 |
| Add9 | Rejected | 01 |
| Repeat options with duplicate note/bar durations | Final approved rates: **1/8, 1/4, 1/2 note, 1 bar, 2 bars** | 05 |
| Repeats as possible timeline pattern | **Audition only**; dragging ignores Repeat settings | 05, 06 |
| Fixed-duration momentary audition | Repeat off: **only while mouse/key held** | 05 |
| Accumulating logs in the VST3 installation folder | **Five session logs in LocalAppData**, direct Explorer link in Settings | 09 |
| Automatically save/restore after crash, briefly accepted | Explicitly withdrawn: **no autosave or crash restoration** | 01, 07, 09 |
| Restore timeline/settings when opening a Live project | Repeatedly rejected: **fresh start** | 01, 07 |
| Online version checking implied | Not requested; **manual self-contained offline Updater.exe** | 08 |
| Changelog might show full history | **Current version only** in About | 08 |
| Possible documentation/readme | **No end-user docs/manual**, engineering evidence and legal notices still permitted | 00, 10 |
| Guarantee MIDI out across DAWs | Unverified and unnecessary: product is internal sound plus **file drag-out**, Live 12 is sole host target | 06, 07 |
| Harmonic/melodic-minor options explored | **Natural minor only**, seven standard triads for that scale | 02 |
| Always use double-accidental notation | Correct theory internally; **simplify double accidentals in display** | 02 |
| Multiple/unknown themes | **One** theme; best implementation delegated, distinct own identity | 03 |

## 3. Important retained requirements that are easy to lose

Manual Save/Load progressions was explicitly accepted before a later question about “presets and templates” offered either none or an example project. The most conservative reconciliation is **retain user-created Save/Load but ship no factory presets/templates/example project**. This pack declares that interpretation instead of quietly equating it with automatic state persistence. It is separate from the confirmed fresh-start/no-autosave requirement.

Keyboard 1–7 audition remains accepted even though external MIDI-keyboard mapping was rejected. Multi-select, copy/paste and duplicate remain accepted even though modifier-drag duplication and arrow nudging were rejected. Twenty-step undo/redo remains accepted even though an earlier “unlimited/proper history” suggestion was rejected. Bar numbers remain accepted despite an earlier ruler rejection.

The four sounds are an actual first-release requirement, not a mockup option or one-sound MVP. Their instrument choice and volume are global, but every pad/block has independent musical voicing. A placed block must not be reinterpreted by later global key changes.

Updates preserve user-owned files and logs. That requirement does not override the deliberate fresh-start behaviour of a new plug-in instance or reopened Live Set.

## 4. Explicit defaults and technical resolutions

These details were not all specified by the user. They are surfaced so Codex does not silently invent contradictory behaviour. Implement them as written unless the subject-owning document permits a proven equivalent refinement. A necessary deviation must preserve confirmed behaviour and be recorded with evidence.

| ID | Category | Resolution and reason | Owner |
|---|---|---|---|
| D01 | D | One restrained dark theme is the starting direction, not a universal claim that dark interfaces are healthier. The user delegated the best single theme. Refine one coherent palette rather than adding a switcher. | 03 |
| D02 | D | Proposed editor dimensions and spacing/type tokens are working design values; actual usability/DPI review controls refinement. They are not user-mandated pixels. | 03 |
| D03 | T/D | Slot 3 is the centre of five positions. To remove ambiguous “C3” naming, default C tonic is **MIDI 60**; verify Live's displayed naming, keep MIDI integers authoritative. | 02 |
| D04 | D | Ascending pad roots run upward from MIDI `60 + tonic pitch class`. Enharmonic key names share the sounding register. | 02 |
| D05 | D | Optional conventional suspensions use a perfect fifth, including on an originally diminished degree. They can contain non-key tones; removing them restores the diatonic triad. | 02 |
| D06 | D | Removing a seventh from third inversion clamps to second. Hiding optional controls does not alter already-selected modifications. | 02 |
| D07 | D | Pads retain a prominent root label with a compact quality/modification cue so unlike harmonies are not visually indistinguishable. No rejected note-info panel is added. | 03 |
| D08 | D | Musical time is integer 960 PPQ ticks, half-open intervals. Edit/slice grid menu includes 1/16, 1/8, 1/4, 1/2 note and 1 bar; both default to 1/4 note, minimum block length stays 1/4 note. | 04 |
| D09 | D | Manual timeline length minimum is one bar. Auto-expansion rounds up to a whole bar, maximum 32. Invalid typed values preserve the previous valid state. | 04 |
| D10 | D | Replacement removes whole intersected destination blocks, not retained fragments. One transaction includes insertion, victims and length effects; preview shows consequences. | 04 |
| D11 | D | Double-click appends at/after the last block on the next valid grid position; it does not backfill deliberate gaps. An explicit export handle separates file drag from block movement. | 03, 04, 06 |
| D12 | D | Group moves snap their earliest-start anchor and preserve relative geometry, including existing internal off-grid offsets after a grid change. Pressing an already-selected member preserves the group for dragging. Pasting may clip at 32 bars; fragments below the confirmed quarter-note minimum are omitted. | 04 |
| D13 | D | Ctrl+A is a scoped editing convenience, not an explicitly accepted voice-turn requirement. Empty click seeks/deselects; an actual empty-space drag selects without seeking. | 04 |
| D14 | D | A split of a selected block selects both resulting pieces; otherwise neither is automatically selected. Both retain harmony and articulate at the split. | 04, 05 |
| D15 | D | Undo history covers document transactions, not every pointer frame or global sound/viewport preference. Maximum 20; new edits invalidate redo. | 04 |
| D16 | D | Manual Save/Load uses a versioned private progression format, replaces the timeline atomically and is undoable. It does not load global sound/preferences or enable automatic restoration. | 01, 07 |
| D17 | D | A new processor starts fresh; reopening the editor of the same running processor preserves its session. Ordinary host state callbacks do not erase an active document. | 01, 07 |
| D18 | D | Piano is the initial sound, a conservative fixed level is chosen and tested; Sync defaults off; selected Repeat rate defaults to 1/4 note while Repeat itself is off. | 01, 05 |
| D19 | D | New pad or rate during Repeat takes effect immediately and resets repeat phase; voicing edits change notes immediately without a new rate phase. Internal gate/release is fixed, no user duration control. | 05 |
| D20 | D | A momentary preview owns the internal sound while the timeline cursor keeps moving; releasing returns to the currently active timeline chord. Repeat latch has the same ownership until stopped. | 05 |
| D21 | D | Local Stop freezes the cursor; Return to Start resets it. Sync on while host playing starts local bar 1; Sync off stops host-driven playback. Host-owned local transport controls are disabled/clearly indicated. | 05 |
| D22 | T | Host stopped/playing flags do not distinguish a universal “pause” command. Every observed stopped-to-playing edge starts local bar 1; continuously playing host seeks are ignored. | 05 |
| D23 | D | Non-4/4 host meter disables timeline timing/playback with a clear state rather than reinterpreting bars or changing the host. Missing tempo uses last valid value, with declared fallback only if none exists. | 05 |
| D24 | T/D | Export is type-0 SMF, one track, 960 PPQ, fixed channel 1/velocity 100. Necessary structural header/EOT are retained; optional musical metadata and controllers are not. | 06 |
| D25 | D/G | External drag uses a private immutable `chords.mid` snapshot. Complete clip extent, particularly trailing silence and an empty timeline, must be proven in actual Live, not guessed from SMF theory. | 06 |
| D26 | D | Global default VST3 location, stable IDs and one installer identity; Updater is a separate entry point even if the packager shares full-payload machinery with Setup. No online service is implied. | 08 |
| D27 | D | Five logs are shared by instances in an interactive host-process session. Scans/editor reopen do not consume sessions; active files are not destroyed by concurrency. Storage/queue caps explicitly mark loss/truncation. | 09 |
| D28 | T | Native crash reporting is best effort. Preserve pre-fault evidence without hijacking Ableton's handlers; do not claim every crash produces a final report. | 09 |
| D29 | D/G | Main-branch workflow, pinned dependencies, public release assets and actual commit evidence support the requested continual GitHub updates. Owner/authentication/project licence cannot be invented. | 07, 10 |
| D30 | T/G | “One click” means a self-contained minimal-interaction package, not bypassing Windows consent, software licensing, host locks or account permissions. | 08, 10 |

## 5. Requirement-to-test map

Each row identifies a retained requirement and the tests that provide its principal evidence. This is a navigation map, not a replacement for the detailed specs or their complete test sets.

| Requirement | Final behaviour | Spec | Acceptance IDs |
|---|---|---|---|
| REQ-01 | ChordCanvas product name and VST3 instrument identity | 00, 07 | ENV-02, REL-10, REPO-01 |
| REQ-02 | Windows 11 x64 / Live 12 only | 00, 07 | ENV-01, ENV-02, REL-02 |
| REQ-03 | Autonomous complete build with actual evidence | 00, 10 | ENV-07, REPO-06 |
| REQ-04 | Public owner repository, exact name, ongoing upload | 10 | REPO-01, REPO-02 |
| REQ-05 | Reproducible source, packages, tags/releases | 07, 08, 10 | REPO-03, REPO-04, REL-11 |
| REQ-06 | C Major default / seven scale-ordered pads | 01, 02 | MUS-03, MUS-04, STATE-01 |
| REQ-07 | All 42 spelled keys, separate enharmonics | 02 | MUS-01, MUS-05 |
| REQ-08 | Verified theory, natural minor only | 02 | MUS-02, MUS-03, MUS-04 |
| REQ-09 | Correct spelling with double-accidental substitution | 02 | MUS-05, MUS-06 |
| REQ-10 | Per-pad/per-block independent inversions | 02 | MUS-08, MUS-10, MUS-15 |
| REQ-11 | Five octave positions, middle 3 | 02 | MUS-07, MUS-08, MUS-13 |
| REQ-12 | Optional diatonic sevenths | 02 | MUS-09, MUS-10 |
| REQ-13 | Optional exclusive Sus2/Sus4, seventh allowed | 02 | MUS-11, MUS-12 |
| REQ-14 | Optional feature switches separately off by default | 01, 02, 03 | MUS-17, STATE-01 |
| REQ-15 | Block inherits pad, then independent | 02, 04 | MUS-15, TL-02, TL-03 |
| REQ-16 | Key change resets pads, never placed blocks | 02 | MUS-14, AUD-08, MIDI-06 |
| REQ-17 | Pad-only reset | 02, 03 | MUS-16 |
| REQ-18 | Eight-bar default, 32-bar cap, 4/4 | 04, 05 | TL-01, TL-14, AUD-21 |
| REQ-19 | One-bar additions, quarter-note minimum | 04 | TL-02, TL-05, TL-13 |
| REQ-20 | Editable/stepped bar count and auto-expansion | 03, 04 | TL-01, TL-02, TL-14 |
| REQ-21 | Body move / resize both edges | 03, 04 | TL-04, TL-05 |
| REQ-22 | Non-overlap, original-length replacement, multiple victims | 04 | TL-06, TL-07, TL-08 |
| REQ-23 | Gaps allowed/deletion does not ripple | 04 | TL-10, AUD-11 |
| REQ-24 | Destructive shortening, extension does not restore | 04 | TL-15, TL-16 |
| REQ-25 | Strict configurable edit grid | 04 | TL-11 |
| REQ-26 | Razor click, separate configurable slice grid | 04 | TL-12, TL-13 |
| REQ-27 | Shift/marquee multi-select and delete | 04 | TL-10, TL-17 |
| REQ-28 | Copy/paste at playhead preserving offsets | 04 | TL-18, TL-19 |
| REQ-29 | Partial paste/clipping at maximum boundary | 04 | TL-20, TL-21 |
| REQ-30 | Duplicate immediately after selection | 04 | TL-22 |
| REQ-31 | Ctrl+Z / Ctrl+Y, 20 whole undo levels | 04 | TL-23, TL-24 |
| REQ-32 | Double-click pad addition | 03, 04 | TL-03, UX-06 |
| REQ-33 | Playhead, seeking, active-chord highlight | 03, 05 | AUD-11, AUD-12 |
| REQ-34 | Bar numbers, zoom/Fit/Ctrl-wheel | 03, 04 | TL-27, TL-28 |
| REQ-35 | Shift-wheel horizontal scrolling / scrollbar | 03, 04 | TL-27 |
| REQ-36 | Block-local controls and anchored small-block menu | 03 | TL-28, UX-07 |
| REQ-37 | Four real included audition timbres | 05 | AUD-01, REL-02 |
| REQ-38 | Global instrument dropdown and volume dial | 03, 05 | AUD-02, UX-10 |
| REQ-39 | Momentary pad/key 1–7 audition | 05 | AUD-03, AUD-20 |
| REQ-40 | Repeat latch/switch/stop, default off | 05 | AUD-04, AUD-06, STATE-01 |
| REQ-41 | Five approved global repeat rates | 05 | AUD-05, AUD-06 |
| REQ-42 | Immediate voicing/rate changes | 05 | AUD-06, AUD-07 |
| REQ-43 | Host tempo, independent audition transport | 05 | AUD-09, AUD-10 |
| REQ-44 | Repeat never copied to timeline/export | 04, 05, 06 | TL-02, MIDI-06 |
| REQ-45 | Local playback loops complete timeline, no toggle | 05 | AUD-11, SCOPE-02 |
| REQ-46 | Sync start/stop, local bar-one restart, no position chase | 05 | AUD-13, AUD-14, AUD-15, AUD-16 |
| REQ-47 | Direct block click audition without modifier | 03, 05 | AUD-17, UX-06 |
| REQ-48 | Whole-timeline drag to a single clip | 06 | MIDI-07, MIDI-09, MIDI-10 |
| REQ-49 | Exact note pitches, timings, lengths and silence | 06 | MUS-18, MIDI-02, MIDI-03, MIDI-14 |
| REQ-50 | Notes only except required file structure | 06 | MIDI-01, MIDI-05 |
| REQ-51 | Default `chords.mid`, simple export errors | 06 | MIDI-07, MIDI-13 |
| REQ-52 | Fresh reopened projects, no autosave/restore | 01, 07 | STATE-01, STATE-02, STATE-08 |
| REQ-53 | User Save/Load, no factory templates | 01, 07 | STATE-05, STATE-06, STATE-07 |
| REQ-54 | Offline Setup.exe and self-contained Updater.exe | 08 | REL-01, REL-03, REL-08 |
| REQ-55 | Standard VST3 location, stable in-place update identity | 07, 08 | REL-03, REL-10 |
| REQ-56 | Preserve user files and diagnostics | 08, 09 | LOG-12, REL-04, REL-12 |
| REQ-57 | About version/current changelog only | 08 | STATE-09, REL-11 |
| REQ-58 | Five detailed timestamped diagnostic text sessions | 09 | LOG-01, LOG-03, LOG-05 |
| REQ-59 | LocalAppData logs, direct Explorer link | 09 | LOG-01, LOG-02 |
| REQ-60 | Best-effort crash context, no host/real-time damage | 07, 09 | LOG-06, LOG-08, LOG-09, LOG-10 |
| REQ-61 | Own design language, one theme, interaction-first | 03 | UX-01, UX-02, UX-12 |
| REQ-62 | Seven distinct degree colours with readable states | 03 | UX-03, UX-04, UX-11 |
| REQ-63 | Coherent gesture ownership/focus/scaling | 03 | UX-05, UX-06, UX-07, UX-09 |
| REQ-64 | No withdrawn features or end-user doc package | 01 | SCOPE-01, SCOPE-02, SCOPE-03 |
| REQ-65 | Safe rights/permissions/private-data handling | 07, 08, 09, 10 | ENV-07, LOG-11, REL-09, REPO-05 |
| REQ-66 | Complete real host/installer/UI validation, honest status | 00, 11 | REPO-06, SCOPE-04 |

## 6. Specific gates that remain for the implementation environment

This handover resolves ordinary implementation detail but does not fabricate answers to facts that require the owner's machine or permissions:

**Host import:** actual trailing silence/full clip extent and empty-timeline handling in Live 12. Required tests must prove them. Do not quietly add dummy notes, unwanted musical metadata, or a manual clip-resize instruction to disguise a failure.

**Host transport:** actual stopped/playing notifications, tempo availability and Session/global-transport behaviour. Preserve bar-one restart/no-position-chase without pretending to observe a nonexistent pause event.

**Environment and access:** installed Live/build tools, actual UI automation, account identity, GitHub authentication, administrator consent and authorised code-signing facilities.

**Rights:** project distribution licence and dependency/sample redistribution obligations. “Public GitHub” does not choose an open-source licence on the owner's behalf. Use original sound synthesis/cleared assets; do not bundle Ableton content.

**Crash safety:** demonstrate which faults can be reported without taking over the host. Do not make binary promises about recovering from arbitrary native corruption.

These gates are implementation work or genuine permission decisions, not excuses to ask the user to reconfirm settled features.

## 7. Handover maintenance

When the user later changes a requirement, edit the subject-owning spec, this register, affected acceptance tests and relevant implementation together. Retain a brief revision note in source history. Do not patch only the master brief and leave contradictory detailed documents behind.

`BUILD_STATUS.md` can report progress and blocked tests, but cannot overrule a requirement. A source reference can justify a technical correction, but cannot silently authorise a product change. Before every release, re-run the scope review so rejected features do not return as a library default or a convenience added during debugging.
