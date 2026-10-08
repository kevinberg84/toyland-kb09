cmake_minimum_required(VERSION 3.22)
project(SH09 VERSION 0.13.14 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(JUCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../JUCE" CACHE PATH "JUCE source directory")
add_subdirectory("${JUCE_DIR}" JUCE)

# AU is a native macOS target. AAX is opt-in because building its binary does
# not supply the AAX digital signature required by normal Pro Tools releases.
option(KB09_BUILD_AU "Build the macOS Audio Unit" ${APPLE})
option(KB09_BUILD_AAX "Build an AAX Native development binary" OFF)
set(KB09_AAX_SDK_ROOT "${JUCE_DIR}/modules/juce_audio_plugin_client/AAX/SDK"
    CACHE PATH "AAX SDK root (Interfaces, Libs and Utilities directories)")
set(KB09_PLUGIN_FORMATS VST3 Standalone)
if(KB09_BUILD_AU)
    if(NOT CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        message(FATAL_ERROR "KB09_BUILD_AU requires a native macOS build environment.")
    endif()
    list(APPEND KB09_PLUGIN_FORMATS AU)
endif()
if(KB09_BUILD_AAX)
    if(NOT (CMAKE_SYSTEM_NAME STREQUAL "Windows" OR CMAKE_SYSTEM_NAME STREQUAL "Darwin"))
        message(FATAL_ERROR "KB09_BUILD_AAX is supported on Windows and macOS only.")
    endif()
    if(NOT EXISTS "${KB09_AAX_SDK_ROOT}/Interfaces/ACF"
       OR NOT EXISTS "${KB09_AAX_SDK_ROOT}/Libs/AAXLibrary/source"
       OR NOT EXISTS "${KB09_AAX_SDK_ROOT}/Utilities/PlugIn.ico")
        message(FATAL_ERROR "KB09_AAX_SDK_ROOT must identify a complete JUCE-compatible AAX SDK.")
    endif()
    juce_set_aax_sdk_path("${KB09_AAX_SDK_ROOT}")
    list(APPEND KB09_PLUGIN_FORMATS AAX)
    message(STATUS "AAX Native enabled; host loading/signing must be verified separately.")
endif()
message(STATUS "KB-09 plug-in formats: ${KB09_PLUGIN_FORMATS}")

enable_testing()
add_subdirectory(VSTLab)
juce_add_console_app(SH09Host PRODUCT_NAME "SH09 Validation Host")
target_sources(SH09Host PRIVATE Tests/Host.cpp)
target_compile_definitions(SH09Host PRIVATE JUCE_PLUGINHOST_VST3=1 JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
target_link_libraries(SH09Host PRIVATE juce::juce_audio_utils juce::juce_recommended_config_flags)
juce_add_plugin(SH09
    COMPANY_NAME "Toyland" BUNDLE_ID "local.privateinstruments.sh09"
    ICON_BIG "Assets/Icons/KB-09.png" ICON_SMALL "Assets/Icons/KB-09.png"
    PLUGIN_MANUFACTURER_CODE PrIv PLUGIN_CODE Sh09 PRODUCT_NAME "Toyland KB-09"
    FORMATS ${KB09_PLUGIN_FORMATS} IS_SYNTH TRUE NEEDS_MIDI_INPUT TRUE NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE COPY_PLUGIN_AFTER_BUILD FALSE VST3_CATEGORIES Instrument Synth
    AU_MAIN_TYPE kAudioUnitType_MusicDevice AAX_CATEGORY SWGenerators
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE)

# JUCE 9's rcfile rule omits Info.txt from its dependencies. Without this,
# an incremental version change leaves stale Explorer file-version metadata.
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    get_target_property(kb09_generated_sources SH09 JUCE_GENERATED_SOURCES_DIRECTORY)
    get_target_property(kb09_info_file SH09 JUCE_INFO_FILE)
    add_custom_command(OUTPUT "${kb09_generated_sources}/SH09_resources.rc" APPEND
        DEPENDS "${kb09_info_file}")
endif()
juce_add_binary_data(KB09Artwork SOURCES Assets/Photoreal/chassis-black.png Assets/Photoreal/chassis-personal-aged.png Assets/Photoreal/chassis-stains.png Assets/Photoreal/knob-personal-overhead-v5.png Assets/Photoreal/fader-personal-overhead.png Assets/Photoreal/front-rail-grain.png Assets/Photoreal/keys.png Assets/Photoreal/marks.png Assets/Photoreal/marks-aged.png)
target_sources(SH09 PRIVATE Source/Plugin/Processor.cpp Source/GUI/Editor.cpp Source/GUI/WindowSizing.cpp Source/GUI/PresetFiles.cpp Source/DSP/Engine.cpp)
target_compile_definitions(SH09 PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(SH09 PRIVATE KB09Artwork juce::juce_audio_utils juce::juce_recommended_config_flags)
add_executable(SH09DSPTests Tests/DSPTests.cpp Source/DSP/Engine.cpp)
target_compile_features(SH09DSPTests PRIVATE cxx_std_20)
add_test(NAME SH09DSP COMMAND SH09DSPTests)
add_executable(SH09Render Tests/Render.cpp Source/DSP/Engine.cpp)
target_compile_features(SH09Render PRIVATE cxx_std_20)
juce_add_console_app(SH09ProcessorTests PRODUCT_NAME "SH09 Processor Tests")
target_sources(SH09ProcessorTests PRIVATE Tests/ProcessorTests.cpp)
target_link_libraries(SH09ProcessorTests PRIVATE SH09 juce::juce_audio_utils juce::juce_recommended_config_flags)
add_test(NAME SH09Processor COMMAND SH09ProcessorTests)
juce_add_console_app(SH09PresetFilesTests PRODUCT_NAME "KB09 Preset File Tests")
target_sources(SH09PresetFilesTests PRIVATE Tests/PresetFilesTests.cpp)
target_link_libraries(SH09PresetFilesTests PRIVATE SH09 juce::juce_audio_utils juce::juce_recommended_config_flags)
add_test(NAME SH09PresetFiles COMMAND SH09PresetFilesTests)
juce_add_console_app(SH09GUIRender PRODUCT_NAME "Toyland KB-09 GUI Render")
target_sources(SH09GUIRender PRIVATE Tests/GUIRender.cpp)
target_compile_definitions(SH09GUIRender PRIVATE JUCE_MODAL_LOOPS_PERMITTED=1)
target_link_libraries(SH09GUIRender PRIVATE SH09 KB09Artwork juce::juce_audio_utils juce::juce_recommended_config_flags)

juce_add_console_app(KB09UpdateTests PRODUCT_NAME "KB09 Update Tests")
target_sources(KB09UpdateTests PRIVATE Tests/UpdateTests.cpp)
target_link_libraries(KB09UpdateTests PRIVATE SH09 juce::juce_audio_utils juce::juce_recommended_config_flags)
add_test(NAME KB09Updates COMMAND KB09UpdateTests)
