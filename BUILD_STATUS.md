# ChordCanvas: build status and continuation ledger

## Current execution record — 18 September 2026

### Iteration 4 continuation — actual Microsoft build and failed host load

First `Scripts/build-plugin.ps1` exited **0**: Release AMD64 VST3 linked and all six Microsoft CTest suites passed, **27.70 s / 789,531 assertions**. Independent music21/mido/log parsing passed. Component/controller IDs and SDK-generated module metadata inspected; DLL imports only Windows OS libraries. Actual first DLL SHA256 `f41e1987a81c2b2422b32f7941c43644a00f66ae4296e0caa938b42efedab17d`. This is development compilation evidence, not release or clean-machine acceptance.

**Live load FAIL:** Live 12.4.3 scanned ChordCanvas and displayed it as VST3, but could not open it. Private loader evidence specifically says the processor loaded and the instrument has no valid event input bus. Corrected JUCE input-bus declaration while continuing to ignore incoming notes. Both subsequent rebuilds failed at link with **LNK1104** because Live holds the failed DLL open. Requested owner close Live safely; no force-close or unsaved-work dismissal. Custom VST3 scanning was enabled for this development test; it was originally off, as was system VST3 scanning. Review/restore the test preference after installed-host testing.

Generated Windows numeric version now derives from release metadata as **0.1.0.1** (build 1); original locked DLL remains **0.1.0.0**. The new resource and event-bus fixes have not been linked/host-tested. Reduced resize edge widths on extremely narrow blocks to retain audition/body targets; optional controls increase minimum height to 608 to avoid a clipped lane. Prepared pinned upstream runtime notices, including explicit IJG acknowledgement; package/compiler distribution clearance remains pending.

**New verified toolchain licensing gate:** the actual official Microsoft 2026 Build Tools terms require a valid Visual Studio Community/Professional/Enterprise licence for development of this project's own source. Standalone Build Tools exemption covers third-party open-source dependencies. Earlier installation authority and AGPL approval do not prove that Microsoft licence. Asked owner which valid edition covers the machine and paused Microsoft project builds. A separate `build/plugin-next` build was stopped after identifying that gate. No Microsoft-built binary has been distributed. Terms: https://visualstudio.microsoft.com/license-terms/vs2026-ga-diagnostic-buildtools/ . Independent LLVM core/packaging work remains available. Existing Build Tools installations are preserved.

Follow-up diagnostics regression additionally verifies that session.end exists; diagnostics CTest **PASS 2.03 s**, independent parser on 12 synthetic files in `build/core/log-tests-3896` **PASS**.

Latest independent `Scripts/build-core.ps1`: **6/6 PASS**, **13.19 s / 792,591 assertions**. Strict parser on `build/core/log-tests-16640` passed 12 synthetic files; music21 10.5.0 all 30,870 voicings/42 scales and mido 1.3.3 three exact EOT fixtures passed again. `engineering/core-evidence.json` contains actual refreshed executable hashes. `engineering/vst3-build-evidence.json` records first build PASS separately from host FAIL and paused retries. ENV-01 recorded PASS for inspected environment/output; ENV-02 FAIL for actual host load. Other host/GUI/native drag tests retain their earlier honest status.

No accepted Setup.exe, Updater.exe, stable tag or binary release exists. Source iteration is being reviewed for a meaningful WIP commit; packaging still needs metadata/uninstaller transaction, durable recovery, OS checks and actual executable tests. Await owner Live closure and Visual Studio licence verification for further Microsoft host work; continue independent implementation/tests meanwhile.

### Iteration 4 — native editor in progress and tested packaging foundations

