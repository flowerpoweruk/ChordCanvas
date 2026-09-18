"""Independent mido verification; no host claims. Run after core_tests."""
import json
import sys
from pathlib import Path
import mido

root = Path(sys.argv[1])
results = []
for filename in ('chords.mid', 'empty.mid'):
    file = mido.MidiFile(root / filename, clip=False)
    assert file.type == 0 and file.ticks_per_beat == 960 and len(file.tracks) == 1
    tick = 0
    notes = []
    active = set()
    at_tick = []
    previous = -1
    for message in file.tracks[0]:
        tick += message.time
        assert message.type in ('note_on', 'note_off', 'end_of_track')
        if tick != previous:
            at_tick = []
        if message.type == 'note_off':
            assert message.channel == 0 and message.velocity == 0
            assert message.note in active
            assert 'note_on' not in at_tick
            active.remove(message.note)
        elif message.type == 'note_on':
            assert message.channel == 0 and message.velocity == 100
            assert message.note not in active
            active.add(message.note)
        else:
            assert not active and tick == 30720
        if message.type != 'end_of_track':
            notes.append((tick, message.type, message.note))
        at_tick.append(message.type)
        previous = tick
    if filename == 'chords.mid':
        expected = [(3840, 'note_on', n) for n in (60,64,67)]
        expected += [(7680, 'note_off', n) for n in (60,64,67)]
        expected += [(7680, 'note_on', n) for n in (64,67,72)]
        expected += [(11520, 'note_off', n) for n in (64,67,72)]
        assert notes == expected, (notes, expected)
    else:
        assert notes == []
    results.append({'file': filename, 'status': 'PASS', 'endTick': tick, 'events': len(notes)})
print(json.dumps({'parser': 'mido '+str(mido.version_info), 'results': results}, indent=2))
