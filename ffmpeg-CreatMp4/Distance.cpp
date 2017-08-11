//
// Created by yfei on 17-8-10.
//
#include <math.h>
#include "Distance.h"

double Distance::GetDistanace() {

    double d_ab = sqrt(pow((A.x-B.x),2.0)+pow((A.y-B.y),2.0));
    double d_tb = sqrt(pow((test.x-B.x),2.0)+pow((test.y-B.y),2.0));
    double d_ta = sqrt(pow((test.x-A.x),2.0)+pow((test.y-A.y),2.0));

    double cos_a =(pow(d_ta,2.0)+pow(d_ab,2.0)-pow(d_tb,2.0))/(2*d_ab*d_ta);
    double sin_a = sqrt(1-pow(cos_a,2.0));
    dist = sin_a * d_ta ;
    return  dist ;
}
