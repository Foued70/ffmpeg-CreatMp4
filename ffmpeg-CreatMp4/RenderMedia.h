//
// Created by yfei on 17-8-11.
//

#ifndef FFMPEG_CREATMP4_RENDERMEDIA_H
#define FFMPEG_CREATMP4_RENDERMEDIA_H

#include <iostream>
#include "ppm.h"
#include "fradata.h"
#include <Dense>   //Call the Matrix Lib
using namespace Eigen;
using namespace std;
typedef struct{

    unsigned int FrameId ;
    bool change ;
    double widthgoal;
    double heightgoal;
    double scale[2];
    double zoom[2]={1,1};
    double XYbias[2];
    unsigned int  XYsize[2];
    Matrix<double,3,3> PerspectiveMatrix;
}OneFrameParam;

class RenderMedia {

public:
    const char* input_renderMp4_file_name ;
    int width ;
    int height ;
    int outwidth ;
    int outheight ;
    unsigned int frameNum ;
   // int STREAM_FRAME_RATE[2] ;
   // double STREAM_DURATION ;
    vector <OneFrameParam> VideoInputParam ;
    vector < image > VideoFramedata ;
    int render();
private:
    void ImagePerspective(ppm &InImage, ppm &OutImage,double zoom[2],
                          double XYbias[2],Matrix<double,3,3> PerspectiveMatrix);
};

#endif //FFMPEG_CREATMP4_RENDERMEDIA_H
