# ChordCanvas: music-theory specification

**Authority:** Musical identity, pitch calculation, spelling and voicing.  
**Read with:** [Product](01_PRODUCT_SPEC.md), [audio](05_AUDIO_AUDITION_SPEC.md), [MIDI export](06_MIDI_EXPORT_SPEC.md).  
**Evidence:** S10–S14 in [technical references](13_VERIFIED_TECHNICAL_REFERENCES.md). Formulae below are the product's deterministic definitions, not a licence to guess from chord names.

## 1. Non-negotiable accuracy

[C] Never use an LLM at runtime to guess scale notes, chord qualities, accidentals or inversions. Build a deterministic theory module with independent expected fixtures and exhaustive tests. Do not use the same implementation to generate both actual and expected test values.

Separate four concepts: a written pitch name, its pitch class modulo 12, its absolute MIDI note number, and its display label. Enharmonic equivalence must not erase the selected spelling. An octave number in an external library must not be treated as an unambiguous MIDI pitch.

The GUI, audition engine, manual progression files and MIDI exporter must use the same resolved chord data. No separate UI-only shortcut table that can disagree with the audible notes.

## 2. Complete key inventory

[C] Include these 21 tonic names, each with **Major** and **Minor** as separate selectable key entries:

| Letter | Natural | Sharp | Flat |
|---|---|---|---|
| C | C | C♯ | C♭ |
| D | D | D♯ | D♭ |
| E | E | E♯ | E♭ |
| F | F | F♯ | F♭ |
| G | G | G♯ | G♭ |
| A | A | A♯ | A♭ |
| B | B | B♯ | B♭ |

There must be **42 written key choices**. This inventory includes A and B, despite an earlier spoken list stopping at G. It includes uncommon tonics. Do not replace C♯ Major and D♭ Major with a single entry called C♯/D♭ Major. Do not include double-accidental tonic names.

[D] The selector may use one searchable list or a clearly coupled tonic/mode selection, provided every exact written key is available and the active combined key is obvious. Display the word **Minor**, not a menu of minor-scale variants.

## 3. Scale construction

Use 12-tone equal-tempered MIDI pitches. Natural letter pitch classes are:

```text
C=0, D=2, E=4, F=5, G=7, A=9, B=11
Major offsets:         [0, 2, 4, 5, 7, 9, 11]
Natural minor offsets: [0, 2, 3, 5, 7, 8, 10]
```

[C] Minor always uses the natural-minor offsets above. Do not substitute a raised leading tone, a major dominant or a harmonic/melodic-minor scale because a textbook discusses common-practice minor harmony. The product intentionally offers this one unambiguous minor palette.

For each scale, advance letter names one step per degree from the selected tonic. Compute the accidental required to match each interval. Retain that theoretical letter/accidental pair internally, including double accidentals when necessary. Compute pitch classes with a modulo operation that handles negative values correctly.

For ascending MIDI realization, extend the seven-note scale by octaves:

```text
extendedDegree(k) = baseTonicMidi + offsets[k % 7] + 12 * floor(k / 7)
```

Here `k` is zero-based and non-negative. This avoids flattening wrapped chord tones back into the first octave.

## 4. Diatonic triads

[C] Pad `d`, where `d=0..6`, is built from extended scale degrees `d`, `d+2`, `d+4`. The seven default qualities must be:

| Degree | Major key | Natural minor key |
|---|---|---|
| 1 | major | minor |
| 2 | minor | diminished |
| 3 | minor | major |
| 4 | major | minor |
| 5 | major | minor |
| 6 | minor | major |
| 7 | diminished | major |

Derive quality from relative semitones, not from a manually guessed string. Triad interval signatures are major `[0,4,7]`, minor `[0,3,7]`, diminished `[0,3,6]`. The unmodified Major/natural-Minor palette does not produce augmented triads.

Examples that must pass:

