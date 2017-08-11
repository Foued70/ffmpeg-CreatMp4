#ifndef RGB2YUV_H
#define RGB2YUV_H

#include <iostream>
#include <vector>
#include "libppm/ppm.h"

typedef unsigned char uint8_t;

#define COLORSIZE 256

struct RGB
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

    /**
     * @brief RGB2YUV420
     * @param rgbBufIn   rgb数据输入
     * @param yuvBufOut  yuv数据输出
     * @param nWidth
     * @param nHeight
     */

    void rgb2yuv420 ( ppm &image, uint8_t *yuvBufOut , int nWidth, int nHeight) ;

    void ImageProcessing(ppm &InImage ,ppm &OutImage,double dushu,double s,double x_bias,
                     double y_bias,double Shear[2],unsigned int x_size,unsigned int y_size) ;

#endif // RGB2YUV_H
