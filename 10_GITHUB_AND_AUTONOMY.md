# ChordCanvas: GitHub and autonomous execution

**Authority:** authorised build workflow, public repository, meaningful iteration and delivery reporting.  
**Read with:** [00_MASTER_BRIEF.md](00_MASTER_BRIEF.md), [07_TECHNICAL_ARCHITECTURE.md](07_TECHNICAL_ARCHITECTURE.md), [08_INSTALLER_UPDATER_RELEASES.md](08_INSTALLER_UPDATER_RELEASES.md).  
**Primary references:** S01–S02, S17–S19 and S25–S26 in [13_VERIFIED_TECHNICAL_REFERENCES.md](13_VERIFIED_TECHNICAL_REFERENCES.md).

## 1. Instruction to Codex

[C] You are responsible for building ChordCanvas end to end in the owner's authorised environment. Implement the full agreed product, create the Windows packages, test the installed plug-in in Ableton Live 12, and keep the public source repository current. Do not turn this into a tutorial, ask the owner to write code, or declare completion after generating a scaffold.

Read all specification files before implementation. Keep a working plan in `BUILD_STATUS.md`, inspect actual tool capabilities and environment state, and make sensible ordinary implementation decisions yourself. The user has already resolved the feature set. Do not repeat the interview or ask whether conventional behaviour should work as specified.

[C] GUI/UI/UX work is continuous, not a final polish task. The repository history must contain actual interaction/design iteration and tests, not only backend code followed by a last-minute skin.

## 2. Authorisation is real, not created by a prompt

[G] The user intends Codex to have control of their development computer. Verify what is actually available: filesystem, terminal, network, installed build tools, Windows UI automation, host access and authenticated GitHub account. The instruction to work autonomously is not evidence that a particular tool can control Ableton's GUI.

Do not bypass sandbox restrictions, authentication, administrator consent, paid licence activation, code-signing permissions or a required owner licensing decision. Do not extract tokens from unrelated files or accounts. Do not broadly disable Windows protections, weaken folder ACLs, run Live as administrator, or delete the owner's data to make a test pass.

If a genuine gate is blocked, record the exact barrier and the minimum required permission or input. Continue independent authorised work where useful, but do not mark the full release complete. An unavailable host or lack of GUI automation is not cured by saying “tested in Live” after a unit test. Never invent a successful installer run, screenshot, command output, release URL or Git commit.

Do not promise background/asynchronous completion unless the execution environment actually provides it. Save a precise continuation ledger when an execution session ends.

## 3. Preflight and ownership discovery

Before creating remote resources:

1. Inspect the working directory and preserve existing uncommitted user work. Determine whether it is already a Git repository and whether any existing ChordCanvas files need a non-destructive merge.
2. Inspect authorised GitHub authentication using the connected tool or authenticated CLI. Discover the actual owner identity; do not assume it from a name in conversation.
3. Verify the intended account, repository access and the ability to create a **public** repository. Never print secrets in logs or commit credentials.
4. Check whether `<authenticated-owner>/ChordCanvas` already exists. If it is the same authorised project, inspect and continue it. If it contains unrelated work or ownership is ambiguous, do not overwrite, delete, make private data public or create an invented substitute name.
5. Verify access to the installed Windows 11 x64 environment and Ableton Live 12. Record exact editions/builds/toolchain versions; do not assume Suite is required for this VST3 instrument.
6. Complete dependency and asset licence due diligence before committing third-party source/assets or distributing packages. Public visibility does not itself choose a project licence.

These are inspections, not repeated preference questions. Ask only when a true unresolved ownership, access or rights decision cannot be settled from available authorisation.

## 4. Repository contract

[C] Create a new **public GitHub repository named exactly `ChordCanvas` in the owner's account** when the preflight confirms that action is safe and authorised. This is a repository for source control; a separate GitHub Projects planning board was not requested.

[C] Keep it current throughout development. Commit and push each meaningful, internally coherent iteration rather than leaving the repository empty until the final release. Do not interpret “always update and upload” as pushing every keystroke, broken temporary binary, credential or private diagnostic.

[D] Use a simple main development branch unless a short-lived feature branch materially helps a safe change. Avoid a complex branching/review workflow that requires the owner to approve routine steps. Use normal fast-forward updates and explicit merges where needed; do not force-push over user work.

Source to include:

