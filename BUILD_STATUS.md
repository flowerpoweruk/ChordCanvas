# ChordCanvas: build status and continuation ledger

## Current execution record — 18 September 2026

### Iteration 15 — official validator correction and inspected native MIDI import

Iteration 14 committed/pushed as **6717f55f5c3a05ef0aacb67b4db2e319add318dc**; actual remote main matched. Clean reviewed native build actually **exit 0 / 13 of 13 CTest PASS / 46.76 s**. Pinned Steinberg VST3 SDK **v3.8.0_build_66 / 9fad9770f2ae8542ab1a548a68c1ad1ac690abe0**, MIT licence read, development-only validator built successfully with **43 self-tests PASS**. Validator SHA256 **38a2eda8595d8643b62c318f9817243859a9871ca972b0bff6c3293650d57d4d**. Initial actual product validation: **46 PASS / 1 FAIL**, unnamed JUCE factory program. Corrected VST3 program count to zero using the pinned wrapper's supported zero-program path, preserving the exclusion of factory presets. Actual native rebuild **exit 0**, official validation **47 PASS / 0 FAIL / exit 0**, corrected development plugin SHA256 **0694fb993900bab091ba29dad9e0c535648eff01d7232113befb5ebe0b872d09**. This hash predates the next clean committed build receipt.

Later actual inspection resolves iteration 14's import uncertainty: native drag DID create an Arrangement MIDI clip in Live 12.4.3. Clip Detail shows simultaneous C3/E3/G3 and velocity 100, start 1.1.1, end 2.1.1, length **one bar**, although the actual exported file requests **eight bars / EOT 30,720**. Basic import works; full trailing-rest extent **FAIL**, consistent with the earlier browser tests. No fabricated padding notes or manual clip resizing applied. Saving/exporting remains disabled; owner Live work was not closed. Complete GUI/DPI, installed package/update/clean-machine and release acceptance remain open.

Next: clean committed native build, genuine product Setup.exe and self-contained Updater.exe compilation, actual installation and installed host testing. Licensing authority is settled; local candidate packaging clearance established in iteration 14. No completed product installer claimed before its actual creation and tests.
### Iteration 14 — actual corrected VST3 load, native processing and first GUI review

Iteration 13 committed/pushed as **a492b65cddae7b50f654171b6c61b0d5d32511c2**; remote main actually matched. Acquired free Community C++ toolchain builds the separate clean `build/plugin-next` inputs successfully, actual **exit 0 / 13 of 13 CTest PASS / 47.22 s**. Real factory/component/controller buses and processing lifecycle pass the native probe at 44.1/48/96 kHz, zero/nonuniform buffers and incoming MIDI with fresh silent output. Editor creates/attaches successfully. The actual instrument scans/opens in **Live 12.4.3**, reports stopped **120 BPM**, inserts exactly one C block on pad double-click, and audition drives track/Main audio meters. Closing/reopening only the editor retains that block/eight-bar document. No owner Live closure or forced DLL replacement: new separate build/scanning directory avoided the existing lock.

First real rendered review found broken minus symbols. Pinned JUCE constructor inspection identified UTF-8 passed to its ASCII `const char*` overload. Typed UTF-8 minus/middle-dot and explicit MSVC `/utf-8` fix it; button typography increased to 14 and toggle/dial focus borders added. Actual dirty development `build/plugin-reviewed` **exit 0 / 13 of 13 PASS / 48.15 s**, native processing probe **exit 0 PASS**, actual native HWND view/Settings reviewed with corrected symbols/accessibility text. Independent music21 **10.5.0 / 30,870 note-and-Unicode-label combinations / 42 written scales PASS**; standalone core **792,659 assertions PASS**. Validated ASCII/GUID narrowing warnings corrected. One remaining synthetic ParticipantTests warning corrected and actually rebuilt warning-free; focused participant **1/1 PASS / 4.59 s**. First focused command failed because ctest was absent from PATH; explicit tool path rerun passed. Full mandated DPI/dense/gesture/listening review remains unfinished.

Actual export handle generated immutable whole eight-bar **53-byte** `chords.mid` with three C notes and endpoint **30,720**, then native completion truthfully reported import unconfirmed. The short automated gesture did **not** establish an inspected imported clip. Do not relabel the resulting host time selection as an import. Earlier actual Live browser trailing/empty extent failures remain open. Host saving/exporting still visibly disabled; no activation attempted. Private real host logs/screenshots remain ignored.

