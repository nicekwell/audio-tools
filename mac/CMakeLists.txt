cmake_minimum_required(VERSION 2.8)
include_directories(../android/jni/libaudio_tools/include)
link_directories(${MAKE_CURRENT_SOURCE_DIR}/)
link_libraries(samplerate)

set(SRC main.c
        ../android/jni/libaudio_tools/audio_tools.c 
        ../android/jni/libaudio_tools/queue.c 
        ../android/jni/libaudio_tools/resample.c )

add_executable(test ${SRC})