Owner explicitly authorised the Microsoft-signed Visual Studio 2026 C++ Build Tools installation and agreed to handle UAC. First launch from a redirected per-user path failed before consent; copied the identical signed bootstrapper to ignored workspace tools and launched with normal RunAs. Actual installer exit **0**, with `vswhere -products '*'` confirming complete, launchable Visual Studio Build Tools 2026 **18.10.1 / 18.10.12210.168**, no reboot required. Bootstrapper SHA256 `c71a953a56448193b971b21a5a79ed8c32b4376b9e88fb0365e5c46a6013a804`, valid Microsoft signature. **Preflight correction:** the initial `vswhere` default-product query excluded Build Tools. The corrected query also discovers an older complete 2022 Build Tools installation in a nonstandard location; the earlier inference that all MSVC was absent was incorrect. Newly selected compiler actually reports **MSVC 19.51.36257.0**; build script selects x64 and SDK **10.0.26100.0**.

Downloaded pinned approved JUCE 9.0.2 and wrote the actual VST3-only processor/editor integration. JUCE's MinGW rejection was respected without patching upstream. New native editor has seven degree colours, independent pad/block controls, collision previews, selected-chord editing for very narrow blocks, a separate layered popover, transport/settings, manual persistence and native MIDI drag. Fixed unrelated gesture releases of keyboard previews and retained Repeat controls in Settings to avoid trapping a latch. Processor host state remains minimal; no host-position sync. Audio diagnostic sink publication is atomic. **Native compilation is running; no VST3 scan, rendered GUI, sound/listening or native host drag PASS yet.**

Export preparation uses unique session/export directories under the per-user Temp root, exact write/read-back validation and exclusive leases captured through native drag completion. Files remain for a 24-hour grace period; stale completed directories are cleaned only by verified owned names/files, never through recursive deletion. Admission is bounded to 256 retained exports. Failed cleanup preserves its lease marker. The new test initially failed because its simulated receiving host still had the file open; added refusal/recovery assertions and closed the reader before expecting cleanup. Packaging compiler errors in path iteration/name lookup/missing exception header were corrected, then rebuilt.

Inno Setup **7.1.0 x64** compiler installed per user into ignored tools and executed successfully. Official download SHA256 `0362a383ed217d4c4239b5933866dd96d3eb2102737da92f80f6057a4b40df2f`, valid **Pyrsys B.V.** signature. Its actual licence permits use and redistribution subject to copyright retention; no purchase or subscription was made. Packaging contains tested release-version ordering, strict owned manifests, Windows SHA256, AMD64 PE checks and same-volume staged replacement with retained old bundle and rollback. Numeric versions are bounded release SemVer; prerelease/build suffixes are explicitly refused by this initial package format. Tests use a development AMD64 executable as a **synthetic payload, not a working VST3**. Final Inno definitions, OS checks, metadata/uninstall transaction, crash recovery, bounded backup cleanup and actual package acceptance remain outstanding.

Independent log parsing caught an intermittent shutdown defect after the C++ suite passed: compaction could place a truncation marker after session.end. Fixed by compacting/reserving terminal space before assigning the end record, and added a final dropped-event summary and regression assertion. Latest independent parser PASS on 12 synthetic files. A prior parser invocation incorrectly supplied the build root rather than a synthetic session directory and failed; this was invocation error, not a PASS.

Latest `Scripts/build-core.ps1`: **6/6 PASS**, total **18.37 s** (core 15.20, diagnostics 1.84, export cache .09, versions .03, payload .26, transaction .86), **789,423 assertions** in the concurrent core run. Verified four real rollback boundaries, absent-install refusal, file locks without host closure, numeric 1.9.0 → 1.10.0 update, identical same version, owned repair, downgrade refusal, unknown-file and other-vendor preservation. Independent music21 10.5.0 and mido 1.3.3 rechecks PASS; log parser `build/core/log-tests-21492` PASS. Actual hashes and tested source `1d29a0a+dirty` are recorded in `engineering/core-evidence.json`. These checks do not pass complete W/U/H acceptance requirements.

Iteration 3 was committed and pushed as `1d29a0a`. No accepted stable release, Setup.exe or Updater.exe is claimed. Live's trailing/empty clip import failures and disabled saving remain release gates. Next: finish Microsoft VST3 compilation, inspect actual editor in Live, complete transactional offline packaging and execute acceptance against installed artifacts.