Read full acquired Community/runtime linkage/redistribution documents and inspect actual reviewed VST3/helper AMD64 imports: Windows OS DLLs only; static Release `/MT`, no developer tools, debug runtime or VC DLL prerequisite enters payload. Pinned notice generation actually PASS; local product candidate packaging rights established, with Microsoft runtime rights retained. This does not prove clean-machine/installed runtime or stable public release acceptance. Full acceptance catalogue re-read. Sanitized observations/hashes/limits: `engineering/native-review-evidence.json`; acceptance ENV-02 and STATE-03 updated for the actual development binary only.

Next: freeze reviewed source, rebuild clean matching committed inputs, compile genuine **Setup.exe / Updater.exe**, execute actual global install/update and installed Live/GUI acceptance. Full MIDI clip extent/empty import, host save/reopen, complete UI/DPI, clean-machine and stable release gates remain open. **No accepted product installer or stable release exists yet.**

Actual first hosted CI run **FAIL** before compilation: the unauthenticated repository-visibility API call exceeded the shared runner IP rate limit. Retrieved actual job log; no compilation/test success inferred. Workflow now uses its existing read-only repository token for that visibility request and selects Enterprise explicitly. This requires no added token permission or financial commitment. Hosted image actually reported `windows-2025-vs2026 / 20260907.229.1`; static workflow validation precedes the next real run. Raw job logs are not committed.

### Iteration 13 — native ABI processing probe and free Community acquisition

Iteration 12 committed/pushed as **fb1c46bf08f2661681d0d5708fe12492623c76bc**; actual remote main matched. The owner renewed the instruction to proceed through delivery after the free Community licensing route was explained. Proceeded with the necessary free Microsoft-signed Community prerequisite under that end-to-end authority; no paid commitment. Rechecked the exact bootstrapper SHA256 and valid Microsoft signature, then launched `--passive --wait --norestart` with native C++/Windows 11 SDK components. Actual Windows consent process is waiting for the owner's UAC approval; no consent automation, installation success or licence acquisition completion is assumed. Private launch/result records remain ignored.

Extended the development-only native VST3 host with `--probe-processing`: actual bus inspection, stereo activation, float processing lifecycle at 44.1/48/96 kHz and zero/1/17/127/256/1024-frame calls. It supplies a clearly synthetic later host position/tempo and incoming note event to check that a fresh instance stays silent and ignores incoming MIDI. It opens no audio device and does not replace Live acceptance.

Actual LLVM harness compile **PASS**, script parser **PASS**; 16 upstream SDK pragma-pack warnings, no project diagnostic. Existing older DLL initializes the component/controller/connections, then correctly fails the required event-input-bus precondition, **exit 1**. This independently reproduces the previously observed actual Live failure. Missing explicit module refusal **exit 1 PASS**. Corrected native bus/lifecycle/MIDI-ignore checks remain **NOT RUN**, rather than being inferred from the added probe. Sanitized hashes/procedure/results: `engineering/native-processing-evidence.json`; raw outputs stay private/ignored.

Community bootstrapper actually completed **exit 0**. `vswhere` independently confirms complete Community **18.10.12210.168**, C++ workload present and no reboot required. The free OSI route is now acquired. Build selection prefers an installed Community/Professional/Enterprise edition rather than standalone Build Tools. Added a standard public-repository `windows-2025` native compilation workflow with pinned checkout/CMake and read-only permissions; YAML/embedded PowerShell validation PASS. It uploads no binary/private logs and claims no host acceptance. Actual CI run NOT RUN. Installer API probe now tests Windows Server refusal in hosted CI, preserving the shipping Win11 restriction; actual local native DLL probe **1/1 PASS, .09 s**. Server branch NOT RUN locally. Initial workflow parser dependency was absent; installed isolated PyYAML 6.0.3 and then validated successfully.

Next: rebuild clean committed inputs in a separate directory, retest editor creation/processing and actual Live/GUI, then build and execute genuine VST3 packages. Owner-safe Live closure, full trailing/empty MIDI clip extent, disabled host saving, clean-machine/installed package/runtime/release gates remain open. **No accepted Setup.exe/Updater.exe or stable release exists.**