| Key | Seven pads in order |
|---|---|
| C Major | C, Dm, Em, F, G, Am, Bdim |
| A Minor | Am, Bdim, C, Dm, Em, F, G |
| F Minor | Fm, Gdim, A♭, B♭m, Cm, D♭, E♭ |
| D♭ Major | D♭, E♭m, Fm, G♭, A♭, B♭m, Cdim |
| C♯ Major | C♯, D♯m, E♯m, F♯, G♯, A♯m, B♯dim |
| C♭ Major | C♭, D♭m, E♭m, F♭, G♭, A♭m, B♭dim |

These fixtures are derived from the interval definitions above. Test all 42 choices, not just these examples.

## 5. Display spelling and double-accidental substitution

[C] Correct theoretical spelling remains the internal truth. Correct **single** accidentals such as E♯, B♯, C♭ and F♭ must remain where appropriate. The explicit user exception is to simplify notes that would need a **double accidental** in the display.

[C] For example, G♯ Major remains selected as G♯ Major. Its theoretical seventh-degree F𝄪 is displayed as G. The sounding pitch is unchanged. A block rooted on that degree can therefore display `Gdim` even though its internally stored theoretical root is F𝄪. Do not switch the whole key to A♭ Major.

[D] Deterministic substitution policy: if `abs(accidental) <= 1`, preserve the theoretical spelling. Otherwise choose an enharmonic spelling with the fewest accidentals. Prefer a natural letter when possible; break any remaining sharp/flat tie using the originating key's accidental direction, then a fixed documented fallback. Never vary labels randomly between frames or components.

Examples: F𝄪 → G, C𝄪 → D, E𝄫 → D, B𝄫 → A. Substitution is a display transformation, not a transposition, and does not alter quality.

The default GUI does not include a rejected chord-note information panel. Detailed spellings can exist in tests, internal data and diagnostic snapshots without adding that panel.

## 6. Inversions

[C] Triads have Root, 1st and 2nd inversion. With a seventh enabled, add 3rd inversion. Display these ordinary labels, not Roman numerals masquerading as inversions. Pads and sufficiently wide blocks use compact increment/decrement controls rather than inversion dropdowns. Small-block controls may live in the approved popover.

Given a sorted, close-position chord, rotate the lowest `n` tones to the top, adding 12 semitones to each moved tone. Do not add duplicate octave notes. Inversion and octave controls are independent; changing one must not reset the other.

Default C-Major MIDI examples at octave position 3:

```text
Root: [60, 64, 67]
1st:  [64, 67, 72]
2nd:  [67, 72, 76]
Cmaj7, 3rd inversion: [71, 72, 76, 79]
```

[D] If a seventh is removed while 3rd inversion is selected, clamp to 2nd inversion. Do not retain an invalid index, crash or add a phantom note. This is a documented boundary default, not a newly requested musical feature.

## 7. Five octave positions and absolute register

[C] Present five positions, labelled **1, 2, 3, 4, 5**, with **3** selected initially. Position 1 shifts the complete chord down two octaves; 2 down one; 3 leaves it unchanged; 4 up one; 5 up two. Include the seventh when present.

[T/D] Earlier discussion used “C3” without consistently distinguishing octave naming conventions from selector positions. Use these exact integers as the implementation baseline:

```text
C-major tonic at position 3: MIDI 60
C-major root-position chord: [60, 64, 67]
octaveOffset = 12 * (octavePosition - 3)
baseTonicMidi = 60 + normalisedTonicPitchClass
```

This is an explicit engineering resolution, using the intended Ableton-style middle-C naming rather than treating the label “C3” as MIDI 48 by accident. Verify the displayed host naming during Live testing; MIDI integers are authoritative. It does not change the user's five-position requirement.

[D] Realise successive scale degrees upward from `baseTonicMidi`, including crossing an octave. Enharmonic keys with the same tonic pitch class use the same initial register. Do not place all chord roots in a single written-letter octave if that would reverse their ascending sounding order.

