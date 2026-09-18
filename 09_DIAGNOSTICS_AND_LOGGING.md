# ChordCanvas: diagnostics and logging

**Authority:** runtime observability, five-session rotation, crash evidence and access to logs.  
**Read with:** [07_TECHNICAL_ARCHITECTURE.md](07_TECHNICAL_ARCHITECTURE.md), [08_INSTALLER_UPDATER_RELEASES.md](08_INSTALLER_UPDATER_RELEASES.md), [11_ACCEPTANCE_TESTS.md](11_ACCEPTANCE_TESTS.md).  
**Primary references:** S03, S22–S24 in [13_VERIFIED_TECHNICAL_REFERENCES.md](13_VERIFIED_TECHNICAL_REFERENCES.md).

## 1. Purpose and boundaries

[C] Produce detailed, timestamped `.txt` diagnostic logs that help Codex reconstruct what happened inside ChordCanvas before an error. Include relevant user actions, state changes, validation failures, build information and best-effort crash information. Retain the newest five diagnostic sessions rather than endlessly accumulating files or overwriting the only evidence at each launch.

[C] Logs belong under:

```text
%LOCALAPPDATA%\ChordCanvas\Logs\
```

Resolve the actual per-user Windows known folder; do not assume a particular user name or drive letter. The running plug-in must not need administrator privileges to write logs. Do not write these runtime files into `Program Files`, the VST3 bundle, the public source repository or the current Ableton project directory.

[C] The separate Settings section includes a plainly labelled **Open Logs Folder** control. Clicking it opens the exact folder in Windows Explorer. Create the directory if necessary. A safe shell/known-folder API is preferable to constructing a command string from unescaped path text. A failure produces a simple, useful error, not a silent click or an endless retry.

[T] “Everything the user was doing” means relevant **ChordCanvas actions**, not surveillance of the computer. Do not record unrelated keystrokes, microphone input, vocals playing in Live, browser activity, screen recordings, credentials or other applications' data. No telemetry, automatic bug upload or cloud logging is part of the product.

## 2. Session and rotation model

The conversation specified five sessions but not whether opening several plug-in instances counts as several sessions. Use this explicit engineering policy:

[D] One interactive **Ableton process session** shares one diagnostic file across its ChordCanvas instances. Give each instance a stable identifier for that process lifetime. Opening a second instance does not consume a second retained session; reopening an editor does not start a new session. A fresh Live process starts a new session when ChordCanvas is used interactively. Plug-in scan/validation processes must not repeatedly evict the user's interactive history.

Possible filename:

```text
ChordCanvas_20260918T004300Z_<session-id>.txt
```

The UTC start time and collision-resistant session ID identify the session. File timestamps alone are not the authority because copies and file-system operations can change them.

Rotation rules:

1. Count the current session in the limit of **five session `.txt` files**.
2. Before admitting a new session, remove the oldest completed retained session when needed. Preserve the newest sessions, not the fifth newest forever.
3. Do not truncate or delete a log currently being written by another active Live process. Coordinate safely across processes; a mutex cannot be acquired from the audio callback.
4. [D] In the unusual case of more than five concurrent interactive host processes, preserve the five active files and report degraded logging for the additional session rather than silently exceeding the limit or deleting live evidence. Keep a bounded in-memory trail until a slot becomes available. This exception must be visible in Settings and covered by a test.
5. Use atomic creation and safe replacement so simultaneous instances cannot select the same filename or rotate the same file twice.
6. Dispose of normal session services cleanly. A missing end marker means **unclean/unknown termination**, not proof that ChordCanvas caused a crash.

[D] A scanner may emit a small, separate bounded diagnostic outside the five interactive-session namespace when necessary for development. It must not be mistaken for a user session, repeatedly allocate files, or hide a scan failure. Do not ship an uncontrolled second logging system.

## 3. File format

[C] Files must be plain UTF-8 text, intelligible to a person and easy for Codex to parse. Use one documented schema, stable event names and explicit units. Include a human-readable header followed by structured event lines; JSON Lines inside a `.txt` file is an acceptable implementation.

[D] Recommended event fields:

