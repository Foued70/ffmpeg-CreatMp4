//
// Created by yfei on 17-8-11.
//
#include "ppm.h"
extern "C"{
#include "inttypes.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavfilter/avfiltergraph.h"
#include "libswscale/swscale.h"

};
#include "Video2FrameRgb.h"

int Video2FrameRgb::readmedia() {
    AVFormatContext *pFormatCtx;
    AVCodecContext *pCodecCtx;
    AVCodec *pCodec;
    AVFrame *pFrame, *pFrameRGB;
    AVPacket *packet;
    uint8_t *out_buffer;

    static struct SwsContext *img_convert_ctx;
    int videoStream, i, numBytes;
    int ret, got_picture;

    av_register_all(); //初始化FFMPEG  调用了这个才能正常适用编码器和解码器

    //Allocate an AVFormatContext.
    pFormatCtx = avformat_alloc_context();

    if (avformat_open_input(&pFormatCtx, input_MP4_file_name, NULL, NULL) != 0) {
        printf("can't open the file. \n");
        return -1;
    }

    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        printf("Could't find stream infomation.\n");
        return -1;
    }

    videoStream = -1;

///循环查找视频中包含的流信息，直到找到视频类型的流
///便将其记录下来 保存到videoStream变量中
///这里我们现在只处理视频流  音频流先不管他
    for (i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
        }
    }

///如果videoStream为-1 说明没有找到视频流
    if (videoStream == -1) {
        printf("Didn't find a video stream.\n");
        return -1;
    }

///查找解码器
    pCodecCtx = pFormatCtx->streams[videoStream]->codec;
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);

    if (pCodec == NULL) {
        printf("Codec not found.\n");
        return -1;
    }

///打开解码器
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        printf("Could not open codec.\n");
        return -1;
    }

    pFrame = av_frame_alloc();
    pFrameRGB = av_frame_alloc();

    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
                                     pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,
                                     AV_PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);

    numBytes = avpicture_get_size(AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);

    out_buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
    avpicture_fill((AVPicture *) pFrameRGB, out_buffer, AV_PIX_FMT_RGB24,
                   pCodecCtx->width, pCodecCtx->height);

    width = pCodecCtx->width;
    height = pCodecCtx->height;

    long memory_size = pCodecCtx->width * pCodecCtx->height;

    packet = (AVPacket *) malloc(sizeof(AVPacket)); //分配一个packet
    av_new_packet(packet, memory_size); //分配packet的数据

    av_dump_format(pFormatCtx, 0, input_MP4_file_name, 0); //输出视频信息
    STREAM_DURATION = pFormatCtx->duration/1000000 ;
    STREAM_FRAME_RATE[0] = pFormatCtx->streams[videoStream]->avg_frame_rate.num;
    STREAM_FRAME_RATE[1] = pFormatCtx->streams[videoStream]->avg_frame_rate.den;

    // 开始读取帧数据
    int index = 0;
    VideoFrameData.clear() ;

    while (1) {
        if (av_read_frame(pFormatCtx, packet) < 0) {
            break; //这里认为视频读取完了
        }

        if (packet->stream_index == videoStream) {
            ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);

            if (ret < 0) {
                printf("decode error.\n");
                return -1;
            }
            frameNum++;
            if (got_picture) {
                sws_scale(img_convert_ctx,
                          (uint8_t const *const *) pFrame->data,
                          pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data,
                          pFrameRGB->linesize);

                uint8_t *yuvBufIn;    //allocate the memory for frame-data
                yuvBufIn = (uint8_t *) malloc(sizeof(uint8_t) * memory_size * 3);
                memcpy(yuvBufIn, pFrameRGB->data[0], memory_size * 3);

                /* add every frame data from  memory to the image structure  */
                imageContext.r.clear();
                imageContext.g.clear();
                imageContext.b.clear();

                long co = 0;
                for (int y = 0; y < height; y++) {
                    for (int x = 0; x < width; x++) {
                        imageContext.r.push_back(yuvBufIn[co++]);
                        imageContext.g.push_back(yuvBufIn[co++]);
                        imageContext.b.push_back(yuvBufIn[co++]);
                    }
                }
                VideoFrameData.push_back(imageContext) ;
                free(yuvBufIn);  // free the memory space in yuvBufIn
                yuvBufIn = NULL;
                index++;

            }
        }

        av_free_packet(packet);    // free the space

    }
    av_free(out_buffer);
    av_free(pFrameRGB);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);
    return 0 ;
}