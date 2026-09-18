# ChordCanvas: timeline interaction specification

**Authority:** Time representation, editing transactions, selection, collisions and boundaries.  
**Read with:** [GUI/UX](03_GUI_UX_DESIGN_SPEC.md), [audio](05_AUDIO_AUDITION_SPEC.md), [export](06_MIDI_EXPORT_SPEC.md).

## 1. Time model and invariants

[C] One chord lane. Only 4/4. Initial length 8 bars; maximum 32 bars; length changes in whole-bar increments. New blocks default to one bar. Minimum block duration is one quarter note. Empty gaps are permitted anywhere, including the beginning and end.

[D] Use an integer musical clock with **960 ticks per quarter note**:

```text
1 quarter note = 960 ticks
1 bar in 4/4 = 3840 ticks
8 bars = 30720 ticks
32 bars = 122880 ticks
```

Store block intervals as half-open ranges `[startTick, endTick)`. Adjacent blocks sharing a boundary do not overlap. A valid model satisfies:

```text
1 <= lengthBars <= 32
0 <= block.startTick < block.endTick <= lengthBars * 3840
block.endTick - block.startTick >= 960
no two block intervals intersect
unique block IDs
resolved pitches valid and independent of the current global key
```

No hidden overlap layer, z-order workaround or “last block wins during playback” repair is acceptable. The editing model itself must remain non-overlapping.

[D] Future malformed input, file loads and debug commands pass through the same validation path as UI edits. Do not use GUI pixels as musical state.

## 2. Grid and minimum length

[C] Snapping is always enforced. The editing grid and razor grid are separately changeable in Settings. Both initially equal one quarter note. The user's phrase “quarter bar” for the razor is equivalent because the product supports only 4/4.

[D] Offer `1/16 note`, `1/8 note`, `1/4 note`, `1/2 note`, `1 bar` for each grid. Finer grids permit finer **positions**, not blocks shorter than the confirmed quarter-note minimum. For example, a quarter-note block may start on an eighth-note subdivision.

[D] Snap prospective positions to the nearest grid line, choosing the later line at an exact halfway tie. Then constrain to valid bounds. Where minimum duration matters, choose the nearest valid grid point that satisfies it. If none exists for a proposed slice, refuse that slice without changing the block.

[D] Changing a grid does not re-quantise existing material. All new drag/resize/slice/playhead gestures use the selected grid. Existing positions that are off the newly chosen grid remain stored exactly. There is no modifier to bypass snapping.

## 3. Add from a pad

[C] Dragging a pad onto an empty timeline area creates a one-bar block with the pad's inversion, octave, optional seventh and suspension settings. The new block is independent. Repeats settings are ignored.

[D] A pad drag carries a snapshot captured when the drag begins. Position snaps on drop. Ghost feedback shows the future interval and all blocks that would be replaced. A cancelled drag does nothing and adds no history entry.

[C] Double-clicking a pad also adds it. [D] Interpret “next available spot” as **append at the first editing-grid position at or after the last existing block's end**, or bar 1 when empty. With the original grid this is normally immediately after that block; after a grid change, round forward rather than overlapping it. Do not fill earlier deliberate gaps. Auto-expand if necessary. A successful append is one transaction; a full timeline prevents it without a warning popup.

[D] At the 32-bar boundary, a pad drop that begins before the end may be clipped to the available duration, but only if at least a quarter note remains. Do not create a shorter fragment. Dropping at or beyond the hard endpoint does nothing. The explicit one-bar default remains in all unconstrained cases.

## 4. Move blocks

[C] Click and drag the body to move a block left or right. Preserve its notes and original duration. An existing block's movement is separate from a pad replacing an existing interval.

[D] For a single block, snap its candidate start to the editing grid. For a multi-selection, use the earliest selected start as the anchor: snap the candidate anchor, then apply its translation delta to every selected block, maintaining all internal spacing and durations. If a grid change left internal offsets off the new grid, preserve those offsets rather than distorting the group; strict snapping applies to its anchor. Choose the nearest valid anchor grid line within the beginning/32-bar bounds rather than compressing the group. Remove all moving originals from collision consideration before calculating the transaction. Do not allow a group to replace itself.

[C] A moved block keeps its own length when replacing other blocks. [D] A simple move is bounded so its duration is retained; it does not truncate the moved source at the right endpoint. Clipboard paste has its own explicitly requested clipping behaviour.

[D] Selection follows moved blocks. The playhead does not move merely because a block moves. During drag, do not mutate the committed model on every mouse movement; preview the candidate and commit once on release.

