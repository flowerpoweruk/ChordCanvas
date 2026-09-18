# ChordCanvas: GUI, UI and UX design specification

**Authority:** Interaction architecture, information hierarchy, visual language and GUI review.  
**Read with:** [Timeline interactions](04_TIMELINE_INTERACTION_SPEC.md), [audition](05_AUDIO_AUDITION_SPEC.md), [acceptance tests](11_ACCEPTANCE_TESTS.md).

## 1. Product position

[C] ChordCanvas must have a **distinct visual identity and a design language developed for its actual workflow**. It must not resemble a lazy AI-generated prototype, a generic web dashboard or an Ableton clone. GUI, UI and UX are fundamental to success, not a final styling pass.

Treat a confusing gesture, inaccurate hit area, clipped control, weak hierarchy or inconsistent state as a product defect. “All buttons work” is not a sufficient design review. The interface should support repeated musical decisions without forcing the user to work around the interface.

[C] One theme only. The user delegated which theme works best. [D] Begin with a restrained dark, ink-like workspace, neutral surfaces and a deliberately selected seven-colour harmonic palette. This is a starting design direction, not a claim that dark themes universally reduce fatigue. Codex may refine that single theme from rendered evidence; do not add a theme switcher.

## 2. Design before building the full UI

Before the complete interface is implemented, establish a small internal design system in source and engineering evidence. It must define:

- Layout regions, alignment rules, content hierarchy and density.
- Typography roles, spacing tokens, control heights, border/radius rules and icon construction.
- Semantic colour roles, seven degree identities, state overlays and contrast checks.
- Focus, pointer capture, gesture priority, drag thresholds and keyboard scope.
- Selected, hovered, sounding, repeating, dragging, drop-target, disabled and error states.
- Narrow-block adaptation, popover positioning, window resizing and DPI behaviour.

Build at least one interactive vertical slice with actual production-model chord data. Exercise the pad-to-timeline-to-MIDI flow before styling all screens. Static mockups alone are not proof of usable interactions.

Use JUCE's native component architecture with a deliberate custom visual system. A special rendering technology is not required. Do not add a browser runtime merely because web component libraries are familiar.

## 3. Primary screen hierarchy

[C] Key selection is at the top. Instrument selection is a dropdown at the top. Volume is a **dial**, not a slider. Repeats sits immediately above the seven pads. The timeline sits below the pads.

[D] Use the following structural arrangement, refining exact proportions through real rendering:

```text
ChordCanvas                  Key [C Major]     Sound [Piano]   Volume (dial)   Settings

Repeats [off/on]   Rate [1/8 note | 1/4 note | 1/2 note | 1 bar | 2 bars]

[ pad 1 ] [ pad 2 ] [ pad 3 ] [ pad 4 ] [ pad 5 ] [ pad 6 ] [ pad 7 ]
  root      root      root      root      root      root      root
  inversion and octave controls within each pad
  optional chord controls appear only when enabled

Play  Stop  Return to Start  Sync    Select / Razor     Undo Redo      Length [8]  - + Fit

bar numbers and local playhead
| chord block |    gap    | longer chord block                | ... |
                    horizontal scrollbar

Save Progression   Load Progression                     [ Drag MIDI to Ableton ]
```

This is a functional wireframe, not permission to use default widgets or impose a generic boxed-card grid. It does not prescribe exact pixels, colours or decorative chrome.

The seven pads and timeline should be the immediate visual focus. The product name, Settings and secondary actions should not compete with musical content. Do not insert a large header, sidebar navigation, statistics cards or unused dashboard space.

## 4. The seven pads

[C] Display seven distinct degree colours, ordered consistently from degree 1 to 7. Each block inherits its source degree identity. Key changes do not remap previously placed block colours. Colours must be visibly distinguishable; do not produce seven nearly identical shades.

[C] The root name is the primary pad label. [D] A small quality/extension indicator may make minor/diminished/modified harmony unambiguous without replacing the simple root-led face with a note list. A subtle 1–7 keyboard hint is acceptable; it is not a Roman-numeral overlay.

[C] Each pad has its own inversion control, five-position octave selector and reset control. Inversion should be changeable without a dropdown. Separate embedded controls from the audition/drag face with clear hit areas and stable positions. Controls must remain readable when optional features are enabled.

