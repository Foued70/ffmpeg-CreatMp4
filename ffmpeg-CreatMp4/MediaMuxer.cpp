//
// Created by yfei on 17-8-11.
//
#include "MediaMuxer.h"
#include "ppm.h"
extern "C"{
#include "inttypes.h"
#include <libavformat/avformat.h>
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavfilter/avfiltergraph.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "libavutil/avutil.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/avassert.h"
#include "libavutil/channel_layout.h"
#include "libavutil/opt.h"
#include "libavutil/mathematics.h"
#include "libavutil/timestamp.h"

};

void rgb2yuv(ppm &image, unsigned char *yuvBufOut, int nWidth, int nHeight) ;

#define errReport(info, val) do{\
	fprintf(stderr, "ERR(func=%s,line=%d): %s code=%d\n",__FUNCTION__, __LINE__, info, val);\
	exit(0);\
}while (0);

void MediaMuxer::add_stream(OutputStream *ost, AVFormatContext *oc, AVCodec **codec, enum AVCodecID codec_id){
    AVCodecContext *c;
    int i;
    *codec = avcodec_find_encoder(codec_id);
    if (!(*codec)) errReport("avcodec_find_encoder", -1);

    ost->st = avformat_new_stream(oc, *codec);
    if (!ost->st)  errReport("avformat_new_stream", -1);

    ost->st->id = oc->nb_streams - 1;
    c = ost->st->codec;
    switch ((*codec)->type){
        case AVMEDIA_TYPE_AUDIO:
            c->sample_fmt = AV_SAMPLE_FMT_FLTP;
            // c->sample_fmt = AV_SAMPLE_FMT_S16P;
            //(*codec)->sample_fmts ? (*codec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP ;
            c->bit_rate = 64000;
            c->sample_rate = 44100;
            if ((*codec)->supported_samplerates){
                c->sample_rate = (*codec)->supported_samplerates[0];
                for (i = 0; (*codec)->supported_samplerates[i]; i++){
                    if ((*codec)->supported_samplerates[i] == 44100)
                        c->sample_rate = 44100;
                }
            }
            c->channels = av_get_channel_layout_nb_channels(c->channel_layout);
            c->channel_layout = AV_CH_LAYOUT_STEREO;
            if ((*codec)->channel_layouts){
                c->channel_layout = (*codec)->channel_layouts[0];
                for (i = 0; (*codec)->channel_layouts[i]; i++){
                    if ((*codec)->channel_layouts[i] == AV_CH_LAYOUT_STEREO)
                        c->channel_layout = AV_CH_LAYOUT_STEREO;
                }
            }
            c->channels = av_get_channel_layout_nb_channels(c->channel_layout);{
        AVRational r = { 1, c->sample_rate };
        ost->st->time_base = r;
    }
            break;
        case AVMEDIA_TYPE_VIDEO:

            c->qmin = 10;
            c->qmax = 51;

            c->codec_id = codec_id;
            c->bit_rate =400000;
            c->width = width;
            c->height = height;
            {
                AVRational r = {STREAM_FRAME_RATE[1], STREAM_FRAME_RATE[0]};
                ost->st->time_base = r;
            }

            c->time_base = ost->st->time_base;
            c->gop_size = 250;
            c->pix_fmt = AV_PIX_FMT_YUV420P;
            if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO)	c->max_b_frames = 2;
            if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO)	c->mb_decision = 2;
            break;
        default:
            break;
    }
    // Some formats want stream headers to be separate. //
    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
}

void MediaMuxer::Add_audio_video_streams(OutputStream *video_st, OutputStream *audio_st, AVFormatContext *oc,
                                       AVOutputFormat *fmt, AVCodec *audio_codec, AVCodec *video_codec){

    if (fmt->video_codec != AV_CODEC_ID_NONE){
        add_stream(video_st, oc, &video_codec, fmt->video_codec);
        video_st->st->codec->width = width;
        video_st->st->codec->height = height;

    }
    if (fmt->audio_codec != AV_CODEC_ID_NONE){
        add_stream(audio_st, oc, &audio_codec, fmt->audio_codec);
    }

}



