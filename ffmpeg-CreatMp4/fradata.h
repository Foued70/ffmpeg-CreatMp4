//
// Created by yfei on 17-8-2.
//

#ifndef FFMPEG_YUV2H264_FRADATA_H
#define FFMPEG_YUV2H264_FRADATA_H

#include "iostream"
#include "vector"
typedef struct {
public:
    //arrays for storing the R,G,B values
    std::vector<uint8_t > r;
    std::vector<uint8_t > g;
    std::vector<uint8_t > b;
}image;

#endif //FFMPEG_YUV2H264_FRADATA_H
