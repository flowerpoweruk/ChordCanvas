# ChordCanvas: installer, updater and releases

**Authority:** Install/update behaviour, release identity and Windows packaging.  
**Read with:** [Autonomy](10_GITHUB_AND_AUTONOMY.md), [diagnostics](09_DIAGNOSTICS_AND_LOGGING.md), [acceptance](11_ACCEPTANCE_TESTS.md).  
**Sources:** S03, S17–S21 and S24 in [reference register](13_VERIFIED_TECHNICAL_REFERENCES.md).

## 1. Required deliverables

[C] Every release build produces:

- **`Setup.exe`** for a fresh installation.
- **`Updater.exe`** as a separate, self-contained in-place update package.

Both contain the exact release VST3, required audition sound resources, runtime prerequisites where needed and release metadata. The user should not unzip folders, find a VST directory, install JUCE or compile anything.

[C] `Updater.exe` does not download a payload or check a website. Run it manually when the owner receives an updated release. Do not add a background agent, auto-update check, service, scheduled task or required account.

[D] A full-payload updater is acceptable; a binary delta patch is not required. What makes it an update is stable installation identity, safe in-place replacement and preservation of user-owned files, not a small download size.

## 2. Windows scope and location

[C] Target Windows 11 x64. Install to the standard global 64-bit VST3 location:

```text
C:\Program Files\Common Files\VST3\ChordCanvas.vst3\
```

[T] Resolve the appropriate 64-bit Windows known folder rather than hard-coding drive C or using a 32-bit redirected Program Files directory. The path above is the expected default on a typical installation. VST3 bundles must preserve the directory structure emitted by the pinned SDK/JUCE build.