AVFrame *alloc_picture(enum AVPixelFormat pix_fmt, int width, int height){
    AVFrame *picture;
    int ret;
    picture = av_frame_alloc();
    if (!picture)	return NULL;

    picture->format = pix_fmt;
    picture->width = width;
    picture->height = height;
    ret = av_frame_get_buffer(picture, 32);
    if (ret < 0)	errReport("av_frame_get_buffer", ret);
    return picture;
}

AVFrame * MediaMuxer::get_video_frame(OutputStream *ost){
    AVCodecContext *c = ost->st->codec;
    AVRational r = { 1, 1 };
    if (av_compare_ts(ost->next_pts, ost->st->codec->time_base, STREAM_DURATION, r) >= 0)
        return NULL;
    int ret = av_frame_make_writable(ost->frame);
    if (ret < 0)	errReport("av_frame_make_writable", ret);
    int i = ost->next_pts;
    if(i>=VideoFramedata.size()){
        ost->frame->data[0] = VideoFramedata[VideoFramedata.size()-1].data1;
        ost->frame->data[1] = VideoFramedata[VideoFramedata.size()-1].data2;
        ost->frame->data[2] = VideoFramedata[VideoFramedata.size()-1].data3;
    }else {
        ost->frame->data[0] = VideoFramedata[i].data1;
        ost->frame->data[1] = VideoFramedata[i].data2;
        ost->frame->data[2] = VideoFramedata[i].data3;
    }
    ost->frame->pts = ost->next_pts++;
    return ost->frame;
}

int write_frame(AVFormatContext *fmt_ctx, const AVRational *time_base, AVStream *st, AVPacket *pkt){
    av_packet_rescale_ts(pkt, *time_base, st->time_base);
    pkt->stream_index = st->index;
    //	log_packet(fmt_ctx, pkt);
    return av_interleaved_write_frame(fmt_ctx, pkt);
}

int MediaMuxer::Write_video_frame(AVFormatContext *oc, OutputStream *ost){
    int ret, got_packet = 0;
    AVCodecContext *c;
    AVFrame *frame;
    AVPacket pkt = { 0 };
    c = ost->st->codec;
    frame = get_video_frame(ost);
    av_init_packet(&pkt);
    ret = avcodec_encode_video2(c, &pkt, frame, &got_packet);
    if (ret < 0)	errReport("avcodec_encode_video2", ret);
    if (got_packet)
        ret = write_frame(oc, &c->time_base, ost->st, &pkt);  //写入帧数据
    else
        ret = 0;
    if (ret < 0)  errReport("write_frame", ret);
    return (frame || got_packet ) ? 0 : 1;
}



void Open_video(AVFormatContext *oc, AVCodec *codec, OutputStream *ost, AVDictionary *opt_arg){
    int ret;
    AVCodecContext *c = ost->st->codec;
    AVDictionary *opt = NULL;
    av_dict_copy(&opt, opt_arg, 0);
    ret = avcodec_open2(c, codec, &opt);
    if (ret < 0)	errReport("avcodec_open2", ret);

    av_dict_free(&opt);
    ost->frame = alloc_picture(c->pix_fmt, c->width, c->height);
    if (!ost->frame)	errReport("alloc_picture", -1);

}


int Open_coder_muxer(AVOutputFormat **fmt, AVFormatContext **oc, const char *filename){
    av_register_all();
    avformat_alloc_output_context2(oc, NULL, NULL, filename);
    if (!oc){
        printf("Could not deduce output format from file extension: using MPEG.\n");
        avformat_alloc_output_context2(oc, NULL, "mpeg", filename);
        if (!oc)	return -1;
    }
    *fmt = (*oc)->oformat;
    return 0;
}

