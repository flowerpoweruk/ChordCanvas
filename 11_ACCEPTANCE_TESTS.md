# ChordCanvas: acceptance tests and release evidence

**Authority:** observable proof of the specified product.  
**Status at handover:** these are required tests, **not executed results**.  
**Read with:** every domain specification, [12_DECISIONS_AND_TRACEABILITY.md](12_DECISIONS_AND_TRACEABILITY.md) and [BUILD_STATUS.md](BUILD_STATUS.md).

## 1. Evidence rules

Every test must be recorded as `NOT RUN`, `PASS`, `FAIL` or `BLOCKED`. A test whose required host or permission is unavailable is not a pass. Keep the scenario ID, exact source/build version, environment, procedure, expected result, observed result and evidence location. Record regressions and re-test after relevant changes. Do not create fake screenshots, command output, installer results or “manual verification” notes.

Evidence layers used below:

- **C:** deterministic core/unit/property tests independent of JUCE UI and Live.
- **I:** executable integration/audio/plug-in harness tests, including actual binary validation.
- **U:** real rendered UI and input interactions in Ableton Live 12 on Windows 11 x64.
- **H:** actual Live behaviour, including transport, imported clips and host lifecycle.
- **W:** actual Windows installation/update/uninstallation and filesystem behaviour.
- **R:** inspected source, dependency rights, repository/release state or security evidence.

A pure test cannot stand in for a required `U`, `H` or `W` test. Use disposable/synthetic test Sets, not the owner's unsaved work. Automated testing can drive the UI when authorised tools support it; screenshots alone do not prove a drag, note lifetime or update works.

All arithmetic examples use the specified **960 ticks per quarter note**, **3,840 ticks per 4/4 bar**, eight-bar endpoint **30,720**, and maximum endpoint **122,880**. Intervals are half-open `[start,end)`.

## 2. Environment and early feasibility gates

| ID | Scenario and required observation | Evidence |
|---|---|---|
| ENV-01 | Discover and record actual Windows architecture/build, Live 12 build, JUCE revision, compiler, SDK and CMake version. Verify x64 output rather than inferring it from the machine name. | W, R |
| ENV-02 | A minimal ChordCanvas VST3 instrument scans, appears and opens in the installed Live 12. Its audio reaches the track/master through ordinary host routing. No Max for Live component or external sound-bank download is needed. | H |
| ENV-03 | Native file drag from the plug-in into Live starts correctly and produces an importable MIDI clip. Test the actual JUCE/native implementation rather than a browser approximation. | U, H |
| ENV-04 | An eight-bar exported progression with its last note ending in bar 4 imports with the intended eight-bar clip extent, including trailing silence. Inspect Session clip loop/end markers and Arrangement placement. This is an early release-blocking gate; source documentation alone does not establish the result. | H |
| ENV-05 | Observe real host tempo, stopped/playing edges and available position flags. Start Live at a later song position and demonstrate local bar-one sync. Record what global transport actions actually report; do not claim a generic separate pause event. | H |
| ENV-06 | Build and run a small install/update vertical slice with stable VST3 identity. Updating a loaded bundle is handled safely without force-closing an unsaved Live Set. | W, H |
| ENV-07 | Validate actual filesystem/network/GitHub/GUI-control permissions and required third-party licensing authority. A blocked gate is explicitly recorded. | R |

Do these early enough to prevent a complete-looking UI from hiding a failed integration contract.

## 3. Music theory and note generation

