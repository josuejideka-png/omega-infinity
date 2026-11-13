cmake_minimum_required(VERSION 3.22)
project(OMEGA_INFINITY VERSION 1.1.0)

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.4
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(OMEGA_INFINITY
    COMPANY_NAME "OMEGA STUDIO PRODUCTION"
    BUNDLE_ID com.omegastudio.omegainfinity
    PLUGIN_MANUFACTURER_CODE Omeg
    PLUGIN_CODE Oinf
    FORMATS VST3 Standalone
    PRODUCT_NAME "OMEGA INFINITY"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE)

juce_generate_juce_header(OMEGA_INFINITY)
target_sources(OMEGA_INFINITY PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)
target_compile_features(OMEGA_INFINITY PRIVATE cxx_std_17)
target_compile_definitions(OMEGA_INFINITY PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(OMEGA_INFINITY
    PRIVATE juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags
           juce::juce_recommended_lto_flags
           juce::juce_recommended_warning_flags)
