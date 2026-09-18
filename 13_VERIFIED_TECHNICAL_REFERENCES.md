# ChordCanvas: verified technical references and evidence limits

**Reference review date:** 18 September 2026.  
**Purpose:** portable primary-source references for the technical constraints in this handover, not proof that ChordCanvas has been implemented or tested.  
**Read with:** [07_TECHNICAL_ARCHITECTURE.md](07_TECHNICAL_ARCHITECTURE.md), [11_ACCEPTANCE_TESTS.md](11_ACCEPTANCE_TESTS.md) and [12_DECISIONS_AND_TRACEABILITY.md](12_DECISIONS_AND_TRACEABILITY.md).

## 1. How to use this register

The sources below were inspected through their official documentation or source-owner publications. Links are included so Codex can re-check the exact APIs and licence text for the revision it actually pins. Documentation at a `master` branch or a general product page can change; record the eventual commit/version and do not silently float dependencies.

User-defined behaviour is grounded in the conversation, not in competitor features. Product choices such as replacement semantics, repeat priority, five octave slots or the dark-theme starting direction are explicit decisions/defaults, not claims that a standard mandates them.

The sources support individual technical facts. They do **not** establish actual Live drag/import results, exact Windows permissions on the owner's machine, a licence grant for this project, availability of GUI automation, or success of any future build. Those are implementation gates.

## 2. Codex instructions and permissions

### S01. OpenAI: custom instructions with AGENTS.md