| ID | Scenario and required observation | Evidence |
|---|---|---|
| MUS-01 | Enumerate exactly 42 key entries: seven letters × natural/sharp/flat × Major/Minor. Check unique spelled identities, including B♯, E♯, F♭ and C♭. Their collections reduce to 24 unique tonic-pitch-class/mode pairs; do not remove enharmonic entries. | C, U |
| MUS-02 | Every major scale matches `[0,2,4,5,7,9,11]`; every minor scale matches `[0,2,3,5,7,8,10]`. Theoretical letters advance once per degree, even when an internal double accidental is needed. | C |
| MUS-03 | All 294 default triads match major qualities `M,m,m,M,M,m,dim` or natural-minor qualities `m,dim,M,m,m,M,M`, with roots in ascending scale-degree order. Use independent fixtures, not only the production generator to validate itself. | C |
| MUS-04 | C Major yields C, Dm, Em, F, G, Am, Bdim; F Minor yields Fm, Gdim, A♭, B♭m, Cm, D♭, E♭. No harmonic-minor dominant is silently introduced. | C, U |
| MUS-05 | C♯ Major and D♭ Major have equivalent sounding notes/registers but preserve distinct labels; C♯ Major includes E♯m and B♯dim. C♭ Major retains valid C♭/F♭ spellings. | C, U |
| MUS-06 | G♯ Major retains its key label, stores F-double-sharp theoretically and displays the simplified seventh-degree root G. Display substitutions preserve pitch and quality; no double-accidental glyph leaks into normal labels. | C, U |
| MUS-07 | Five octave positions are 1–5, default 3. C-Major tonic root at position 3 is `[60,64,67]`; slots 1/2/4/5 shift all notes by −24/−12/+12/+24. Inspect the actual Live pitch naming separately; do not substitute ambiguous octave text for integer assertions. | C, H |
| MUS-08 | C-Major inversions at slot 3 are `[60,64,67]`, `[64,67,72]`, `[67,72,76]`. Octave and inversion operations commute in pitch result and do not reset each other's values. | C, U |
| MUS-09 | Optional sevenths follow the originating scale: Cmaj7 `[60,64,67,71]`; G7 in C Major; Bm7♭5 in C Major; Cm7 in F Minor. Check every degree in all keys. | C |
| MUS-10 | Seventh-enabled chords offer 3rd inversion; Cmaj7 gives `[71,72,76,79]`. Removing the seventh from 3rd inversion clamps to 2nd without a phantom note or invalid control state. | C, U |
| MUS-11 | Sus2 and Sus4 are exclusive, use the specified conventional `[0,2,7]` and `[0,5,7]` sets, and restore the original diatonic triad when removed. Test an originally diminished chord and an out-of-key suspension. | C, U |
| MUS-12 | Sus plus seventh uses the stored diatonic seventh and a truthful name; C in C Major is Cmaj7sus4, not C7sus4. Swapping suspension immediately changes pitches without adding both suspended tones. | C, U |
| MUS-13 | Exercise all **30,870** combinations: 42 keys × 7 degrees × 5 slots × [3 suspension states × (3 triad inversions + 4 seventh inversions)]. Pitches stay in 0–127, ordered correctly, without duplicates or unintended wrapping. | C |
| MUS-14 | Place several modified chords, then change the global key. Existing notes, names, origin context and colours remain unchanged; pads reset to the new key's defaults. Change an existing block's seventh afterwards and verify it still uses its original key. | C, U, H |
| MUS-15 | A new block inherits every current pad setting, but changing the pad afterwards does not change the placed block, and editing the block does not change the pad. Test drag and double-click add. | C, U |
| MUS-16 | Reset affects exactly one pad: root position, slot 3, no seventh/suspension. Other pads, blocks and timeline length are untouched. There is no block-reset button. | C, U |
| MUS-17 | Seventh/Sus2/Sus4 have separate Settings visibility switches, all off initially. Hiding an already-used control does not silently remove its notes; labels remain accurate. | C, U |
| MUS-18 | The notes used by audition, timeline playback and export are identical for the same semantic chord and revision. Compare integers, not just labels or a listening impression. | C, I, H |

The Cartesian test count includes suspended and unsuspended states even when their resulting pitches coincide. It is a coverage count, not a count of unique musical chords.

## 4. Timeline, editing and history

