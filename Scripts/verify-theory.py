"""Compare production C++ output against pinned, development-only music21."""
import json
import subprocess
import sys
from pathlib import Path
from music21 import pitch, scale, harmony, __version__

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
    expected = [int(pitches[degree+n].midi) for n in (0,2,4)]
    if sus:
        expected = [base+n for n in sus_intervals[sus]]
    if seventh:
        expected.append(int(pitches[degree+6].midi))
    for _ in range(inversion):
        expected.append(expected.pop(0)+12)
    expected = [n+12*(octave-3) for n in expected]
    assert actual == expected, (fields[:8], actual, expected)
    count += 1
assert count == 30870
target = Path(sys.argv[2]) if len(sys.argv) > 2 else None
if target:
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text(json.dumps({'provenance': 'music21 '+__version__+' MajorScale/MinorScale, explicit normalized register; no production generator used', 'fixtures': fixtures}, indent=2)+'\n', encoding='utf-8')
print(json.dumps({'status': 'PASS', 'oracle': 'music21 '+__version__, 'combinations': count, 'writtenScales': len(oracles)}, indent=2))