### Iteration 12 — actual native editor-creation failure and initialization correction

Iteration 11 committed/pushed as **d9116af28d1135309af09a782f7c4e0df3bf021a**; actual remote main matched. Community installation authority/UAC remains pending. No further Microsoft project compilation, owner Live closure or binary distribution.

Built a minimal LLVM development-only native VST3 view host using the pinned SDK interfaces, IHostApplication, connected component/controller, IComponentHandler and HWND frame. It explicitly identifies itself as a development view host, opens no audio device, supplies no pretend Live clock and never substitutes for actual Live acceptance. `Scripts/build-vst3-view-host.ps1` actual compile **PASS**; PowerShell parser **PASS**; explicit absent-module refusal **exit 1 PASS**. Initial missing Windows DPI declaration corrected by an explicit supported Windows target. Eight upstream SDK pragma-pack warnings remain documented.

The existing unchanged first-build VST3 (**f41e1987... / numeric 0.1.0.0**) initializes its factory/component/controller/connections, then **crashes at createView(editor)** before rendering. Actual native process **exit -1073741819 / 0xC0000005**, reproduced under bundled LLVM LLDB in only our disposable process. Actual fault reads address 0x40 with rcx=0; optimized DLL lacks available function/line PDB, so nearest-export debugger labels are not asserted as function attribution. Owner Live remains running; no owner work touched. Raw debugger/session material remains private/ignored.

Pinned JUCE source confirms `setResizeLimits` immediately constrains zero-size bounds and can invoke `resized()`. Our constructor called it before constructing the pad children, while layout dereferences every pad. Moved resizability/limits after all child construction and callback wiring. This corrects the source-identified unsafe initialization path consistent with the observed failure; **corrected native compilation/retest NOT RUN**. No rendered screenshot/gesture/design pass claimed. Evidence and exact binary/harness hashes: `engineering/editor-creation-evidence.json`; design record updated.

The preceding unchanged core suite remains **13/13 PASS / 17.75 s**, and isolated actual Inno scenarios **13 PASS**. These do not verify the uncompiled editor correction. Continue with the pending free Community installation, separate-directory native build and actual editor/Live retests; full MIDI clip extent, disabled Live saving, installed product/elevation/clean-machine/runtime/release gates remain open. **No accepted Setup.exe/Updater.exe or stable release exists.**

### Iteration 11 — actual isolated Inno execution and observed uninstall completion

Iteration 10 committed/pushed as **62a9d1d36bcb973d3c861eb2c457f4443fe29b12**; actual remote main matched. Owner-authorised AGPL/public source remains in effect. Investigated Microsoft Community 2026 terms in full: individual use and OSI-licensed application development provide a free route. Requested minimum authority to install Microsoft-signed Community (verified valid signature; SHA256 **e99867faceaa394f1c5b22b83ffacaf6d81b0e5f847b71e99123ca0d96289433**). Pending response/UAC; no Community installation or Microsoft project compilation performed. Live remains open; no owner work closed. A new separate native build directory can avoid the old development DLL link lock once licensing is resolved.

Shared the production transaction event script with an explicit development-only Inno fixture. Its adapter is pinned at build time to a generated project build/UUID directory and owner marker, with a distinct per-user uninstall identity and HKCU entry; it cannot use shipping Program Files/HKLM paths. Production packaging never builds or includes this adapter. No synthetic production-path installer was executed.

Actual **13 disposable Inno process scenarios PASS**: no-install updater refusal, fresh Setup, numeric 1.9.0→1.10.0 full-payload update, identical-version no-op, downgrade/corruption/file-in-use refusals, late update finalisation failure **exit 66** with byte-exact previous files/registration restored, retry to 1.11.0, real uninstall, late fresh finalisation failure **exit 66** restoring absence, fresh retry and second real uninstall. User-owned/unrelated-vendor sentinels retain their hashes; actual uninstall worker logs close successfully with removed-all/no-restart, owned directories and registration absent. Existing durable transaction markers are absent afterward. These are **synthetic AMD64 payload/per-user development tests**, not shipping VST3/HKLM/elevation/clean-machine/network-disabled or Live acceptance.

