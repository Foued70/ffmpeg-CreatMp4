//
// Created by yfei on 17-8-11.
//

#ifndef FFMPEG_CREATMP4_VIDEO2FRAMERGB_H
#define FFMPEG_CREATMP4_VIDEO2FRAMERGB_H

#include "iostream"
#include "vector"
#include "fradata.h"

class Video2FrameRgb {
public:
    const char *input_MP4_file_name;
    std::vector <image> VideoFrameData ;
    int width;
    int height;
    int frameNum=0;
    double STREAM_DURATION ;
    double STREAM_FRAME_RATE[2];
    int readmedia() ;

private:
    image imageContext  ;
};


#endif //FFMPEG_CREATMP4_VIDEO2FRAMERGB_H