| ID | Scenario and required observation | Evidence |
|---|---|---|
| TL-01 | Fresh timeline is eight bars; stepper changes one bar at a time; typed valid integers work; minimum is one and maximum 32. Invalid/empty/fractional/out-of-range entries do not corrupt state. | C, U |
| TL-02 | A normal pad drop creates exactly one bar, snapped to the edit grid, independent of current Repeat rate. A drop requiring more visible bars auto-expands by whole bars up to 32. | C, U |
| TL-03 | Double-click a configured pad appends one block after the last block at the next valid non-overlapping grid position. It does not fill a deliberate earlier gap or add two blocks. With no blocks, it starts at tick 0. | C, U |
| TL-04 | Move a block by its body. Start changes, duration and notes do not. Drag threshold distinguishes audition/click from move. Preview and committed position agree. | C, U |
| TL-05 | Resize either edge shorter/longer. The fixed opposite edge stays fixed, musical notes stay identical and duration never becomes less than 960 ticks. Left-edge resizing does not recalculate a chord. | C, U |
| TL-06 | With A `[0,3840)` and B `[3840,7680)`, insert C `[2880,6720)`. A and B are removed completely; C retains its own 3840-tick length. No automatic ripple or retained fragments. Undo restores both originals. | C, U |
| TL-07 | Blocks sharing a boundary, such as `[0,3840)` and `[3840,7680)`, do not collide. Intervals with any positive overlap do. Test moving a block over its own old interval without treating itself as an external victim. | C |
| TL-08 | Moving/resizing one long block across several others replaces all intersected destinations atomically. Preview identifies the victims; failure leaves the original document unchanged. | C, U |
| TL-09 | Group moves preserve spacing, durations and relative notes; selected original blocks do not replace each other. Use its earliest start as the snapped anchor, preserving internal offsets after a grid change; constrain that anchor at zero/32 bars without negative starts or arbitrary reshaping. | C, U |
| TL-10 | Delete a middle block: empty space remains and later blocks stay at their positions. Leading/internal/trailing gaps are all valid. Delete/Backspace removes all and only selected blocks. | C, U |
| TL-11 | Edit and slice grids are separately configurable. Defaults are 960 ticks. At a finer edit grid, starts can use that grid while minimum duration stays 960; existing blocks are not requantised merely by changing Settings. No bypass modifier or snap-off mode exists. | C, U |
| TL-12 | Razor-click a block `[0,3840)` at tick 1920: produce `[0,1920)` and `[1920,3840)` with the exact same harmony. One undo restores the original. No forced selection of only the left piece. | C, U |
| TL-13 | Razor-click at a block edge, in empty space or at a position producing a piece under 960 ticks: no invalid split, no zero-length block, no unexpected deletion. Click resolves to the configured slice grid, not the playhead. | C, U |
| TL-14 | Resize/insert operations at the 32-bar boundary cannot extend the document beyond tick 122880. Drops wholly outside the allowed timeline are rejected without a warning cascade. | C, U |
| TL-15 | Shorten a 16-bar document to 8 bars with a crossing block `[28800,32640)` and a later block at 34560. Crossing block becomes `[28800,30720)`; later block disappears. Extend back to 16: neither removed tail nor later block reappears. | C, U |
| TL-16 | A shortening boundary that would leave a remnant shorter than 960 ticks removes that remnant, preserving the minimum-duration invariant. Undo of shortening restores the original length and full original contents in one transaction. | C, U |
| TL-17 | Shift-click and marquee selection work independently and as specified. A drag beginning over an embedded control is not a marquee; a marquee does not seek playback. Pressing an already-selected member and then dragging preserves/moves the group instead of collapsing it on mouse-down. Focus and selected styling agree with the actual selected IDs. | C, U |
| TL-18 | Copy blocks with relative offsets 0 and 7680; paste at playhead 1920. Starts become 1920 and 9600, preserving the copied internal gap. No global-key reinterpretation occurs. | C, U |
| TL-19 | Pasting onto occupied time uses the same replacement transaction as drag, including multiple victims and one undo. A selection can be pasted back over its original area without duplicate IDs. | C, U |
| TL-20 | Paste one 3840-tick chord at 120960: keep `[120960,122880)` and discard the excess. Later copied blocks starting at/after 122880 are omitted; earlier valid blocks remain. The entire paste is not rejected. | C, U |
| TL-21 | If a final pasted remnant is under 960 ticks, omit that piece; do not create an illegal short block. A paste with no valid surviving material makes no destructive change. | C |
| TL-22 | Ctrl+D places the duplicate after the selected group's extent, retaining relative gaps. Ctrl+V uses the playhead instead. Duplication follows documented clipping/collision rules near bar 32. | C, U |
| TL-23 | Undo/Redo covers add, move, both-edge resize, replacement, delete, paste, duplicate, slice, block musical changes, manual load and destructive length changes as whole transactions. Undoing a replace restores every victim and the former timeline length. | C, U |
| TL-24 | After 21 committed transactions, only the most recent 20 are undoable. Pointer motion does not consume separate levels. A new edit after undo clears the redo branch; Ctrl+Y never resurrects discarded redo state. | C, U |
| TL-25 | Ctrl+A selects timeline blocks only with appropriate focus. Text fields keep their normal select/copy/paste/delete semantics. No global keyboard interception occurs. | U |
| TL-26 | Clicking empty timeline space seeks the local snapped cursor and deselects according to the declared default; dragging empty space beyond the threshold becomes marquee rather than seeking. | U |
| TL-27 | Plus/minus and Ctrl+wheel zoom; Fit shows the complete current timeline; Shift+wheel and the scrollbar pan horizontally. These gestures are scoped to hover/focus as specified and keep hit-testing correct after zoom. | U |
| TL-28 | Bar numbers remain visible and correctly aligned at all useful zoom levels. Quarter-note blocks remain editable; small-block controls collapse to the anchored popover rather than overlapping text or resize handles. | U |
| TL-29 | Move, slice, paste or shorten during playback: audio follows a consistent new revision, removed notes are released, and visual playhead/selection references do not point at deleted blocks. No callback mutates the document directly. | I, U, H |
| TL-30 | Property/fuzz tests perform bounded random edit sequences and assert no overlap, valid IDs, valid notes, starts ≥0, durations ≥960, end ≤122880, and consistent Undo/Redo. A failed validation leaves the old snapshot intact. | C |