AVFrame *alloc_audio_frame(enum AVSampleFormat sample_fmt, uint64_t channel_layout, int sample_rate, int nb_samples){
    AVFrame *frame = av_frame_alloc();
    if (!frame)	errReport("av_frame_alloc", -1);
    frame->format = sample_fmt;
    frame->channel_layout = channel_layout;
    frame->sample_rate = sample_rate;
    frame->nb_samples = nb_samples;
    if (nb_samples){
        int ret = av_frame_get_buffer(frame, 0);
        if (ret < 0)	errReport("av_frame_get_buffer", ret);
    }
    return frame;
}

void Open_audio(AVFormatContext *oc, AVCodec *codec, OutputStream *ost, AVDictionary *opt_arg){
    AVCodecContext *c;
    int nb_samples, ret;
    AVDictionary *opt = NULL;
    c = ost->st->codec;
    av_dict_copy(&opt, opt_arg, 0);
    ret = avcodec_open2(c, codec, &opt);
    av_dict_free(&opt);
    if (ret < 0)  errReport("avcodec_open2", ret);
    // init signal generator
    ost->t = 0;
    ost->tincr = 2 * M_PI * 110.0 / c->sample_rate;
    // increment frequency by 110 Hz per second
    ost->tincr2 = 2 * M_PI * 110.0 / c->sample_rate / c->sample_rate;
    if (c->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE)
        nb_samples = 10000;
    else
        nb_samples = c->frame_size;
    ost->frame = alloc_audio_frame(c->sample_fmt, c->channel_layout, c->sample_rate, nb_samples);
    ost->tmp_frame = alloc_audio_frame(AV_SAMPLE_FMT_S16, c->channel_layout, c->sample_rate, nb_samples);
    // create resampler context
    ost->swr_ctx = swr_alloc();
    if (!ost->swr_ctx)  errReport("swr_alloc", ret);
    // set options
    av_opt_set_int(ost->swr_ctx, "in_channel_count", c->channels, 0);
    av_opt_set_int(ost->swr_ctx, "in_sample_rate", c->sample_rate, 0);
    av_opt_set_sample_fmt(ost->swr_ctx, "in_sample_fmt", AV_SAMPLE_FMT_S16, 0);
    av_opt_set_int(ost->swr_ctx, "out_channel_count", c->channels, 0);
    av_opt_set_int(ost->swr_ctx, "out_sample_rate", c->sample_rate, 0);
    av_opt_set_sample_fmt(ost->swr_ctx, "out_sample_fmt", c->sample_fmt, 0);
    // initialize the resampling context
    if ((ret = swr_init(ost->swr_ctx)) < 0)   errReport("swr_init", ret);
}


// audio file

AVFrame * MediaMuxer::get_audio_frame(OutputStream *ost){
    AVFrame *frame = ost->tmp_frame;
    int j, i, v;
    int16_t *q = (int16_t*)frame->data[0];
    AVRational r = { 1, 1 };
    // check if we want to generate more frames
    if (av_compare_ts(ost->next_pts, ost->st->codec->time_base, STREAM_DURATION, r) >= 0)
        return NULL;
    for (j = 0; j < frame->nb_samples; j++){
        //v = (int)(sin(ost->t) * 10000)+10000;
        v = temp_voice[idx++];
        for (i = 0; i < ost->st->codec->channels; i++)
            *q++ = v;
        ost->t += ost->tincr;
        ost->tincr += ost->tincr2;
    }
    frame->pts = ost->next_pts;
    ost->next_pts += frame->nb_samples;
    return frame;
}

