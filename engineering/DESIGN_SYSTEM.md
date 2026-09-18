# ChordCanvas interaction and visual system

Engineering design baseline; rendered review has not yet run.

18 September development review: the existing native VST3 fails during editor creation in the separate native view host, before it can render. Source inspection found resize limits triggering layout before pad construction; initialization order corrected in source. Corrected compilation/rendering remains pending. Actual exit/debugger observations and their limits are recorded in `editor-creation-evidence.json`; there is no invented first-open screenshot or GUI pass.

The composition occupies one ink workspace, with seven coloured vertical strips joining the pad faces to their lower voicing controls. The horizontal timeline uses the same degree accents on rectangular blocks. The first view gives musical surfaces priority over the product wordmark and secondary actions. Settings replaces the composition view within the editor, with persistent transport status and a stop route.

Tokens: background #11171D; surface #1C252E; raised #26323E; ordinary text #E9EEF2; secondary text #ABBAC6. Degree accents: #F1AD67, #E4D16B, #86C88A, #6BC8C4, #79AEEB, #B59BE5, #E397BA. Spacing 4/8/12/16/24; controls 24 or 28 high; corners 2 pixels; Segoe UI system typography at 12/14/18/24. Numeric roles are aligned and compact. No gradients or decorative shadows.

Audition/drag pad face excludes reset, inversion, octave and optional feature controls. A 6-pixel drag threshold transfers ownership and ends the held audition. Double-click append owns the second press and cancels any repeat action from the first constituent press. Pointer release outside the editor and keyboard focus loss release momentary preview; latched repeats are independent.

Timeline priority: active popover, embedded control, razor, resize edge, block body, empty canvas. A body press preserves a selected group until drag/click arbitration. A marquee commits selection on release and never seeks. Drag and resize previews show entire intersected destination blocks as hatched victims; committed musical state changes only on release. Selection uses a light outline; sounding uses an upper activity bar; override is explicitly labelled.

At narrow block widths, a 24-pixel menu trigger opens a bounded anchored voicing popover. Hit areas can expand around a selected short block without changing its depicted time. Zoom keeps pointer time fixed, horizontal scrolling changes the transform only. Shortcuts defer to any focused text editor; keyboard audition uses local key state with OS-repeat suppression.

Initial editor 1040×640; current native minimum 1000×560, increasing to 1000×608 when optional controls are visible. These remain provisional until rendered host review. Extremely narrow blocks retain a body target through scaled edge regions and a full-size selected-chord editing control without depicting a longer duration. Review at all mandated DPI scales, long labels, optional features, 32 bars, minimum durations and window edges before release. Contrast calculations and real rendering evidence are still required.