| Field | Meaning |
|---|---|
| `schema` | Diagnostic schema version |
| `utc` | UTC timestamp with millisecond precision where available |
| `mono_ms` | Monotonic elapsed time since session start, for ordering when the clock changes |
| `seq` | Monotonically increasing event sequence |
| `level` | `debug`, `info`, `warning`, `error` or `fatal` |
| `session` / `instance` | Local opaque identifiers, not user names |
| `thread_role` | Message, audio-summary, worker, installer or crash-reporter context |
| `event` | Stable machine-readable action/event identifier |
| `transaction` | ID joining preview, commit, history and errors for one operation |
| `revision_before` / `revision_after` | Relevant immutable timeline revisions |
| `details` | Typed event payload; musical time is integer ticks unless expressly labelled otherwise |

Illustrative **synthetic** event, not evidence of a real test:

```json
{"schema":1,"utc":"2026-09-18T00:43:10.123Z","mono_ms":10123,"seq":84,"level":"info","session":"example-session","instance":"example-instance","thread_role":"message","event":"timeline.replace.commit","transaction":"example-tx","revision_before":12,"revision_after":13,"details":{"incoming_block":"b8","start_tick":3840,"duration_tick":3840,"removed_block_ids":["b3","b4"],"midi_notes":[60,64,67],"timeline_end_tick":30720}}
```

Never log locale-dependent numbers without explicit interpretation. Keep schema evolution backward-readable or record migration/version rules.

## 4. Session header and diagnostic snapshots

Record enough environment detail to reproduce failures without indiscriminate collection:

- Product version, build configuration, source commit if available, binary/build ID, diagnostic schema and theory schema.
- Windows version/architecture, plug-in format and host-reported host/version information when available. Unknown values must say unknown rather than inventing `Live 12`.
- Sample rate, buffer size, audio bus configuration, active instrument, output level, host tempo/meter validity and relevant transport flags.
- ChordCanvas editor dimensions, effective display scale and renderer choice; no screen capture by default.
- Feature switches, snapping/slicing divisions, timeline length, repeat settings and local/sync state.

[D] Emit a compact validated ChordCanvas state snapshot at instance creation, major state-changing transactions and handled errors, with rate-limiting/coalescing where appropriate. Include block IDs, starts, durations, origin key/degree, inversion, octave, enabled modifications, resolved notes and selected IDs. This permits reconstruction without requiring every mouse coordinate. Snapshots are **diagnostic data only**, never autosave files or automatic restoration inputs.

Do not persist entire host state blobs, unrelated tracks or arbitrary dropped-file contents. User-created progression content is potentially private even without a name; treat logs accordingly.

## 5. Actions and failures to capture

Capture semantic operations rather than a noisy stream of every pointer pixel:

| Area | Required evidence |
|---|---|
| Keys and harmony | Old/new key identity, pad reset, optional-feature changes, inversion/octave/seventh/suspension changes, resolved pitches, rejected invalid states |
| Timeline | Add, move, both-edge resize, replacement target IDs, slice position and result, delete, selection, paste, duplicate, length change, clipping, auto-expansion, undo/redo |
| UI | Gesture begin/commit/cancel, owning hit region, tool switch, relevant focus loss, popover failures, wheel modifier interpretation, editor/DPI changes |
| Playback | Local start/stop/seek, sync state changes, host start/stop observations, repeat start/switch/stop/rate, preview ownership, active-note cleanup |
| Timing | Tempo/meter changes, validity loss, sample-rate/buffer changes, queue overflow or discontinuity; do not emit one line per audio sample or normal block |
| Files | Manual save/load begin/result, schema/validation outcome, export revision, event count, expected end tick, external-drag start/end, file I/O error codes |
| Errors | Component, operation, exception class/message where safe, Windows/system error code, expected/observed invariant, correlation ID, recent state revision |
| Lifecycle | Instance creation/destruction, editor open/close, normal session end, startup evidence of an unclean previous termination |

[D] Log a gesture's start and final outcome, and retain a bounded sample of intermediate state only when diagnostically valuable. Rate-limit repetitive errors and state the suppressed count. Audio note-transition summaries can be captured through a bounded real-time-safe channel; overflowing that channel must create a later explicit loss marker rather than blocking audio.

Avoid an expensive full snapshot on every short repeat. Tie snapshots to meaningful state revisions and include the current revision in lightweight playback events.