## 5. Audition, repeat and transport

| ID | Scenario and required observation | Evidence |
|---|---|---|
| AUD-01 | All four sounds are usable from the installed package without network access. Piano, Guitar, Strings and Pad are distinguishable timbres, not renamed copies or silent placeholders. Check all five octave slots and chords with four notes. | I, H, R |
| AUD-02 | The top instrument dropdown affects all internal audition/playback voices; the volume **dial** scales sound smoothly. Neither changes any exported MIDI note/timing/velocity/program event. | I, U, H |
| AUD-03 | Repeat off: mouse-down on a pad sounds the chord; mouse-up stops its held notes, allowing a natural release envelope. Number keys 1–7 behave identically. OS key-repeat events do not accumulate duplicate voices. | I, U |
| AUD-04 | Repeat on: one click starts indefinite repeats; releasing the button does not stop; clicking the same active pad again stops; a different pad switches immediately. Keys 1–7 use the same latch behaviour. | I, U |
| AUD-05 | Expose exactly the five approved rates: 1/8 note, 1/4 note, 1/2 note, 1 bar, 2 bars. At 120 BPM their start intervals are 0.25, 0.5, 1, 2 and 4 seconds. They share one global rate control. | C, I, U |
| AUD-06 | Repeat rate change takes effect immediately using the declared phase policy. Turning Repeat off releases its notes immediately. No fixed repeat-count limit or sustain-duration setting appears. | I, U |
| AUD-07 | Change inversion, octave, seventh or suspension while a chord sounds: the new pitches become active immediately, old pitches are safely released, and no retrigger by the user is needed. Check both held and repeat audition. | I, U |
| AUD-08 | Change global key while a pad sounds: its new key/reset voicing is used immediately according to the pad-identity policy. Existing timeline notes do not change. No orphaned old-key voices remain. | I, U |
| AUD-09 | Repeats run while Live is stopped and follow the host-reported tempo. Starting/stopping the host does not accidentally reassign a repeat to exported/timeline content. | H |
| AUD-10 | Change host tempo, including during playback/repetition. Future timing follows it without drift caused by a wall-clock UI timer. Check at least 60, 120 and 180 BPM and a supported tempo change/automation case in Live. | I, H |
| AUD-11 | Local Play begins at the local cursor, loops the full eight-bar length through empty space, and continues indefinitely. There is no loop toggle. The currently active block is highlighted and silent gaps are visibly silent. | I, U, H |
| AUD-12 | Seeking into the middle of a sounding block starts its current chord for the remaining interval; seeking into a gap is silent. Stop freezes local position; Return to Start resets it as specified. | I, U |
| AUD-13 | Sync on; Live starts at a later song position: ChordCanvas starts at local tick 0. Stop Live: ChordCanvas stops/releases and freezes visibly. Start Live again: local tick 0, not the former local position. | H |
| AUD-14 | While Live remains playing, seek/change its song position or run a host loop. ChordCanvas must not jump to that position. It keeps its local timeline phase under the current tempo. | H |
| AUD-15 | Launch a Session clip while global transport is already running. Do not pretend this is a new start edge unless the actual available host signal says so. Verify and describe the supported global-transport behaviour accurately. | H |
| AUD-16 | Toggle Sync on while Live plays: start from local bar 1. Toggle it off: stop the host-driven local playback. Local controls clearly reflect ownership; the plug-in does not issue host play/stop commands. | I, U, H |
| AUD-17 | Click a timeline block without Shift to audition its actual voicing. Clicking a control, beginning a move or resizing must not launch an unintended second preview. A split boundary retriggers notes in the specified off-before-on order. | I, U |
| AUD-18 | During timeline playback, a held pad/block preview temporarily owns the internal sound while the playhead continues; release returns to the currently active timeline chord, not the chord from before preview began. Repeat override lasts until stopped. | I, U, H |
| AUD-19 | Beginning local timeline playback clears an old latched preview per the declared ownership policy. Repeated rapid clicks/edits never stack stale voices or leave notes on indefinitely. | I, U |
| AUD-20 | Losing focus while a momentary key/mouse audition is held cleans up that hold. Closing/reopening the editor does not destroy the running session; latched repeat behaviour follows the spec rather than treating every focus loss as a stop. | I, U, H |
| AUD-21 | Host meter changes away from 4/4: timeline playback/edit timing is safely disabled as specified, with a concise unsupported-meter state. Pads remain usable; the plug-in never changes Live's meter. Return to 4/4 restores valid operation. | I, H |
| AUD-22 | Missing tempo/position validity and non-finite host values are handled without a crash/divide-by-zero. Last valid tempo/fallback behaviour is logged truthfully; ordinary Live operation must not depend on fallback. | C, I, H |
| AUD-23 | Stop, unload, prepare/release, bypass, sample-rate change and source-owner changes release the correct active notes safely. Test at 44.1/48/96 kHz and representative supported block sizes, including non-uniform process block lengths. | I, H |
| AUD-24 | Stress 32 bars of minimum-duration chords, repeats and UI edits with four-note voices. No logging/file I/O, blocking locks or uncontrolled allocation runs on the audio callback. Record measured performance and test environment rather than claiming universal zero latency. | I, H, R |

