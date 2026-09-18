#include "Processor.h"
#include "Editor.h"
#include "Persistence/Progression.h"

namespace cc {
Processor::Processor() : AudioProcessor(BusesProperties().withOutput("Output",juce::AudioChannelSet::stereo(),true)) {
    session.publish=[this](const AudioFrame& frame){engine.input.publish(frame);};
    session.actionEvent=[this](const std::string& action,const Timeline&,uint64_t revision){
        auto* details=new juce::DynamicObject;
        details->setProperty("action",juce::String(action));
        details->setProperty("transaction",static_cast<juce::int64>(revision));
        details->setProperty("revision_before",static_cast<juce::int64>(revision-1));
        details->setProperty("revision_after",static_cast<juce::int64>(revision));
        event("timeline.commit",juce::var(details));snapshot();
    };
    session.send();
}
Processor::~Processor() { cancelPendingUpdate();if(logs)event("instance.destroy"); }
bool Processor::isBusesLayoutSupported(const BusesLayout& layout) const {
    return layout.getMainInputChannelSet().isDisabled() && layout.getMainOutputChannelSet()==juce::AudioChannelSet::stereo();
}
void Processor::prepareToPlay(double rate,int) { engine.prepare(rate);triggerAsyncUpdate(); }
void Processor::releaseResources() { engine.reset();triggerAsyncUpdate(); }
void Processor::handleAsyncUpdate() { session.cancelPreview(); }
HostClock Processor::clock() const noexcept {
    HostClock result;
    if(auto* playhead=getPlayHead())if(auto position=playhead->getPosition()) {
        result.bpm=position->getBpm().orFallback(0.0);
        result.playing=position->getIsPlaying();
        if(auto meter=position->getTimeSignature()){result.numerator=meter->numerator;result.denominator=meter->denominator;}
    }
    // Host song/PPQ position is deliberately never used.
    return result;
}
void Processor::processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer& incoming) {
    juce::ScopedNoDenormals guard;incoming.clear();
    if(buffer.getNumChannels()!=2){buffer.clear();return;}
    engine.process(buffer.getWritePointer(0),buffer.getWritePointer(1),buffer.getNumSamples(),clock());
}
void Processor::processBlockBypassed(juce::AudioBuffer<float>& buffer,juce::MidiBuffer& incoming) {
    bypassPreviewCleanup.store(true,std::memory_order_relaxed);
    incoming.clear();if(buffer.getNumChannels()!=2){buffer.clear();return;}
    engine.process(buffer.getWritePointer(0),buffer.getWritePointer(1),buffer.getNumSamples(),clock(),true);
}
void Processor::getStateInformation(juce::MemoryBlock& data) {
    auto minimal=Session::hostState();data.replaceAll(minimal.data(),minimal.size());
}
void Processor::setStateInformation(const void*,int) {} // Fresh processor already has defaults; do not reset an existing one.
juce::AudioProcessorEditor* Processor::createEditor() { return new Editor(*this); }
void Processor::interactive() {
    if(logs)return;
    try { logs=LogService::interactive(); }
    catch(const std::exception&) { loggingFailure="Local diagnostics unavailable";return; }
    instance=logs->instanceId();
    engine.diagnosticInstance.store(instance,std::memory_order_relaxed);
    engine.diagnostics.store(logs.get(),std::memory_order_release);
    event("instance.interactive");snapshot();
}
void Processor::event(const char* name,juce::var details) {
    if(logs)logs->post(instance,name,details.isVoid() ? "{}" : juce::JSON::toString(details,true).toStdString());
}
void Processor::snapshot() {
    if(!logs)return;
    auto* data=new juce::DynamicObject;
    data->setProperty("revision",static_cast<juce::int64>(session.document.revision()));
    data->setProperty("progression",juce::JSON::parse(juce::String(saveProgression(session.document.state()))));
    data->setProperty("key",juce::String(keyLabel(session.key)));
    data->setProperty("sound",static_cast<int>(session.sound));data->setProperty("gain",session.gain);
    data->setProperty("repeat",session.repeats);data->setProperty("repeat_ticks",session.repeatTicks);data->setProperty("sync",session.sync);
    data->setProperty("edit_grid",session.settings.editGrid);data->setProperty("slice_grid",session.settings.razorGrid);
    juce::Array<juce::var> selected,blocks,pads;
    for(auto id:session.selected)selected.add(static_cast<juce::int64>(id));data->setProperty("selected",juce::var(selected));
    for(auto& block:session.document.state().blocks) {
        auto* entry=new juce::DynamicObject;entry->setProperty("id",static_cast<juce::int64>(block.id));
        auto resolved=resolve(block.chord);juce::Array<juce::var> notes;for(int n=0;n<resolved.count;++n)notes.add(resolved.notes[n]);
        entry->setProperty("notes",juce::var(notes));entry->setProperty("label",juce::String(resolved.label));blocks.add(juce::var(entry));
    }
    for(auto chord:session.pads) {
        auto* entry=new juce::DynamicObject;auto resolved=resolve(chord);juce::Array<juce::var> notes;for(int n=0;n<resolved.count;++n)notes.add(resolved.notes[n]);
        entry->setProperty("degree",chord.degree);entry->setProperty("octave",chord.octave);entry->setProperty("inversion",chord.inversion);entry->setProperty("seventh",chord.seventh);entry->setProperty("suspension",static_cast<int>(chord.suspension));entry->setProperty("notes",juce::var(notes));pads.add(juce::var(entry));
    }
    data->setProperty("resolved_blocks",juce::var(blocks));data->setProperty("pads",juce::var(pads));
    event("state.snapshot",juce::var(data));
}
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new cc::Processor; }