### Iteration 3 — live session, diagnostics and actual import failures

User authorised resumption after interruptions and reconfirmed the VST3 deliverable. Owner then explicitly authorised **AGPLv3 for ChordCanvas and JUCE**. Applied that route to original source; no paid commitment or assumed JUCE tier. Official latest-release API reports JUCE 9.0.2, published 7 September 2026. Downloaded the exact tag into ignored development dependencies; actual checkout `72782788ce18c2d4d760b28e0921d6ffc6431102`. Transitive notices/build verification remain outstanding.

Completed processor-owned session orchestration: pad ownership, OS-repeat suppression, repeat latch and phase identities, focus cleanup, key changes preserving existing blocks, paste selection, minimal host state/fresh-start semantics. Corrected bypass consumption of stale preview frames, strict grid rejection and signed-boundary validation/slicing. Random command tests now compare complete undo/redo snapshots and note-range invariants.

Added worker diagnostics with a bounded preallocated audio ring, five-session leased-file rotation, active-session preservation, degraded memory fallback/slot recovery and bounded compaction retaining snapshots. Review/parser found and fixed out-of-order compaction replay, duplicated snapshots, lost snapshots on delayed admission, same-second rotation ordering and multiline JSON snapshot records. No host-handler takeover or minidumps added.

Latest `Scripts/build-core.ps1`: **PASS**, core 14.00 s / diagnostics 1.77 s / total 15.83 s; 790,078 assertions (concurrent count varies). Four-producer ring test and actual owned-directory I/O failure passed. `Scripts/verify-logs.py` parsed 12 synthetic files: strict UTF-8/JSON Lines, increasing sequence, monotonic time, newest five ordinals, recovered memory snapshot and replace → resize → undo → export-error correlation PASS. Independent music21 10.5.0 recheck PASS: 30,870 voicings/42 scales. mido 1.3.3 PASS: chords, empty and exact ENV-04 extent fixtures, each EOT at 30,720. `llvm-readobj` inspected the development core executable as COFF x86-64/AMD64/64-bit. These builds carry source stamp `1a35c74dc8b866376ebec3b13fa8db39ad46f828+dirty`, accurately identifying the tested uncommitted iteration; they are not installed release binaries.

**Actual Live 12.4.3 import gate FAIL:** eight-bar `chords.mid` ending its last note at tick 11,520 imported as three bars in Session and Arrangement. Exact ENV-04 `extent-gate.mid` ending at tick 15,360 imported as four bars: Start/Loop Start 1.1.1, End/Loop End 5.1.1, length 4.0.0; expected end 9.1.1/length 8.0.0. Leading/internal rests survived; EOT trailing silence did not. Empty eight-bar file left a Session destination empty. Tests used **Live's browser drag**, not a JUCE plug-in native drag; no instrument playback or cancellation/lifecycle PASS asserted. Arrangement tempo-import prompt answered No, preserving host tempo. No fake notes or manual-resize substitution. Raw host screenshots/accessibility remain private. See `engineering/live-import-evidence.json`.

Live still reports **Saving and exporting are deactivated**; no activation attempted. Save/reopen acceptance is blocked. VST3/editor, native export gesture and package acceptance remain unfinished. Repository commit `1a35c74` was pushed successfully; main was rechecked with `git ls-remote`. Current evidence receipts include actual development executable/fixture SHA256 values. All 148 scenarios now have explicit result records with required evidence layers; passing pure/integration checks does not imply GUI/host/package acceptance. Records produced before the subsequent licensing reply preserve the earlier blocked route; update it as dependency review/build proceeds.

Next: compile the actual approved JUCE VST3 instrument/editor vertical slice and verify scan/native drag in Live, while investigating the clip-extent incompatibility. Source, GUI, packaging and binary release are still incomplete; no Setup.exe/Updater.exe or stable release is claimed. Earlier paragraphs below are historical and superseded by this iteration.

