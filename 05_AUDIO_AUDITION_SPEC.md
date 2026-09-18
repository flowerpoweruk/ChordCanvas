# ChordCanvas: audio, audition and transport

**Authority:** Internal sound generation, note ownership, repeat behaviour and playback clock.  
**Read with:** [Theory](02_MUSIC_THEORY_SPEC.md), [timeline](04_TIMELINE_INTERACTION_SPEC.md), [architecture](07_TECHNICAL_ARCHITECTURE.md).  
**Technical references:** S04, S05 and S15 in [reference register](13_VERIFIED_TECHNICAL_REFERENCES.md).

## 1. Four local audition sounds

[C] Ship **Piano, Guitar, Strings and Pad** in the first release. The top-level sound dropdown chooses one globally for pad audition and internal timeline playback. The sound choice does not alter chord notes or MIDI export.

[T] MIDI messages are instructions, not recorded instrument sounds. ChordCanvas is a VST3 instrument because it generates its own audio. Do not depend on Ableton stock instruments, Microsoft MIDI synthesis, another installed plug-in, a soundfont already on the owner's computer or a post-install download.

[D] Build a lightweight polyphonic engine using original synthesis, cleared bundled samples, or a mixture. A small sampler for Piano/Guitar and simple sustained synthesis for Strings/Pad is a reasonable route, but no specific sample library was approved. Resolve asset rights before publication. Do not bundle ripped commercial instrument samples or select an arbitrary “free download” without redistribution rights.

[D] These are useful preview timbres, not a request to build four full commercial instruments. All four must be recognisably distinct, pitched correctly, usable over the required register, gain-balanced and free of clicks or glaring aliasing. Do not substitute the same oscillator with four labels. Do not add a preset browser, effects panel or sound-design controls.

[C] A single global **volume dial** adjusts preview output only. [D] Use gain smoothing and a conservative initial gain, with sufficient headroom for chords and release tails. No MIDI CC7/CC11 or velocity change is emitted when the dial moves.

[D] Switching sounds while a chord is active updates it promptly using a short release/crossfade or equivalent click-free transition. Do not leave voices from the old sound indefinitely active. The sound dropdown must not freeze the audio callback while loading assets.

## 2. Note ownership and immediate changes

Maintain explicit source ownership for each active voice: held pad, repeat pad, block audition or timeline playback. Avoid using one undifferentiated “all notes off” routine that accidentally cancels another active source.

[C] Changing the inversion or octave of a currently auditioned pad takes effect immediately, including while Repeats is running. No extra click/retrigger is required. Preserve the other chord settings.

[D] “Immediately” means the next available audio processing boundary, with a small click-prevention envelope where necessary, not an impossible zero-time UI-to-hardware guarantee. Reconcile old/new note sets safely and avoid doubled held notes. An active block's voicing edits should use the same immediate-update policy.

[D] Changing key while a pad is sounding updates that active pad to its regenerated default chord at the same degree. Existing timeline blocks remain unchanged. Updating optional feature visibility alone does not change audible pitches.

[C] Do not add a visible panic/all-notes-off button or Escape panic shortcut; both were rejected. Internal safety cleanup remains required on release, bypass, teardown, processing reset and loss of held-input ownership.

## 3. Repeats off: held audition

[C] With Repeats off, pressing a pad starts its chord. Releasing stops the held chord. Keyboard number keys **1 through 7** work the same way: key down starts, key up stops.

[D] A short natural release tail is allowed for click-free audio. “Stop” must release the note immediately, not sustain until some arbitrary fixed audition duration. Do not expose a duration/gate/velocity control.

[D] Only the most recently pressed pad is the active pad-audition source. Suppress operating-system key-repeat events so holding a number key does not retrigger it. Handle pointer capture/release even when the pointer leaves the editor. On keyboard focus loss, end held-key notes that can no longer reliably receive key-up. Repeating/latching mode is handled separately below.

[C] Pads can be auditioned with Live's transport stopped. Do not gate them behind host playback or Sync. [G] Prove the plug-in actually receives enough processing in Live's stopped state; a standalone test does not establish this host behaviour.

[D] Plain timeline-block audition uses that block's stored voicing while held and is separate from the pad repeat latch. Repeat mode is a pad feature, not a property of a selected block.