Test every supported combination to ensure MIDI values remain within 0–127. If a future input exceeds the range, reject it explicitly rather than wrapping modulo 128 or silently dropping notes.

## 8. Diatonic sevenths

[C] The optional seventh is taken from extended degree `d+6` of the chord's **originating** key. It is not always ten semitones above the root.

| Degree | Major key seventh quality | Natural minor seventh quality |
|---|---|---|
| 1 | maj7 | m7 |
| 2 | m7 | m7♭5 |
| 3 | m7 | maj7 |
| 4 | maj7 | m7 |
| 5 | 7 | m7 |
| 6 | m7 | maj7 |
| 7 | m7♭5 | 7 |

Correct examples: C in C Major becomes Cmaj7; G becomes G7; Bdim becomes Bm7♭5. In F Minor, Cm becomes Cm7, not C7. These values remain anchored to a placed block's original context even after the global key changes.

Do not borrow the example `C7sus4` from the earlier conversation as evidence that every seventh should be dominant. Labels must match the actual seventh.

## 9. Sus2 and Sus4: explicit implementation policy

[C] Both are optional, independently exposed from Settings, mutually exclusive in a chord, and allowed with a seventh. The conversation did **not** define altered-fifth handling or whether a suspension must remain entirely in key.

[D] Use conventional suspended interval sets for the optional mode:

```text
Sus2: [0, 2, 7]
Sus4: [0, 5, 7]
```

Selecting a suspension replaces the triad's third and uses a perfect fifth. Thus an initially diminished pad becomes an ordinary suspended chord while the suspension is enabled, rather than a misleadingly labelled suspended-flat-fifth chord. Returning to None restores the correct diatonic triad. An enabled seventh still comes from the stored originating scale.

[D] Conventional suspensions can introduce an out-of-key note. Do not secretly change a sus4 to a sharp-fourth chord to force it into the scale. The promise of seven diatonic triads applies to the default triads, not to every optional modification. State this briefly in the control tooltip, without adding a warnings panel or recommendation engine.

Compute interval spelling from the root, then apply the double-accidental display policy. Omit a minor-triad suffix for a suspended chord. Use `Csus2`, `Csus4`, `Cmaj7sus4` and `G7sus4` where those notes warrant those names. Verify this chosen convention independently, using S14 and explicit interval tests rather than accepting a library's preferred chord name without checking pitches.

## 10. Stored block identity

Each block needs sufficient immutable origin information to remain musically independent of the global key:

```text
origin key: written tonic + mode + theory schema version
origin degree: 0..6
origin scale: validated interval/spelling context, or equivalent deterministic data
base diatonic triad and diatonic seventh
current suspension, seventh-enabled flag, inversion, octave position
resolved MIDI notes and display chord label
origin degree colour identity
```

[D] Store a canonical semantic representation and validate any cached notes against it. Never silently recompute from the current global key on playback, export, paste or reload. Version the format so a future change cannot reinterpret old saved progressions unnoticed.

Pad reset affects only that pad. It returns root position, octave slot 3, no seventh and no suspension, with the current key's triad. Changing key resets all seven pad values but not feature-visibility switches for the current instance.

## 11. Independent verification requirements

Test the complete Cartesian combination of all keys, seven degrees, five octave positions, all valid inversions, suspension states and seventh states. Separately test:

- Interval and quality patterns for all unmodified chords.
- Theoretical scale spelling before display substitution.
- Equal MIDI content but different written identity for enharmonic keys.
- No double-accidental glyph in displayed labels after substitution; valid single accidentals remain.
- Correct dominant/major/minor/half-diminished seventh qualities.
- Immediate sounding changes and exact equality of audition, stored notes and exported notes.
- Independence of existing blocks after global key changes.

A pinned development-only reference such as music21 may generate independent fixtures after its behaviour is checked. Do not ship Python/music21 as a runtime dependency. Do not use a generic minor-harmony function that raises the leading tone instead of the specified natural-minor scale. Retain fixtures, provenance and regression tests in the public source project.