### Iteration 2 — resumed and verified independent core

User authorised continuation after the interruption. Fixed missing LLVM runtime search-path startup by static runtime linking and a development PATH fallback; CTest now completes. Latest core including progression persistence compiled in Release. `Scripts/build-core.ps1`: PASS, 13.46 seconds on the second run. Actual core output reports 30,870 voicing cases, MIDI range 36–125, 20,000 random commands, 30,000 concurrent mailbox publications and 683,816 assertions on the first successful run (concurrent-consumption assertion count varies).

Four original sounds produced finite signal at 48 kHz with observed RMS/peak: Piano .0373431/.169964; Guitar .0174645/.155925; Strings .0279154/.0924241; Pad .0326126/.10709. Automated signal/release checks passed; this is not listening or installed-host sound acceptance.

Independent `Scripts/verify-midi.py build/core`: mido 1.3.3 PASS for exact 12 events, fixed channel/velocities, off-before-on ordering, event allowlist and EOT 30720; empty file has zero notes/EOT 30720. Fixed the initial reporting-only mido version attribute error and reran. `Scripts/verify-theory.py build/core/theory_probe.exe Tests/Fixtures/music21-scales.json`: music21 10.5.0 PASS for all 30,870 voicings and 42 scale spellings; retained independently generated scale fixtures. Initial oracle run completed comparisons but failed writing a missing fixture directory; fixed parent creation and reran successfully. Static executable imports were inspected; LLVM libc++/libunwind DLL imports are absent after static linking.

Standard configured Git Credential Manager provides authenticated **flowerpoweruk** access without interactive login. Credentials were used transiently, never printed or stored in source. Rechecked repository absence via API and created exact public repository **https://github.com/flowerpoweruk/ChordCanvas**. Initial commit `0dd2647` pushed to main successfully. This resolves repository-creation access; browser login is no longer needed for source control. JUCE/project licensing remains unresolved; no JUCE was downloaded or licence assigned.

Live 12.4.3 launched; launch initially timed out waiting for its window but subsequent returned-window discovery succeeded. Inspected the actual disposable Untitled workspace. First screen explicitly reported “Saving and exporting are deactivated.” No licence activation attempted. MIDI extent investigation is ongoing; no plug-in/native-drag or project-lifecycle PASS follows from this observation. Host screenshots stay private.

Previous iteration/handover content below is historical. All current unit results are evidence-layer results, not substitutes for required U/H/W acceptance. UI, diagnostics and Setup/Updater remain under development, and no stable release exists.

**Incomplete development iteration, not a release.** Development source version 0.1.0. The original handover tables below remain historical; this execution record is the current status. No VST3, Setup.exe or Updater.exe has been delivered. All 148 acceptance scenarios are individually recorded as NOT RUN in `engineering/acceptance-results.json`.

Read AGENTS.md, master brief and every numbered specification 01–13 in full before implementation, re-reading truncated sections separately. Initial folder contained only the 16 handover Markdown files and was not a Git repository. Initialised local main without overwriting unrelated work.

Actual environment: Windows 11 Home 10.0.26200/build 26200/64-bit; PowerShell 7.6.5; Git 2.55.0.windows.2. Installed Ableton Live 12 Suite executable reports product/file version **12.4.3**, initially not running. MSVC not found (`vswhere -all -format json` returned `[]`); Windows SDK include directory 10.0.26100.0 exists. CMake, gh and Inno Setup were not found in inspected standard locations/PATH. Current terminal token is not elevated administrator. Current-user code-signing certificate enumeration returned no certificate; no signing authority established.

Downloaded portable official project tools to a per-user development directory outside Git: LLVM-MinGW 20260908 UCRT x86_64, Clang 23.1.1/LLVM commit 6dfe1677ab8dffbc6ec13d53a1e0215d75147689; CMake 4.4.3. Archive SHA256 respectively `1bcf74d06b724aeecaa6412ca85f5b26fb1da770e7cdcefa9263c9c5c3ad34b6` and `4d52ebab7193a698651639ed80d8d04fd903358843572cf44c7fd234cb7c26ab`. Using LLVM-MinGW for the independent core is a development choice because MSVC is absent, not release toolchain acceptance.

