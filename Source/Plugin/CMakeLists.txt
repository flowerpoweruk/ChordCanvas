juce_add_plugin(ChordCanvas
    COMPANY_NAME "ChordCanvas"
    BUNDLE_ID "org.chordcanvas.instrument"
    PLUGIN_MANUFACTURER_CODE ChCv
    PLUGIN_CODE Cc01
    FORMATS VST3
    PRODUCT_NAME "ChordCanvas"
    DESCRIPTION "Chord progression canvas"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES Instrument Synth)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/ChordCanvasRelease.h"
    "#pragma once\ninline constexpr const char* chordCanvasRelease = R\"ccmanifest(${CC_RELEASE_JSON})ccmanifest\";\n")
target_sources(ChordCanvas PRIVATE Processor.cpp Editor.cpp)
configure_file(Version.rc.in "${CMAKE_CURRENT_BINARY_DIR}/ChordCanvasVersion.rc" @ONLY)
target_compile_definitions(ChordCanvas PRIVATE
    JUCE_USER_DEFINED_RC_FILE="${CMAKE_CURRENT_BINARY_DIR}/ChordCanvasVersion.rc")
target_include_directories(ChordCanvas PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
target_compile_definitions(ChordCanvas PUBLIC
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_REPORT_APP_USAGE=0
    CC_JUCE_REVISION="${CC_JUCE_REVISION}")
target_link_libraries(ChordCanvas PRIVATE chordcanvas_core juce::juce_audio_processors juce::juce_cryptography
    PUBLIC juce::juce_recommended_config_flags)