Initial fixture marker CRLF caused ownership refusal; an actual 60-second timeout exposed unsuppressed custom dialogs. Corrected marker and shared production messages to documented `SuppressibleMsgBox(..., IDOK)`; normal interactive messages retained. Only identity-verified own disposable fixture processes were terminated. A second run revealed premature uninstall checking: the launcher returns while its temporary worker finishes. Verification now observes actual worker log completion and absence rather than inferring success from launch exit. Failures preserved in sanitized evidence; raw logs remain private/ignored.

Updated production Setup/Updater definitions both **compiled PASS** with synthetic inputs only; unexecuted/non-deliverable. `Scripts/build-core.ps1` **13/13 PASS, 17.75 s / 791,511 assertions**; independent strict parser **12 synthetic files / five clock changes PASS**. Actual hashes, scenario exits, observed worker completions and limits recorded in `engineering/inno-runtime-evidence.json` and updated core receipt.

Continue: obtain the pending free Community installation authority/UAC, compile the corrected VST3/helper in a separate directory, run actual rendered GUI and Live acceptance, resolve trailing/empty MIDI import and host save restrictions, then execute legally cleared product packages. **No accepted Setup.exe/Updater.exe, stable tag or binary release exists.**

### Iteration 10 — semantic audition/selection diagnostics and failure isolation

Iteration 9 committed/pushed as **d624b97585a540e77e3d4827e93f07d5cf001082**; actual remote main matched. Pending Microsoft entitlement and owner-safe Live closure still prevent corrected native compilation/load. No owner work dismissed, Microsoft project build executed or binary distributed.

Added message-thread semantic observations after valid session operations: pad/block press/release, source ownership, repeats/rate, key/voicing reset/change, local Play/Stop/seek/Sync, selection and clipboard. Suppressed OS repeat produces no fake press. Post-paste/duplicate selection emits after new IDs are selected rather than leaving the diagnostic snapshot at the preceding selection. Added a validated selection API for marquee/empty selection.

Diagnostic callback exceptions are isolated after publication/commit, counted and cannot interrupt owning release or falsely report an already committed musical edit as failed. Actual new integration test PASS for callback failures, earlier/newer source ownership, held/latching modes, transport, clipboard and final selection. Core behaviour and existing musical/transaction tests remain passing.

**Native JUCE source only, uncompiled:** formatter records targets/active sources, keyboard ownership, resolved notes/modifications, old/new key, revision, requested transport/seek serials, repeat latch/rate, selected IDs and callback-failure count. Major selection/pad/key changes emit coherent snapshots; duplicate UI snapshots removed. Feature visibility now appears in snapshots. Actual native semantic log/GUI review is still required; these are not host/UX passes.

`Scripts/build-core.ps1` **13/13 PASS, 17.87 s / 792,535 assertions**; new session_events **.04 s PASS**. Independent parser **12 synthetic files / five clock changes PASS**; build-script PowerShell parser PASS. Current executable hashes and scope in `engineering/core-evidence.json` and `engineering/session-events-evidence.json`. Historical measured audio rows remain in `engineering/clock-evidence.json`; no new performance run was needed for these message-only observer changes.

Review bounded selection inputs at 512, validates into a temporary before replacement and preserves the old selection on refusal. Focused follow-up core/session_events **2/2 PASS, 2.53 s**; formatter ownership uses RAII for diagnostic exception safety (native source uncompiled).

Next dependent work: establish the pending valid Visual Studio route, safely release Live's old failed DLL, compile/link the corrected native instrument/helper, review actual GUI/diagnostics, then execute real packages and installed host acceptance. Further deficiencies found during native review, trailing/empty MIDI import, disabled Live saving and package/release-runtime gates remain open. **No accepted Setup.exe or Updater.exe, stable tag or binary release exists.**

### Iteration 9 — explicit tempo availability and bounded runtime metadata

Iteration 8 committed/pushed as **d10607ecbbd172e135fca23a825f5838aa109b0c**; actual remote main matched. Microsoft entitlement and safe Live closure remain pending; Live still holds the old failed-load DLL. No further Microsoft project build, owner-work closure or binary distribution.

Added primitive bounded audio clock observations for exact tempo, current availability, ever-valid status, host playing, explicit meter, prepared sample rate, actual callback buffer size and bypass. Unchanged callbacks remain quiet; a refused admission is counted without a per-block retry storm. All JSON formatting/resource reads/writes stay on worker/message threads. Four-producer stress now checks the larger double/integer/boolean payload for tearing.

