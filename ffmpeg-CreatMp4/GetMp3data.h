//
// Created by yfei on 17-8-11.
//

#ifndef FFMPEG_CREATMP4_GETMP3DATA_H
#define FFMPEG_CREATMP4_GETMP3DATA_H

#include <iostream>
#include <vector>
class GetMp3data {
public:
    const char *input_MP3_file_name;
    std::vector <int16_t > temp_voice ;
    int getaudiodata() ;
};

#endif //FFMPEG_CREATMP4_GETMP3DATA_H
