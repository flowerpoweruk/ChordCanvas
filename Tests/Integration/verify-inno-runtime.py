"""Execute disposable Inno packages; never install a product VST3 or touch HKLM."""
import argparse
import ctypes
import hashlib
import json
import pathlib
import subprocess
import time
import uuid
import winreg

parser = argparse.ArgumentParser()
for name in ("cmake", "compiler", "make", "iscc", "synthetic_pe"):
    parser.add_argument("--" + name.replace("_", "-"), required=True)
args = parser.parse_args()
project = pathlib.Path(__file__).resolve().parents[2]
identity = str(uuid.uuid4())
root = project / "build" / ("inno-runtime-" + identity)
root.mkdir()
(root / "fixture.owner").write_text("ChordCanvasInnoFixture1\n" + identity + "\n", encoding="utf-8", newline="\n")
(root / "VST3").mkdir()
key = r"Software\Microsoft\Windows\CurrentVersion\Uninstall\{" + identity + "}_is1"
results = []


def check(condition, message):
    if not condition:
        raise RuntimeError(message)


def sha(file):
    return hashlib.sha256(pathlib.Path(file).read_bytes()).hexdigest()


def command(argv, label):
    completed = subprocess.run([str(x) for x in argv], capture_output=True, timeout=60)
    (root / (label + ".txt")).write_bytes(completed.stdout + completed.stderr)
    check(completed.returncode == 0, label + " failed; private output retained")


def package(version, output, update, corrupt=False):
    folder = root / "inputs" / output
    binary = folder / "ChordCanvas.vst3/Contents/x86_64-win/ChordCanvas.vst3"
    binary.parent.mkdir(parents=True)
    binary.write_bytes(pathlib.Path(args.synthetic_pe).read_bytes())
    receipt = folder / "ChordCanvas.vst3/chordcanvas.payload"
    receipt.write_text("ChordCanvasPayload1\nEE56A5B5-24E6-4F83-B3AF-1C73BCD93594\n" + version +
                       "\nContents/x86_64-win/ChordCanvas.vst3\n" + sha(binary) +
                       "\tContents/x86_64-win/ChordCanvas.vst3\n", encoding="utf-8", newline="\n")
    if corrupt:
        with binary.open("ab") as stream:
            stream.write(b"deliberate integrity failure")
    definitions = ['[Files]', 'Source: "' + str(root / "native/fixture/ChordCanvasInstaller.dll") + '"; Flags: dontcopy noencryption']
    for file in (binary, receipt):
        relative = file.relative_to(folder)
        definitions.append('Source: "' + str(file) + '"; DestDir: "{tmp}\\Payload\\' + str(relative.parent) + '"; Flags: dontcopy noencryption')
    definitions += ["[UninstallDelete]"]
    for file in (binary, receipt):
        relative = file.relative_to(folder / "ChordCanvas.vst3")
        definitions.append('Type: files; Name: "' + str(root / "VST3/ChordCanvas.vst3" / relative) + '"')
    for folder_name in ("Contents/x86_64-win", "Contents", ""):
        definitions.append('Type: dirifempty; Name: "' + str(root / "VST3/ChordCanvas.vst3" / folder_name) + '"')
    definitions += ['Type: files; Name: "{app}\\ChordCanvasInstaller.dll"',
                    'Type: files; Name: "{app}\\chordcanvas.install"', 'Type: dirifempty; Name: "{app}"']
    include = folder / "inputs.iss"
    include.write_text("\n".join(definitions) + "\n", encoding="utf-8")
    command([args.iscc, "/DCC_Version=" + version, "/DCC_Update=" + str(int(update)),
             "/DCC_FixtureId=" + identity, "/DCC_FixtureRoot=" + str(root),
             "/DCC_Inputs=" + str(include), "/DCC_Output=" + output,
             "/DCC_OutputDir=" + str(root / "packages"), project / "Tests/Integration/InnoRuntime.iss"], "compile-" + output)
    return root / "packages" / (output + ".exe")


def run(file, name, success):
    artifact_hash = sha(file)
    process = subprocess.run([str(file), "/VERYSILENT", "/SUPPRESSMSGBOXES", "/NORESTART", "/SP-", "/LOG=" + str(root / (name + ".log"))], timeout=60)
    results.append({"scenario": name, "exitCode": process.returncode, "packageSha256": artifact_hash})
    check((process.returncode == 0) == success, name + ": unexpected actual process exit " + str(process.returncode))
    return process.returncode


def uninstall(name):
    run(root / "ChordCanvas/unins000.exe", name, True)
    # Inno's original uninstaller can return while its temporary worker is
    # finishing self-removal. Observe completion, rather than assuming the
    # launcher's zero exit means deletion has already finished.
    deadline = time.monotonic() + 10
    while time.monotonic() < deadline:
        log = (root / (name + ".log")).read_text(encoding="utf-8-sig", errors="strict")
        if "Uninstallation process succeeded." in log and "Removed all? Yes" in log and "Log closed." in log:
            check(snapshot() == ({}, {}), "actual uninstaller left product files/registration")
            check(not (root / "ChordCanvas").exists() and not (root / "VST3/ChordCanvas.vst3").exists(), "uninstall left owned directories")
            results[-1]["workerCompletion"] = "Actual closed Inno log reports success, removed all, no restart; owned directories and HKCU entry absent"
            check("Need to restart Windows? No" in log, "actual uninstaller requested restart")
            return
        time.sleep(.1)
    raise RuntimeError("actual temporary uninstaller worker did not finish successfully")