The initial 120 BPM fallback is **preview-only**; timeline waits for a valid host tempo. First valid tempo during an already-playing Sync starts at local zero; later missing/nonfinite timing retains the last valid tempo and phase. Actual clock integration PASS, including **20,000 unchanged callbacks** and queue-full coalescing. Existing direct-equation audio fidelity remains PASS.

Session header records compiled build number/configuration/source/build ID, theory schema, actual native Windows build/architecture, supplied wrapper format/host classification and host executable fixed product version when available, with explicit provenance/unknown values and no executable paths. Guarded header initialization as well as the existing worker storage operations. Synthetic core executables have no version resource; that unknown branch is tested. Successful Live host-version discovery remains unrun.

**Native JUCE source only, not compiled/host-tested:** Processor forwards actual JUCE classification/format and reports prepare/release/bus metadata on message thread. Editor reports unavailable/retained tempo, disables timeline Play before any valid tempo/under unsupported meter, and coalesces dimensions/transform scale/peer platform scale/renderer observations. Actual missing peer stays unknown. These changes do not constitute GUI/host acceptance.

Latest `Scripts/build-core.ps1` **12/12 PASS, 17.93 s / 790,327 assertions**. Independent log parser **12 synthetic files / five actual clock changes PASS**, including exact fractional tempo/fallback/retention/meter/bypass, header metadata and existing rotation/compaction/error correlation. Re-measured all **72 offline audio cases PASS**: worst average CPU fraction **0.275020213**, worst call/deadline **0.5292**, zero guarded C++ allocations/deallocations. Normal-priority offline observations, not a host timing guarantee or full CRT/JUCE allocation proof. Receipts/current hashes and full measurement rows in `engineering/clock-evidence.json` and `engineering/core-evidence.json`.

Corrected VST3/native GUI, actual packages and all outstanding host/release gates remain open. No accepted Setup.exe, Updater.exe, stable tag or binary release. Next: complete further independent semantic diagnostic coverage, then licensed clean native build and actual installed host/package acceptance when pending external constraints are resolved.

### Iteration 8 — production uninstall metadata and native offline package integration

Iteration 7 committed/pushed as **bbd4a277a22207d61b611354f72bc97ad8616033**; actual remote main matched. Owner-safe Live closure and Visual Studio entitlement remain pending. Live still runs; no force-close, further Microsoft project compilation or binary distribution.

Implemented the production immutable installer-directory participant: exclusive durable descriptor/snapshot, prior uninstaller-directory retention, final owned-file receipt, exact known registration/version checks and joint primary bundle decision. A separate pinned retirement marker survives final auxiliary-directory removal; unknown files, reparse points, ownership conflicts and loaded uninstallers stop safely. Mutable logs/progressions never enter this directory.

Actual **23 abruptly exited disposable processes PASS** across fresh/update bundle/registration/sealing and metadata staging/rollback/retirement boundaries. Tests use synthetic AMD64 executables and generated HKCU leaves, not a VST3, actual Inno uninstaller or real product HKLM key. Initial path/registration test failures and a misplaced hook-loop compile failure were corrected; later successful tests do not erase those failures.

Added AMD64 native helper exports for OS/architecture, source preflight, finalisation/abort and safe uninstall preflight. Actual helper **FileVersion 0.1.0.1 / ProductVersion 0.1.0**, DLL load/export/support/version probe PASS; absent-source updater returned **-2**, no installation started. No rollback runs under DLL loader lock. Inno holds the product Global mutex throughout uninstall and rechecks before deletion. Verified pinned Inno source opens .dat exclusively before callbacks; only uninstall exempts that engine-owned file's second content read. Install/update integrity remains strict and tested.

Shared Inno definitions implement fixed known-folder/identity, native x64 Windows 11, compact automatic path flow, admin elevation, offline payload and explicit no application closure/restart. Both definitions **compiled successfully** with synthetic development payloads; private Setup.exe/Updater.exe fixtures were **never executed or delivered**. Generated uninstall rules list exact owned files and empty directories, never common VST3 parent deletion. The product builder checks numeric binary versions, SDK class IDs, pinned compiler/notices, complete native payload validation and clean committed Release build receipts/hashes before creating unaccepted candidates. Actual product-input invocation **REFUSED the stale 0.1.0.0 VST3** before output creation. Positive product packaging path remains unrun.

