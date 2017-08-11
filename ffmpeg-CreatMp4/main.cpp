#include <iostream>
#include "Video2FrameRgb.h"
#include "json/json.h"
#include <fstream>
#include "InputParameter.h"
#include "RenderMedia.h"
#include "GetPerspectiveMatrix.h"
#include "FrameComposed.h"
#include "GetMp3data.h"
#include "MediaMuxer.h"

using namespace std ;


int main() {

    InputParameter InputDataContent ;
    InputDataContent.input_BackgroundMP4_file_name = "../Media/bg.mp4" ;
    InputDataContent.input_MaskMP4_file_name =  "../Media/mask.mp4";
    InputDataContent.input_MP3_file_name = "../Media/21.mp3";
    InputDataContent.input_jsonfile_file_name = "../Media/data2.json" ;
    InputDataContent.input_renderMp4_file_name = "../Media/apple1.mp4" ;
    InputDataContent.output_MP4_file_name = "../Media/out.mp4" ;

    Video2FrameRgb MediaContentMask;         //读入mask视频
    MediaContentMask.input_MP4_file_name = InputDataContent.input_MaskMP4_file_name ;
    int retMask = MediaContentMask.readmedia() ;
    if(retMask<0){ cout<<"read mask media error!"<< endl; return - 1;}

    Video2FrameRgb MediaContentBg;     //读入bg视频
    MediaContentBg.input_MP4_file_name = InputDataContent.input_BackgroundMP4_file_name ;
    int retBg = MediaContentBg.readmedia() ;
    if(retBg<0){ cout<<"read bg media error!"<< endl; return - 1;}

    RenderMedia renderMp4Content ;
    renderMp4Content.input_renderMp4_file_name = InputDataContent.input_renderMp4_file_name ;
    renderMp4Content.outwidth = MediaContentBg.width ;
    renderMp4Content.outheight = MediaContentBg.height ;
    renderMp4Content.frameNum = MediaContentBg.frameNum ;

    Json::Reader reader;
    Json::Value root;
    //从json文件中读取数据
    ifstream in( InputDataContent.input_jsonfile_file_name, ios::binary);
    if( !in.is_open() )
    {
        cout << "Error opening file\n";
        return -1;
    }
    if(reader.parse(in,root)) {
        for(int i=0;i<root.size();i++){
            OneFrameParam frametemp ;
            frametemp.FrameId=i;
            frametemp.scale[0]=640;
            frametemp.scale[1]=442;
            frametemp.change=root[i]["switch"].asBool() ;

            if(frametemp.change) {
                GetPerspectiveMatrix perspectiveDoing;
                perspectiveDoing.x[0] = 0;
                perspectiveDoing.x[1] = 639;
                perspectiveDoing.x[2] = 0;
                perspectiveDoing.x[3] = 639;

                perspectiveDoing.y[0] = 441;
                perspectiveDoing.y[1] = 441;
                perspectiveDoing.y[2] = 0;
                perspectiveDoing.y[3] = 0;

                perspectiveDoing.u[0] = root[i]["tl_x"].asDouble()-0;
                perspectiveDoing.u[1] = root[i]["tr_x"].asDouble()+0;
                perspectiveDoing.u[2] = root[i]["bl_x"].asDouble()-0;
                perspectiveDoing.u[3] = root[i]["br_x"].asDouble()+0;

                perspectiveDoing.v[0] = root[i]["tl_y"].asDouble();
                perspectiveDoing.v[1] = root[i]["tr_y"].asDouble();
                perspectiveDoing.v[2] = root[i]["bl_y"].asDouble();
                perspectiveDoing.v[3] = root[i]["br_y"].asDouble();
                frametemp.PerspectiveMatrix = perspectiveDoing.PerspectiveMatrix();
            }

            frametemp.XYbias[0]=320;
            frametemp.XYbias[1]=222;
            frametemp.XYsize[0]=640;
            frametemp.XYsize[1]=442;
            renderMp4Content.VideoInputParam.push_back(frametemp) ;
        }
    }
    in.close();  //关闭json文件

    int retRender = renderMp4Content.render() ; //渲染动作
    if(retRender<0)
    {
        cout << "reand Mp4 error!"<< endl ;
        return -1;
    }

    FrameComposed frameMake;
    frameMake.videoframe1 = MediaContentBg.VideoFrameData ;
    frameMake.videoframe2 = renderMp4Content.VideoFramedata ;
    frameMake.videoframemask = MediaContentMask.VideoFrameData ;
    frameMake.addimage();

    GetMp3data MP3ReadContext ;   //得到mp3文件数据
    MP3ReadContext.input_MP3_file_name = InputDataContent.input_MP3_file_name ;
    int ret_mp3 = MP3ReadContext.getaudiodata() ;
    if(ret_mp3<0)
    {
        cout << "get mp3 error "<< endl ;
        return -1 ;
    }

    MediaMuxer creatMp4;
    creatMp4.output_MP4_file_name = InputDataContent.output_MP4_file_name;
    creatMp4.videoframe_in = frameMake.videoframegoal ;
    creatMp4.temp_voice = MP3ReadContext.temp_voice ;
    creatMp4.STREAM_DURATION = 9 ;
    creatMp4.STREAM_FRAME_RATE[0] = 25 ;
    creatMp4.STREAM_FRAME_RATE[1] = 1;
    creatMp4.width = MediaContentBg.width;
    creatMp4.height = MediaContentBg.height;
    int retMediaMux = creatMp4.mediamux() ;
    if(retMediaMux<0){cout << "mux mp4 error !"<< endl ;return -1 ;}

    return 0;
}