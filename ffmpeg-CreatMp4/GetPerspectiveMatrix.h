//
// Created by yfei on 17-8-11.
//

#ifndef FFMPEG_CREATMP4_GETPERSPECTIVEMATRIX_H
#define FFMPEG_CREATMP4_GETPERSPECTIVEMATRIX_H

#include <Dense>   //Call the Matrix Lib
using namespace Eigen;

class GetPerspectiveMatrix {
public:
    double x[4]={0,0,0,0};
    double y[4]={0,0,0,0};
    double u[4]={0,0,0,0};
    double v[4]={0,0,0,0};

    Matrix<double,3,3> PerspectiveMatrix(); //@return PerspectiveMatrix

private:
    Matrix<double , 8,8> Matr;   // 8x8线性方程组系数矩阵
    Matrix<double , 3,3> Juzhen ;  // the goal:PerspectiveMatrix
};

#endif //FFMPEG_CREATMP4_GETPERSPECTIVEMATRIX_H
