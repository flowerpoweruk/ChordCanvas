"""Engineering colour-role measurements, not full rendered accessibility tests."""
import re, json
from pathlib import Path
source=Path('Source/Plugin/Editor.cpp').read_text(encoding='utf-8')
colour=lambda h:tuple(int(h[i:i+2],16)/255 for i in (0,2,4))
tokens={name:colour(value) for name,value in re.findall(r'(\w+)\s*\{0xff([0-9a-f]{6})\}',source.split('class Skin')[0])}
degrees=[colour(value) for value in re.findall(r'juce::Colour\(0xff([0-9a-f]{6})\)',source.split('class Skin')[0])]
def lum(rgb):
    channels=[x/12.92 if x<=.04045 else ((x+.055)/1.055)**2.4 for x in rgb]
    return sum(x*w for x,w in zip(channels,(.2126,.7152,.0722)))
def ratio(a,b):
    lo,hi=sorted((lum(a),lum(b)))
    return (hi+.05)/(lo+.05)
def blend(fg,bg,alpha):return tuple(a*alpha+b*(1-alpha) for a,b in zip(fg,bg))
rows=[]
for role in ('text','secondary'):
    for background in ('ink','surface','raised'):
        rows.append(dict(role=role,background=background,ratio=ratio(tokens[role],tokens[background]),target=4.5))
for degree, accent in enumerate(degrees,1):
    for role in ('text','secondary'):
        for alpha,name in ((.10,'pad'),(.17,'block')):
            rows.append(dict(role=role,background=name,degree=degree,ratio=ratio(tokens[role],blend(accent,tokens['surface'],alpha)),target=4.5))
    rows.append(dict(role='degree-strip',degree=degree,ratio=ratio(accent,blend(accent,tokens['surface'],.17)),target=3))
    rows.append(dict(role='control-boundary',degree=degree,ratio=ratio(tokens['line'],blend(accent,tokens['surface'],.10)),target=3))
for bg in ('ink','surface','raised'):
    rows.append(dict(role='control-boundary',background=bg,ratio=ratio(tokens['line'],tokens[bg]),target=3))
result=dict(scope='Source colour-role calculations; no formal accessibility certification or full rendered-state pass',rows=rows,minimumText=min(r['ratio'] for r in rows if r['target']==4.5),minimumNonText=min(r['ratio'] for r in rows if r['target']==3),failures=[r for r in rows if r['ratio']<r['target']])
Path('build').mkdir(exist_ok=True)
Path('build/contrast-role-measurements.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8')
print(json.dumps({k:v for k,v in result.items() if k!='rows'},indent=2))
raise SystemExit(1 if result['failures'] else 0)