## 4. Repeats on: a global pad-audition mode

[C] The **Repeats** switch is above the pads. It defaults off on each new instance. A global rate selector provides exactly these five choices:

| Display | Interval in quarter-note beats | Interval in 4/4 bars |
|---|---:|---:|
| 1/8 note | 0.5 | 0.125 |
| 1/4 note | 1 | 0.25 |
| 1/2 note | 2 | 0.5 |
| 1 bar | 4 | 1 |
| 2 bars | 8 | 2 |

[D] Initially select 1/4 note, but leave Repeats off. The earlier five-item list containing both quarter-note/quarter-bar and half-note/half-bar duplicates is superseded.

[C] Clicking a pad starts it repeating indefinitely. Clicking a different pad switches to that chord immediately. Clicking the same active pad stops the latch. Keyboard 1–7 performs the same latch actions when Repeats is on. Releasing the key does not stop a latched repeat.

[C] Changing the rate while active applies immediately. Turning Repeats off stops the repeated chord immediately. Do not let a lingering latch turn into a permanent held note.

[D] A switch to a different pad or repeat rate triggers the current/new chord immediately and restarts the interval schedule from that instant. A voicing change updates the currently sounding chord immediately but leaves the current repeat interval phase intact. This distinction makes immediate changes deterministic without repeatedly shifting the rhythm on every voicing edit.

[D] At each repeat boundary, release the preceding articulation before triggering the new one. Use a fixed internal gate close to the interval length, with only the short release required for clean articulation. There is no user-visible gate, duration or strum feature. Every chord tone is triggered together.

[C] Repeats follows host **tempo**, not host running status. It can continue when Live stops and does not become part of the progression. A pad dragged/double-clicked into the timeline still creates a normal one-bar block; MIDI export has no repeat pattern.

[D] A latched repeat continues when focus moves back to Ableton or the editor is hidden, because listening alongside Live is a core workflow. Indicate an active latch clearly when the editor is visible. Plug-in bypass, removal or teardown always releases it. A deliberate timeline Play command may clear an existing latch under the audition-priority policy below.

## 5. Internal timeline playback

[C] Local Play auditions the blocks in timeline order using their exact start/end times and voicings, locked to host tempo. Continue through all gaps to the full selected timeline length, then loop to bar 1. Looping is always enabled; no loop toggle or loop-region feature.

[C] Show a moving playhead and distinguish the sounding block. The user may position the snapped playhead and start there. Selecting a different global key does not retune existing blocks.

[D] Local Play starts from the current playhead. Local Stop releases timeline notes and freezes that position; a return-to-start control resets to bar 1. This preserves the accepted ability to audition from a position without imposing host transport control.

[D] Seeking into the middle of a block immediately starts that block's notes for the remaining duration. Seeking into a gap is silent. Editing or deleting the current block updates/releases its voices at an audio-safe boundary. Do not wait for the next loop to reflect an explicit current-block edit.

[D] At adjacent chord boundaries, release prior note ownership before re-articulating the next chord, even when some pitches repeat. At the timeline wrap, end the final block and begin any first block without cumulative timing drift. Sustained timbre release tails may decay across a boundary; the underlying note durations must still be exact.

## 6. Sync: start/stop, not song-position following

[C] **Sync** sits alongside local playback controls. When enabled, a host playback start starts ChordCanvas at its **own bar 1**, regardless of whether Live starts at bar 1, 5, 17 or elsewhere. Host stop stops sounding timeline notes. Never set the internal playhead equal to Live's absolute song position.

[T] JUCE/VST3 provides a playing-state flag and timing context, not a universal separate host “pause” button event. Sources S04/S05 substantiate that limitation. The earlier promise of distinguishing every play/stop/pause gesture must not be repeated without actual Live evidence.

[D] Implement this observable state machine:

| Condition | Timeline behaviour |
|---|---|
| Sync off | Local controls determine timeline running state; tempo still follows host |
| Sync on, host stopped | Armed; no host-driven timeline audio |
| Host stopped → playing with Sync on | Reset local tick to 0 and start |
| Host playing → stopped with Sync on | Release timeline notes, freeze visible local position |
| Next host stopped → playing | Restart from local tick 0, not frozen position |
| Host seeks while still playing | Do not relocate ChordCanvas |
| Host arrangement loop wraps while still playing | Continue ChordCanvas's own length/phase |
| Enable Sync while host already playing | Start immediately at local bar 1 |
| Disable Sync while running | Stop the host-driven timeline; return local control without changing chords |