`Scripts/build-core.ps1` latest **11/11 PASS, 18.26 s / 792,363 assertions**; native recovery-error follow-up **1/1 PASS .12 s**. Independent strict parser **12 synthetic logs PASS**. Native payload inspector PASS on a synthetic 1.10.0 owned AMD64 manifest; wide-character Windows paths supported. PowerShell parser PASS. Receipts/hashes/scope in `engineering/installer-evidence.json` and updated `engineering/core-evidence.json`. LLVM helper remains development-only; its Windows/UCRT imports include a private UCRT contract, and clean-machine runtime/redistribution acceptance is unrun. Inno's actual installed licence permits any-purpose use; compiler branding does not replace that inspected licence text.

Fresh independent rechecks also PASS: music21 10.5.0 all **30,870 voicings / 42 written scales**, and mido 1.3.3 **three exact full-extent EOT fixtures**. These file/oracle checks do not change failed actual Live trailing/empty import evidence.

Remaining: licensed clean Microsoft VST3/helper build, corrected actual Live load, package execution/elevation/offline update/cancellation/rollback/uninstall, actual native GUI/listening/DPI/fresh-start/drag acceptance, trailing/empty clip import and disabled Live save gates, runtime/dependency clearance and release. **No accepted Setup.exe, Updater.exe, stable tag or binary release exists.** Continue independent diagnostics/GUI source work while pending external gates remain.

### Iteration 7 — typed registry snapshots and joint durable decision

Iteration 6 committed and pushed as **9f7170be704d65660c4cac0a3fbaf22f06b4e727**; actual remote main matched. Pending owner-safe host closure and Microsoft entitlement confirmation remain unresolved; no further Microsoft project compilation or binary distribution.

Added bounded native 64-bit registry leaf images with typed byte-exact values and durable exclusive snapshot writes. Unknown values/subkeys, unsupported types, malformed/truncated snapshots and size violations stop safely; no recursive key deletion or ACL changes. Names are printable ASCII for the owned installer schema; Unicode value contents are preserved. Registry capture checks last-write consistency, and review tightened cumulative capture/file-read allocation bounds.

Extended the primary bundle journal to pin a metadata participant descriptor digest. The participant must verify its recovery data before the bundle can change, share the primary commit/rollback decision, validate final version consistency and retain its descriptor until primary marker removal. Standalone recovery cannot silently discard required metadata. This development journal schema now has six fields; no released installer used the preceding five-field schema.

Actual **eleven abruptly exited disposable child processes PASS** for joint bundle/registry recovery: five boundaries each for fresh/update, plus changed-descriptor obstruction/retry. Wrong final metadata version prevents commit and restores both prior identities. Tests use generated **HKCU** fixture leaves; no real product uninstall key or HKLM key was touched. Production uninstaller-directory participant is still outstanding; these are not accepted package tests.

First compilation failed on ambiguous binary helpers and a mixed auto declaration in a fixture; corrected, then `Scripts/build-core.ps1` **9/9 PASS, 10.54 s / 792,535 assertions**. Reviewed-bounds follow-up registry/participant **2/2 PASS, 1.84 s**. Strict parser on **12 synthetic logs PASS**. Actual updated hashes and receipts in `engineering/core-evidence.json` and `engineering/metadata-evidence.json`.

Next: production metadata/uninstaller backup, native Inno integration and offline package definitions, followed by actual executable tests. Corrected VST3 and all mandatory Live/GUI/release gates remain open. Setup.exe/Updater.exe are unfinished; no stable release is claimed.

### Iteration 6 — measured sound-engine performance and fidelity

Iteration 5 committed and pushed as **24a5981aa97a3a03de69e3d126ef584b030f0c19**; actual remote main matched. Pending owner-safe Live closure and Visual Studio entitlement confirmation remain unchanged; no further Microsoft project build or binary distribution.

Measured 72 offline cases at 44.1/48/96 kHz, buffers 64/256/1024, all four sounds, dense 32-bar four-tone timeline edits and fastest repeats at stress tempo 999 BPM. Production bounded audio diagnostics attached. Baseline worst average CPU fraction **0.750503258**, worst measured call/deadline **1.845**: an actual performance defect. Cached per-articulation frequency/envelope coefficients, fixed prepared 8192-entry interpolated sine table and inaudible envelope floor remove repeated expensive synthesis work. Original harmonics/Nyquist limit, four distinct sounds and note timing preserved.

