//
// Created by yfei on 17-8-11.
//

#ifndef FFMPEG_CREATMP4_FRAMECOMPOSED_H
#define FFMPEG_CREATMP4_FRAMECOMPOSED_H

#include "fradata.h"
#include "vector"
class FrameComposed {

public:
    std::vector<image> videoframe1;
    std::vector<image> videoframe2;
    std::vector<image> videoframemask;
    std::vector<image> videoframegoal;
    void addimage();

};

#endif //FFMPEG_CREATMP4_FRAMECOMPOSED_H