## 6. MIDI export and import into Live

| ID | Scenario and required observation | Evidence |
|---|---|---|
| MIDI-01 | File is valid SMF type 0, one track, positive 960-tick PPQ, correct chunk lengths/deltas and final End-of-Track at the requested timeline endpoint. Independently parse without auto-creating missing note-offs. | C, I |
| MIDI-02 | Put C `[60,64,67]` at `[0,3840)` and an inverted chord at `[7680,9600)`. Export exactly those pitches/on/off ticks, simultaneous chord starts, channel 1, fixed default velocity, no arpeggiation or performance-pattern data. | C, I, H |
| MIDI-03 | Preserve leading, internal and trailing silence in file time. A silent interval is not removed because the last block is earlier than the endpoint. No dummy audible note or velocity-zero filler is added. | C, H |
| MIDI-04 | Adjacent chords sharing a pitch and razor-split identical chords have note-offs before note-ons at the shared tick. Reimport does not merge them into an unintended single sustained note. | C, H |
| MIDI-05 | File includes no CC, program change, pitch bend, aftertouch, instrument-selection data, tempo map, time-signature event or track-name text. The necessary SMF header/division/EOT remain; “no metadata” does not mean an invalid file. | C |
| MIDI-06 | Global key changes, instrument choice, volume, Repeat rate and preview activity do not alter stored timeline export. All valid octave/inversion/seventh/sus states export their actual current notes. | C, I |
| MIDI-07 | Drag exports the **whole current timeline** as one clip, not just selected blocks. User-visible filename is `chords.mid`; no filename prompt. Concurrent/repeated drags use safe private unique directories. | U, H |
| MIDI-08 | Starting an export takes one consistent timeline snapshot. Editing during file preparation/drag cannot create a file mixing old starts with new notes or a stale endpoint. | I, U |
| MIDI-09 | Native external drag is distinct from moving internal blocks. Use the dedicated export handle/region. Copy semantics do not delete internal blocks or the only file before Live consumes it. | U, H |
| MIDI-10 | Drop into a Session MIDI clip slot and into an Arrangement MIDI track area. Inspect actual imported pitches, durations, start offset, full clip length and audible playback with a user-selected instrument. Test repeat drags and cancellation. | H |
| MIDI-11 | Reopen a Live Set containing an imported clip after temporary export cleanup: clip notes remain. This is separate from ChordCanvas's deliberately fresh project state. | H |
| MIDI-12 | Export an empty eight-bar timeline according to the valid-empty-file policy. Verify actual Live handling. If Live cannot create the promised silent clip, record the incompatibility explicitly rather than inserting fake notes or claiming success. | C, H |
| MIDI-13 | Unwritable export directory, failed write and native-drag failure preserve document state and show a simple error. Native-drag callback completion is not misreported as confirmed Live import success. | I, U |
| MIDI-14 | A full 32-bar/minimum-block-density export has valid sorted events, balanced notes, no data beyond tick 122880 and exact host playback duration. Compare independent parser results with Live. | C, H |

