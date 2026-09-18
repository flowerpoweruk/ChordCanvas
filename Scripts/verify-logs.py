"""Parse only synthetic integration evidence. Never upload runtime user logs."""
import json
import sys
from datetime import datetime
from pathlib import Path

root = Path(sys.argv[1])
release = json.loads((Path(__file__).resolve().parents[1] / 'release/current.json').read_text(encoding='utf-8'))
version, build_number = release['version'], release['buildNumber']
files = sorted(root.rglob('ChordCanvas_*.txt'))
assert len(list(root.glob('ChordCanvas_*.txt'))) == 5
assert len(list((root / 'concurrent').glob('ChordCanvas_*.txt'))) == 5
assert len(list((root / 'bounded').glob('ChordCanvas_*.txt'))) == 1
correlation = None
retained_ordinals = []
recovered_memory = False
for path in files:
    lines = path.read_text(encoding='utf-8', errors='strict').splitlines()
    assert lines[0] == 'ChordCanvas diagnostic session; UTF-8 JSON Lines; schema 1'
    header, *events = [json.loads(line) for line in lines[1:]]
    assert header['schema'] == 1 and header['event'] == 'session.header'
    assert header['version'] == version and header['source'] != 'unknown'
    assert header['build_number'] == build_number and header['build_configuration'] in ('Release', 'Debug', 'RelWithDebInfo', 'MinSizeRel')
    assert header['build_id'] == f'{version}.{build_number}@' + header['source']
    assert header['theory_schema'] == 1 and header['windows_version'].startswith('10.0.')
    assert header['native_architecture'] == 'x64'
    assert header['host_version'] == 'unknown' and header['host_version_source'] == 'unavailable'
    assert 'C:\\' not in json.dumps(header) and 'C:/' not in json.dumps(header)
    previous_seq, previous_ms = 0, 0
    for event in events:
        assert event['schema'] == 1 and event['session'] == header['session']
        assert isinstance(event['details'], dict)
        assert event['seq'] > previous_seq, (path.name, previous_seq, event['seq'])
        assert event['mono_ms'] >= previous_ms
        assert event['utc'].endswith('Z')
        datetime.fromisoformat(event['utc'])
        assert event['level'] in ('info', 'warning', 'error')
        previous_seq, previous_ms = event['seq'], event['mono_ms']
    assert events[-1]['event'] == 'session.end'
    if path.parent == root:
        ordinals = [event['details']['ordinal'] for event in events if event['event'] == 'synthetic.session.ordinal']
        assert len(ordinals) == 1
        retained_ordinals.extend(ordinals)
        assert any(event['event'] == 'audio.transition' for event in events)
    if path.parent.name == 'bounded':
        assert path.stat().st_size <= 16384
        assert any(event['event'] == 'state.snapshot' for event in events)
        assert any(event['event'] == 'log.history_truncated' for event in events)
    if path.parent.name == 'concurrent' and any(event['event'] == 'state.snapshot' for event in events):
        assert path.stat().st_size <= 16384
        assert any(event['event'] == 'state.snapshot' and event['details']['revision'] == 99 for event in events)
        assert any(event['event'] == 'log.history_truncated' for event in events)
        recovered_memory = True
    if path.parent.name == 'correlation':
        correlation = events

assert retained_ordinals == [1, 2, 3, 4, 5], retained_ordinals
assert recovered_memory
assert correlation is not None
actions = [event for event in correlation if event['event'].endswith('.commit')]
assert [event['event'] for event in actions] == [
    'timeline.replace.commit', 'timeline.resize.commit', 'timeline.undo.commit']
for transaction, event in enumerate(actions, start=1):
    assert event['instance'] == 1
    assert event['details'] == {
        'transaction': transaction,
        'revision_before': transaction + 1,
        'revision_after': transaction + 2,
    }
snapshots = [event['details'] for event in correlation if event['event'] == 'state.snapshot']
assert len(snapshots) == 3
for transaction, snapshot in enumerate(snapshots, start=1):
    assert snapshot['transaction'] == transaction and snapshot['revision'] == transaction + 2
    progression = snapshot['progression']
    assert progression['schema'] == 1 and progression['bars'] == 8
    assert len(progression['blocks']) == 1
    block = progression['blocks'][0]
    assert block['start'] == 2880
    assert block['end'] == (7680 if transaction == 2 else 6720)
errors = [event for event in correlation if event['event'] == 'export.error']
assert len(errors) == 1 and errors[0]['level'] == 'error'
assert errors[0]['details']['transaction'] == 4
assert errors[0]['details']['revision'] == snapshots[-1]['revision']
assert errors[0]['details']['state_changed'] is False
clocks = [event for event in correlation if event['event'] == 'audio.clock']
assert len(clocks) == 5
assert all(event['thread_role'] == 'audio-summary' for event in clocks)
for event in clocks:
    details = event['details']
    assert details['sample_rate_hz'] == 48000 and details['buffer_frames'] == 64
    assert details['fallback_120_bpm'] == (not details['tempo_ever_known'])
assert clocks[0]['details']['tempo_bpm'] == 120 and clocks[0]['details']['tempo_available'] is False
assert clocks[0]['details']['fallback_120_bpm'] is True
assert clocks[1]['details']['tempo_bpm'] == 91.125 and clocks[1]['details']['tempo_available'] is True
assert clocks[1]['details']['host_playing'] is True
assert clocks[2]['details']['tempo_bpm'] == 91.125 and clocks[2]['details']['tempo_available'] is False
assert clocks[2]['details']['fallback_120_bpm'] is False
assert clocks[3]['details']['tempo_bpm'] == 112.875 and clocks[3]['details']['meter_numerator'] == 3 and clocks[3]['details']['meter_denominator'] == 8
assert clocks[3]['details']['host_playing'] is False
assert clocks[4]['details']['bypassed'] is True
print(json.dumps({
    'parser': 'Python standard JSON/UTF-8 parser', 'status': 'PASS',
    'synthetic_files': len(files), 'sequence': 'strictly increasing',
    'monotonic_time': 'nondecreasing',
    'clock_changes': len(clocks), 'quiet_callbacks_coalesced': True,
    'correlation': 'replace -> resize -> undo -> actual file-open error, unchanged document',
}, indent=2))
