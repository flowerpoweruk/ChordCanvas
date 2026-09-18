# ChordCanvas: MIDI export and native drag-out

**Authority:** The exported progression, file validity, native drag lifetime and host import evidence.  
**Read with:** [Timeline](04_TIMELINE_INTERACTION_SPEC.md), [theory](02_MUSIC_THEORY_SPEC.md), [acceptance tests](11_ACCEPTANCE_TESTS.md).  
**Sources:** S06–S09, S15 and S16 in [technical references](13_VERIFIED_TECHNICAL_REFERENCES.md).

## 1. User-visible result

[C] The user drags the completed progression from ChordCanvas into an Ableton MIDI track. The result is **one MIDI clip containing the entire timeline**, including leading, internal and trailing silence. Do not export only the current selection or only the occupied span.

[C] Exact notes, inversion, octave, optional chord modifications, start positions and durations must match the independent blocks. The current global key does not reinterpret existing blocks at export time.

[C] Default filename: **`chords.mid`**. Do not open a filename prompt for every drag. The file creates ordinary note data; the user chooses a destination sound in Ableton. Preview instrument and volume are not exported.

[C] The source timeline is unchanged. Drag-out is a copy operation, not a destructive move of blocks. The destination is an Ableton MIDI track/clip slot chosen by the user. Do not pretend the VST3 directly controls Live's track creation or installs a stock instrument on it.

[D] Provide a clear dedicated **Drag MIDI to Ableton** handle in the timeline region. A block-body drag remains an internal move. The export handle makes the external gesture unambiguous and avoids unexpected rearrangement or export.

## 2. Exactly which events belong in the file

[C] Export note-on and note-off chord events only as musical events. No control changes, program changes, bank selection, pitch bend, aftertouch, MPE, sustain pedal, instrument names, key signatures, lyrics, proprietary sequencer payloads or decorative markers.

[T] A literally metadata-free Standard MIDI File is not a valid interpretation. Include the mandatory file/chunk structure, timing division and **End-of-Track** event. End-of-Track is structural, not a requested expressive MIDI feature. See S08/S09 for file/event APIs and S07 for the MIDI-file format's purpose.

[D] Use **SMF type 0**, one track, one MIDI channel (channel 1), 960 pulses/ticks per quarter note. Emit explicit note-off events. Use a fixed note-on velocity of 100 and note-off velocity of 0 as implementation constants. No velocity control is exposed.

[D] Do not embed a tempo map or time-signature meta-event in the normal export. The product is fixed to 4/4 and the clip plays at Live's current project tempo. The file's timing division preserves musical positions. A standalone MIDI-file player may use its own tempo; that is outside this product's target workflow.

Always distinguish a note-off message with velocity 0 from a note-on at velocity 0. Do not introduce zero-velocity dummy notes to force apparent clip length.

## 3. Time conversion and silence

The canonical timeline uses 960 ticks per quarter note. Export those exact integers:

```text
bar 1 start = tick 0
bar n start = (n - 1) * 3840
block note-on = block.startTick
block note-off = block.endTick
End-of-Track = timelineLengthBars * 3840
```

[C] Leading silence is retained because the first note-on remains at its real positive tick. Internal silence is retained by event timestamps. Trailing silence is represented by End-of-Track at the full timeline end, not moved to the last sounding note.

[D] A completely empty timeline exports a valid one-track file containing an End-of-Track marker at the selected length. Test whether Live creates the intended silent clip. An empty-file import failure must not crash or corrupt the source; report the specific host limitation rather than filling it with fake notes.

[D] At any shared tick, emit note-offs before note-ons on the same channel/pitch. This allows adjacent identical chords and razor-sliced halves to re-articulate. Finish all active notes at their exact end; do not tie across neighbouring blocks unless a future user requirement explicitly adds that feature.

The 32-bar maximum is already enforced in the model. Export validates it again but must not silently shorten a corrupt model or delete invalid notes while claiming exact export. Fail with a simple actionable error when the data is invalid.

## 4. Trailing-rest preservation is an early release gate

[T/G] A file with a correctly positioned End-of-Track event is **not by itself proof** that Live's created clip retains the same end/loop marker. A host may infer clip bounds from its own import rules. The retrieved Live manual confirms MIDI import and incorporation into the Set, not every edge case of trailing-silence interpretation.