- This specification pack and its deliberate, versioned revisions.
- Complete C++ source, JUCE integration, CMake/build definitions and pinned dependency manifests.
- Internal design tokens/components and original or correctly licensed runtime assets.
- Installer/updater source, version/changelog generation and release scripts.
- Unit, property, integration and packaging tests, independent music fixtures and sanitised test evidence.
- Required licence/third-party notices and reproducibility instructions for developers in engineering material, not an unwanted end-user manual.

Exclude:

- Credentials, token files, `.env` secrets, signing private keys/certificates, account IDs that are not needed publicly.
- Private session logs, minidumps, user progressions, actual Ableton projects, unrelated vocal/audio material or identifiable user screenshots.
- Build caches, SDK installations, local machine paths and unlicensed proprietary dependencies/assets.
- Huge generated build trees or duplicate binary history. Publish release installers as release assets rather than committing every binary to Git.

Public screenshots must use synthetic progressions and avoid exposing the owner's unrelated workspace. A `LICENSE` file is permitted/required when rights warrant it; “no docs” is not permission to omit legally required attribution.

## 5. Meaningful commits and working ledger

[D] Commit subjects should explain a real increment, for example `Implement destructive timeline shortening with undo` or `Fix repeat ownership on editor focus loss`. Do not label a failing feature “complete” in a commit message.

For each meaningful iteration:

1. Implement a coherent change.
2. Run the directly relevant tests and inspect actual behaviour where required.
3. Update `BUILD_STATUS.md` with changes, test results, known problems and exact next steps.
4. Review the diff for accidental feature additions, secrets, private data, generated clutter and scope contradictions.
5. Commit and push the authorised files. Record the actual commit ID and push outcome.

A failing work-in-progress commit can be preserved when necessary, but label it and do not publish it as a tested release. Do not hide failed tests to keep the status page green.

## 6. Releases and version correspondence

[C] Release tags use semantic versions such as `v1.0.0`. About, current-release changelog, build version, installer/updater metadata and the tagged source must agree.

For a release candidate:

- Build packages from the intended, reproducible commit. Resolve any release metadata generation that would otherwise make the binary's source identity ambiguous.
- Run the release acceptance gates using the actual packaged artifacts, including clean install and upgrade.
- Verify both `.exe` files, hashes and version metadata. Keep private symbols available for diagnostics without committing private dumps.
- Tag the tested commit explicitly. Do not let an automated release command invent a tag on whatever branch happens to be checked out.
- Create the corresponding public GitHub Release and attach `Setup.exe`, `Updater.exe`, checksums and concise current-version release notes. Follow the licence gate in the architecture spec before public binary distribution.
- Verify that the published assets correspond to the tested local bytes and that the repository source matches the release.

[D] A short package manifest is useful release engineering evidence; it is not a new in-app download service or a user manual. No automatic online version check should be added to ChordCanvas.

## 7. Autonomous build order

Follow a risk-first sequence rather than implementing every feature before discovering host integration fails:

1. Read/reconcile all specifications; inspect permissions, host/toolchain and licensing gates.
2. Establish a minimal real VST3 instrument build that scans and opens in Live 12, with the intended editor and native file drag capability.
3. Prove MIDI drag import, trailing-silence clip boundaries, tempo/transport observations and packaging update behaviour early enough to change the implementation safely.
4. Establish the interaction/design system and thin vertical slice: one working key/pad, actual sound, one editable block, drag-out and matching notes.
5. Build exhaustive theory coverage, editing/history, audio ownership, optional controls, save/load and complete logging.
6. Iterate actual UI rendering and gestures at supported DPI/scales while adding features. Test negative and boundary cases, not just a happy-path demo.
7. Complete installer/updater, versioning and release verification; repeat host acceptance using installed binaries.
8. Publish only evidence-backed source/release state and deliver the concrete files and locations.

Do not replace the four sounds with placeholders until an unspecified later version. Do not defer GUI/UX validation until all code is otherwise called finished.

## 8. Stop conditions and honest delivery

The build is complete only when the required acceptance tests and release gates have passed or an explicitly permitted best-effort limitation is truthfully documented. A permission barrier, missing licence decision, failing native drag or untested host installation is not an optional caveat on a “complete” release.

If blocked, deliver the useful authorised progress, identify the specific remaining gate, and preserve a reproducible continuation state. Do not send the owner a broken `.exe` named as the final product or report a hypothetical GitHub URL as existing.

The final Codex delivery should identify actual tested host/OS builds, product version, repository/release URLs, both installer paths, source commit, relevant test evidence and any real limitation. Do not produce the rejected end-user documentation package. The owner should not need a compiler, JUCE installation or Python to use the installed plug-in.