[C] No user-selectable custom VST3 directory. No requirement to run Ableton as administrator. [D] Keep all necessary immutable sound resources inside the bundle or a clearly owned installation directory included in the package. Mutable files belong under `%LOCALAPPDATA%\ChordCanvas\`, not inside the bundle.

[D] Detect unsupported architecture/OS before modifying the installation. Do not label Windows ARM emulation as tested x64 hardware support. Select appropriate packaging architecture options after verifying the installed packager's documentation, not by copying an outdated script.

## 3. What one-click can and cannot mean

[C] Aim for one launch and automatic use of the agreed default path, without a configuration wizard. [D] A compact progress/result window and a Cancel control are appropriate. Suppress unnecessary welcome, folder selection, component selection and marketing pages.

[T] Writing the global VST3 directory generally requires administrator elevation. Windows UAC or reputation warnings cannot honestly be promised away. Do not disable UAC/SmartScreen, weaken ACLs, add exclusions, forge a signature or run Live permanently elevated to avoid these constraints.

[G] Sign the installers/binary only if authorised signing credentials are genuinely available. Do not purchase a certificate or claim a self-signed file has public trust. When unsigned, report that status truthfully. A valid signature alone is not a guarantee of no reputation prompts.

Keep any unavoidable consent or file-in-use message specific and understandable. “One click” must not mean silently destroying unsaved work.

## 4. Packaging architecture

[D] Use a mature native Windows installer technology such as Inno Setup, with separate fresh-install/update modes sharing the same product identity and manifest. Another justified native packager is acceptable if it meets every test and does not introduce a network dependency.

The source repository must contain both packaging definitions, payload manifests and a script that reproducibly creates the two named `.exe` artifacts from the same versioned build. Do not hand-edit the executable contents after building.

[D] Define one stable product/installer identifier at the first release and preserve it forever unless a later explicit migration is required. The updater must not create a second installed-app entry. Provide one normal Windows uninstall entry. Do not create a desktop shortcut that implies a standalone instrument application exists.

[D] Check required native runtime dependencies on a clean machine. Either use a suitable runtime linkage or bundle permitted runtime redistributables as part of the offline installer. The end user must not be sent to find Visual C++ DLLs manually.

## 5. Fresh installation

Before changes: validate OS/architecture, payload integrity, required disk space, target-directory ownership and any existing installation.

On a genuinely fresh machine: install the complete bundle, required assets and release information; register the single uninstall entry; return a clear success/failure result. Do not write a fake first-run log from an elevated installer into an administrator profile as if it were the user's plug-in log.

[D] If Setup finds an existing ChordCanvas installation, use the same safe replacement checks as Updater rather than uninstalling it first. Never recursively delete the common VST3 directory. Do not remove other vendors' files or user-saved progressions.

[G] Verify that Live discovers the installed plug-in with its normal system VST3-folder setting. An application cannot guarantee discovery when the owner has disabled plug-in scanning. During authorised test setup, inspect that setting; do not silently rewrite unrelated Live preferences.

## 6. In-place update behaviour

[C] Each `Updater.exe` is self-contained and replaces the installed ChordCanvas version without requiring a fresh setup. Preserve logs and explicit user-saved content. Do not remove the installation before confirming the new payload can be installed.

[D] Required version handling:

| Installed state | Updater behaviour |
|---|---|
| No ChordCanvas installation | Explain that Setup.exe is required; do not silently become a fresh installer |
| Older valid version | Perform safe update |
| Same version and identical payload | Report already up to date; no destructive work |
| Same version but inconsistent payload | Report the inconsistency; controlled repair only after verifying product ownership and payload |
| Newer version installed | Refuse downgrade by default |
| Unrecognised/conflicting installation | Stop safely; do not overwrite an unrelated product |

[D] Compare semantic versions numerically, not lexicographically. Do not classify `1.10.0` as older than `1.9.0`. Pre-release version handling must be defined consistently in test/release scripts.

[C] Preserve VST3 component/controller IDs, manufacturer/product identifiers, installation identity and required paths. Existing Live Sets must not see a differently identified plug-in after the update.

## 7. Live/file-in-use safety

[T] A loaded VST3 binary may be locked by Live. Do not force-delete or force-close the host to satisfy “autonomous”. Inno's default close-application behaviour can close/restart applications in silent mode; explicitly configure a safe policy rather than relying on that default (S20).

[D] Detect any process using ChordCanvas's installed files. Show a concise request to save/close the relevant Live instance and offer retry/cancel. Do not terminate the process, auto-dismiss an unsaved-work dialogue, close unrelated applications or reboot automatically.

A necessary host-closure step is a real external constraint, not an implementation question the owner should repeatedly answer. Codex's automated packaging tests must use disposable saved test Sets, not the user's creative work.

## 8. Staging, validation and rollback

[D] Stage the new bundle separately on the same volume. Validate its expected architecture, version, owned file list and hashes before replacing anything. Retain a recoverable previous bundle until the new install transaction finishes.

[D] Prefer an atomic rename/swap where platform constraints allow; otherwise implement a tested rollback-aware sequence with explicit state. If any write, rename, validation or metadata update fails, restore the last valid installation and report the failure. Never leave a half-old/half-new bundle while reporting success.

[D] Include the bundle and any out-of-bundle assets in the same logical versioned transaction. Do not replace the audio binary but accidentally leave mismatched sound resources or an old About changelog.

Temporary installation backups are not musical autosave. Clean them after a successful transaction according to a bounded policy. Never delete user logs/saved progressions during that cleanup.

## 9. Versions, About and changelog

[C] Use semantic versioning, initially `1.0.0` for the first accepted stable release. Distinguish specification version from application version.

[D] Maintain a single structured source of release metadata: semantic version, build/commit identifier, release date and **current-version changes**. Generate the plug-in About content, installer version fields, package names/manifests and Git tag from that source. Do not keep several manually synchronised version strings.

[C] About shows only the installed version's changelog. It must refresh because the updated release contains the new metadata, not because the user connects to the internet. The full project history can remain in Git and GitHub Releases without appearing in About.

[D] On Windows, derive a valid numeric file version from the semantic version and a documented build number. A displayed version must match the installed binary, not a file downloaded by the updater or a stale cached preference.

## 10. Release artifacts and repository

[C] Push meaningful source iterations. Tag stable release commits, for example `v1.0.0`, and publish the corresponding packages when authorised and legally cleared.

[D] Use a per-version output folder containing `Setup.exe`, `Updater.exe`, checksums, a release manifest and test evidence references. Keep matching debug symbols outside the end-user install where useful for crash analysis. Do not ship raw session logs, real crash dumps, secrets, the entire development toolchain or an end-user manual.

[D] GitHub Release assets may retain the exact executable names because the release itself supplies version context. Source tags must reproduce the binaries' inputs. Never overwrite a published tag or quietly replace a released binary with different content under the same version.

## 11. Required update test

Build a disposable prior test version with the same product identity and install it. Create representative user log/saved-progression files. Update using the newer self-contained Updater with networking disabled. Verify new audio/GUI/version/changelog, unchanged user files, one uninstall entry and stable Live plug-in identity.

Test cancellation, files in use, insufficient permission/space, corrupted payload, same version, newer version, no installation and a simulated mid-transaction failure. Record rollback results. Test actual release packages, not only the unbundled installer source.

The first stable release can still include Updater.exe targeting that release for users of a prior test build. Do not pretend it updates a nonexistent installation.