[Official AGENTS.md documentation](https://learn.chatgpt.com/docs/agent-configuration/agents-md)  
[Original Codex documentation entry](https://developers.openai.com/codex/guides/agents-md/)

Supports the repository instruction-discovery mechanism. Codex reads applicable `AGENTS.md` guidance; the documented default combined instruction size is 32 KiB. A folder full of arbitrary Markdown documents is not automatically equivalent to reading their complete contents. This pack therefore uses a compact root `AGENTS.md` with an explicit instruction to open every specification file. Re-check the installed client's behaviour and any higher-priority local guidance.

### S02. OpenAI: Windows sandbox

[Official Windows sandbox documentation](https://learn.chatgpt.com/docs/windows/windows-sandbox)  
[Original Codex Windows entry](https://developers.openai.com/codex/windows/)

Supports the distinction between running in a Windows environment and having particular filesystem/network/approval permissions. The instruction to work autonomously cannot create credentials or authorisation. This source does not prove that the user's Codex session has an Ableton GUI-control tool.

## 3. VST3, JUCE and host integration

### S03. Steinberg: plug-in locations

[Official VST3 plug-in location reference](https://steinbergmedia.github.io/vst3_dev_portal/pages/Technical%2BDocumentation/Locations%2BFormat/Plugin%2BLocations.html)

Documents Windows VST3 locations, including the standard global Common Files VST3 directory, and relevant write-permission distinctions. It supports separating installed binaries from writable per-user logs. Resolve actual Windows known folders instead of hard-coding a specific account or system drive.

### S04. JUCE: AudioPlayHead::PositionInfo

[Official PositionInfo API](https://docs.juce.com/master/classjuce_1_1AudioPlayHead_1_1PositionInfo.html)

Documents playing/recording/looping state and available timing fields. Many timing values are optional. It does not provide a universal separate “pause button pressed” event or tell the plug-in which Session clip the user clicked. ChordCanvas's bar-one restart rule is a product state machine built on available host observations.

### S05. Steinberg: ProcessContext

[Official VST3 ProcessContext API](https://steinbergmedia.github.io/vst3_doc/vstinterfaces/structSteinberg_1_1Vst_1_1ProcessContext.html)

Documents transport flags, timing validity and musical context. This supports checking validity before using tempo/position and not inventing host events. It does not mandate ChordCanvas's local loop length or position-independent sync behaviour.

### S06. JUCE: DragAndDropContainer

[Official native drag-and-drop API](https://docs.juce.com/master/classjuce_1_1DragAndDropContainer.html)

Documents external file drag initiation and its asynchronous lifetime. Starting a native drag is different from proving that the target accepted/imported a file. Use the appropriate mouse gesture and copy semantics, retain the snapshot file long enough, and test the actual Windows-to-Live interaction.

### S07. MIDI Association: Standard MIDI Files

[Official Standard MIDI Files overview](https://midi.org/standard-midi-files)

Provides the official overview of timestamped MIDI-file interchange. The overview was inspected; this handover does **not** claim to have reviewed a separately gated full specification PDF. Use actual parser/file tests and the APIs below for the proposed export representation. No source here proves Live will preserve a particular clip end marker from an empty/trailing-silent file.

### S08. JUCE: MidiFile

[Official MidiFile API](https://docs.juce.com/master/classjuce_1_1MidiFile.html)

Documents file types, tracks, PPQ time division and tick-based timestamps. Its reader can automatically create missing note-offs by default; disable that repair when validating correctness, or use an independent parser. The exporter must request the intended file type rather than assuming the API's default matches this specification.

### S09. JUCE: MidiMessage

[Official MidiMessage API](https://docs.juce.com/master/classjuce_1_1MidiMessage.html)

Documents note messages and structural End-of-Track creation, as well as configurable note-name conventions. It supports the distinction between exact MIDI integers and displayed octave names, and between unwanted musical metadata and required file structure. Verify the final byte stream independently.

## 4. Music-theory references

### S10. Open Music Theory: scales

[Open Music Theory scale definitions](https://openmusictheory.github.io/scales.html)

Provides major and minor scale interval definitions and discussion of minor variants. The product deliberately selects natural minor only. The existence of harmonic/melodic variants is not permission to add them to the key selector.

### S11. Open Music Theory: triads and seventh chords

[Open Music Theory triad/chord reference](https://openmusictheory.github.io/triads.html)

Supports interval-based triad qualities, seventh-chord distinctions and inversion concepts. Minor-key discussions may include common-practice raised-leading-tone harmony; do not copy such a table as the product's natural-minor palette. ChordCanvas's seven default qualities must be derived from its explicit scale offsets.

### S12. music21: scale module

[Official music21 scale documentation](https://music21.org/music21docs/moduleReference/moduleScale.html)

Documents `MinorScale` as natural minor/Aeolian, making it a possible independent development-only fixture source. Inspect each helper's semantics: a leading-tone convenience function may raise the seventh instead of returning the natural seventh scale degree. Do not ship music21/Python as a required plug-in runtime.

### S13. music21: chord module

[Official music21 chord documentation](https://music21.org/music21docs/moduleReference/moduleChord.html)

Provides an independent implementation reference for chord, pitch and inversion operations. A library's preferred label is not necessarily the owner's display policy. Compare pitches, spelling and intervals explicitly. Reading documentation does not mean the library has been executed against ChordCanvas.

### S14. music21: harmony module

[Official music21 harmony documentation](https://music21.org/music21docs/moduleReference/moduleHarmony.html)

Contains conventional suspended-chord representations and naming examples. Supports checking ordinary sus2/sus4 meanings. The decision to use a perfect fifth when modifying an originally diminished pad is explicitly this pack's engineering policy, not an inference that all such choices must remain diatonic.

## 5. Ableton Live 12

### S15. Ableton: working with instruments and effects

[Live 12 official plug-in/instrument documentation](https://www.ableton.com/en/live-manual/12/working-with-instruments-and-effects/)

Confirms VST3 support and the placement of instrument plug-ins on MIDI tracks with audio output. Also provides host scanning/plug-in usage context. It does not establish universal live MIDI-output compatibility across DAWs, and this product does not depend on that claim.

### S16. Ableton: managing files and Sets

[Live 12 official MIDI-file import documentation](https://www.ableton.com/en/live-manual/12/managing-files-and-sets/)

Explains MIDI-file import and that imported MIDI data becomes part of the Set rather than remaining linked to the original file. This supports eventual safe temporary-export cleanup after consumption. The source does not promise the intended eight-bar clip extent when notes end earlier, or the creation of a fully empty clip; those remain actual-host acceptance tests.

## 6. Licensing and publication

### S17. JUCE: product/licensing overview

[Official JUCE plans and licensing FAQ](https://juce.com/get-juce/)

Provides the current licensing entry point and explicitly directs readers to the binding licence terms rather than treating the FAQ as a substitute. Prices, eligibility and plans are not fixed assumptions in this handover. Check the actual chosen revision and owner's rights.

### S18. JUCE: JUCE 9 EULA

[Official JUCE 9 licence](https://juce.com/legal/juce-9-licence/)

The official current EULA was inspected as a relevant licensing source. This does not require the project to pin JUCE 9 or establish that the user qualifies for a particular plan. The terms corresponding to the selected implementation/dependency revision and licence route must be checked before use and distribution.

### S19. GitHub: licensing a repository

[Official GitHub repository licensing guidance](https://docs.github.com/en/repositories/managing-your-repositorys-settings-and-features/customizing-your-repository/licensing-a-repository)

Explains why repository visibility is distinct from granting a licence. The request for a public ChordCanvas repository does not itself authorise Codex to choose any copyright licence on the owner's behalf. Keep project and third-party rights as an explicit gate; do not insert an arbitrary MIT licence simply because a template includes one.

### S27. JUCE: source-owner framework licence file

[Official JUCE source licence file](https://raw.githubusercontent.com/juce-framework/JUCE/master/LICENSE.md)

The inspected framework source licence explicitly describes dual licensing of JUCE modules under AGPLv3 or the JUCE licence and identifies third-party dependencies. This is the direct source for that dual-licensing statement, rather than an assumption from “public source”. Re-check the licence file at the actual pinned commit and resolve the authorised route; this handover is not legal clearance for a particular distribution.

## 7. Windows packaging and diagnostics

### S20. Inno Setup: CloseApplications

[Official CloseApplications setting](https://jrsoftware.org/ishelp/topic_setup_closeapplications.htm)

Documents application-closing behaviour and the risk of force-closing programs with unsaved work. Packaging must explicitly avoid an unattended path that closes Ableton and loses the user's Set. Inno Setup is an example mature packager, not a requirement to invent custom update machinery from scratch.

### S21. Inno Setup: 64-bit install mode

[Official architecture/install-mode setting](https://jrsoftware.org/ishelp/topic_setup_architecturesinstallin64bitmode.htm)

Supports selecting the correct 64-bit install mode/paths for the x64 payload. A broadly “compatible” architecture switch must not be used to claim Windows ARM support that was never tested or requested.

### S22. Microsoft: MiniDumpWriteDump

[Official minidump API and cautions](https://learn.microsoft.com/en-us/windows/win32/api/minidumpapiset/nf-minidumpapiset-minidumpwritedump)

Warns about dumping an unstable process from within itself and recommends a separate process where possible. This supports a best-effort crash-evidence design rather than a guaranteed crash report or a host-wide handler takeover. Dump creation and privacy must be validated separately from ordinary text logging.

### S23. Microsoft: known folder identifiers

[Official KNOWNFOLDERID reference](https://learn.microsoft.com/en-us/windows/win32/shell/knownfolderid)

Supports resolving LocalAppData and installed common/program folders with Windows APIs. A path example in the specification is not permission to assume every computer uses `C:` or the same user profile name.

### S24. Microsoft: User Account Control

[Official UAC behaviour](https://learn.microsoft.com/en-us/windows/security/application-security/application-control/user-account-control/how-it-works)

Explains elevation/consent mechanics. A minimal-interaction installer can still require legitimate Windows approval; a prompt cannot eliminate it. The normal running plug-in should not require elevation.

## 8. GitHub automation

### S25. GitHub CLI: repository creation

[Official gh repo create reference](https://cli.github.com/manual/gh_repo_create)

Documents repository creation and visibility/source options. Use the actual authenticated owner, inspect existing repositories first and verify the result. The handover supplies future instructions; it has not itself created the user's remote repository.

### S26. GitHub CLI: release creation

[Official gh release create reference](https://cli.github.com/manual/gh_release_create)

Documents release/tag/asset behaviour. Ensure the tag points to the tested commit; do not rely on an automatically created tag targeting the wrong branch. Verify uploaded asset hashes against the actual tested packages.

## 9. What remains unverified by this research

No ChordCanvas executable has been compiled, scanned, auditioned, installed, updated or UI-tested as part of writing this pack. No owner GitHub repository or public software release has been created by this handover task. No production sound assets have been selected or licensed. No claim is made that music21 or the future C++ engine has passed the acceptance suite.

In particular, full clip boundaries in Live, no-note export behaviour, exact transport notifications, sandbox/UI-control capabilities and safe crash capture require direct execution evidence. The pack marks them as gates and specifies how to test them instead of disguising them as established facts.

During implementation, record any source/API/rights change affecting the specification. Verify first; then update the relevant technical document, decision register and tests together. Do not reinterpret an absent search result as proof that a feature is impossible or already supported.
