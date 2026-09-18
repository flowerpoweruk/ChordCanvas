# ChordCanvas: product specification

**Authority:** Product scope, terminology, lifecycle and default state.  
**Read with:** [Master brief](00_MASTER_BRIEF.md), [decision register](12_DECISIONS_AND_TRACEABILITY.md).

## 1. What the product is

[C] ChordCanvas helps an Ableton Live user construct chord progressions by listening and manipulating visible chord blocks. It is especially useful alongside a vocal already playing in Live, but it never imports the vocal. The human chooses the harmony. There is no AI model, recommendation engine or automatic composition feature in the released scope.

Its primary workflow is:

1. Select a Major or Minor key at the top.
2. Audition any of seven chord pads and adjust that pad's inversion or octave.
3. Drag a pad into the timeline, or double-click to append it.
4. Move, resize, slice, duplicate and edit the resulting blocks.
5. Play the looping timeline using a built-in sound, optionally starting/stopping with Live.
6. Drag the entire timeline into an Ableton MIDI track as a single MIDI clip.

The source timeline remains intact after export. The destination instrument is chosen in Ableton; the exported file does not carry ChordCanvas's audition timbre or volume.

## 2. Terminology

| Term | Meaning |
|---|---|
| Pad | One of seven auditionable chord controls, in ascending scale-degree order |
| Block | An independently editable chord instance on the timeline |
| Timeline | The single horizontal chord-arrangement lane, including its empty time |
| Playhead | ChordCanvas's visible local playback/insertion position |
| Key | Selected written tonic plus Major or Minor, not just a MIDI pitch class |
| Octave position | One of five choices, 1 to 5; default 3; offsets from -2 to +2 octaves |
| Inversion | Reordering a chord so a different chord tone is lowest |
| Repeats | Global pad-audition repetition, not a pattern stored in a block |
| Sync | Following host playback transitions and tempo without following host song position |
| Fresh start | The default state of a newly created plug-in processor or reopened Live project |

Do not use “track” interchangeably for the internal timeline and an Ableton track. Do not call a note-name display a “root notes” list: a chord has a single root and multiple chord tones.

## 3. First-open state

The following is the single authoritative default table. Other documents may describe its behaviour but must not silently change its values.

| Item | Default | Basis |
|---|---|---|
| Selected key | C Major | [C] |
| Pad count/order | Seven, scale degrees 1 through 7 left to right | [C] |
| Pad harmony | C, Dm, Em, F, G, Am, Bdim | [C] plus deterministic scale construction |
| Every pad inversion | Root position | [C] |
| Every pad octave | Position 3 of 5 | [C] |
| Seventh/Sus2/Sus4 visibility | Off independently | [C] |
| Seventh value | Off | [D] follows default triads |
| Suspension value | None | [D] follows default triads |
| Timeline content | Empty | [C] |
| Timeline length | 8 bars | [C] |
| Timeline length range | 1 to 32 bars, integer steps | Maximum/steps [C]; minimum [D] |
| New block duration | 1 bar, except necessary boundary clipping | [C] |
| Minimum block duration | 1 quarter note | [C] |
| Editing grid | 1/4 note | [C] |
| Razor grid | 1/4 note | [C], equal to quarter bar in 4/4 |
| Snapping | Always enforced | [C] |
| Playhead | Beginning of bar 1 | [D] |
| Timeline playback | Stopped | [D] safe start |
| Timeline looping | Always enabled | [C] |
| Sync | Off | [D] avoid unexpected sound on insertion |
| Repeats | Off | [C] |
| Selected repeat rate | 1/4 note, among the five approved rates | [D]; choices themselves [C] |
| Audition sound | Piano | [D] |
| Preview volume | Conservative fixed initial gain, initially -12 dB | [D], final gain verified by listening/peak tests |
| Current editing tool | Select/move, not razor | [D] |
| Selection and clipboard | Empty instance-local state | [D] |
| Zoom | Fit all eight bars | [D] |
| Undo/redo | Empty, maximum 20 retained edit transactions | [C] |
| UI theme | One theme; the design brief supplies the starting direction | [C] one theme; styling [D] |

Do not interpret the preview gain as a MIDI velocity setting. No velocity control is exposed.

## 4. Key changes and independent chord instances

[C] Changing key immediately regenerates the seven pads and resets their octave, inversion and chord-modification values to default. The selected key name is retained exactly, including enharmonic spelling.

[C] Previously placed blocks remain unchanged: pitches, original harmonic context, inversion, octave, optional seventh/suspension, length, position, label and degree colour. A block is a snapshot of the pad when created, not a live link to it.

[C] Both drag-add and double-click-add inherit the source pad's current chord values. Subsequent pad edits do not affect blocks. Subsequent block edits do not affect pads. Changing the global audition instrument or gain changes sound only.

The details of stored harmonic context and regeneration are in the theory and architecture specifications.

## 5. Optional chord features

[C] Settings has three separate switches: **Show Seventh**, **Show Sus2**, **Show Sus4**. All start off each new instance. When enabled, the corresponding controls are available on pads and blocks, including their small-block popovers.

