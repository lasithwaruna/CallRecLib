cmake_minimum_required(VERSION 3.4.1)

add_library(CallRecFix SHARED
            audio.h
            Audio.cpp
            CallRecFix.cpp
            )

# Include libraries needed for callrecfix lib
target_link_libraries(CallRecFix
                      android
                      log)