The frozen position during a stop is the visual pause; the next host start still honours the repeatedly confirmed start-at-bar-one rule. Do not add a host-position follower to solve a pause ambiguity.

[D] While Sync is on, make the local Play/Stop controls clearly host-owned/disabled rather than creating competing transport masters. The local playhead may still be inspected or repositioned; the next host start resets it. Repeats remains a separate pad audition mode and does not become transport-following.

[C] Do not launch Live, start its transport, change its loop or reposition its playhead in response to ChordCanvas's local controls. Sync is one-way observation.

[G] Validate actual Session View global transport behaviour in Live 12. Launching an individual Session clip while Live is already playing is not necessarily a new host start; do not claim to detect a specific clip's launch via a standard VST3 transport flag.

## 7. Preview priority: explicit default, not a hidden assumption

The user did not specify whether clicking pads while the timeline plays should layer two competing harmonies. [D] Use **preview override**:

- Timeline time continues to advance normally.
- A held pad or held block audition temporarily replaces the internal timeline's audible chord, rather than mixing an unrelated harmony over it.
- Releasing the held audition returns to the chord currently under the playhead, for its remaining duration.
- A latched repeating pad overrides timeline audio until stopped/switched off. The UI must identify the preview source so silence from the timeline is not mistaken for failure.
- Explicitly starting local timeline playback clears an earlier pad repeat latch, while leaving the Repeats switch itself available for subsequent use.

This uses one clear audition focus without changing timeline content. Implement ownership and handover without hanging notes. Do not stop the vocal or any other audio in Live.

## 8. Clock, tempo and meter

[C] Always use the DAW tempo. No editable BPM field. Host tempo changes affect timeline playback and Repeats immediately.

[D] Read host position/tempo only through the supported audio callback API. Validate finite positive tempo. Retain the last valid host tempo if a callback omits it. If no valid tempo has ever been received, use a clearly logged internal fallback of 120 BPM only to keep preview safe, expose an unobtrusive unavailable-host-tempo state and verify this fallback is not reached in normal Live operation. Do not present it as a user-entered tempo.

[D] Derive elapsed musical time from processed samples and tempo, not GUI timers. Keep phase across tempo changes rather than restarting the progression. Handle event boundaries inside the audio block, sample-rate changes, different buffer sizes and a tempo value changing between callbacks. Accuracy is limited by the host's supplied timing updates; do not claim finer host automation information than is provided.

[C] Only 4/4 is supported. [D] If the host explicitly reports another meter, stop/disable timeline playback and show a concise “ChordCanvas timeline requires 4/4” state; held pad audition may still work. Do not silently reinterpret bars as a different meter. Do not add a meter selector. Export remains the defined four-beats-per-bar format.

## 9. Audio safety and host integration

Clear output buffers appropriately, generate finite samples, avoid denormals where relevant, and control peaks. Preload/decode sound assets off the audio thread. Smooth gain/voice transitions. Do not perform disk I/O, logging format work, mutex waits, asset downloads, GUI calls or heap growth in the real-time processing path.

Provide enough polyphony for the largest chord and controlled release overlap. Reject voice leaks with automated counts and long-run tests. Respond safely to bypass, processing suspension, prepare/release cycles, rapid editor changes and deletion during a held audition.

No external MIDI controller mapping, live MIDI-output routing or user soundfont import is required. The instrument's event-bus declarations must truthfully match its implemented behaviour and pass VST3 validation; registration details belong to the architecture spec.

## 10. Required proof

Capture tests for all four sounds, all five repeat rates at several host tempos, key-up/mouse-up outside the pad, OS key repeat suppression, latched repeat switches, immediate voicing edits, local seeking into a chord, always-on looping through silence, Sync from a nonzero host position, host stop/restart, tempo changes, editor closure and focus transitions.

A MIDI-event unit test alone does not prove sound generation, absence of clicks or correct behaviour in Live. Retain audio-engine tests and actual host evidence separately.