int MediaMuxer::Write_audio_frame(AVFormatContext *oc, OutputStream *ost){
    AVCodecContext *c;
    AVPacket pkt = { 0 }; // data and size must be 0;
    AVFrame *frame;
    int ret, got_packet, dst_nb_samples;
    av_init_packet(&pkt);
    c = ost->st->codec;
    frame = get_audio_frame(ost);
    if (frame){
        dst_nb_samples = av_rescale_rnd(swr_get_delay(ost->swr_ctx, c->sample_rate) + frame->nb_samples, c->sample_rate, c->sample_rate, AV_ROUND_UP);
        av_assert0(dst_nb_samples == frame->nb_samples);
        ret = av_frame_make_writable(ost->frame);
        if (ret < 0) errReport("av_frame_make_writable", ret);
        ret = swr_convert(ost->swr_ctx,ost->frame->data, dst_nb_samples,
                          (const uint8_t **)frame->data, frame->nb_samples);
        if (ret < 0)	errReport("swr_convert", ret);
        frame = ost->frame;
        AVRational r = { 1, c->sample_rate };
        frame->pts = av_rescale_q(ost->samples_count, r, c->time_base);
        ost->samples_count += dst_nb_samples;
    }
    ret = avcodec_encode_audio2(c, &pkt, frame, &got_packet);
    if (ret < 0)  errReport("avcodec_encode_audio2", ret);
    if (got_packet){
        ret = write_frame(oc, &c->time_base, ost->st, &pkt);
        if (ret < 0)  errReport("write_frame", ret);
    }
    return (frame || got_packet) ? 0 : 1;
}

void Close_stream(AVFormatContext *oc, OutputStream *ost){
    avcodec_close(ost->st->codec);
    av_frame_free(&ost->frame);
    av_frame_free(&ost->tmp_frame);
    sws_freeContext(ost->sws_ctx);
    swr_free(&ost->swr_ctx);
}

int MediaMuxer::mediamux() {

    image2yuv() ;

    AVOutputFormat *fmt;
    AVFormatContext *oc;
    OutputStream video_st = { 0 }, audio_st = { 0 };
    AVCodec *audio_codec = NULL, *video_codec = NULL;
    AVDictionary *opt = NULL;

    int videoFrameIdx=0, audioFrameIdx=0 ;

    av_register_all();
    avformat_alloc_output_context2(&oc, NULL, NULL, output_MP4_file_name);
    if (!&oc){
        printf("Could not deduce output format from file extension: using MPEG.\n");
        avformat_alloc_output_context2(&oc, NULL, "mpeg", output_MP4_file_name);
        if (!&oc)	return -1;
    }
    fmt = oc->oformat;
    Add_audio_video_streams(&video_st, &audio_st, oc, fmt, audio_codec, video_codec);
    Open_video(oc, video_codec, &video_st, opt);
    Open_audio(oc, audio_codec, &audio_st, opt);
    av_dump_format(oc, 0, output_MP4_file_name, 1);  //打印输出MP4格式信息
    if (!(fmt->flags & AVFMT_NOFILE)){
        int ret = avio_open(&oc->pb, output_MP4_file_name, AVIO_FLAG_WRITE);
        if (ret < 0)	errReport("avio_open", ret);
    }
    int ret = avformat_write_header(oc, &opt);   // 写输出MP4文件头
    if (ret < 0)  errReport("avformat_write_header", ret);

    //写文件内容
    while (encode_video || encode_audio) {
        // 时间戳比较，判断下一帧写入视频帧或音频帧
        if (encode_video && (!encode_audio || av_compare_ts(video_st.next_pts,
                                                            video_st.st->codec->time_base, audio_st.next_pts, audio_st.st->codec->time_base) <= 0)){
            // 写入视频帧数据
            encode_video = !Write_video_frame(oc, &video_st);
            if (videoFrameIdx % 3 == 0) printf("\n");
            if (encode_video)
                printf("v:%d ", videoFrameIdx++);
            else
                printf("Video ended, exit.\n");

        }
        else {
            // 写入音频帧数据
            encode_audio = !Write_audio_frame(oc, &audio_st);
            if (encode_audio)
                printf("a:%d ", audioFrameIdx++);
            else
                printf("Audio ended, exit.\n");
        }
    }

    av_write_trailer(oc); //写文件尾
    Close_stream(oc, &video_st);   // 关闭音视频流
    Close_stream(oc, &audio_st);

    if (!(fmt->flags & AVFMT_NOFILE))  avio_closep(&oc->pb);
    avformat_free_context(oc);
    printf("Procssing end.\n");

    return 0;

}

