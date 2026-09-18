"""Compare production C++ output against pinned, development-only music21."""
import json
import io
import subprocess
import sys
from pathlib import Path
from music21 import pitch, scale, harmony, __version__
import mido

letters = 'CDEFGAB'
exe = Path(sys.argv[1])
rows = subprocess.check_output([str(exe)], encoding='utf-8').splitlines()
oracles = {}
fixtures = []
for letter in range(7):
    for accidental in (0, 1, -1):
        for mode in (0, 1):
            tonic_name = letters[letter] + ('#' if accidental == 1 else '-' if accidental == -1 else '')
            tonic = pitch.Pitch(tonic_name)
            # Select explicit sounding register while retaining written tonic identity.
            tonic.octave = 4
            while tonic.midi < 60:
                tonic.octave += 1
            while tonic.midi >= 72:
                tonic.octave -= 1
            s = (scale.MajorScale if mode == 0 else scale.MinorScale)(tonic)
            pitches = s.getPitches(tonic, tonic.transpose('P15'))
            assert len(pitches) == 15
            names = [(letters.index(p.step), int(p.accidental.alter) if p.accidental else 0) for p in pitches[:7]]
            assert [int(p.midi-tonic.midi) for p in pitches[:7]] == ([0,2,4,5,7,9,11] if mode == 0 else [0,2,3,5,7,8,10])
            oracles[(letter, accidental, mode)] = pitches, names
            fixtures.append({'tonic': tonic_name, 'mode': mode, 'scaleMidi': [int(p.midi) for p in pitches], 'scaleNames': names})
count = 0
sus_intervals = {}
for sus in (1, 2):
    symbol = harmony.ChordSymbol('Csus2' if sus == 1 else 'Csus4')
    sus_intervals[sus] = [int(p.midi-symbol.pitches[0].midi) for p in symbol.pitches]
    assert sus_intervals[sus] == ([0,2,7] if sus == 1 else [0,5,7])
for line in rows:
    fields = line.split('\t')
    letter, accidental, mode, degree, octave, sus, seventh, inversion = map(int, fields[:8])
    actual = list(map(int, fields[8].split(',')))
    actual_names = [tuple(map(int, n.split(':'))) for n in fields[9].split(',')]
    pitches, names = oracles[(letter, accidental, mode)]
    assert actual_names == names, (fields[:8], actual_names, names)
    base = int(pitches[degree].midi)
    root_letter, root_accidental = names[degree]
    if abs(root_accidental) <= 1:
        root_label = letters[root_letter] + {0: '', 1: '\u266f', -1: '\u266d'}[root_accidental]
    else:
        # Independent explicit enharmonic table for the documented display policy.
        sharp_names = ('C', 'C\u266f', 'D', 'D\u266f', 'E', 'F', 'F\u266f', 'G', 'G\u266f', 'A', 'A\u266f', 'B')
        flat_names = ('C', 'D\u266d', 'D', 'E\u266d', 'E', 'F', 'G\u266d', 'G', 'A\u266d', 'A', 'B\u266d', 'B')
        root_label = (flat_names if accidental < 0 else sharp_names)[base % 12]
    third = int(pitches[degree+2].midi) - base
    fifth = int(pitches[degree+4].midi) - base
    seventh_interval = int(pitches[degree+6].midi) - base
    if sus:
        quality = (('maj7' if seventh_interval == 11 else '7') if seventh else '') + ('sus2' if sus == 1 else 'sus4')
    elif seventh:
        quality = ('maj7' if seventh_interval == 11 else '7') if third == 4 else ('m7\u266d5' if fifth == 6 else 'm7')
    else:
        quality = '' if third == 4 else ('dim' if fifth == 6 else 'm')
    assert fields[10] == root_label + quality, (fields[:8], fields[10], root_label + quality)
    expected = [int(pitches[degree+n].midi) for n in (0,2,4)]
    if sus:
        expected = [base+n for n in sus_intervals[sus]]
    if seventh:
        expected.append(int(pitches[degree+6].midi))
    for _ in range(inversion):
        expected.append(expected.pop(0)+12)
    expected = [n+12*(octave-3) for n in expected]
    assert actual == expected, (fields[:8], actual, expected)
    exported = mido.MidiFile(file=io.BytesIO(bytes.fromhex(fields[11])), clip=False)
    assert exported.type == 0 and exported.ticks_per_beat == 960 and len(exported.tracks) == 1
    tick = 0
    parsed = []
    for message in exported.tracks[0]:
        tick += message.time
        if message.type == 'end_of_track':
            assert tick == 30720
            parsed.append((tick, message.type))
        else:
            assert message.type in ('note_on', 'note_off') and message.channel == 0
            parsed.append((tick, message.type, message.note, message.velocity))
    expected_events = [(3840, 'note_on', note, 100) for note in expected]
    expected_events += [(7680, 'note_off', note, 0) for note in expected]
    expected_events.append((30720, 'end_of_track'))
    assert parsed == expected_events, (fields[:8], parsed, expected_events)
    count += 1
assert count == 30870
target = Path(sys.argv[2]) if len(sys.argv) > 2 else None
if target:
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text(json.dumps({'provenance': 'music21 '+__version__+' MajorScale/MinorScale, explicit normalized register; no production generator used', 'fixtures': fixtures}, indent=2)+'\n', encoding='utf-8')
print(json.dumps({'status': 'PASS', 'oracle': 'music21 '+__version__, 'parser': 'mido '+str(mido.version_info), 'combinations': count, 'writtenScales': len(oracles), 'unicodeChordLabels': count, 'serializedMidiChords': count}, indent=2))