## 6. Real-time safety and storage limits

[C/T] Never open, write, flush, rotate or compress files from the real-time audio callback. Never wait on a logging mutex or perform JSON formatting there. Use bounded, preallocated event transport to a non-real-time writer. If evidence cannot be queued safely, preserve audio and report dropped diagnostics on the worker side.

[D] Flush periodically, approximately once per second under normal operation, and promptly after handled errors on a safe thread. Exact flush mechanics are an implementation detail; do not claim the last second is guaranteed to survive power loss.

[D] Bound each session text file to approximately **32 MiB**. When it would exceed the bound, compact on the worker thread using safe replacement: preserve the session header, a recent valid state snapshot and the latest event window. Add a conspicuous `log.history_truncated` marker with sequence/time range and counts. There must still be one session `.txt` file, not unlimited rotated fragments masquerading as one session. This is a declared storage default, not a promise to retain an arbitrarily long session in full.

Disk full, denied access, missing directories and rotation races must not crash the plug-in. Settings should show logging unavailable/degraded and the last useful reason. Use a bounded in-memory fallback. Do not repeatedly open modal errors for the same failure while the user plays music.

Installer/updater logs are separate installation diagnostics and must not overwrite the five runtime sessions. Preserve them only with a separately documented bounded policy; do not install a background collector.

## 7. Crash evidence: best effort, not a false guarantee

[C] Attempt to capture exception details, relevant stack/module/build information and the last ChordCanvas actions when a fault permits it. Preserve the session trail so Codex can correlate symptoms with the source version and operation.

[T] A plug-in shares a process with its host. An access violation, host failure, forced termination or power loss may prevent any final write. A normal C++ exception guard cannot promise to recover from every native crash. Do not present “crash logging always succeeds” as an acceptance claim.

Required safe baseline:

1. Catch and log recoverable exceptions at appropriate plug-in boundaries without swallowing corruption or leaving invalid musical state active.
2. Maintain useful pre-fault evidence asynchronously during normal operation.
3. Mark orderly shutdown when possible. On a later session, identify prior logs lacking a normal end as unclean/unknown, without falsely assigning blame.
4. Preserve source/build identifiers so any available address or stack can be symbolised with matching private build symbols.
5. Never silently install a process-wide exception filter or crash handler that takes over Ableton's crash reporting or other plug-ins' handlers.

[G] Additional minidump integration is permitted only after its safety and permissions are established in the actual host. Microsoft's `MiniDumpWriteDump` guidance warns about calling into a damaged target and favours a separate process where possible; a crash path can deadlock. Do not add a permanent service merely to claim compliance. Record exactly which failure modes produced text evidence, which produced a dump, and which could not be captured.

A dump may contain private host memory. Keep it private, locally bounded and outside the public repository; do not automatically attach it to a GitHub issue/release or transmit it. The `.txt` session trail remains the required accessible baseline. No crash recovery/autosave feature should be reintroduced under the name of diagnostics.

## 8. Error presentation and developer usefulness

[C] A failed MIDI export or other failed user-requested file operation produces a simple error message. State what failed and what remains safe, not an opaque code alone. Keep technical codes/correlation IDs in the log, with a compact reference in the message when useful.

[D] Show one actionable notification per distinct failure, not a cascade. Preserve the last valid document after a failed edit or load. Never claim export/import succeeded because a native drag merely started.

A useful error record answers: which build, which operation, what input/state, what was expected, what actually happened, whether any state changed, and where to find related events. Local source paths, account names and absolute user-file paths should be redacted where not necessary. Prefer a basename or a session-local file token for imported/saved progression paths.

## 9. Verification required

Demonstrate normal five-session rotation, multiple instances, editor reopen, simultaneous host processes, unwritable storage, bounded overflow, external clock changes, privacy filtering and Explorer access. Exercise a recoverable error and a deliberately isolated crash test without risking the owner's live project. Inspect the resulting text as both a human and a parser.

A statement that logging code exists is not evidence that it captured useful fault context. Record actual files/evidence privately and put only sanitised examples in the public repository. Tests and pass/fail records belong in `BUILD_STATUS.md` and the acceptance evidence referenced there.