`ENV-04`, `MIDI-10` and `MIDI-12` must be investigated early. A generic statement that MIDI supports timestamps is not proof of the host's clip-boundary interpretation. Do not silently substitute a manual “resize the clip yourself” workflow for the confirmed complete-timeline drag requirement.

## 7. State, settings and manual save/load

| ID | Scenario and required observation | Evidence |
|---|---|---|
| STATE-01 | Fresh processor: C Major, slot 3/root pads, empty eight bars, optional modifications hidden, Repeat off, Sync off and all declared global defaults. | C, H |
| STATE-02 | Edit timeline/settings; save and reopen the Live Set. ChordCanvas starts fresh by design, with no recovered timeline or automatic last-preset load. Other tracks/clips in the test Set are not altered. | H |
| STATE-03 | Close and reopen only the editor in the same live processor. Current timeline, controls and playback remain; editor construction does not reset the session. | U, H |
| STATE-04 | Host requests state/bypasses/reactivates/changes audio setup during an existing session: valid minimal host state handling does not accidentally erase the document. New processor restoration still starts fresh. | I, H |
| STATE-05 | Manual Save/Load is available, with no factory presets/templates/example projects. Save an exact progression, start fresh, explicitly Load, and compare every block's semantic notes, origins, timing and length. | C, U |
| STATE-06 | Manual Load replaces the timeline in one undoable transaction but does not unexpectedly replace current instrument/key or add auto-restore. Retain the defined global-settings boundary. | C, U |
| STATE-07 | Corrupt/oversized/unsupported-schema/invalid-note/overlapping/out-of-range saved data is rejected atomically with a simple error. No partial load, arbitrary file execution or silent repair that changes music. | C, I, U |
| STATE-08 | No periodic autosave, crash-recovery prompt, automatic restore or session-state persistence is hidden in logs. Logs may describe private state but are never loaded as a recovery feature. | R, H |
| STATE-09 | About shows the running binary's version and current-version changes only, including after an actual update. It does not show the entire historical release changelog. | U, W |

## 8. GUI, UI and UX acceptance

| ID | Scenario and required observation | Evidence |
|---|---|---|
| UX-01 | Inspect a real running build with seven pads, populated timeline and Settings. ChordCanvas has a deliberate identity, coherent type/spacing/control language and one theme. No generic dashboard/cards, placeholder controls, arbitrary gradients/glass or copied Ableton skin. Record concrete defects and fixes. | U, R |
| UX-02 | Core workflow is clear: choose key → audition/modify → add/move/resize/slice → play → drag MIDI. Primary operations do not require unexplained hidden gestures. The design was iterated during implementation, not added only after functionality. | U, R |
| UX-03 | All seven degree colours are distinct and inherited by blocks. Selection, playback, repeat activity, disabled state and errors are not conveyed by colour alone or confused with degree colours. Labels remain readable. | U |
| UX-04 | Long/unusual key names, diminished/seventh/sus labels, slot/inversion controls and narrow blocks remain legible. No clipping, colliding click targets, missing accidental glyphs or label changes that falsify notes. | U |
| UX-05 | Test actual Windows scaling at 100%, 125%, 150% and 200%, supported editor sizes, minimum size and resize transitions. Text, hit regions, popovers, native menus and export drag line up with their rendered controls. | U |
| UX-06 | Embedded controls beat edge/body hit regions; razor beats body audition; resize handles remain reachable. Double-click-to-add does not add two blocks, and repeat/held previews do not leave stuck notes after a drag. | U, I |
| UX-07 | Small-block menu opens an anchored popover with all enabled controls, remains on-screen and closes predictably. Editing it changes only its associated block; selection and playhead do not jump unexpectedly. | U |
| UX-08 | Hover/pressed/selected/playing/repeating/disabled/focus/error states are all implemented consistently. Destructive replacement and snapping have truthful previews; invalid operations do not merely change the cursor then fail silently. | U |
| UX-09 | Keyboard focus is visible; number keys and Ctrl shortcuts are scoped to the plug-in/document, not intercepted while typing or using Live elsewhere. No rejected global Escape panic behaviour is added. | U, H |
| UX-10 | Instrument dropdown, volume dial, Repeat controls, length stepper, local transport/Sync, export handle and separate Settings are discoverable without oversizing everything. Button labels match actions. | U |
| UX-11 | Test contrasting states numerically and visually. Engineering targets are 4.5:1 for ordinary text and 3:1 for important non-text controls where applicable; do not call this a formal accessibility certification. | U, R |
| UX-12 | Record a complete interaction test with a dense 32-bar progression, undo after destructive actions, narrow blocks and a native drag to Live. No UI-only mockup is accepted as proof. | U, H |

