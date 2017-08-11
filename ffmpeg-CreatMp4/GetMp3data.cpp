#include "GetMp3data.h"

extern "C"{
#include "inttypes.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavfilter/avfiltergraph.h"
};

#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio

int GetMp3data::getaudiodata()
{
    AVFormatContext *ifmt_ctx_a;
    int videoStream ;
    int ret=0;
    /*** the audio format can be .mp3 .m4a .aac  ans so on  **/
    av_register_all();

    //Allocate an AVFormatContext.
    ifmt_ctx_a = avformat_alloc_context();

    if (avformat_open_input(&ifmt_ctx_a, input_MP3_file_name, NULL, NULL) != 0) {
        printf("can't open the file. \n");
        return -1;
    }

    if (avformat_find_stream_info(ifmt_ctx_a, NULL) < 0) {
        printf("Could't find stream infomation.\n");
        return -1;
    }

    videoStream = -1;

    ///循环查找视频中包含的流信息，直到找到音频类型的流
    ///便将其记录下来 保存到voiceStream变量中
    ///这里我们现在只处音频流  视频流先不管
    for (int i = 0; i < ifmt_ctx_a->nb_streams; i++) {
        //  if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
        if (ifmt_ctx_a->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            videoStream = i;
        }
    }

    ///如果videoStream为-1 说明没有找到音频流
    if (videoStream == -1) {
        printf("Didn't find a video stream.\n");
        return -1;
    }

    AVCodecContext *pCodecCtx;
    AVCodec *pCodec = NULL ;
    ///查找解码器
    pCodecCtx = ifmt_ctx_a->streams[videoStream]->codec;
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

    AVSampleFormat sfmt = pCodecCtx->sample_fmt;
    AVPacket packet;

    av_init_packet(&packet);

    AVFrame *frame = av_frame_alloc();

    int buffer_size = AVCODEC_MAX_AUDIO_FRAME_SIZE+ FF_INPUT_BUFFER_PADDING_SIZE;
    uint8_t buffer[buffer_size];

    packet.data=buffer;
    packet.size =buffer_size;

    int len;
    int frameFinished=0;

    while(av_read_frame(ifmt_ctx_a,&packet) >= 0)
    {
        if(packet.stream_index==videoStream)
        {
            //printf("Audio Frame read \n");
            int len=avcodec_decode_audio4(pCodecCtx, frame, &frameFinished, &packet);
            if(frameFinished)
            {
                if (sfmt==AV_SAMPLE_FMT_S16P)
                { // Audacity: 16bit PCM little endian stereo
                    int16_t* ptr_l = (int16_t*)frame->extended_data[0];
                    int16_t* ptr_r = (int16_t*)frame->extended_data[1];
                    for (int i=0; i<frame->nb_samples; i++)
                    {
                        temp_voice.push_back(*ptr_l++);
                        //    temp_voice.push_back(*ptr_r++);
                    }
                }
                else if (sfmt==AV_SAMPLE_FMT_FLTP)
                { //Audacity: big endian 32bit stereo start offset 7 (but has noise)
                    float* ptr_l = (float*)frame->extended_data[0];
                    float* ptr_r = (float*)frame->extended_data[1];
                    for (int i=0; i<frame->nb_samples; i++)
                    {
                        temp_voice.push_back( (int16_t)*ptr_l++);
                        //  temp_voice.push_back( (int16_t)*ptr_r++);

                    }
                }
            }
        }
    }
    avcodec_close(pCodecCtx);
    avformat_close_input(&ifmt_ctx_a);
    return  0 ;
}