Final `Scripts/measure-audio.ps1` **PASS**: worst average CPU fraction **0.326283710**, worst call/deadline **0.822300000**, zero guarded C++ allocations/deallocations. These are normal-priority offline observations on an AMD Ryzen 9 270 (8 cores/16 threads), not a universal timing guarantee or Live callback pass. The guard covers C++ new/delete, not arbitrary CRT malloc or the JUCE/host wrapper. Baseline development executable was rebuilt before hashing; its hash is unknown, while measured JSON/source blob and final executable/source hashes are recorded honestly in `engineering/audio-evidence.json`.

Independent direct-equation fidelity **PASS**: 72 held/released sound/register/rate cases, four-tone chords, nonuniform 17/127/256-frame blocks; **1,128,600 samples**, max absolute difference **2.98023e-8**, RMS **9.85111e-10** (limit 2e-6). Follow-up passed after correcting a test pattern that initially skipped the intended 127-frame buffer. `Scripts/build-core.ps1` **7/7 PASS, 8.52 s / 792,361 assertions**; independent parser **12 synthetic logs PASS**. Relevant tests cover release expiry, repeat timing, safe bypass and unchanged timbre RMS/peaks. Actual listening, installed host stress, GUI/DPI and native drag acceptance remain unrun/failed as previously recorded.

No completed Setup.exe or Updater.exe is claimed. Next independent work: installer metadata/uninstaller participation and native adapter; resume corrected VST3 link/Live proof once pending external gates are resolved.

### Iteration 5 — retained finalisation and actual interrupted-bundle recovery

Iteration 4 was committed and pushed successfully as **7fef8c2794c0bbf26581fd47d82e76ff3d3edc02**; `git ls-remote` confirmed matching main. Source and honest first-build/host-failure receipts are public. No binaries were distributed. Owner Live closure and Visual Studio licence confirmation remain pending; Microsoft project compilation is paused.

Extended the packaging core with a transaction object retaining the old bundle and mutex through installer finalisation. A bounded, flushed ownership journal precedes changes; uncommitted scope exit restores the old bundle, and a later process recovers known interrupted replacements. Committed but locked cleanup retains at most one backup and marker; another transaction must recover before staging more. Used a product-specific **Global** mutex for the global installation path rather than a session-local mutex. No privilege or security setting was changed. Complete metadata/uninstaller participation is still outstanding; this is not a finished installer.

Actual integration checks now launch this project's disposable child test executable and abruptly exit at real operation boundaries: **nine interruption/recovery cases PASS**, including each of four update and four fresh-install boundaries plus an unexpected-file obstruction/retry. Cross-process mutex refusal, retained finalisation rollback and bounded locked-backup cleanup PASS. Unknown files/journals are preserved, as are unrelated vendor fixtures. Payloads remain synthetic AMD64 test executables, never claimed as a VST3. Added captured receipt hashes and refusal of ownership-receipt case aliases; owned bundle manifest format uses printable ASCII names, while user progression paths are unaffected.

A first run failed export_cache after Windows reused a PID and the fixture found retained files from an earlier test. This was a real isolation failure. All I/O integration fixtures now use exclusive GUID workspaces. Latest `Scripts/build-core.ps1` **6/6 PASS**, **15.65 s / 792,757 assertions**; follow-up payload/transaction checks after the Global mutex change **2/2 PASS, 3.05 s**. Five repeated export isolation checks **PASS, .22 s**. Independent strict parser on 12 synthetic logs in `build/core/log-tests-32660-584D8705-7BB8-448B-BB0A-B64790E4C9A8` **PASS**. Physical power loss is not tested; incomplete markers stop safely. See `engineering/packaging-evidence.json` and updated executable hashes in `engineering/core-evidence.json`.

Remaining release gates: toolchain licence verification, owner-safe Live restart, corrected VST3 link/load, native drag extent behaviour, actual GUI/audio/DPI/fresh-start acceptance, complete metadata/uninstaller rollback, native offline packages and their installed tests. Setup.exe/Updater.exe remain unfinished. Continue independent performance/installer implementation; do not label any package accepted while these gates are open.

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
