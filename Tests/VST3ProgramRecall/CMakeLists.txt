cmake_minimum_required(VERSION 3.22)
project(KB09VST3ProgramRecall LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(SDK "" CACHE PATH "Official Steinberg VST3 SDK source checkout")
set(SDK_LIBS "" CACHE PATH "Built SDK Release library directory")
if(NOT WIN32 OR NOT EXISTS "${SDK}/public.sdk/source/vst/hosting/module_win32.cpp")
    message(FATAL_ERROR "This standalone protocol probe requires Windows and an official VST3 SDK checkout.")
endif()
add_executable(KB09VST3ProgramRecall probe.cpp
    "${SDK}/public.sdk/source/vst/hosting/module_win32.cpp"
    "${SDK}/public.sdk/source/vst/hosting/plugprovider.cpp"
    "${SDK}/public.sdk/source/common/memorystream.cpp")
target_include_directories(KB09VST3ProgramRecall PRIVATE "${SDK}")
target_compile_definitions(KB09VST3ProgramRecall PRIVATE RELEASE=1 _CRT_SECURE_NO_WARNINGS)
target_link_libraries(KB09VST3ProgramRecall PRIVATE
    "${SDK_LIBS}/sdk_hosting.lib" "${SDK_LIBS}/sdk_common.lib"
    "${SDK_LIBS}/base.lib" "${SDK_LIBS}/pluginterfaces.lib"
    ole32 shell32 user32 advapi32)
