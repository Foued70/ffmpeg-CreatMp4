//
// Created by yfei on 17-8-10.
//

#ifndef FFMPEG_YUV2H264_DISTANCE_H
#define FFMPEG_YUV2H264_DISTANCE_H


typedef  struct{
    double x ;
    double y ;
}point;

class Distance {
public:
    double dist ;
    point test ;
    point A;
    point B;
    double GetDistanace() ;
};


#endif //FFMPEG_YUV2H264_DISTANCE_H