[D] Preserve enough pad-face area for comfortable auditioning. Do not let tiny arrows or a reset glyph consume the entire actionable surface. Pad reset should be discoverable with a tooltip, and must never reset the other pads.

### Pad gesture ownership

[D] Resolve the shared click/hold/drag surface deliberately:

1. Embedded controls receive the gesture first and never initiate a block drag.
2. Pressing the pad face starts held audition when Repeats is off, or performs the repeat latch action when it is on.
3. Passing a modest documented drag threshold converts the gesture into a pad-to-timeline drag. End any temporary held audition at that transition; do not leave a held note hanging if release occurs in another window.
4. A valid drop creates exactly one block using a snapshot of the source pad at drag start. Cancelling creates nothing.
5. A double-click appends exactly one block, not two. It must not accidentally leave Repeats latched by counting the two constituent clicks independently.

The precise double-click arbitration is an engineering task: test real mouse event ordering. Do not remove double-click add or hold-to-audition to avoid solving it.

## 5. Timeline block design

[C] Blocks are rectangular, with a readable chord label. Their bodies move horizontally, and both edges resize. Their inversion and octave controls are inside the block. Optional Seventh/Sus2/Sus4 controls follow Settings visibility.

[C] When a block becomes too narrow for its controls, collapse them into a compact menu trigger **inside that block**, opening a popover anchored to it. Never require zooming merely to reach a block's inversion or octave.

[D] Adapt by measured available width, not by fragile hard-coded character counts. Preserve the label or a meaningful compact form, selection indication and menu trigger. At extreme zoom-out, retain a reachable minimum hit target or selected-block overlay without misrepresenting the block's time width.

[D] The popover must remain within the plug-in's available bounds, open away from screen edges, identify the chord being edited and close predictably. Clicking the popover must not reposition, resize, slice or re-audition its owner unintentionally. Moving/deleting its owner must update or close the popover safely.

Separate block states visibly:

| State | Required distinction |
|---|---|
| Selected | Clear outline or equivalent, retained when not sounding |
| Sounding | Separate active treatment linked to current playback; distinguish a timeline block under the playhead from one temporarily muted by a pad-preview override |
| Hovered | Subtle affordance, not confused with selection |
| Edge-hover | Resize cursor and visible edge emphasis |
| Dragging | Ghost at prospective time, original/source relation obvious |
| Replacement target | Clearly marked before a destructive drop |
| Razor-hover | Visible snapped cut line, including invalid-cut feedback |
| Clipboard result | Newly inserted blocks selected so outcome is visible |

Do not rely on colour alone to distinguish selection from active playback.

## 6. Conflicting gestures must be resolved, not ignored

The default gesture priority is:

1. Popover or embedded control.
2. Active razor tool acting on a block.
3. Block resize edge.
4. Block body selection/audition/move.
5. Empty-canvas click versus marquee drag.
6. Dedicated MIDI drag handle for an external file drag.

This prevents a control click from slicing a block or an export drag from rearranging it. Razor mode has a clearly visible active state and cursor. Returning to the select tool is easy. Do not add an unrequested split-at-playhead command as a substitute.

[D] A plain block-face press auditions it while held and selects an unselected block. Pressing an already-selected member must preserve its multi-selection until release/drag arbitration; a drag moves that group, whereas a completed plain click may collapse to the clicked member. A drag threshold converts the press into a move and ends temporary audition. Shift-click is multi-selection, not a required audition modifier. Handle multi-select audition consistently without starting several stuck chords. The last explicitly auditioned chord is the preview source.

[D] Clicking empty timeline space positions the snapped playhead and clears selection. Dragging empty space beyond a threshold starts a marquee instead and does not move the playhead as a side effect. This was not finally specified by the user; it is the documented standard interaction default.

## 7. Keyboard and pointer navigation

[C] Implement 1–7 pad audition; Ctrl+C/V; Ctrl+D; Ctrl+Z/Y; Delete/Backspace; Shift-click multi-selection; marquee selection; Ctrl+wheel timeline zoom; Shift+wheel horizontal scroll; plus/minus zoom and Fit; and a horizontal scrollbar.

[D] Ctrl+A selects all blocks when the timeline has focus. The earlier suggestion was not explicitly confirmed, but it is a compatible conventional completion of the selection model. Do not use a global keyboard hook.