GitHub connector authenticated as **flowerpoweruk**. `get_repo flowerpoweruk/ChordCanvas` returned 404. Connector has no create-repository operation. Browser at GitHub repository creation redirects to sign-in; owner sign-in requested and page retained. No remote repository or push exists yet. Repository-local Git author uses the authenticated owner's public noreply identity. No project licence selected on owner's behalf. Official JUCE source licence and JUCE 8 EULA inspected; **JUCE not downloaded, pinned, combined or built**, pending authorised AGPLv3 route or existing JUCE licence/version/tier. Owner licensing question remains pending.

Computer Use skill and all required guidance/API/confirmation documents read. Initialised Windows app control and listed installed Live successfully. A Live launch attempt was stopped by the physical Escape key; no host screen or test outcome recorded. Stopped desktop interaction for this turn; no further desktop input is authorised in the interrupted turn.

### Iteration 1: independent core and design baseline

Source written under development:

- Theory: 42 written key identities, natural minor, theoretical versus display spelling, origin-based triads/sevenths, conventional suspensions, five octave positions, inversions and clamp policy.
- Timeline: validated half-open independent blocks, strict grids, whole-block replacement, group movement, both-edge resizing, slicing, destructive trim, clipboard boundary clipping and 20-transaction history.
- MIDI: type-0, one-track, 960 PPQ, explicit off-before-on events and EOT at full timeline endpoint, including empty exports.
- Audio: original additive preview sounds, fixed voices, triple-buffer UI/audio handoff, sample-driven local/repeat clocks, host-edge sync, meter/tempo handling and preview override. No listening or host acceptance claimed.
- Bounded versioned semantic progression JSON, validated atomic document load and origin preservation. Added after first compilation; compilation still unverified.
- `engineering/DESIGN_SYSTEM.md`: palette, hierarchy, gesture priorities, previews, focus and narrow-block adaptation. No rendered editor exists yet.
- Development metadata with stable manufacturer/product codes and installer UUID; no stable release tag.

Commands and observed results:

1. `git init -b main`; repository-local public author identity configured.
2. `Scripts/build-core.ps1`: CMake configured, theory/timeline/MIDI/audio compiled and core_tests.exe linked successfully. **This was before persistence was added.** CTest started but did not finish normally during this session; no assertion output or result obtained. The test process showed approximately 0.02 CPU seconds after several minutes. A loader/runtime issue is only a hypothesis. Stopped this project's test process during interruption cleanup. Successful linking is not a test PASS.
3. Development-only pip install to ignored `build/python` requested music21 10.5.0 and mido 1.3.3. Reached “Installing collected packages”; verify completion before use. Independent parser/oracle has not run.
4. `git diff --check` completed without output before staging. Final source preserved as explicitly labelled work in progress; no release claimed.

Executable tests written but **not passed**: 30,870 independent interval-oracle voicings; 20,000 random editing commands; 30,000 concurrent mailbox publications; finite/peak/release audio checks; repeat timing at three sample rates/tempos; sync/meter checks; progression round-trip/invalid input; independent mido export verification. Source test counts are coverage intent, not acceptance evidence.

Open defects/risks: diagnose executable startup/hang and runtime DLL requirements; compile persistence; review all modules; measure/optimise audio CPU and balance sounds by actual listening. UI, diagnostics and packaging are outstanding. None of the actual Live, MIDI extent, GUI/DPI, installation/update, crash or public-release gates passed.