[C] Sus2 and Sus4 are mutually exclusive. Seventh can coexist with either. Seventh chords have a fourth inversion state; triads have three total states.

[D] These switches control availability of editing controls, not destructive rewriting. Hiding an option does not silently remove it from a block that already uses it. That block's chord label stays accurate. Resetting a pad restores the default triad. Re-enable an option to edit it again. A fresh instance has no modified blocks to carry forward.

Do not add a separate reset control to timeline blocks; that was rejected.

## 6. Project and editor lifecycle

[C] The user repeatedly chose **start fresh** when reopening a Live project. Do not override this because DAW project recall would normally be expected. Automatic autosave and crash restoration were explicitly removed.

[D] Apply fresh-start behaviour when a new processor instance is created, including normal Live-project reopening. Return a valid versioned minimal state object to the host, but do not serialize or restore the working progression, selection, pad customisations, repeat latch or session settings through host project state.

[D] Closing only the plug-in editor window does not destroy the processor. Reopening that editor shows the current live instance unchanged. Resizing the window, hiding it, switching to Settings and reopening a popover must not erase work. Audio may continue when the editor is closed if its instance is playing.

[D] Ignore incoming host-restored working state for a fresh instance; do not indiscriminately clear an already active instance every time the host calls a state API. Host duplication can create a fresh instance. Do not introduce lifecycle crashes. Test and document the interaction of host freeze/render with this intentionally non-restoring policy rather than promising that every host operation can reconstruct unsaved musical state in a new processor.

[C] An update preserves logs and explicitly saved user files. “Preserve settings during update” is not permission to add automatic session-state restoration. Installation identity and user-owned files survive; musical session defaults still reset as specified.

## 7. Manual save/load without factory content

The user accepted saving/loading their own progressions. A later “None” answered a question about shipping presets/templates or an example project. **Reconciled interpretation:** retain explicit manual save/load, but ship no factory progression presets, example project or template browser. This is recorded separately from confirmed verbatim decisions in the decision register.

[D] Provide compact **Save Progression** and **Load Progression** actions. A saved file is a versioned ChordCanvas data file, not a MIDI import feature. It contains timeline length and independent block data sufficient to reproduce the progression exactly. Do not store the active repeat latch, running transport, undo history, personal machine data or a restored startup state.

[D] Use a native file dialogue only when the user explicitly invokes Save/Load. A suggested extension is `.chordcanvas.json`. Validate before replacing the current timeline. Successful Load is one undoable transaction; cancelled/invalid loads do not alter the current work. Existing global key/pads stay as selected unless the user changes them separately. Mixed-origin-key blocks must survive save/load.

[D] Loading content with optional chord features does not change their sound merely because the controls are hidden. Labels remain truthful. Never automatically reload the last saved file at startup.

No dedicated end-user manual, tutorial, onboarding wizard or sample progression is required. Tooltips and actionable error messages are part of the interface, not substitute documentation.

## 8. Settings and About

[C] Settings is its **own section** in the plug-in, not a modal settings window or collapsible sidebar. Return to the main workspace without losing the session. It contains the independent optional-chord switches, edit grid, razor grid, About information and log access.

[C] About displays the installed semantic version and **only that version's changelog**. It does not display a cumulative release history. A repository's Git history and GitHub Releases may naturally retain older releases; that is separate from the About UI.

[C] **Open Logs Folder** opens the real `%LOCALAPPDATA%\ChordCanvas\Logs\` folder in Windows Explorer. It is a functional link/button, not a path that must be manually copied.

[D] Required third-party licence notices can be accessible from About and included with the installation. Do not omit legal notices to satisfy the request for no end-user docs.

## 9. Explicit exclusions

Do not implement rejected ideas simply because a competitor offers them or because the framework makes them easy. Exclude:

- Max for Live delivery; other DAW support commitments; extra plug-in formats; a user-facing standalone application.
- Vocal/audio import, waveform display, vocal BPM entry, vocal key detection, pitch extraction and vocal/chord compatibility hints. The earlier request to choose a vocal-detection stack is obsolete.
- Generic next-chord hints, borrowed-chord mode, substitution suggestions, random generation, progression templates, “complexity” or automated voice-leading features.
- Harmonic/melodic-minor selectors, user-configurable key-change points, global transpose, add9, arbitrary bass-note selection and open/wide/spread voicing modes.
- Strumming, arpeggiation, rhythm styles, humanisation, velocity/gate controls, metronome and recording/capture of pad performance.
- MIDI-file import into the block timeline, external MIDI-keyboard mapping to the seven pads, favourites, chord-information panels, Roman-numeral overlays, block locks and A/B arrangements.
- A clear-timeline button, loop on/off toggle, loop-region feature, auto-close gaps, default ripple-reordering, snap-off/bypass, drag-to-copy modifiers, arrow-key nudging and middle-mouse panning.
- Automatic project recall, autosave, crash recovery, background update checks, cloud services, accounts, telemetry and end-user manuals.

Do not remove accepted Ctrl+D duplicate, Ctrl+C/V clipboard, marquee selection, Ctrl+wheel zoom, Shift+wheel horizontal scroll, bar numbers, reset-on-pad or keyboard 1–7 audition merely because similar rejected ideas exist.