[D] Shortcuts are scoped to the focused ChordCanvas editor/region. Typing `1` into a numeric field must not play pad 1; Ctrl+A in a number field must select its text rather than the timeline; Undo in an active text edit should finish/cancel that field coherently before acting on the timeline.

Host keyboard conflicts must be tested in Live. Request focus normally and release it predictably. Do not remap Live shortcuts, change the user's preferences or require the plug-in to intercept keys globally.

[C] Only Ctrl+wheel and Shift+wheel over the timeline invoke the specified timeline actions. [D] Ctrl wins if both modifiers are held; a plain wheel is not an accidental zoom command. Zoom should keep the point under the pointer stable; explicit plus/minus can centre on the playhead.

Do not add Alt/Ctrl-drag duplication, arrow nudging, middle-mouse pan or a panic shortcut. System-level safety cleanup of stuck internal notes is still mandatory.

## 8. Settings section and notices

[C] Settings is a separate view/section within the plug-in, with a clear route back to the composition workspace. Keep the order compact: optional chord features, edit/slice grids, About/current changelog, Open Logs Folder.

[D] Do not stop playback just because the user opens Settings. Keep current running/repeating status visible or preserve an obvious route to its controls. Do not allow a settings view to trap the user with hidden, uncontrollable audio.

Errors need plain-language messages with what failed and a relevant next action. Detailed technical data goes into logs. Simple error popups are accepted; routine edits do not need confirmation dialogs. Destructive replacement is previewed and undoable. Timeline shortening is destructive and undoable without a repeated warning workflow.

## 9. Visual system and prohibited patterns

[C] Reject oversized rounded cards, arbitrary gradients, glass effects, huge padding, decorative dashboards, generic framework buttons, placeholder typography, toy-like knobs, inconsistent radii, uneven spacing, gratuitous animation and random accent colours.

[D] Use a small spacing scale and a limited type scale. Keep numeric values tabular where useful. Establish a consistent distinction between clickable surfaces, selectors and informational text. The volume dial must read as a precise audio control, not a decorative circular graphic.

[D] Use legally available typography, preferably a system font initially. Do not copy or redistribute proprietary font files. Check sharp/flat/minus/degree glyphs at real sizes; provide a vector or text fallback where a glyph is missing.

[D] Establish contrast targets of at least 4.5:1 for ordinary text and 3:1 for large text/essential non-text boundaries as measurable design targets, not a claim of complete accessibility certification. Do not desaturate the seven degree colours until they become indistinguishable. Use labels and outline patterns to supplement them.

[D] Use restrained animation only when it communicates state, such as a stable playhead or a short active-state transition. Audio scheduling must never depend on animation timing.

## 10. Window sizing, scaling and performance

[D] Begin with a practical desktop editor around 1040×640 logical pixels and a tested minimum around 900×560, then refine to fit the seven pads and real controls. These are provisional design values, not fixed user requirements. Use layout constraints rather than a screenshot stretched to every size.

Test 100%, 125%, 150% and 200% Windows scaling; different window sizes; moving between monitors; long enharmonic key labels; all optional controls enabled; shortest blocks; dense 32-bar material; and popovers at every edge. Do not make controls or labels disappear on a typical laptop display.

Prefer at least roughly 24-logical-pixel targets for frequently used small desktop controls, with expanded invisible edge-hit areas where appropriate. Avoid web-style oversized touch targets that waste timeline space. Balance density and reliable acquisition through testing.

UI activity must not block the audio thread. Use rate-limited repainting and a production model separate from components. A narrow-window compromise must not alter notes, time values or exported MIDI.

## 11. Required design review evidence

Codex must capture and inspect the **actual rendered application**, not just source code. Review at least:

- First-open workspace and Settings.
- A mixed-chord progression, selected versus sounding states and replacement preview.
- Short-block popovers and razor hover at minimum duration.
- Repeats active, all optional chord controls visible and an unusual enharmonic key.
- Dense/zoomed timeline, a large DPI scale and the smallest supported editor.

Record the problems found, the changes made and the retested result in engineering evidence. Images must come from the real running build, with private host/project information excluded. A visually acceptable screenshot does not replace testing the corresponding interaction. Do not claim user testing unless an actual user session occurred.