def snapshot():
    files = {}
    for directory in (root / "VST3/ChordCanvas.vst3", root / "ChordCanvas"):
        if directory.exists():
            for file in directory.rglob("*"):
                if file.is_file():
                    files[str(file.relative_to(root)).replace("\\", "/")] = sha(file)
    values = {}
    try:
        with winreg.OpenKey(winreg.HKEY_CURRENT_USER, key, 0, winreg.KEY_READ | winreg.KEY_WOW64_64KEY) as leaf:
            for index in range(winreg.QueryInfoKey(leaf)[1]):
                name, value, kind = winreg.EnumValue(leaf, index)
                values[name] = (value, kind)
    except FileNotFoundError:
        pass
    return files, values


try:
    command([args.cmake, "-S", project, "-B", root / "native", "-G", "MinGW Makefiles",
             "-DCMAKE_CXX_COMPILER=" + args.compiler, "-DCMAKE_MAKE_PROGRAM=" + args.make,
             "-DCMAKE_BUILD_TYPE=Release", "-DCHORDCANVAS_BUILD_PLUGIN=OFF", "-DCHORDCANVAS_INNO_FIXTURE=ON",
             "-DCC_INNO_FIXTURE_ID=" + identity, "-DCC_INNO_FIXTURE_ROOT=" + root.as_posix()], "configure")
    command([args.cmake, "--build", root / "native", "--target", "inno_fixture", "--parallel", "4"], "build")
    old = package("1.9.0", "SetupOld", False)
    next_version = package("1.10.0", "UpdaterNext", True)
    future = package("1.11.0", "UpdaterFuture", True)
    corrupted = package("1.11.0", "UpdaterCorrupt", True, True)
    before = snapshot()
    run(next_version, "updater-without-installation", False)
    check(snapshot() == before, "absent-install updater changed installation")
    run(old, "fresh-setup", True)
    check(snapshot()[1]["DisplayVersion"][0] == "1.9.0", "actual Inno registration version")
    check((root / "ChordCanvas/chordcanvas.install").exists(), "actual finalisation did not seal receipt")
    sentinel = root / "user-owned.txt"
    sentinel.write_text("preserved saved progression/log sentinel\n", encoding="utf-8")
    other = root / "VST3/OtherVendor.txt"
    other.write_text("preserved unrelated vendor\n", encoding="utf-8")
    preserved = (sha(sentinel), sha(other))
    run(next_version, "numeric-full-payload-update", True)
    check(snapshot()[1]["DisplayVersion"][0] == "1.10.0", "numeric update did not replace actual registration")
    before = snapshot()
    run(next_version, "same-version-no-op", False)
    check(snapshot() == before, "same-version package changed actual files/registration")
    run(old, "downgrade-refused", False)
    check(snapshot() == before, "downgrade altered installation")
    run(corrupted, "corrupt-payload-refused", False)
    check(snapshot() == before, "corrupt payload altered installation")
    kernel = ctypes.WinDLL("kernel32", use_last_error=True)
    kernel.CreateFileW.argtypes = (ctypes.c_wchar_p, ctypes.c_uint32, ctypes.c_uint32, ctypes.c_void_p, ctypes.c_uint32, ctypes.c_uint32, ctypes.c_void_p)
    kernel.CreateFileW.restype = ctypes.c_void_p
    kernel.CloseHandle.argtypes = (ctypes.c_void_p,)
    handle = kernel.CreateFileW(str(root / "VST3/ChordCanvas.vst3/Contents/x86_64-win/ChordCanvas.vst3"), 0x80000000, 1, None, 3, 0x80, None)
    check(handle not in (None, ctypes.c_void_p(-1).value), "own disposable file lock unavailable")
    try:
        run(future, "files-in-use-refused", False)
        check(snapshot() == before, "locked-file package changed installation")
    finally:
        kernel.CloseHandle(handle)
    (root / "fail-finalise").write_text("fixture fault\n", encoding="utf-8")
    code = run(future, "late-finalisation-rollback", False)
    check(code == 66, "late native failure did not propagate custom setup exit")
    check(snapshot() == before, "late finalisation failed to restore byte-exact old files/registration")
    (root / "fail-finalise").unlink()
    run(future, "retry-after-rollback", True)
    check(snapshot()[1]["DisplayVersion"][0] == "1.11.0", "retry after rollback failed")
    check((sha(sentinel), sha(other)) == preserved, "update changed unrelated/user files")
    uninstall("actual-uninstaller")
    (root / "fail-finalise").write_text("fixture fault\n", encoding="utf-8")
    check(run(old, "fresh-late-finalisation-rollback", False) == 66, "fresh finalisation failure exit")
    check(snapshot() == ({}, {}) and not (root / "ChordCanvas").exists(), "fresh rollback did not restore absence")
    (root / "fail-finalise").unlink()
    run(old, "fresh-retry-after-rollback", True)
    uninstall("fresh-retry-uninstaller")
    check((sha(sentinel), sha(other)) == preserved, "uninstall changed unrelated/user files")
    for file in (root / "VST3/.ChordCanvas.transaction", root / ".ChordCanvas.metadata", root / ".ChordCanvas.metadata.retired"):
        check(not file.exists(), "actual package left unfinished recovery state")
    evidence = {"status": "PASS", "scope": "Development-only synthetic AMD64 payload, generated folder, per-user Inno and HKCU; no product installation, VST3 host, elevation, clean-machine or network-isolation acceptance", "scenarios": results}
except Exception as error:
    evidence = {"status": "FAIL", "error": str(error), "scenarios": results}
    raise
finally:
    (root / "runtime-evidence.json").write_text(json.dumps(evidence, indent=2) + "\n", encoding="utf-8")
    print(root)
print("PASS: actual disposable Inno package processes, extraction/ABI/finalisation, numeric update, ownership-safe refusals, late rollback, retry and real uninstall")