void MediaMuxer::image2yuv() {

    VideoFramedata.clear() ;

    for(int j=0;j<videoframe_in.size();j++){
        ppm image_s(width,height) ;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                image_s.r.push_back(videoframe_in[j].r[y*width+x]);
                image_s.g.push_back(videoframe_in[j].g[y*width+x]);
                image_s.b.push_back(videoframe_in[j].b[y*width+x]);

            }

        }
        //    image_s.write("wode.ppm") ;
        long numsize = width * height;
        uint8_t *yuvBufOut_temp;
        yuvBufOut_temp = (uint8_t *) malloc(
                sizeof(uint8_t) * numsize * 1.5);  // apply for a space for the yuv data

        rgb2yuv(image_s, yuvBufOut_temp, width, height);

        FrameData datatemp;
        memcpy(datatemp.data1, yuvBufOut_temp, numsize);
        memcpy(datatemp.data2, yuvBufOut_temp + numsize, numsize / 4);
        memcpy(datatemp.data3, yuvBufOut_temp + numsize * 5 / 4, numsize / 4);
        free(yuvBufOut_temp);
        yuvBufOut_temp = NULL ;
        VideoFramedata.push_back(datatemp); // add the frame data for muxer ;
    }

}

void rgb2yuv(ppm &image, unsigned char *yuvBufOut, int nWidth, int nHeight) {
    int pix = 0;
    int pixP4;

    int IMGSIZE = nWidth * nHeight;

    //表的初始化
    unsigned short Y_R[COLORSIZE],Y_G[COLORSIZE],Y_B[COLORSIZE],U_R[COLORSIZE],U_G[COLORSIZE],U_B[COLORSIZE],V_G[COLORSIZE],V_B[COLORSIZE]; //查表数组V_R[COLORSIZE]

    for(int i = 0; i < COLORSIZE; i++)
    {
        Y_R[i] = (i * 1224) >> 12; //Y对应的查表数组0.2988
        Y_G[i] = (i * 2404) >> 12;  //0.5869
        Y_B[i] = (i * 469)  >> 12; //0.1162
        U_R[i] = (i * 692)  >> 12; //U对应的查表数组0.1688
        U_G[i] = (i * 1356) >> 12;  //0.3312
        U_B[i] = i /*(* 2048) */>> 1; //0.5
//   V_R[i] = (i * 2048) >> 12; ///V对应的查表数组
        V_G[i] = (i * 1731) >> 12;  //0.4184
        V_B[i] = (i * 334)  >> 12; //0.0816
    }

    for(int y = 0; y < nHeight; y++) //line
    {
        for(int x=0;x < nWidth;x++)//pixf
        {
            RGB rgbByte ;
            rgbByte.r = image.r[pix] ;
            rgbByte.g = image.g[pix] ;
            rgbByte.b = image.b[pix] ;

            int     i = Y_R[rgbByte.r] + Y_G[rgbByte.g] + Y_B[rgbByte.b];
            yuvBufOut[pix]= i; //Y
            if((x%2==1)&&(y%2==1))
            {
                pixP4=(nWidth>>1) *(y>>1) + (x>>1);
                i=U_B[rgbByte.b] - U_R[rgbByte.r] - U_G[rgbByte.g]+128;

                yuvBufOut[pixP4+IMGSIZE]     = i;

                i=U_B[rgbByte.r] - V_G[rgbByte.g] - V_B[rgbByte.b]+128;

                yuvBufOut[pixP4 + 5 * IMGSIZE /4] = i;

            }

            pix++;
        }

    }
}
