#include "Editor.h"
#include "Persistence/Progression.h"
#include "Export/Midi.h"
#include "Export/TemporaryMidi.h"
#include <juce_cryptography/juce_cryptography.h>
#include "ChordCanvasRelease.h"
#include <fstream>
#include <algorithm>
#include <filesystem>
#include <cmath>

namespace cc {
namespace ui {
const juce::Colour ink {0xff11171d},surface {0xff1c252e},raised {0xff26323e},text {0xffe9eef2},secondary {0xffabbac6},line {0xff6d8191};
const std::array<juce::Colour,7> degree {{juce::Colour(0xfff1ad67),juce::Colour(0xffe4d16b),juce::Colour(0xff86c88a),juce::Colour(0xff6bc8c4),juce::Colour(0xff79aeeb),juce::Colour(0xffb59be5),juce::Colour(0xffe397ba)}};
juce::Font font(float size) { return juce::Font(juce::FontOptions("Segoe UI",size,juce::Font::plain)); }
void textAt(juce::Graphics& g,const juce::String& label,juce::Rectangle<int> bounds,float size=14,juce::Colour colour=text) {
    g.setFont(font(size));g.setColour(colour);g.drawText(label,bounds,juce::Justification::centredLeft,true);
}
bool optional(const Session& s) { return s.settings.showSeventh || s.settings.showSus2 || s.settings.showSus4; }
}
class Skin final : public juce::LookAndFeel_V4 {
public:
    Skin() {
        setColour(juce::TextButton::textColourOffId,ui::text);setColour(juce::TextButton::textColourOnId,ui::text);
        setColour(juce::ComboBox::textColourId,ui::text);setColour(juce::ComboBox::backgroundColourId,ui::surface);
        setColour(juce::PopupMenu::backgroundColourId,ui::surface);setColour(juce::PopupMenu::textColourId,ui::text);
        setColour(juce::PopupMenu::highlightedBackgroundColourId,ui::raised);setColour(juce::PopupMenu::highlightedTextColourId,ui::text);
        setColour(juce::TextEditor::backgroundColourId,ui::surface);setColour(juce::TextEditor::textColourId,ui::text);
        setColour(juce::TextEditor::outlineColourId,ui::line);setColour(juce::TextEditor::focusedOutlineColourId,ui::text);
        setColour(juce::Slider::textBoxTextColourId,ui::text);setColour(juce::Slider::textBoxBackgroundColourId,ui::ink);
    }
    juce::Font getTextButtonFont(juce::TextButton&,int) override { return ui::font(14); }
    juce::Font getComboBoxFont(juce::ComboBox&) override { return ui::font(14); }
    juce::Font getPopupMenuFont() override { return ui::font(14); }
    void drawButtonBackground(juce::Graphics& g,juce::Button& b,const juce::Colour&,bool hover,bool down) override {
        auto r=b.getLocalBounds().toFloat().reduced(.5f);
        g.setColour(down || b.getToggleState() ? ui::raised.brighter(.12f) : hover ? ui::raised : ui::surface);g.fillRoundedRectangle(r,2);
        g.setColour(b.getToggleState() || b.hasKeyboardFocus(false) ? ui::text : ui::line);g.drawRoundedRectangle(r,2,b.getToggleState() ? 1.5f : 1);
        // An inset ring remains distinct even on an already selected button.
        if(b.hasKeyboardFocus(false)){g.setColour(ui::text);g.drawRoundedRectangle(r.reduced(2),2,1);}
    }
    void drawComboBox(juce::Graphics& g,int w,int h,bool,int,int,int,int,juce::ComboBox& c) override {
        g.setColour(ui::surface);g.fillRect(0,0,w,h);g.setColour(c.hasKeyboardFocus(false) ? ui::text : ui::line);g.drawRect(0,0,w,h);
        juce::Path p;p.startNewSubPath(w-17.0f,h*.42f);p.lineTo(w-12.0f,h*.61f);p.lineTo(w-7.0f,h*.42f);g.strokePath(p,juce::PathStrokeType(1.5f));
    }
    void drawToggleButton(juce::Graphics& g,juce::ToggleButton& b,bool hover,bool down) override {
        g.setColour(hover || down ? ui::raised : ui::surface);g.fillRect(b.getLocalBounds());
        g.setColour(b.hasKeyboardFocus(false) ? ui::text : ui::line);g.drawRect(4,5,18,18);if(b.getToggleState()){g.setColour(ui::text);g.fillRect(8,9,10,10);}
        if(b.hasKeyboardFocus(false)){g.setColour(ui::text);g.drawRect(b.getLocalBounds().reduced(1));}
        ui::textAt(g,b.getButtonText(),b.getLocalBounds().withTrimmedLeft(30),14);
    }
    void drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float pos,float a,float z,juce::Slider& slider) override {
        auto centre=juce::Point<float>(x+w*.5f,y+h*.5f);float radius=std::min(w,h)*.5f-4;
        juce::Path track;track.addCentredArc(centre.x,centre.y,radius,radius,0,a,z,true);g.setColour(ui::line);g.strokePath(track,juce::PathStrokeType(2));
        juce::Path arc;arc.addCentredArc(centre.x,centre.y,radius,radius,0,a,a+pos*(z-a),true);g.setColour(ui::text);g.strokePath(arc,juce::PathStrokeType(2));
        auto angle=a+pos*(z-a);g.drawLine(centre.x+std::sin(angle)*radius*.48f,centre.y-std::cos(angle)*radius*.48f,centre.x+std::sin(angle)*radius*.84f,centre.y-std::cos(angle)*radius*.84f,2);
        if(slider.hasKeyboardFocus(false)){g.setColour(ui::text);g.drawRect(x,y,w,h);}
    }
    void drawScrollbar(juce::Graphics& g,juce::ScrollBar&,int x,int y,int w,int h,bool vertical,int thumbStart,int thumbSize,bool hover,bool) override {
        g.setColour(ui::surface);g.fillRect(x,y,w,h);g.setColour(hover ? ui::text : ui::line);
        if(vertical)g.fillRect(x+2,thumbStart,w-4,thumbSize);else g.fillRect(thumbStart,y+2,thumbSize,h-4);
    }
};
class Voicing final : public juce::Component {
public:
    Voicing(Editor& e,std::function<Chord()> read,std::function<void(Chord)> write) : editor(e),get(std::move(read)),set(std::move(write)) {
        for(auto* b:{&previous,&next,&seventh,&sus2,&sus4}){addAndMakeVisible(b);b->setWantsKeyboardFocus(true);}
        previous.onClick=[this]{mutate([](Chord& c){c.inversion=std::max(0,c.inversion-1);});};
        next.onClick=[this]{mutate([](Chord& c){c.inversion=std::min(c.seventh ? 3 : 2,c.inversion+1);});};
        seventh.onClick=[this]{mutate([](Chord& c){setSeventh(c,!c.seventh);});};
        sus2.onClick=[this]{mutate([](Chord& c){c.suspension=c.suspension==Suspension::sus2 ? Suspension::none : Suspension::sus2;});};
        sus4.onClick=[this]{mutate([](Chord& c){c.suspension=c.suspension==Suspension::sus4 ? Suspension::none : Suspension::sus4;});};
        for(int o=0;o<5;++o){addAndMakeVisible(octaves[o]);octaves[o].setButtonText(juce::String(o+1));octaves[o].setWantsKeyboardFocus(true);octaves[o].onClick=[this,o]{mutate([o](Chord& c){c.octave=o+1;});};}
        refresh();
    }
    void mutate(const std::function<void(Chord&)>& op) { auto c=get();op(c);set(c);editor.changed(); }
    void refresh() {
        auto c=get();previous.setEnabled(c.inversion>0);next.setEnabled(c.inversion<(c.seventh ? 3 : 2));
        for(int o=0;o<5;++o)octaves[o].setToggleState(c.octave==o+1,juce::dontSendNotification);
        seventh.setToggleState(c.seventh,juce::dontSendNotification);sus2.setToggleState(c.suspension==Suspension::sus2,juce::dontSendNotification);sus4.setToggleState(c.suspension==Suspension::sus4,juce::dontSendNotification);
        auto& s=editor.processor.session.settings;seventh.setVisible(s.showSeventh);sus2.setVisible(s.showSus2);sus4.setVisible(s.showSus4);resized();repaint();
    }
    void resized() override {
        previous.setBounds(0,0,24,24);next.setBounds(getWidth()-24,0,24,24);
        for(int o=0;o<5;++o){int x=o*getWidth()/5,right=(o+1)*getWidth()/5;octaves[o].setBounds(x,28,right-x-1,24);}
        std::vector<juce::Button*> enabled;for(auto* b:{&seventh,&sus2,&sus4})if(b->isVisible())enabled.push_back(b);
        for(size_t i=0;i<enabled.size();++i){int x=static_cast<int>(i)*getWidth()/static_cast<int>(enabled.size());enabled[i]->setBounds(x,56,getWidth()/static_cast<int>(enabled.size())-2,24);}
    }
    void paint(juce::Graphics& g) override { static const char* names[] {"Root","1st inv","2nd inv","3rd inv"};ui::textAt(g,names[get().inversion],{28,0,getWidth()-56,24},12); }
private:
    Editor& editor;std::function<Chord()> get;std::function<void(Chord)> set;
    juce::TextButton previous {u8"\u2212"},next {"+"},seventh {"7th"},sus2 {"Sus2"},sus4 {"Sus4"};
    std::array<juce::TextButton,5> octaves;
};
class Pad final : public juce::Component {
public:
    Pad(Editor& e,int d) : editor(e),degree(d),voicing(e,[this]{return editor.processor.session.pads[degree];},[this](Chord c){editor.processor.session.changePad(degree,c);}) {
        addAndMakeVisible(voicing);addAndMakeVisible(reset);reset.setWantsKeyboardFocus(true);reset.setTooltip("Reset this pad to its key's default chord");reset.onClick=[this]{editor.processor.session.resetPad(degree);editor.changed();};
    }
    void resized() override { int height=ui::optional(editor.processor.session) ? 80 : 52;voicing.setBounds(8,getHeight()-height-12,getWidth()-16,height);reset.setBounds(getWidth()-52,8,44,24); }
    void refresh() { voicing.refresh();resized();repaint(); }
    void paint(juce::Graphics& g) override {
        auto r=getLocalBounds();g.setColour(ui::surface);g.fillRect(r);g.setColour(ui::degree[degree].withAlpha(.10f));g.fillRect(r);g.setColour(ui::degree[degree]);g.fillRect(0,0,3,getHeight());
        auto c=resolve(editor.processor.session.pads[degree]);ui::textAt(g,juce::String(degree+1),{12,8,24,24},12,ui::secondary);
        ui::textAt(g,juce::String(c.rootLabel),{14,42,getWidth()-24,36},28);auto quality=c.label.substr(c.rootLabel.size());
        ui::textAt(g,quality.empty() ? "Major" : juce::String(quality),{14,80,getWidth()-24,22},14,ui::secondary);
        if(editor.processor.session.activePad()==degree){g.setColour(ui::text);g.drawRect(r.reduced(1),2);g.fillRect(8,35,getWidth()-16,3);}
    }
    void mouseDown(const juce::MouseEvent& e) override {
        editor.grabKeyboardFocus();dragging=false;chord=editor.processor.session.pads[degree];
        if(e.getNumberOfClicks()>1){editor.processor.session.cancelPreview();return;}
        editor.processor.session.pressPad(degree);repaint();
    }
    void mouseDoubleClick(const juce::MouseEvent&) override { editor.processor.session.cancelPreview();editor.processor.session.edit([this](auto& d){return d.append(chord,editor.processor.session.settings.editGrid);});editor.changed(); }
    void mouseDrag(const juce::MouseEvent& e) override { if(e.getDistanceFromDragStart()<6)return;if(!dragging){dragging=true;editor.processor.session.cancelPreview();editor.beginPadDrag(chord,e);}editor.updatePadDrag(e); }
    void mouseUp(const juce::MouseEvent& e) override { if(dragging)editor.finishPadDrag(e);else editor.processor.session.releasePad(degree);dragging=false;repaint(); }
private:
    Editor& editor;int degree;Voicing voicing;juce::TextButton reset {"Reset"};bool dragging=false;Chord chord;
};
class Canvas final : public juce::Component {
public:
    explicit Canvas(Editor& e) : editor(e) { setWantsKeyboardFocus(true); }
    bool razor=false,fitted=true;
    double scale=.03,pan=0;
    std::function<void()> viewChanged;
    float x(int tick) const { return static_cast<float>(tick*scale-pan); }
    int tick(float pixel) const { return static_cast<int>(std::floor((pixel+pan)/scale)); }
    juce::Rectangle<float> rectangle(const Block& b) const { return {x(b.start),32,std::max(1.0f,static_cast<float>((b.end-b.start)*scale)),static_cast<float>(getHeight()-38)}; }
    void fit() { fitted=true;scale=static_cast<double>(getWidth())/(editor.processor.session.document.state().bars*bar);pan=0;updateView(); }
    void zoom(double factor,float anchor) { double time=(anchor+pan)/scale;fitted=false;scale=std::clamp(scale*factor,static_cast<double>(getWidth())/hardEnd,.5);pan=time*scale-anchor;updateView(); }
    void updateView() { pan=std::clamp(pan,0.0,std::max(0.0,hardEnd*scale-getWidth()));if(viewChanged)viewChanged();refreshControls();repaint(); }
    void refresh() { if(fitted)fit();else updateView(); }
    void refreshControls() {
        controls.clear();auto& s=editor.processor.session;
        for(auto& b:s.document.state().blocks) {
            auto r=rectangle(b);int needed=ui::optional(s) ? 80 : 52;
            if(r.getWidth()<160 || r.getHeight()<needed+40 || r.getRight()<0 || r.getX()>getWidth())continue;
            auto id=b.id;auto c=std::make_unique<Voicing>(editor,[this,id]{if(auto b=editor.processor.session.document.find(id))return b->chord;return Chord{};},[this,id](Chord c){editor.processor.session.edit([id,c](auto& d){return d.voicing(id,c);});});
            addAndMakeVisible(*c);c->setBounds(static_cast<int>(r.getX())+8,static_cast<int>(r.getY())+34,static_cast<int>(r.getWidth())-16,needed);controls.push_back(std::move(c));
        }
    }
    void paint(juce::Graphics& g) override {
        g.fillAll(ui::ink);auto& s=editor.processor.session;int end=s.document.state().bars*bar;
        g.setColour(ui::surface);g.fillRect(0,32,std::max(0,static_cast<int>(x(end))),getHeight()-38);
        int first=std::max(0,tick(0)/ppq),last=std::min(hardEnd/ppq,tick(static_cast<float>(getWidth()))/ppq+1);
        for(int beat=first;beat<=last;++beat){int px=static_cast<int>(x(beat*ppq));g.setColour(beat%4==0 ? ui::line.withAlpha(.6f) : ui::line.withAlpha(.18f));g.drawVerticalLine(px,28.0f,static_cast<float>(getHeight()));if(beat%4==0)ui::textAt(g,juce::String(beat/4+1),{px+4,0,40,24},12,ui::secondary);}
        auto active=editor.processor.engine.uiBlock.load();bool overridden=editor.processor.engine.uiOverride.load();
        for(auto& b:s.document.state().blocks) {
            auto r=rectangle(b);if(r.getRight()<0 || r.getX()>getWidth())continue;
            g.setColour(ui::degree[b.chord.degree].withAlpha(.17f));g.fillRect(r);g.setColour(ui::degree[b.chord.degree]);g.fillRect(r.withWidth(3));
            auto label=juce::String(resolve(b.chord).label);ui::textAt(g,label,r.toNearestInt().reduced(9,4).withHeight(24),16);
            bool selected=std::find(s.selected.begin(),s.selected.end(),b.id)!=s.selected.end();g.setColour(selected ? ui::text : ui::degree[b.chord.degree].withAlpha(.6f));g.drawRect(r.reduced(.75f),selected ? 2.0f : 1.0f);
            bool expanded=r.getWidth()>=160 && r.getHeight()>=(ui::optional(s) ? 120 : 92);
            if(!expanded){auto menu=menuRect(b);g.setColour(ui::raised);g.fillRect(menu);g.setColour(ui::text);for(int dot=0;dot<3;++dot)g.fillEllipse(menu.getCentreX()-7+dot*5,menu.getCentreY()-1,2,2);}
            if(b.id==active){g.setColour(overridden ? ui::secondary : ui::text);g.fillRect(r.withHeight(3));}
            if(std::find(victims.begin(),victims.end(),b.id)!=victims.end()){g.saveState();g.reduceClipRegion(r.toNearestInt());g.setColour(ui::text.withAlpha(.6f));for(float j=r.getX()-r.getHeight();j<r.getRight();j+=8)g.drawLine(j,r.getBottom(),j+r.getHeight(),r.getY(),1);g.restoreState();}
        }
        if(validPreview){g.setColour(ui::text);for(auto& b:preview.blocks)if(std::find(incoming.begin(),incoming.end(),b.id)!=incoming.end())g.drawRect(rectangle(b),2);}
        if(mode==Mode::marquee){g.setColour(ui::text.withAlpha(.08f));g.fillRect(marquee);g.setColour(ui::text);g.drawRect(marquee,1);}
        if(razor && hovered && mode==Mode::none){auto b=s.document.find(hovered);if(b){int t=snap(hoverTick,s.settings.razorGrid);bool legal=t-b->start>=ppq && b->end-t>=ppq;g.setColour(legal ? ui::text : ui::secondary);g.drawVerticalLine(static_cast<int>(x(t)),32.0f,static_cast<float>(getHeight()-6));}}
        int playhead=static_cast<int>(x(static_cast<int>(editor.processor.engine.uiTick.load())));g.setColour(ui::text);g.drawVerticalLine(playhead,24.0f,static_cast<float>(getHeight()));
        if(hasKeyboardFocus(false)){g.setColour(ui::text);g.drawRect(getLocalBounds(),1);}
    }
    juce::Rectangle<float> menuRect(const Block& b) const { auto r=rectangle(b);float w=std::min(24.0f,r.getWidth());return {r.getRight()-w,r.getBottom()-28,w,24}; }
    const Block* hit(juce::Point<float> point) const { if(point.y<32 || point.y>getHeight()-6)return nullptr;int t=tick(point.x);for(auto& b:editor.processor.session.document.state().blocks)if(t>=b.start && t<b.end)return &b;return nullptr; }
    void mouseMove(const juce::MouseEvent& e) override {
        auto b=hit(e.position);hovered=b ? b->id : 0;hoverTick=tick(e.position.x);
        bool edge=b && (std::abs(e.position.x-x(b->start))<edgeWidth(*b) || std::abs(e.position.x-x(b->end))<edgeWidth(*b));
        setMouseCursor(razor ? juce::MouseCursor::CrosshairCursor : edge ? juce::MouseCursor::LeftRightResizeCursor : juce::MouseCursor::NormalCursor);repaint();
    }
    void mouseExit(const juce::MouseEvent&) override { hovered=0;repaint(); }
    void mouseDown(const juce::MouseEvent& e) override {
        grabKeyboardFocus();editor.popover(0);auto& s=editor.processor.session;auto b=hit(e.position);before=s.document.state();down=e.position;mode=Mode::none;dragging=false;validPreview=false;victims.clear();incoming.clear();
        if(!b){mode=Mode::marquee;originalSelection=s.selected;return;}
        owner=b->id;
        bool expanded=rectangle(*b).getWidth()>=160 && rectangle(*b).getHeight()>=(ui::optional(s) ? 120 : 92);
        if(!expanded && menuRect(*b).contains(e.position)){editor.popover(owner);return;}
        if(razor){s.edit([&](auto& d){return d.slice(owner,tick(e.position.x),s.settings.razorGrid);});editor.changed();return;}
        if(e.mods.isShiftDown()){s.select(owner,true);editor.changed();return;}
        if(std::find(s.selected.begin(),s.selected.end(),owner)==s.selected.end())s.select(owner,false);
        group=s.selected;anchor=hardEnd;for(auto id:group)if(auto member=s.document.find(id))anchor=std::min(anchor,member->start);
        offset=tick(e.position.x)-anchor;
        mode=std::abs(e.position.x-x(b->start))<edgeWidth(*b) ? Mode::left : std::abs(e.position.x-x(b->end))<edgeWidth(*b) ? Mode::right : Mode::move;
        if(mode==Mode::move)s.pressBlock(owner);repaint();
    }
    void mouseDrag(const juce::MouseEvent& e) override {
        if(mode==Mode::none || e.getDistanceFromDragStart()<6)return;dragging=true;auto& s=editor.processor.session;
        if(mode==Mode::move && s.activeBlock()==owner)s.releaseMomentary();
        if(mode==Mode::marquee){marquee=juce::Rectangle<float>(down,e.position);repaint();return;}
        candidate=mode==Mode::move ? tick(e.position.x)-offset : tick(e.position.x);previewEdit();
    }
    void mouseUp(const juce::MouseEvent& e) override {
        auto& s=editor.processor.session;
        if(mode==Mode::marquee) {
            if(dragging){auto chosen=e.mods.isShiftDown() ? originalSelection : std::vector<uint64_t>{};for(auto& b:s.document.state().blocks)if(marquee.intersects(rectangle(b)) && std::find(chosen.begin(),chosen.end(),b.id)==chosen.end())chosen.push_back(b.id);s.setSelection(std::move(chosen));}
            else{s.setSelection({});s.seek(tick(e.position.x));}
        }else if(dragging && validPreview)commitGesture();else if(mode==Mode::move)s.select(owner,false);
        if(mode==Mode::move && s.activeBlock()==owner)s.releaseMomentary();clearGesture();editor.changed();
    }
    void mouseWheelMove(const juce::MouseEvent& e,const juce::MouseWheelDetails& wheel) override {
        if(e.mods.isCtrlDown())zoom(wheel.deltaY>0 ? 1.2 : 1/1.2,e.position.x);
        else if(e.mods.isShiftDown()){pan-=(wheel.deltaY+wheel.deltaX)*getWidth()*.35;fitted=false;updateView();}
    }
    void beginPad(Chord chord) { before=editor.processor.session.document.state();padChord=chord;mode=Mode::pad;group.clear(); }
    void padMove(juce::Point<float> point) { candidate=tick(point.x);if(getLocalBounds().toFloat().contains(point)){previewEdit();}else{validPreview=false;victims.clear();incoming.clear();repaint();} }
    void padEnd(juce::Point<float> point) { padMove(point);if(validPreview)commitGesture();clearGesture();editor.changed(); }
private:
    Editor& editor;
    // Preserve a body target when minimum-duration blocks are only a few pixels
    // wide. The fixed Edit Selected Chord control provides the full-size editor.
    float edgeWidth(const Block& block) const { return std::min(5.0f,rectangle(block).getWidth()*.18f); }
    enum class Mode { none,pad,move,left,right,marquee } mode=Mode::none;
    Timeline before,preview;Chord padChord;
    juce::Point<float> down;juce::Rectangle<float> marquee;
    bool dragging=false,validPreview=false;
    uint64_t owner=0,hovered=0;int hoverTick=0,anchor=0,offset=0,candidate=0;
    std::vector<uint64_t> group,victims,incoming,originalSelection;
    std::vector<std::unique_ptr<Voicing>> controls;
    bool operation(Document& d) {
        int grid=editor.processor.session.settings.editGrid;
        if(mode==Mode::pad)return d.add(padChord,candidate,grid);
        if(mode==Mode::move)return d.move(group,candidate,grid);
        return d.resize(owner,mode==Mode::left,candidate,grid);
    }
    void previewEdit() {
        Document d;d.load(before);validPreview=operation(d);victims.clear();incoming.clear();
        if(validPreview){preview=d.state();for(auto& old:before.blocks)if(!d.find(old.id) && std::find(group.begin(),group.end(),old.id)==group.end())victims.push_back(old.id);for(auto& b:preview.blocks)if(mode==Mode::pad ? std::none_of(before.blocks.begin(),before.blocks.end(),[&](auto& old){return old.id==b.id;}) : std::find(group.begin(),group.end(),b.id)!=group.end() || b.id==owner)incoming.push_back(b.id);}
        repaint();
    }
    void commitGesture() { if(editor.processor.session.document.state()!=before)return;editor.processor.session.edit([this](auto& d){return operation(d);}); }
    void clearGesture() { mode=Mode::none;validPreview=false;victims.clear();incoming.clear();marquee={};repaint(); }
};
struct Editor::ExportHandle final : juce::Component {
    explicit ExportHandle(Editor& e) : editor(e) { setMouseCursor(juce::MouseCursor::DraggingHandCursor); }
    Editor& editor;bool started=false;
    void paint(juce::Graphics& g) override { g.setColour(ui::raised);g.fillRect(getLocalBounds());g.setColour(ui::text);g.drawRect(getLocalBounds(),1);ui::textAt(g,"Drag MIDI to Ableton",getLocalBounds().reduced(14,0),14); }
    void mouseDown(const juce::MouseEvent&) override { started=false; }
    void mouseDrag(const juce::MouseEvent& e) override {
        if(started || e.getDistanceFromDragStart()<6)return;started=true;
        try {
            auto snapshot=editor.processor.session.document.state();auto bytes=midi(snapshot);
            auto retained=TemporaryMidi::prepare(bytes,logsFolder().parent_path()/L"Temp");
            auto path=juce::String(retained->file().wstring().c_str());
            juce::FileInputStream input{juce::File(path)};juce::MidiFile parsed;
            if(!parsed.readFrom(input,false) || parsed.getNumTracks()!=1 || parsed.getTimeFormat()!=ppq || parsed.getLastTimestamp()!=snapshot.bars*bar)throw std::runtime_error("MIDI parse-back failed");
            int notes=0;for(auto& block:snapshot.blocks)notes+=resolve(block.chord).count;
            auto* details=new juce::DynamicObject;details->setProperty("revision",static_cast<juce::int64>(editor.processor.session.document.revision()));details->setProperty("end_tick",snapshot.bars*bar);details->setProperty("bytes",static_cast<int>(bytes.size()));details->setProperty("notes",notes);details->setProperty("sha256",juce::SHA256(bytes.data(),bytes.size()).toHexString());editor.processor.event("export.prepared",juce::var(details));
            auto service=editor.processor.logs;auto instance=editor.processor.instance;
            bool launched=juce::DragAndDropContainer::performExternalDragDropOfFiles({path},false,this,[service,instance,retained]{if(service)service->post(instance,"export.drag_ended","{\"host_import_confirmed\":false}");});
            if(!launched)throw std::runtime_error("Native drag failed");
        }catch(const std::exception&){editor.fail("export_midi","Couldn't drag the MIDI file. Retry, or open Logs Folder for details.");}
    }
};
struct Editor::PopoverBackground final : juce::Component {
    void paint(juce::Graphics& g) override { g.fillAll(ui::raised);g.setColour(ui::text);g.drawRect(getLocalBounds(),1); }
};
Editor::Editor(Processor& p) : AudioProcessorEditor(p),processor(p),skin(std::make_unique<Skin>()),canvas(std::make_unique<Canvas>(*this)),exporter(std::make_unique<ExportHandle>(*this)) {
    setLookAndFeel(skin.get());setWantsKeyboardFocus(true);processor.interactive();
    for(auto* c:std::initializer_list<juce::Component*>{&key,&sound,&rate,&volume,&length,&repeats,&settings,&play,&stop,&start,&sync,&select,&razor,&undo,&redo,&minus,&plus,&zoomMinus,&zoomPlus,&fit,&save,&load,&logs,&back,&seventh,&sus2,&sus4,&editGrid,&sliceGrid,&status,&about,&logging,&scrollbar,canvas.get(),exporter.get(),&closePopover,&popoverTitle})addAndMakeVisible(c);
    for(int d=0;d<7;++d){pads[d]=std::make_unique<Pad>(*this,d);addAndMakeVisible(*pads[d]);}
    for(auto* b:std::initializer_list<juce::Button*>{&repeats,&settings,&play,&stop,&start,&sync,&select,&razor,&undo,&redo,&minus,&plus,&zoomMinus,&zoomPlus,&fit,&save,&load,&logs,&back,&seventh,&sus2,&sus4,&closePopover})b->setWantsKeyboardFocus(true);
    auto inventory=keys();for(size_t i=0;i<inventory.size();++i)key.addItem(juce::String(keyLabel(inventory[i])),static_cast<int>(i+1));key.onChange=[this,inventory]{if(key.getSelectedId()>0){processor.session.changeKey(inventory[static_cast<size_t>(key.getSelectedId()-1)]);changed();}};
    sound.addItemList({"Piano","Guitar","Strings","Pad"},1);sound.onChange=[this]{processor.session.sound=static_cast<Sound>(sound.getSelectedId()-1);processor.session.send();processor.event("sound.change");};
    rate.addItemList({"1/8 note","1/4 note","1/2 note","1 bar","2 bars"},1);rate.onChange=[this]{constexpr int ticks[] {480,960,1920,3840,7680};if(rate.getSelectedId()>0)processor.session.setRepeatRate(ticks[rate.getSelectedId()-1]);};
    volume.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);volume.setTextBoxStyle(juce::Slider::TextBoxBelow,false,54,18);volume.setRange(-60,0,.1);volume.setTextValueSuffix(" dB");volume.onValueChange=[this]{processor.session.gain=volume.getValue()<=-60 ? 0 : static_cast<float>(std::pow(10.0,volume.getValue()/20));processor.session.send();};
    repeats.onClick=[this]{processor.session.setRepeats(!processor.session.repeats);changed();};settings.onClick=[this]{settingsOpen=true;popover(0);changed();};back.onClick=[this]{settingsOpen=false;changed();};
    play.onClick=[this]{processor.session.play();};stop.onClick=[this]{processor.session.stop();};start.onClick=[this]{processor.session.seek(0);};sync.onClick=[this]{processor.session.setSync(!processor.session.sync);changed();};
    select.onClick=[this]{canvas->razor=false;changed();};razor.onClick=[this]{canvas->razor=true;changed();};undo.onClick=[this]{processor.session.undo();changed();};redo.onClick=[this]{processor.session.redo();changed();};
    minus.onClick=[this]{processor.session.edit([](auto& d){return d.length(d.state().bars-1);});changed();};plus.onClick=[this]{processor.session.edit([](auto& d){return d.length(d.state().bars+1);});changed();};
    zoomMinus.onClick=[this]{canvas->zoom(1/1.2,canvas->x(static_cast<int>(processor.engine.uiTick.load())));};zoomPlus.onClick=[this]{canvas->zoom(1.2,canvas->x(static_cast<int>(processor.engine.uiTick.load())));};fit.onClick=[this]{canvas->fit();};
    length.setFont(ui::font(14));length.setInputRestrictions(8);length.setSelectAllWhenFocused(true);length.onReturnKey=[this]{applyLength();};length.onFocusLost=[this]{applyLength();};
    save.onClick=[this]{manualFile(true);};load.onClick=[this]{manualFile(false);};
    addAndMakeVisible(editSelected);editSelected.setWantsKeyboardFocus(true);editSelected.onClick=[this]{if(processor.session.selected.size()==1)popover(processor.session.selected.front());};
    logs.onClick=[this]{try{auto path=logsFolder();std::filesystem::create_directories(path);if(!juce::File(juce::String(path.wstring().c_str())).startAsProcess())throw std::runtime_error("Explorer failed");}catch(const std::exception&){fail("open_logs","Couldn't open Logs Folder. Check folder permissions and try again.");}};
    auto features=[this]{processor.session.settings.showSeventh=seventh.getToggleState();processor.session.settings.showSus2=sus2.getToggleState();processor.session.settings.showSus4=sus4.getToggleState();processor.snapshot();changed();};seventh.onClick=features;sus2.onClick=features;sus4.onClick=features;
    for(auto* c:{&editGrid,&sliceGrid})c->addItemList({"1/16 note","1/8 note","1/4 note","1/2 note","1 bar"},1);
    editGrid.onChange=[this]{if(editGrid.getSelectedId()>0)processor.session.settings.editGrid=240<<(editGrid.getSelectedId()-1);};sliceGrid.onChange=[this]{if(sliceGrid.getSelectedId()>0)processor.session.settings.razorGrid=240<<(sliceGrid.getSelectedId()-1);};
    canvas->viewChanged=[this]{scrollbar.setRangeLimits(0,hardEnd*canvas->scale);scrollbar.setCurrentRange(canvas->pan,canvas->getWidth(),juce::dontSendNotification);};
    scrollbar.addListener(this);
    closePopover.onClick=[this]{popover(0);};about.setFont(ui::font(14));about.setColour(juce::Label::textColourId,ui::text);
    auto metadata=juce::JSON::parse(chordCanvasRelease);juce::String changes="ChordCanvas "+metadata["version"].toString()+juce::String(u8" \u00b7 AGPLv3\n\n");
    auto currentChanges=metadata["changes"];if(auto* list=currentChanges.getArray())for(auto& change:*list)changes+=change.toString()+"\n";about.setText(changes,juce::dontSendNotification);
    for(auto* l:{&status,&logging,&popoverTitle}){l->setFont(ui::font(12));l->setColour(juce::Label::textColourId,ui::secondary);}
    // JUCE setResizeLimits constrains the initial zero-size bounds immediately,
    // calling our resized(). Every child and its callbacks must exist first.
    setResizable(true,true);setResizeLimits(1000,560,1800,1200);
    setSize(1040,640);handleAsyncUpdate();startTimerHz(30);processor.event("editor.open");
}
Editor::~Editor() { stopTimer();cancelPendingUpdate();scrollbar.removeListener(this);processor.session.loseFocus();processor.event("editor.close");setLookAndFeel(nullptr); }
void Editor::scrollBarMoved(juce::ScrollBar*,double position) { canvas->fitted=false;canvas->pan=position;canvas->refreshControls();canvas->repaint(); }
void Editor::changed() { triggerAsyncUpdate(); }
void Editor::handleAsyncUpdate() {
    auto& s=processor.session;auto inventory=keys();auto i=std::find(inventory.begin(),inventory.end(),s.key);key.setSelectedId(static_cast<int>(i-inventory.begin()+1),juce::dontSendNotification);
    int minimumHeight=ui::optional(s) ? 608 : 560;
    setResizeLimits(1000,minimumHeight,1800,1200);
    if(getHeight()<minimumHeight)setSize(getWidth(),minimumHeight);
    sound.setSelectedId(static_cast<int>(s.sound)+1,juce::dontSendNotification);volume.setValue(s.gain>0 ? 20*std::log10(s.gain) : -60,juce::dontSendNotification);
    constexpr int repeatRates[] {480,960,1920,3840,7680};for(int r=0;r<5;++r)if(repeatRates[r]==s.repeatTicks)rate.setSelectedId(r+1,juce::dontSendNotification);
    repeats.setToggleState(s.repeats,juce::dontSendNotification);sync.setToggleState(s.sync,juce::dontSendNotification);play.setEnabled(!s.sync);stop.setEnabled(!s.sync);
    select.setToggleState(!canvas->razor,juce::dontSendNotification);razor.setToggleState(canvas->razor,juce::dontSendNotification);
    length.setText(juce::String(s.document.state().bars),false);seventh.setToggleState(s.settings.showSeventh,juce::dontSendNotification);sus2.setToggleState(s.settings.showSus2,juce::dontSendNotification);sus4.setToggleState(s.settings.showSus4,juce::dontSendNotification);
    for(int n=0;n<5;++n){if(s.settings.editGrid==(240<<n))editGrid.setSelectedId(n+1,juce::dontSendNotification);if(s.settings.razorGrid==(240<<n))sliceGrid.setSelectedId(n+1,juce::dontSendNotification);}
    if(popupOwner && !s.document.find(popupOwner))popover(0);if(popup)popup->refresh();
    for(auto& pad:pads)pad->refresh();resized();canvas->refresh();repaint();
}
void Editor::paint(juce::Graphics& g) {
    g.fillAll(ui::ink);ui::textAt(g,"ChordCanvas",{24,24,190,32},24);
    ui::textAt(g,"Key",{240,8,200,16},12,ui::secondary);ui::textAt(g,"Sound",{456,8,140,16},12,ui::secondary);ui::textAt(g,"Volume",{620,8,90,16},12,ui::secondary);
    if(settingsOpen){ui::textAt(g,"Settings",{24,134,280,32},24);ui::textAt(g,"Edit grid",{24,298,160,24});ui::textAt(g,"Slice grid",{320,298,160,24});}
    else ui::textAt(g,"Length",{606,132+(ui::optional(processor.session) ? 218 : 190)+24,48,28},12,ui::secondary);
}
void Editor::resized() {
    key.setBounds(240,28,194,28);sound.setBounds(456,28,140,28);volume.setBounds(628,25,54,56);settings.setBounds(getWidth()-128,28,104,28);status.setBounds(710,24,getWidth()-862,48);
    int padHeight=ui::optional(processor.session) ? 218 : 190,toolbar=132+padHeight+24;
    repeats.setBounds(24,92,100,28);rate.setBounds(136,92,138,28);play.setBounds(24,toolbar,48,28);stop.setBounds(76,toolbar,48,28);start.setBounds(128,toolbar,116,28);sync.setBounds(248,toolbar,52,28);
    select.setBounds(324,toolbar,56,28);razor.setBounds(384,toolbar,56,28);undo.setBounds(464,toolbar,52,28);redo.setBounds(520,toolbar,52,28);
    minus.setBounds(660,toolbar,24,28);length.setBounds(688,toolbar,42,28);plus.setBounds(734,toolbar,24,28);zoomMinus.setBounds(getWidth()-148,toolbar,28,28);zoomPlus.setBounds(getWidth()-116,toolbar,28,28);fit.setBounds(getWidth()-80,toolbar,56,28);
    int padWidth=(getWidth()-48-36)/7;for(int d=0;d<7;++d)pads[d]->setBounds(24+d*(padWidth+6),132,padWidth,padHeight);
    canvas->setBounds(24,toolbar+48,getWidth()-48,getHeight()-toolbar-126);scrollbar.setBounds(24,getHeight()-68,getWidth()-48,14);
    save.setBounds(24,getHeight()-44,144,28);load.setBounds(176,getHeight()-44,144,28);exporter->setBounds(getWidth()-264,getHeight()-44,240,28);
    editSelected.setBounds(344,getHeight()-44,168,28);editSelected.setVisible(!settingsOpen && processor.session.selected.size()==1);
    back.setBounds(getWidth()-172,134,148,28);seventh.setBounds(24,190,252,28);sus2.setBounds(24,222,252,28);sus4.setBounds(24,254,252,28);editGrid.setBounds(24,328,252,28);sliceGrid.setBounds(320,328,252,28);about.setBounds(24,380,getWidth()-48,100);logs.setBounds(24,getHeight()-80,200,28);logging.setBounds(240,getHeight()-80,getWidth()-264,28);
    for(auto& pad:pads)pad->setVisible(!settingsOpen);for(auto* c:std::initializer_list<juce::Component*>{&select,&razor,&undo,&redo,&minus,&plus,&length,&zoomMinus,&zoomPlus,&fit,&save,&load,&scrollbar,canvas.get(),exporter.get()})c->setVisible(!settingsOpen);
    for(auto* c:std::initializer_list<juce::Component*>{&back,&seventh,&sus2,&sus4,&editGrid,&sliceGrid,&about,&logs,&logging})c->setVisible(settingsOpen);
    if(settingsOpen){play.setBounds(24,92,48,28);stop.setBounds(76,92,48,28);start.setBounds(128,92,116,28);sync.setBounds(248,92,52,28);repeats.setBounds(324,92,100,28);rate.setBounds(436,92,138,28);}
    if(popup){auto b=processor.session.document.find(popupOwner);if(b){int px=std::clamp(24+static_cast<int>(canvas->x(b->start)),12,getWidth()-250);int py=std::clamp(canvas->getY()+canvas->getHeight()-110,120,getHeight()-154);popupBackground->setBounds(px,py,240,130);popupBackground->toFront(false);popup->setBounds(px+10,py+40,220,80);popoverTitle.setBounds(px+8,py+4,148,28);closePopover.setBounds(px+164,py+4,64,28);popup->toFront(false);popoverTitle.toFront(false);closePopover.toFront(false);}}
}
void Editor::timerCallback() {
    if(processor.bypassPreviewCleanup.exchange(false,std::memory_order_relaxed))processor.session.cancelPreview();
    bool playing=processor.engine.uiRunning.load(),preview=processor.engine.uiOverride.load();juce::String state=preview ? processor.session.repeatLatched() ? "Repeating" : "Preview" : playing ? "Playing" : processor.session.sync ? "Sync armed" : "Stopped";
    bool meter=processor.engine.uiMeter.load();if(!meter)state="4/4 required";
    bool tempoKnown=processor.engine.uiTempoKnown.load(),tempoAvailable=processor.engine.uiTempoAvailable.load();
    play.setEnabled(!processor.session.sync && meter && tempoKnown);
    auto tempo=tempoKnown ? juce::String(processor.engine.uiTempo.load(),1)+" BPM"+(tempoAvailable ? "" : " (last)") : juce::String("Tempo unavailable");
    status.setText(state+"\n"+tempo,juce::dontSendNotification);
    status.setTooltip(!tempoKnown ? "Host tempo unavailable. Preview uses 120 BPM until the host supplies a tempo." : !tempoAvailable ? "Host tempo unavailable. Using the last valid host tempo." : "Tempo follows the host.");
    if(environmentPoll++%15==0)recordEnvironment();
    canvas->repaint();for(auto& pad:pads)pad->repaint();
    if(settingsOpen){if(processor.logs){auto report=processor.logs->status();logging.setText(report.available ? "Logging active" : juce::String(report.reason),juce::dontSendNotification);}else logging.setText(processor.loggingFailure,juce::dontSendNotification);}
}
void Editor::recordEnvironment() {
    auto transform=juce::Component::getApproximateScaleFactorForComponent(this);double platform=-1;juce::String renderer="unknown";
    if(auto* peer=getPeer()){platform=peer->getPlatformScaleFactor();auto names=peer->getAvailableRenderingEngines();auto current=peer->getCurrentRenderingEngine();if(juce::isPositiveAndBelow(current,names.size()))renderer=names[current];}
    if(reportedWidth==getWidth() && reportedHeight==getHeight() && reportedTransformScale==transform && reportedPlatformScale==platform && reportedRenderer==renderer)return;
    reportedWidth=getWidth();reportedHeight=getHeight();reportedTransformScale=transform;reportedPlatformScale=platform;reportedRenderer=renderer;
    auto* data=new juce::DynamicObject;data->setProperty("width_logical_px",getWidth());data->setProperty("height_logical_px",getHeight());data->setProperty("component_transform_scale",transform);data->setProperty("peer_platform_scale",platform<0 ? juce::var() : juce::var(platform));data->setProperty("renderer",renderer);
    processor.event("editor.environment",juce::var(data));
}
void Editor::popover(uint64_t id) {
    popup.reset();popupBackground.reset();popupOwner=id;closePopover.setVisible(id!=0);popoverTitle.setVisible(id!=0);
    if(id && processor.session.document.find(id)){
        popupBackground=std::make_unique<PopoverBackground>();addAndMakeVisible(*popupBackground);popup=std::make_unique<Voicing>(*this,[this,id]{if(auto b=processor.session.document.find(id))return b->chord;return Chord{};},[this,id](Chord c){processor.session.edit([id,c](auto& d){return d.voicing(id,c);});});addAndMakeVisible(*popup);popoverTitle.setText(juce::String(resolve(processor.session.document.find(id)->chord).label),juce::dontSendNotification);resized();
    }else popupOwner=0;repaint();
}
void Editor::beginPadDrag(Chord chord,const juce::MouseEvent&) { canvas->beginPad(chord); }
void Editor::updatePadDrag(const juce::MouseEvent& e) { canvas->padMove(e.getEventRelativeTo(canvas.get()).position); }
void Editor::finishPadDrag(const juce::MouseEvent& e) { canvas->padEnd(e.getEventRelativeTo(canvas.get()).position); }
void Editor::applyLength() {
    auto value=length.getText();if(value.isNotEmpty() && value.containsOnly("0123456789")){int bars=value.getIntValue();if(bars>=1 && bars<=32)processor.session.edit([bars](auto& d){return d.length(bars);});}changed();
}
void Editor::fail(const char* operation,const juce::String& message) {
    auto* details=new juce::DynamicObject;details->setProperty("operation",operation);details->setProperty("revision",static_cast<juce::int64>(processor.session.document.revision()));details->setProperty("state_changed",false);processor.event("operation.error",juce::var(details));processor.snapshot();
    juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,"ChordCanvas",message,"OK",this);
}
void Editor::manualFile(bool saving) {
    chooser=std::make_unique<juce::FileChooser>(saving ? "Save Progression" : "Load Progression",juce::File(),"*.chordcanvas.json");auto safe=juce::Component::SafePointer<Editor>(this);
    chooser->launchAsync(saving ? juce::FileBrowserComponent::saveMode|juce::FileBrowserComponent::canSelectFiles|juce::FileBrowserComponent::warnAboutOverwriting : juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles,[safe,saving](const juce::FileChooser& dialog){
        if(!safe)return;auto file=dialog.getResult();if(file==juce::File())return;
        try {
            if(saving){auto content=saveProgression(safe->processor.session.document.state());if(!file.replaceWithText(juce::String(content)))throw std::runtime_error("Save failed");safe->processor.event("progression.save");}
            else{if(file.getSize()>1024*1024 || !file.existsAsFile())throw std::runtime_error("Load failed");auto state=loadProgression(file.loadFileAsString().toStdString());safe->processor.session.edit([&](auto& d){return d.load(state);});safe->processor.event("progression.load");safe->changed();}
        }catch(const std::exception&){safe->fail(saving ? "save_progression" : "load_progression",saving ? "Couldn't save the progression. Check the destination and retry." : "Couldn't load this progression. Choose a valid ChordCanvas progression file.");}
    });
}
bool Editor::keyPressed(const juce::KeyPress& keypress) {
    if(dynamic_cast<juce::TextEditor*>(juce::Component::getCurrentlyFocusedComponent()))return false;
    auto& s=processor.session;auto c=keypress.getTextCharacter();int k=keypress.getKeyCode();
    if(!keypress.getModifiers().isCtrlDown() && c>='1' && c<='7'){int d=c-'1';heldKeys[d]=true;s.pressPad(d,true);return true;}
    if(keypress.getModifiers().isCtrlDown()) {
        if(k=='A' && canvas->hasKeyboardFocus(true))s.selectAll();else if(k=='C')s.copy();else if(k=='V')s.paste(static_cast<int>(processor.engine.uiTick.load()));else if(k=='D')s.duplicate();else if(k=='Z')s.undo();else if(k=='Y')s.redo();else return false;changed();return true;
    }
    if(k==juce::KeyPress::deleteKey || k==juce::KeyPress::backspaceKey){s.removeSelected();changed();return true;}return false;
}
bool Editor::keyStateChanged(bool) {
    bool handled=false;for(int d=0;d<7;++d)if(heldKeys[d] && !juce::KeyPress::isKeyCurrentlyDown('1'+d)){heldKeys[d]=false;processor.session.releasePad(d,true);handled=true;}return handled;
}
void Editor::focusOfChildComponentChanged(juce::Component::FocusChangeType) {
    if(!hasKeyboardFocus(true) || dynamic_cast<juce::TextEditor*>(juce::Component::getCurrentlyFocusedComponent())){heldKeys={};processor.session.loseFocus();processor.event("editor.focus_hold_cleanup");}
}
}
