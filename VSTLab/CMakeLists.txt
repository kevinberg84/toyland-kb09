juce_add_plugin(VSTLab
    COMPANY_NAME "Private Instruments" BUNDLE_ID "local.privateinstruments.vstlab" PLUGIN_MANUFACTURER_CODE PrIv
    PLUGIN_CODE Vl09 PRODUCT_NAME "SH09 VST Lab"
    FORMATS VST3 IS_SYNTH TRUE NEEDS_MIDI_INPUT TRUE NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE COPY_PLUGIN_AFTER_BUILD FALSE VST3_CATEGORIES Instrument Synth)
target_sources(VSTLab PRIVATE Lab.cpp)
target_compile_definitions(VSTLab PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(VSTLab PRIVATE juce::juce_audio_utils juce::juce_recommended_config_flags)