## 5. Replacement and overlap policy

[C] No overlap is ever allowed. Incoming/moved/pasted content replaces existing blocks in its way, preserving the incoming content's duration except for the specified hard-boundary clipping. If it intersects several blocks, replace all of them.

[D] Define replacement as **removing each whole intersected destination block**, not trimming its unoverlapped ends into extra fragments. This resolves a detail the user did not explicitly specify. Preview those complete affected blocks before commit.

Example using zero-based bars for clarity:

```text
Existing A: [0, 1)
Existing B: [1, 2)
Incoming X: [0.5, 1.5)
Result: X [0.5, 1.5); A and B removed completely.
```

The incoming chord must not adopt a target's duration. An earlier “replace while keeping target length” suggestion is superseded by the later explicit **keep its original length** decision.

[D] For several pasted/moved blocks, test collisions against the union of their individual intervals, **not their bounding span**. A pre-existing block lying solely in an internal clipboard gap must survive.

[C] Default ripple-reordering/auto-shifting is not part of the final model. It was overtaken by the later replacement and leave-gaps rules. Do not push following chords right, auto-close gaps or add a hidden reorder mode.

A replacement plus all destination removals is one undoable command.

## 6. Resize from either edge

[C] Drag either edge to make a block shorter or longer. The opposite edge remains fixed. The chord identity and notes do not change. Minimum duration is one quarter note.

[D] Snap the moving edge to the editing grid, subject to the fixed edge and minimum duration. Extending into another block applies the same whole-block replacement rule. Shrinking leaves a gap; it does not pull later material along.

[D] The left edge cannot pass the timeline start. The right edge can auto-expand the timeline to a whole number of bars, up to 32. At the hard limit it clamps. Show the candidate duration and replacement region clearly while dragging.

[D] Resizing while the block sounds updates its scheduled end safely. If the current playhead ends up outside the edited interval, stop that chord. If it remains inside, preserve the current voicing until its new end. Update the audio model at a safe processing boundary.

## 7. Razor tool

[C] A razor-blade icon activates a slicing tool. Clicking **on a block** cuts it at the clicked position snapped to the razor grid. This replaces the earlier split-at-playhead suggestion.

[C] The tool cuts only. It does not add notes, change voicing, move the playhead or automatically select the left piece. Both children inherit all musical values from their parent and collectively occupy exactly the original interval.

[D] Preserve existing selection consistently: if the parent was selected, select both children; if not, leave both unselected. This does not create a special auto-select-left behaviour. Assign new block IDs and keep the parent data in the undo command.

[D] A cut must leave both children at least one quarter note long. Clicking the boundary or an invalid too-close position is a no-op with a visible invalid cut preview. Clicking empty canvas in razor mode does not create a block. A quarter-note block therefore cannot be cut into two eighth-note blocks.

[D] Splitting creates distinct note articulations at the split boundary during playback/export: note-off before note-on for identical pitches. The split does not imply a tie. One cut is one undo transaction.

## 8. Selection and audition

[C] Support individual selection, Shift-click multi-selection and drag-marquee selection. Multi-selected blocks can be moved, copied, duplicated or deleted together. Delete and Backspace remove selected blocks, leaving all resulting gaps.

[D] A plain press on an unselected block selects that block and auditions it as voiced. A press on a block already in a multi-selection preserves the group until the gesture is resolved: dragging moves the entire group; releasing without a drag may collapse to that one block. Never clear the group on mouse-down before deciding whether the user is dragging it. A Shift-click toggles membership without clearing other selections. Do not audition every member of a selection simultaneously. Embedded control clicks do not trigger a move or change selection unnecessarily.

[D] A marquee intersects visible block rectangles to select them; without Shift it replaces the selection, with Shift it adds. Selection is not an undoable musical edit. Selection drawing must not change the playhead after the drag threshold is crossed.

[D] Ctrl+A selects all blocks when the timeline owns focus. A plain empty click clears selection and positions the playhead. These complete the conventional interaction model where the user skipped or did not explicitly answer a minor detail.

## 9. Copy, paste and duplicate

[C] Ctrl+C copies the selected blocks. Ctrl+V pastes beginning at the **ChordCanvas playhead**. The earliest copied block starts at that point; preserve every other block's relative offset and the gaps within the group.

[D] Clipboard data is an internal versioned block payload, not MIDI import. Keep it private to the application instance for v1 unless a compatible cross-instance clipboard is deliberately implemented and tested. Do not interpret unrelated system clipboard text as executable commands or valid project data.