A visual review is a release gate, not an optional “nice to have”. A technically functioning interface with unresolved clipped controls or ambiguous destructive gestures fails.

## 9. Diagnostics and crash evidence

| ID | Scenario and required observation | Evidence |
|---|---|---|
| LOG-01 | Normal non-administrator Live use creates readable UTF-8 session `.txt` under the resolved LocalAppData ChordCanvas Logs folder, not Program Files. | W, H |
| LOG-02 | Settings → Open Logs Folder opens that exact directory in Explorer, including paths with spaces/non-ASCII characters. Missing folder is created safely; errors are useful. | U, W |
| LOG-03 | Run six ordinary interactive sessions: only the newest five session files remain. Multiple plug-in instances share the chosen process session with distinct IDs; editor reopening does not rotate. | W, H |
| LOG-04 | Concurrent host sessions, scan processes and more-than-five-active-file conditions follow the documented policy without deleting active logs or silently exceeding the limit. | I, W |
| LOG-05 | Reconstruct a synthetic replace → resize → undo → export-error sequence from timestamps, transaction IDs, revisions and snapshots. A parser can link the action and actual error; logs are not just “something went wrong”. | I, R |
| LOG-06 | Enforce bounded queue/storage behaviour with explicit dropped/truncated markers and preserved recent state. No synchronous file/formatting/rotation work occurs on the audio callback. | I, R |
| LOG-07 | Disk full/denied access/clock change do not crash or spam dialogs; logging degradation is visible and monotonic event ordering remains understandable. | I, W |
| LOG-08 | Inject a recoverable test error: log build ID, state, operation and available exception context. Preserve valid document/audio state and show the required simple error once. | I, U |
| LOG-09 | In an isolated host test, examine a controlled fatal/abrupt termination and a normal shutdown. Preserve pre-fault evidence; distinguish unclean termination from a confirmed ChordCanvas crash. Record what could and could not be written. | I, H |
| LOG-10 | Review any stack/minidump integration for host-handler safety and documented limitations. No process-wide handler takeover, unsolicited service, public dump upload or guaranteed-recovery claim. | R, H |
| LOG-11 | Audit logs and Git diff for passwords, tokens, unrelated keystrokes/audio, user names/paths unnecessarily exposed and private project content in public evidence. Sanitised synthetic fixtures are the only public examples. | R |
| LOG-12 | Updating/uninstalling application binaries does not unintentionally delete retained logs or user-saved progression files. Runtime logging remains writable after update. | W |

## 10. Packaging and releases

