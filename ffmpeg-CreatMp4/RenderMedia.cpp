//
// Created by yfei on 17-8-11.
//
//
// Created by yfei on 17-8-2.
//
#include <iostream>
#include <algorithm>
#include <thread>
#include "inRect.h"
#include "Distance.h"
#include "RenderMedia.h"

using namespace std;

extern "C"{
#include "inttypes.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavfilter/avfiltergraph.h"
#include "libswscale/swscale.h"
};

#define  WHITE 255
#define  BLACK 0
#define COLORSIZE 256
bool flag=0 ;

typedef struct finder_t{
    finder_t(int n) : VideoframeId(n) { } bool operator()(OneFrameParam p)
    { return (VideoframeId == p.FrameId); } int VideoframeId;
}finder_t;

int RenderMedia::render() {

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

    if (avformat_open_input(&pFormatCtx,input_renderMp4_file_name, NULL, NULL) != 0) {
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

    av_dump_format(pFormatCtx, 0,input_renderMp4_file_name, 0); //输出视频信息
   // STREAM_DURATION = pFormatCtx->duration/1000000 ;
   // STREAM_FRAME_RATE[0] = pFormatCtx->streams[videoStream]->avg_frame_rate.num;
   // STREAM_FRAME_RATE[1] = pFormatCtx->streams[videoStream]->avg_frame_rate.den;

    int index = 0;
    VideoFramedata.clear() ; //

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

            if (got_picture) {
                sws_scale(img_convert_ctx,
                          (uint8_t const *const *) pFrame->data,
                          pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data,
                          pFrameRGB->linesize);

                vector <OneFrameParam>::iterator it = find_if(VideoInputParam.begin(),
                                                           VideoInputParam.end(), finder_t(index));
                // 查找查找视频帧Id为需要处理的indexs
                if (it != VideoInputParam.end()) // 找到了FrameID
                {
                    if(it->change==true) {
                        uint8_t *yuvBufIn;    //allocate the memory for frame-data
                        yuvBufIn = (uint8_t *) malloc(sizeof(uint8_t) * memory_size * 3);
                        memcpy(yuvBufIn, pFrameRGB->data[0], memory_size * 3);

                        /* add every frame data from  memory to the image structure  */
                        ppm image(width, height);
                        long co = 0;
                        for (int y = 0; y < height; y++) {
                            for (int x = 0; x < width; x++) {
                                image.r.push_back(yuvBufIn[co++]);
                                image.g.push_back(yuvBufIn[co++]);
                                image.b.push_back(yuvBufIn[co++]);
                            }
                        }
                        free(yuvBufIn);  // free the memory space in yuvBufIn
                        yuvBufIn = NULL;

                        image.write("/home/yfei/movies/outimageinput.ppm") ;

                        it->zoom[0] = it->scale[0]  / double(width) ;
                        it->zoom[1] = it->scale[1] / double(height) ;

                        //   image.write("/home/yfei/movies/wopu.ppm") ;

                        ppm OutImage(outwidth,outheight);  // declarate a new image structure for the result of the image changed
                        /** image process funciton  **/

                        ImagePerspective(image,OutImage,it->zoom,it->XYbias,it->PerspectiveMatrix) ;

                        OutImage.write("/home/yfei/movies/outimage.ppm") ;

                    }
                    else {
                        /** storage the YUV data straight **/
                        image frame_temp_data ;
                        frame_temp_data.r.reserve(outheight*outwidth) ;
                        frame_temp_data.g.reserve(outheight*outwidth) ;
                        frame_temp_data.b.reserve(outheight*outwidth) ;
                        for (int y = 0; y < outheight; y++) {
                            for (int x = 0; x < outwidth; x++) {
                                frame_temp_data.r.push_back(BLACK);
                                frame_temp_data.g.push_back(BLACK);
                                frame_temp_data.b.push_back(BLACK);
                            }
                        }
                        VideoFramedata.push_back(frame_temp_data);
                    }
                }
                index++;
                if (index==frameNum) break ;
            }
        }
        av_free_packet(packet);    // free the space
        // cout <<index <<endl;
    }

    av_free(out_buffer);
    av_free(pFrameRGB);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);
    return 0 ;
}