[C] Paste uses the same replacement policy. It auto-expands the timeline as needed up to 32 bars. If the copied group extends beyond that limit, paste what fits, clip a crossing last block and omit blocks starting beyond the limit. Do **not** reject the entire paste.

[T/D] If clipping would leave a fragment shorter than one quarter note, omit that fragment. This reconciles partial-paste permission with the confirmed minimum duration. Never extend beyond 32 bars to save the fragment.

[C] Ctrl+D duplicates the selection **immediately after the end of that selection**, preserving its spacing. [D] Implement via the same insertion pipeline, with the same expansion, clipping, collision and history behaviour as paste. A right-click Duplicate action may expose the same command; it must not implement different rules.

[D] Successful paste/duplicate selects the newly inserted blocks. Repeated paste uses the current playhead rather than silently advancing it. A paste producing no valid content is a no-op and must not delete destination blocks or create an undo entry.

## 10. Length control and destructive shortening

[C] The length control allows manual integer entry and up/down arrows, in one-bar increments. It initially reads 8. It cannot exceed 32. [D] Minimum length is one bar. Empty/invalid typed text is not committed; revert to the last valid value on cancellation.

[C] Adding or extending content beyond the current length automatically expands to the smallest whole-bar length containing it, capped at 32. Ordinary edits that fit do not change the length. Deleting the final block does not shrink it.

[C] On explicitly shortening the timeline:

1. Remove blocks starting at or beyond the new endpoint.
2. Clip any crossing block to that endpoint.
3. Do not retain hidden material outside the new length.
4. Extending the timeline later creates empty space, not restored chords.

[D] If a clipped remnant would violate the quarter-note minimum, remove it. If the playhead is now beyond the endpoint, move it to the start; if playing, continue from there on the next audio-safe boundary. Preserve valid earlier material.

[C] Undo can restore the prior length and material. This is intentional recovery, not the forbidden reappearance merely from extending the length. One length change, including all clipping/removals, is one transaction.

## 11. Playhead and navigation

[C] Display the local playhead and highlight the currently sounding timeline block. Users can click a position to start playback there. The playhead snaps to the editing grid. Copy/paste uses its position. Sync host starts still reset it to bar 1.

[C] Show bar numbers by default. The later approval supersedes an earlier ruler rejection. [D] Use unobtrusive beat/grid subdivisions rather than a second elaborate ruler feature. Represent the end boundary accurately; it is not a playable extra bar.

[C] Plus/minus buttons zoom; Fit shows the entire timeline; Ctrl+mouse-wheel zooms while hovering over it. Shift+wheel pans horizontally while hovering; provide a horizontal scrollbar. Zoom/scroll never change musical state or create undo commands.

[D] Fit includes the selected full timeline length, including trailing silence. Keep the pointer's time fixed during wheel zoom. Clamp scrolling to valid visible bounds and auto-scroll carefully while dragging near edges. Do not add middle-mouse panning.

## 12. History and atomic commands

[C] Ctrl+Z Undo; Ctrl+Y Redo. Retain at most **20 edit transactions** per instance. New edits after Undo clear the redo branch.

[D] Count a gesture as one transaction: an entire drag, resize, paste group, duplicate group, deletion group, slice, length change, successful progression load or committed block-voicing change. Do not consume 20 undo levels with 20 pointer-move samples. Repeated clicks on a discrete inversion arrow may be separate deliberate edits.

[D] Timeline history covers timeline content and its length, not global pad audition, instrument selection, viewport movement or a changing audio playhead. Preserve coherent selection after Undo/Redo, but do not time-travel playback to a stale position without a boundary reason.

Undo/Redo restores destination blocks removed by replacement and content destroyed by shortening. History belongs to the live instance and is not restored from a Live project or saved progression file.

Every edit command should expose an action name, stable affected IDs, pre/post revision and enough data for tests and diagnostic replay. Use the same command for UI shortcuts and context-menu equivalents.

## 13. Required example tests

At minimum, write tests for adjacent non-overlap; incoming partial intersections removing whole targets; multi-paste internal gaps; self-collision avoidance during group move; slice minimums on a fine grid; destructive shortening followed by extension; Undo restoring a shortening; mixed-origin-key block edits; clipboard clipping at 32 bars; changed grid leaving existing content intact; and 21 edits demonstrating a 20-transaction history bound.

The acceptance document provides numbered scenarios. Pixel-perfect drag automation does not replace direct model tests of these invariants.
