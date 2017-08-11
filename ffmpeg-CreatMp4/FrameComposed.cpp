//
// Created by yfei on 17-8-11.
//

#include "FrameComposed.h"

void FrameComposed::addimage() {

    videoframegoal.clear() ;

    for(int j=0;j<videoframe1.size();j++){
        image imagegoal ;
        imagegoal.r.clear();
        imagegoal.g.clear();
        imagegoal.b.clear();
        for (int i = 0; i < videoframe1[0].r.size(); i++) {
            double r = videoframe2[j].r[i] * (255 - videoframemask[j].r[i]) / 255 +
                       videoframe1[j].r[i] * videoframemask[j].r[i] / 255;
            imagegoal.r.push_back(r > 255 ? 255 : r);

            double g = videoframe2[j].g[i] * (255 - videoframemask[j].g[i]) / 255 +
                       videoframe1[j].g[i] * videoframemask[j].g[i] / 255;
            imagegoal.g.push_back(g > 255 ? 255 : g);

            double b = videoframe2[j].b[i] * (255 - videoframemask[j].b[i]) / 255 +
                       videoframe1[j].b[i] * videoframemask[j].b[i] / 255;
            imagegoal.b.push_back(b > 255 ? 255 : b);
        }
        videoframegoal.push_back(imagegoal);

    }
}