Next steps: diagnose test executable startup (inspect DLL imports; supply portable runtime PATH or static linkage), rebuild latest core and run executable plus independent music21/mido checks. Once rights are authorised, implement JUCE VST3/editor vertical slice and prove trailing silence and empty MIDI import in Live early. Continue full editor/diagnostics, transactional offline packages and installed acceptance suite. Resume desktop input only in a subsequent authorised turn. Create exact public repository after owner sign-in and establish an authorised push method without invented credentials.

No Setup/Updater artifact/hash, release URL, successful host command or screenshot is asserted. Public ledger contains no real session logs, host projects or private diagnostics.

---

## Historical handover baseline

**Initial state:** specification handover only.  
**Specification version:** 1.0, prepared 18 September 2026.  
**Product build:** not created by this handover.  
**Repository:** not created by this handover.  
**Required platform:** Windows 11 x64; Ableton Live 12 only.

This file is maintained by Codex during the actual implementation. It reports facts and does not override any specification. Do not replace `NOT RUN` with `PASS` merely because code or a test description exists.

## 1. Read before work

Read [AGENTS.md](AGENTS.md), [00_MASTER_BRIEF.md](00_MASTER_BRIEF.md), all numbered specifications and the acceptance test catalogue. Confirm the applicable local/global instructions and actual environment permissions. Treat confirmed decisions, explicit engineering defaults and verification gates as different categories.

## 2. Current implementation status

| Workstream | Status | Evidence at handover |
|---|---|---|
| Product specification and decision reconciliation | Prepared | This Markdown pack; not software validation |
| Windows/Live/toolchain discovery | NOT RUN | Must inspect actual authorised machine |
| GitHub identity/access/repository creation | NOT RUN | Must inspect actual authenticated account |
| Project/dependency/asset licence clearance | NOT RUN | Owner rights and chosen dependency revisions not established |
| Minimal VST3 instrument build/scan | NOT RUN | No compiled artifact supplied here |
| Native MIDI drag/full clip extent in Live | NOT RUN | Actual host behaviour required |
| Core theory implementation/independent fixtures | NOT RUN | Formula/specification only |
| Design system and real UI iteration | NOT RUN | Design requirements only; no finished GUI supplied |
| Timeline editing/history | NOT RUN | Required behaviours/tests specified |
| Four local sounds/audio scheduling | NOT RUN | No production engine/assets supplied |
| State/manual progression Save/Load | NOT RUN | Fresh-start contract specified |
| Diagnostics/crash evidence/Explorer control | NOT RUN | Required logging contract specified |
| Setup.exe / Updater.exe | NOT RUN | Packaging specification only |
| Actual Live/UI/installer acceptance suite | NOT RUN | Catalogue supplied; not executed |
| Tagged public release and package verification | NOT RUN | No release created |

## 3. Preflight record to complete from evidence

Record only actual discovered values:

| Item | Current value |
|---|---|
| Windows version/build/architecture | NOT INSPECTED |
| Live 12 edition and exact build | NOT INSPECTED |
| Working directory / existing user changes | NOT INSPECTED; keep private absolute paths out of public commits |
| Available terminal/build/UI-automation tools | NOT INSPECTED |
| GitHub authenticated owner and repository | NOT INSPECTED |
| MSVC / Windows SDK / CMake versions | NOT SELECTED |
| JUCE pinned revision and licence route | NOT SELECTED / NOT CLEARED |
| Other dependencies and asset licences | NOT SELECTED / NOT CLEARED |
| Stable plug-in and installer IDs | NOT GENERATED |
| Signing credentials authorised/available | NOT INSPECTED; never record secret material |
| Current source commit / product version | NO IMPLEMENTATION COMMIT / NO BUILD |

## 4. Early gates