Before completing the full product, drag an eight-bar test file with audible notes only in bars 2–3 into both Arrangement and Session views of the actual Live 12 build. Inspect the created clip's start, end and loop region. Repeat with 1, 9, 16 and 32 bars and with an entirely silent timeline.

Required outcome: one clip with the intended complete timeline span and note timings. If the host trims it despite valid structural timing, research a documented compatible method and test it. **Do not** add fake/inaudible notes, sustain messages, extra tracks, a proprietary undocumented Live-file writer or an unapproved remote-control dependency to hide the failure.

If a remaining host limitation cannot be resolved within the confirmed format/scope, record it as a blocked acceptance gate with the actual evidence. Do not assert that “MIDI drag-out works” means the full requirement passed. Continue all independent work.

## 5. Generate an immutable export snapshot

[D] Create a validated immutable snapshot of the entire timeline at the beginning of the export gesture. Later editing during a native drag must not modify the file under the receiving host.

[D] Prepare MIDI serialization off the real-time thread. For responsive drag initiation, maintain a revision-keyed prepared export after edits or use a short message/worker-thread preparation path. In all cases, the dragged file must match the latest committed revision when the gesture starts, not a stale cache.

[D] Give each drag a unique private directory while keeping the filename:

```text
%LOCALAPPDATA%\ChordCanvas\Temp\<instance-or-session-id>\<export-id>\chords.mid
```

Concurrent plug-in instances and two rapid drags must not overwrite each other's file. Record model revision, note count, timeline length and a content hash in diagnostics. Avoid logging unrelated destination project paths.

Check file creation, stream writes, flush/close, parse-back validity and content size before initiating the external drag. A partially written file must never become the drop payload.

## 6. Native drag mechanics

[T] JUCE's external file-drag API initiates a native OS drag operation. Its returned success indicates that a drag started, not that Ableton accepted the file or created the correct clip. A completion callback indicates the operation ended; it is not a universal proof of destination import success. See S06.

[D] Use a proper source component and a native file list with copy semantics (`canMoveFiles=false` or equivalent for the pinned API). Run this on the appropriate GUI/message thread and respect mouse-drag event requirements. Avoid a custom clipboard masquerading as OS file drag.

[D] Retain the exported temporary file at least through drag completion and an appropriate grace period; preferably retain completed exports until safe per-session cleanup. Never delete it immediately when the native API returns, because the drag is asynchronous and Live may still be reading it.

[D] On startup or a low-priority worker, clean stale ChordCanvas-owned export directories older than a safe retention interval, after checking they are not used by an active instance. Do not clean arbitrary user temporary files. Do not make MIDI export depend on write access to Program Files.

[G] Test normal user-level Live and the plug-in. Do not require Live to run as administrator to make drag/drop work. Installation may require elevation; the audio host should not.

## 7. Error behaviour

[C] An export failure produces a **simple error popup**, not a silent failure. Include the failed action and a useful recovery action, such as retry or Open Logs Folder. Detailed technical information belongs in the session log.

Distinguish:

- Invalid model data or note range.
- File generation/write failure.
- Native drag initiation failure.
- A cancelled drag, which is not an error.
- An unverified destination import, which must not be announced as success without evidence.

[D] A failed export must leave the progression, selection, playhead, undo stack and playback state unchanged. It must not create another block, stop Ableton or modify the destination project through a side channel.

## 8. Not part of this feature

Do not add live MIDI output routing, a MIDI-input chord detector, MIDI-file import, direct Ableton scripting, recording pad performance, per-block exports, audio export or a file-save workflow replacing the required drag-out interaction. Engineering test scripts may write fixture `.mid` files without creating a new user-facing feature.

Do not name a file `.mid` when it actually contains JSON or only raw MIDI bytes. Do not create a MIDI track per chord. Do not make export depend on a selected audition sound.

## 9. Verification

Use an independent MIDI parser for tests, configured not to auto-repair missing note-offs. Check type, track count, division, channel, event allowlist, exact notes, exact ticks and the final structural timestamp. Test key changes, optional chords, all inversions, all octave slots, leading/trailing/internal gaps, repeated identical pitches, razor slices, partial paste at bar 32 and a timeline shortened after containing later material.

Then perform the actual native drag in Live 12. Compare the imported notes and clip bounds to the same expected fixture. A generic plug-in validator cannot establish this integration result.
