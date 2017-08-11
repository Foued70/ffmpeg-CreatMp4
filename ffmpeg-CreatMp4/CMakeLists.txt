cmake_minimum_required(VERSION 3.5)
project(ffmpeg_CreatMp4)

set(CMAKE_CXX_STANDARD 11)
include_directories(${PROJECT_SOURCE_DIR}/lib/include/)
include_directories(${PROJECT_SOURCE_DIR}/Eigen)
link_directories(/${PROJECT_SOURCE_DIR}/lib/lib/)

set(SOURCE_FILES main.cpp
        Video2FrameRgb.cpp Video2FrameRgb.h
        lib_json/json_reader.cpp
        lib_json/json_value.cpp
        lib_json/json_writer.cpp
        ppm.cpp ppm.h
        Distance.cpp Distance.h
        Jud.cpp
        GetPerspectiveMatrix.cpp GetPerspectiveMatrix.h InputParameter.cpp InputParameter.h RenderMedia.cpp RenderMedia.h FrameComposed.cpp FrameComposed.h GetMp3data.cpp GetMp3data.h MediaMuxer.cpp MediaMuxer.h)
add_executable(ffmpeg_CreatMp4 ${SOURCE_FILES})
target_link_libraries(ffmpeg_CreatMp4 libavformat.a libavcodec.a libswscale.a libavcodec.a
        libswresample.a libavutil.a pthread c -lbz2 x264 mp3lame
        -ldl -lmp3lame -lm -llzma -lbz2 -lz -pthread fdk-aac )