| Gate | Required outcome | Status |
|---|---|---|
| G-HOST | Real VST3 instrument loads and sounds in Live 12 | NOT RUN |
| G-DRAG | Native plug-in-to-Live drag imports correct notes | NOT RUN |
| G-EXTENT | Trailing silence/full timeline extent survives import | NOT RUN |
| G-EMPTY | Empty timeline export/host behaviour is truthfully established | NOT RUN |
| G-SYNC | Bar-one local restart and no host-position chasing demonstrated | NOT RUN |
| G-INSTALL | Clean install and safe in-place update proven | NOT RUN |
| G-RIGHTS | Project/dependency/asset rights and public release authority established | NOT RUN |
| G-ACCESS | Required account, filesystem and actual host-control access exists | NOT RUN |
| G-UX | Real rendered interface and complete gestures pass design review | NOT RUN |
| G-CRASH | Supported best-effort evidence demonstrated without host-handler takeover | NOT RUN |

For a blocked gate, record the exact operation, error/evidence, what remains possible and the minimum genuinely necessary permission/input. Do not repeatedly ask the owner about settled product features.

## 5. Acceptance suite status

All listed tests in [11_ACCEPTANCE_TESTS.md](11_ACCEPTANCE_TESTS.md) start **NOT RUN**. Maintain per-test results in a structured evidence file when implementation begins, with this table summarising it.

| Group | Initial status |
|---|---|
| ENV: environment and feasibility | NOT RUN |
| MUS: music theory | NOT RUN |
| TL: timeline and history | NOT RUN |
| AUD: audio/audition/transport | NOT RUN |
| MIDI: file export and actual import | NOT RUN |
| STATE: lifecycle/settings/manual files | NOT RUN |
| UX: actual visual/interaction review | NOT RUN |
| LOG: diagnostics and privacy | NOT RUN |
| REL: installers/updaters/releases | NOT RUN |
| REPO: source/repository/rights/evidence | NOT RUN |
| SCOPE: no withdrawn features | NOT RUN |

Never use a percentage to obscure an untested mandatory host or update gate. Preserve failed test evidence and re-test the exact packaged release after fixes.

## 6. Iteration entry format

For each meaningful implementation iteration, append or maintain an entry containing:

```text
Iteration/date:
Source commit and actual push outcome:
Implemented/changed:
Specification/default decisions affected:
Commands/tests actually run:
Observed results and sanitised evidence locations:
GUI/interaction defects found and fixed:
Open defects or blocked gates:
Next concrete implementation step:
```

This is a template, not a completed test record. Do not put secrets, private logs, user progressions, actual unsanitised Live screenshots or personal machine paths into a public ledger.

## 7. Next actions at handover

Read the complete pack and inspect the authorised Windows/Live/GitHub environment. Resolve actual rights/access gates. Establish and test the minimal VST3/native-drag/installer vertical slice before committing to a full implementation. Begin the interaction design system alongside that slice, not after backend completion.

## 8. Release handoff checklist

Before describing the product as complete, record actual values for the tested version, source commit, repository/release URLs, Setup.exe and Updater.exe hashes/locations, Live/Windows versions, licence/asset notices and test evidence. Confirm version/current changelog consistency and preservation of user files. Distinguish documented best-effort limitations from unresolved mandatory requirements.

Do not supply a false completion report when only the specifications are ready. Conversely, do not use ordinary implementation decisions as a reason to restart the feature interview.

## 9. Specification review performed before handover

The document pack was checked separately from software implementation:

- All 16 Markdown files were parsed; internal file links, code fences, source IDs and test-ID references were checked.
- The final decisions were reconciled against the supplied conversation, with superseded features recorded rather than carried into the build.
- The 66 mapped requirements point to defined scenarios in the 148-test acceptance catalogue.
- The specification's interval arithmetic was recomputed across its 42 key names and 30,870 specified voicing combinations. Those calculations stayed within MIDI 36–125 and matched the stated default triad/inversion fixtures.
- A synthetic SMF example was written and independently parsed with mido to check the stated tick/event/End-of-Track arithmetic. This was not a test of a ChordCanvas exporter or Live's import behaviour.

These checks do not replace the future C++ tests, independent theory-oracle comparison, actual Live testing or installer verification. music21 was not executed in the handover environment; its documentation was used as a reference. All product implementation/acceptance statuses above remain NOT RUN.