| ID | Scenario and required observation | Evidence |
|---|---|---|
| REL-01 | Clean Windows 11 x64 install using `Setup.exe` reaches the standard global VST3 path with complete bundle/assets/dependencies. No compiler/JUCE/Python/network fetch is needed by the user. | W |
| REL-02 | Run Live as a normal user after installation. Scan/rescan as necessary and use all four sounds, editing, export and logs. Installer success alone is not plug-in acceptance. | W, H |
| REL-03 | A self-contained `Updater.exe` updates an existing older ChordCanvas install in place. Same identity/path, new running version/current changelog, no duplicate browser entry or second product installation. | W, H |
| REL-04 | Preserve logs, explicitly saved progressions and permitted user-owned files through the update. Do not add automatic project/settings persistence that contradicts fresh starts. | W |
| REL-05 | Updater on a machine without ChordCanvas reports that Setup is required; same-version, downgrade, newer-installed and invalid-install cases follow the documented version policy without destructive guessing. | W |
| REL-06 | Live has an unsaved test Set and the plug-in bundle is in use. Setup/Updater requests a safe close/retry or refuses; it never force-closes Live or overwrites a loaded binary unsafely. | W, H |
| REL-07 | Failed/cancelled/permission-denied/disk-full/incomplete-payload update leaves the old version usable or restores it safely. Test rollback and version metadata consistency, not just a success path. | W |
| REL-08 | Both executables work offline from their own payload. No background update service or automatic online launch check is installed. | W, R |
| REL-09 | Windows elevation is requested only for installation needs; no runtime admin requirement or weakened ACL. Signing is truthful and uses only authorised credentials; do not promise absence of Windows warnings. | W, R |
| REL-10 | Inspect executable architecture, bundle layout, required runtimes, hashes, stable identifiers and uninstall registration. Reinstall/upgrade/uninstall does not affect another vendor's plug-ins. | W, R |
| REL-11 | About/version metadata, tag, changelog and binary all identify the same release/source commit. Both Setup and Updater are supplied for that release and have been tested as packaged artifacts. | W, R |
| REL-12 | After ordinary uninstall, intended application-owned binaries are gone while user documents/logs are preserved according to the declared policy. Reinstall produces one valid product, not stale duplicate registrations. | W, H |

## 11. Repository, scope and completion

| ID | Scenario and required observation | Evidence |
|---|---|---|
| REPO-01 | Actual authenticated owner's account contains a public repository named exactly ChordCanvas. Existing unrelated repositories/work were not overwritten; no imaginary owner/URL is reported. | R |
| REPO-02 | Meaningful source iterations were committed and pushed during development. Source, assets rights, build/installer scripts, tests and design revisions needed to rebuild are present. | R |
| REPO-03 | Pinned dependencies/build definitions produce the intended release from the tagged source in an appropriate clean Windows environment. Public source does not depend on an undisclosed local file. | I, W, R |
| REPO-04 | Release points at the tested commit and contains actual Setup.exe, Updater.exe and checksums. Downloaded/published bytes match tested artifacts. | R |
| REPO-05 | Owner/project and dependency/asset licensing gates are resolved before public binary distribution. A public repo alone is not treated as an open-source licence grant. Private material/secrets/dumps are absent. | R |
| REPO-06 | BUILD_STATUS records actual results, versions, commands/evidence and remaining gates. No claim of full completion while mandatory host/install/UI tests are unrun or failed. | R |
| SCOPE-01 | Inspect the UI, source feature registration and Settings against the exclusions in the product spec: no vocal import/analysis/waveform, BPM field, prediction hints or other withdrawn features. | R, U |
| SCOPE-02 | No Clear Timeline button, loop toggle/region, snap-off, ripple reorder, lock, MIDI input mapping/import, arpeggiator/strum/humanise, transpose, extra voicing or velocity controls were silently added. | R, U |
| SCOPE-03 | No factory templates, automatic session recovery, end-user manual/README package, online update service, telemetry or theme switcher. Engineering specs and legally necessary notices remain permitted. | R, W |
| SCOPE-04 | Final source-to-product review confirms that all confirmed requirements have implementation and test evidence; declared defaults have not been misrepresented as explicit user decisions. | R |

## 12. Release decision and result record

Before a release, produce a machine-readable or consistently structured result record alongside readable summaries. For each test include:

```text
Test ID:
Status: NOT RUN / PASS / FAIL / BLOCKED
Product version and source commit:
Binary/package hash where relevant:
Windows / Live / toolchain versions:
Procedure actually performed:
Expected result:
Observed result:
Evidence path (sanitised if public):
Defect or limitation:
Retest result and date:
```

This is a result schema, not a filled-in claim. Do not paste invented values into it. Keep private host recordings, diagnostic files and local paths out of public results.

A release requires all mandatory confirmed-behaviour tests to pass. Declared best-effort crash evidence must be assessed against its honest bounded contract, not an impossible guarantee that every hard kill produces a dump. Any unresolved contradiction in Live import/transport must be reported as a real gate, not concealed by changing the expected result after testing.

The final review must exercise the **installed release binaries**, not only a debug build in a development directory. Re-run the applicable subset after every updater release and the complete high-risk regression suite when theory, timing, state or packaging changes.
