# ChordCanvas: instructions for Codex

You are building the complete ChordCanvas product, not a prototype or an implementation plan.

## Mandatory reading

Read [00_MASTER_BRIEF.md](00_MASTER_BRIEF.md), then every numbered specification `01_` through `13_` before implementation. Read [BUILD_STATUS.md](BUILD_STATUS.md) at the start of each resumed session. Do not assume arbitrary Markdown files are automatically included in your context. The master brief lists their ownership and read order.

The compact size of this file is intentional. Keep the full specification in its separate files rather than duplicating it into this entry point. For Codex instruction discovery, see source S01 in [13_VERIFIED_TECHNICAL_REFERENCES.md](13_VERIFIED_TECHNICAL_REFERENCES.md).

## Product constraints

- Product: **ChordCanvas**. C++/JUCE **VST3 instrument**, Windows 11 x64, tested specifically in **Ableton Live 12**.
- Deliver functioning `Setup.exe` and a separate, self-contained `Updater.exe`; code alone is not delivery.
- Create a **public** repository called exactly `ChordCanvas` under the authenticated owner's GitHub account. Commit and push meaningful iterations, not just the final result.
- GUI/UI/UX and distinctive design language are first-class engineering and release criteria. No late cosmetic skin, generic dashboard layout or placeholder control styling.
- Respect all final user decisions and explicit exclusions. Do not reintroduce vocal import, key detection, automatic chord suggestions, automatic project restoration, a clear-timeline button or a loop toggle.
- Defaults and technical corrections are identified in the specifications. Do not claim the owner explicitly chose an engineering default.

## Working behaviour

Act autonomously on ordinary implementation decisions. Inspect documentation and existing state instead of asking the owner questions already answered in the specifications. Keep working through implementation, testing, fixing, packaging and release.

Use the tools actually available in the authorised environment. Do not claim to have mouse/keyboard control, Windows administrator rights, Ableton access, GitHub authentication or code-signing credentials without verifying them. Respect permission boundaries. Never bypass security, create paid commitments or force-close the owner's unsaved work.

Maintain `BUILD_STATUS.md` with actual commands, test outcomes, exact host/tool versions, remaining defects and release gates. A blocker in one external dependency does not justify abandoning independent implementation or testing work.

## Before each meaningful commit

Run the relevant tests; inspect the diff; check for secrets, personal paths, real logs, crash dumps and uncleared assets; update the work ledger. Commit only project changes. Do not overwrite unrelated work or rewrite published history.

## Before claiming completion

Read [11_ACCEPTANCE_TESTS.md](11_ACCEPTANCE_TESTS.md) again. Confirm actual Live 12 testing, MIDI drag/drop including leading/trailing rests, fresh-start semantics, installation/update behaviour and real GUI review. Record failures and unrun tests honestly. Never fabricate screenshots, release URLs, hashes, successful commands or host-testing evidence.

## Code review rules

Treat these as blocking defects: wrong notes or spelling; accidental dependence of existing blocks on the current key; hidden overlapping blocks; wrong clipboard/trim behaviour; audio-thread blocking; hanging notes; host-position sync; stale exported MIDI; destructive update behaviour; project-state restoration contrary to the spec; public exposure of private diagnostics; and unreviewed, generic-looking interaction design.
