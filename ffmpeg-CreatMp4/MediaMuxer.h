//
// Created by yfei on 17-8-11.
//

#ifndef FFMPEG_CREATMP4_MEDIAMUXER_H
#define FFMPEG_CREATMP4_MEDIAMUXER_H

#include <iostream>
#include <vector>

#include "fradata.h"
extern  "C"{
#include <libavformat/avformat.h>
};
#define  COLORSIZE 256
typedef  struct
{

    uint8_t *data1= (uint8_t *)malloc(10000000);
    uint8_t *data2= (uint8_t *)malloc(10000000);
    uint8_t *data3= (uint8_t *)malloc(10000000);

}FrameData;


typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
}RGB;

typedef struct{
    AVStream *st;
    int64_t next_pts;
    int samples_count;
    AVFrame *frame;
    AVFrame *tmp_frame;

    float t, tincr, tincr2;
    struct SwsContext *sws_ctx;
    struct SwrContext *swr_ctx;
} OutputStream;

class MediaMuxer {
public:
    std::vector <image> videoframe_in  ;
    const char *output_MP4_file_name;
    double STREAM_FRAME_RATE[2] ;
    double STREAM_DURATION ;
    std::vector <FrameData> VideoFramedata ;
    std::vector <int16_t > temp_voice ;
    int width ;
    int height ;
    int mediamux() ;

private:
    long idx=0 ;
    int encode_video = 1, encode_audio = 1;

    void image2yuv() ;
    void add_stream(OutputStream *ost, AVFormatContext *oc, AVCodec **codec, enum AVCodecID codec_id);
    void Add_audio_video_streams(OutputStream *video_st, OutputStream *audio_st, AVFormatContext *oc,
                                 AVOutputFormat *fmt, AVCodec *audio_codec, AVCodec *video_codec);
    AVFrame * get_video_frame(OutputStream *ost);
    int Write_video_frame(AVFormatContext *oc, OutputStream *ost);
    AVFrame * get_audio_frame(OutputStream *ost);
    int Write_audio_frame(AVFormatContext *oc, OutputStream *ost);

};



#endif //FFMPEG_CREATMP4_MEDIAMUXER_H