void RenderMedia::ImagePerspective(ppm &InImage, ppm &OutImage, double *zoom, double *XYbias,Matrix<double,3,3> PerspectiveMatrix) {

        double x_cen_init = round( width / 2);   //find the centroid of the original image
        double y_cen_init = round( height / 2);

        Point leftdown_init(0 - x_cen_init, 0 - y_cen_init);
        Point leftup_init(0 - x_cen_init, height - y_cen_init);
        Point rightdown_init(width - x_cen_init, 0 - y_cen_init);
        Point rightup_init(width - x_cen_init, height - y_cen_init);

        Vector3d temp_LeftUp, temp_LeftDown, temp_RightUp, temp_RightDown;

        temp_LeftDown << leftdown_init.x, leftdown_init.y, 1;
        temp_LeftUp << leftup_init.x, leftup_init.y, 1;
        temp_RightDown << rightdown_init.x, rightdown_init.y, 1;
        temp_RightUp << rightup_init.x, rightup_init.y, 1;

        Matrix<double, 3, 3> MatPro, MatShap, MatCl, MatTranslate;

        MatShap << zoom[0], 0, 0,        // Zoom Matrix
                0, zoom[1], 0,
                0, 0, 1;

        MatTranslate << 1, 0, XYbias[0],   //Translate Matrix
                0, 1, XYbias[1],
                0, 0, 1;
        MatPro = PerspectiveMatrix * MatTranslate * MatShap;
        //MatPro = MatTranslate*MatShap ;

        /** define the boundary of the new image ***/
        auto LeftDown = MatPro * temp_LeftDown;
        auto LeftUp = MatPro * temp_LeftUp;
        auto RightDown = MatPro * temp_RightDown;
        auto RightUp = MatPro * temp_RightUp;

        cout << "MatPro" << endl << MatPro << endl;
        double ru_x = RightUp(0) / RightUp(2);
        double rd_x = RightDown(0) / RightDown(2);
        double lu_x = LeftUp(0) / LeftUp(2);
        double ld_x = LeftDown(0) / LeftDown(2);
        double ru_y = RightUp(1) / RightUp(2);
        double rd_y = RightDown(1) / RightDown(2);
        double lu_y = LeftUp(1) / LeftUp(2);
        double ld_y = LeftDown(1) / LeftDown(2);

        double min_x = min(min(min(ru_x, rd_x), lu_x), ld_x);
        double min_y = min(min(min(ru_y, rd_y), lu_y), ld_y);
        double max_y = max(max(max(ru_x, rd_x), lu_x), ld_x);
        double max_x = max(max(max(ru_y, rd_y), lu_y), ld_y);

        int x[4] = {ld_x, lu_x, ru_x, rd_x};
        int y[4] = {ld_y, lu_y, ru_y, rd_y};

   for(int i=0;i<4;i++) {
        if (fabs(x[i] - min_x) < 2)
            x[i] = x[i] - 1 ;
        if (fabs(x[i] - max_x) < 2)
            x[i] = x[i] + 1 ;
        if (fabs(y[i] - min_y) < 2)
            y[i] = y[i] - 1 ;
        if (fabs(y[i] - max_y) < 2)
            y[i] = y[i] + 1 ;
    }
        Point Point_LD(x[0], y[0]);    //define the four boundary point of new image
        Point Point_LU(x[1], y[1]);
        Point Point_RU(x[2], y[2]);
        Point Point_RD(x[3], y[3]);

    long size = outwidth*outwidth ;

    image frame_temp ;
    frame_temp.r.reserve(size) ;
    frame_temp.g.reserve(size) ;
    frame_temp.b.reserve(size) ;
    long int indx = 0;
    long int count = 0;
        /*** the pixel is given the RGB value  ***/
        for (int Col = 0; Col < outheight; Col++) {
            for (int Row = 0; Row < outwidth; Row++) {
                Point Point_test(Row, Col);
                count++;
                indx = Col * outwidth + Row;
                // judge if the current point is in the new image area (if YES then)
                if (IsPointInMatrix(Point_LD, Point_LU, Point_RU, Point_RD, Point_test)) {
                    Vector3d M_point;
                    M_point << Row, Col, 1;
                    Matrix<double,3,1> Point_Original = MatPro.inverse() * M_point; //Mapping to the original point
                    // 采用双线性插值
                    double x_Orignal = Point_Original(0) / Point_Original(2) + x_cen_init;
                    int x_xiao = int(x_Orignal);
                    int x_da = x_xiao + 1;
                    double x_u = x_Orignal - x_xiao;

                    double y_Orignal = Point_Original(1) / Point_Original(2) + y_cen_init;
                    int y_xiao = int(y_Orignal);
                    int y_da = y_xiao + 1;
                    double y_u = y_Orignal - y_xiao;

                    InImage.r[width*height] =  0 ;
                    InImage.g[width*height] =  0 ;
                    InImage.b[width*height] =  0 ;

                    long index_1_1 = width*height;
                    long index_1_2 = width*height;
                    long index_2_1 = width*height;
                    long index_2_2 = width*height;

                    if(x_xiao>=0&&x_xiao<width&&y_xiao>=0&&y_xiao<height)
                        index_1_1 = (long) ( (height-1-y_xiao )* width + x_xiao);
                    if(x_xiao>=0&&x_xiao<width&&y_da>=0&&y_da<height)
                        index_1_2 = (long) ((height -1-y_da)* width + x_xiao);
                    if(x_da>=0&&x_da<width&&y_xiao>=0&&y_xiao<height)
                        index_2_1 = (long) ((height -1-y_xiao) * width + x_da);
                    if(x_da>=0&&x_da<width&&y_da>=0&&y_da<height)
                        index_2_2 = (long) ((height -1-y_da) * width + x_da );

                    // 双线性插值算法
                    OutImage.r[indx] = (unsigned char) InImage.r[index_1_1] * (1 - x_u) * (1 - y_u) +
                                       InImage.r[index_1_2] * (1 - x_u) * y_u
                                       + InImage.r[index_2_1] * x_u * (1 - y_u) + InImage.r[index_2_2] * x_u * y_u;

                    OutImage.g[indx] = (unsigned char) InImage.g[index_1_1] * (1 - x_u) * (1 - y_u) +
                                       InImage.g[index_1_2] * (1 - x_u) * y_u
                                       + InImage.g[index_2_1] * x_u * (1 - y_u) + InImage.r[index_2_2] * x_u * y_u;
                    OutImage.b[indx] = (unsigned char) InImage.r[index_1_1] * (1 - x_u) * (1 - y_u) +
                                       InImage.b[index_1_2] * (1 - x_u) * y_u
                                       + InImage.b[index_2_1] * x_u * (1 - y_u) + InImage.b[index_2_2] * x_u * y_u;

                    //计算到四边的距离
                    Distance DisContxt_01;
                    DisContxt_01.A.x = x[0];DisContxt_01.A.y = y[0];
                    DisContxt_01.B.x = x[1];DisContxt_01.B.y = y[1];
                    DisContxt_01.test.x = Row;DisContxt_01.test.y = Col;

                    Distance DisContxt_12;
                    DisContxt_12.A.x = x[1];DisContxt_12.A.y = y[1];
                    DisContxt_12.B.x = x[2];DisContxt_12.B.y = y[2];
                    DisContxt_12.test.x = Row;DisContxt_12.test.y = Col;

                    Distance DisContxt_23;
                    DisContxt_23.A.x = x[2];DisContxt_23.A.y = y[2];
                    DisContxt_23.B.x = x[3];DisContxt_23.B.y = y[3];
                    DisContxt_23.test.x = Row;DisContxt_23.test.y = Col;

                    Distance DisContxt_30;
                    DisContxt_30.A.x = x[3];DisContxt_30.A.y = y[3];
                    DisContxt_30.B.x = x[0];DisContxt_30.B.y = y[0];
                    DisContxt_30.test.x = Row;DisContxt_30.test.y = Col;

                    //根据接近边界处边界距离模糊化图像
                    if(DisContxt_01.GetDistanace()<=3||DisContxt_12.GetDistanace()<=3
                       ||DisContxt_23.GetDistanace()<=3 ||DisContxt_30.GetDistanace()<=3) {
                        double weight = min(min(min(DisContxt_01.GetDistanace(), DisContxt_12.GetDistanace()),
                                                DisContxt_23.GetDistanace()), DisContxt_30.GetDistanace());
                        double alpha = weight*255/3 ;
                        OutImage.r[indx] = OutImage.r[indx]*alpha/255+0*(255-alpha)/255 ;
                        OutImage.g[indx] = OutImage.g[indx]*alpha/255+0*(255-alpha)/255 ;
                        OutImage.b[indx] = OutImage.b[indx]*alpha/255+0*(255-alpha)/255 ;
                    }
                }else{
                    OutImage.r[indx] = BLACK;
                    OutImage.g[indx] = BLACK;
                    OutImage.b[indx] = BLACK;
                }

                frame_temp.r.push_back(OutImage.r[indx]) ;
                frame_temp.g.push_back(OutImage.g[indx]) ;
                frame_temp.b.push_back(OutImage.b[indx]) ;
            }
        }

        VideoFramedata.push_back(frame_temp) ;
}