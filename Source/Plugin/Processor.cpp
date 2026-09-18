#include "Processor.h"
#include "Editor.h"
#include "Persistence/Progression.h"
#include <algorithm>
#include <cmath>

namespace cc {
Processor::Processor() : AudioProcessor(BusesProperties().withOutput("Output",juce::AudioChannelSet::stereo(),true)) {
    observedKey=juce::String(keyLabel(session.key));
    session.publish=[this](const AudioFrame& frame){engine.input.publish(frame);published={frame.revision,frame.previewOwner,frame.transportSerial,frame.seekSerial,frame.seekTick,frame.localRun};};
    session.semanticEvent=[this](const char* name,int degree,uint64_t block,bool keyboard){semantic(name,degree,block,keyboard);};
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
void Processor::semantic(const char* name,int degree,uint64_t block,bool keyboard) {
    auto currentKey=juce::String(keyLabel(session.key)),previousKey=observedKey;observedKey=currentKey;
    if(!logs)return;
    juce::ReferenceCountedObjectPtr<juce::DynamicObject> data(new juce::DynamicObject);
    data->setProperty("revision",static_cast<juce::int64>(session.document.revision()));data->setProperty("old_key",previousKey);data->setProperty("key",currentKey);
    data->setProperty("target_pad_degree_index",degree<0 ? juce::var() : juce::var(degree));data->setProperty("target_block",static_cast<juce::int64>(block));data->setProperty("keyboard_input",keyboard);
    data->setProperty("preview_pad_degree_index",session.activePad()<0 ? juce::var() : juce::var(session.activePad()));data->setProperty("preview_block",static_cast<juce::int64>(session.activeBlock()));data->setProperty("preview_owner",static_cast<juce::int64>(published.previewOwner));
    data->setProperty("repeat_mode",session.repeats);data->setProperty("repeat_latched",session.repeatLatched());data->setProperty("repeat_ticks",session.repeatTicks);
    data->setProperty("local_run_requested",published.localRun);data->setProperty("sync",session.sync);data->setProperty("transport_serial",static_cast<juce::int64>(published.transportSerial));data->setProperty("seek_serial",static_cast<juce::int64>(published.seekSerial));data->setProperty("seek_tick",published.seekTick);
    data->setProperty("diagnostic_callback_failures",static_cast<juce::int64>(session.diagnosticCallbackFailures()));data->setProperty("clipboard_blocks",static_cast<int>(session.clipboard.size()));
    juce::Array<juce::var> selected;for(auto id:session.selected)selected.add(static_cast<juce::int64>(id));data->setProperty("selected",juce::var(selected));
    const Chord* chord=degree>=0 && degree<7 ? &session.pads[degree] : nullptr;if(block)if(auto* found=session.document.find(block))chord=&found->chord;
    if(chord){auto resolved=resolve(*chord);juce::Array<juce::var> notes;for(int n=0;n<resolved.count;++n)notes.add(resolved.notes[n]);data->setProperty("notes",juce::var(notes));data->setProperty("label",juce::String(resolved.label));data->setProperty("octave",chord->octave);data->setProperty("inversion",chord->inversion);data->setProperty("seventh",chord->seventh);data->setProperty("suspension",static_cast<int>(chord->suspension));}
    event((std::string("session.")+name).c_str(),juce::var(data.get()));
    auto action=std::string_view(name);if(action.starts_with("selection.") || action.starts_with("key.") || action=="pad.voicing" || action=="pad.reset")snapshot();
}
Processor::~Processor() { cancelPendingUpdate();if(logs)event("instance.destroy"); }
bool Processor::isBusesLayoutSupported(const BusesLayout& layout) const {
    return layout.getMainInputChannelSet().isDisabled() && layout.getMainOutputChannelSet()==juce::AudioChannelSet::stereo();
}
void Processor::prepareToPlay(double rate,int frames) { engine.prepare(rate);preparedRate.store(std::isfinite(rate) && rate>=8000 ? rate : 0);preparedFrames.store(std::max(0,frames));lifecycle.store(1);triggerAsyncUpdate(); }
void Processor::releaseResources() { engine.reset();lifecycle.store(2);triggerAsyncUpdate(); }
void Processor::handleAsyncUpdate() { session.cancelPreview();auto phase=lifecycle.exchange(0);if(phase)environment(phase==1 ? "audio.prepare" : "audio.release"); }
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
    try { logs=LogService::interactive(juce::PluginHostType().getHostDescription(),juce::AudioProcessor::getWrapperTypeDescription(wrapperType)); }
    catch(const std::exception&) { loggingFailure="Local diagnostics unavailable";return; }
    instance=logs->instanceId();
    engine.diagnosticInstance.store(instance,std::memory_order_relaxed);
    engine.diagnostics.store(logs.get(),std::memory_order_release);
    event("instance.interactive");environment("instance.environment");snapshot();
}
void Processor::environment(const char* name) {
    if(!logs)return;
    auto* data=new juce::DynamicObject;
    data->setProperty("juce_revision",CC_JUCE_REVISION);
    data->setProperty("sample_rate_requested_hz",preparedRate.load());data->setProperty("maximum_buffer_requested_frames",preparedFrames.load());
    data->setProperty("audio_input_channels",getTotalNumInputChannels());data->setProperty("audio_output_channels",getTotalNumOutputChannels());
    data->setProperty("event_input_declared",true);data->setProperty("incoming_midi_policy","ignored");
    event(name,juce::var(data));
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
    data->setProperty("show_seventh",session.settings.showSeventh);data->setProperty("show_sus2",session.settings.showSus2);data->setProperty("show_sus4",session.settings.showSus4);
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
