//
// Created by BaiYang on 2023-04-12.
//

#ifndef MUSICPLAYER_ANDAUDIO_H
#define MUSICPLAYER_ANDAUDIO_H

#include "AndQueue.h"
#include "AndCallJava.h"

extern "C"
{
#include <libswresample/swresample.h>
#include <libavcodec/codec_par.h>
#include <libavcodec/avcodec.h>

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
};


class AndAudio {
public:
    int ret = 0;
    AVCodecContext *codecCtx = NULL;
    AVCodecParameters *codecpar = NULL;
    AVCodec * avCodec = NULL;

    pthread_t thread_play;

    // 引擎对象
    SLObjectItf engineObject = NULL;
    // 引擎接口
    SLEngineItf engineEngine = NULL;
    // 混音器对象
    SLObjectItf outputMixObject = NULL;
    // 混音器接口
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;
    // 播放器对象
    SLObjectItf pcmPlayerObject = NULL;
    // 播放器操作接口
    SLPlayItf pcmPlayerPlay = NULL;
    // 静音接口
    SLMuteSoloItf  pcmMutePlay = NULL;
    SLVolumeItf pcmVolumePlay = NULL;
    // 缓冲器队列接口
    SLAndroidSimpleBufferQueueItf pcmBufferQueue = NULL;

    AndQueue *queue = NULL;
    AndCallJava *callJava = NULL;
    AndPlayStatus *playStatus = NULL;

    AVPacket *avPacket = NULL;
    AVFrame *avFrame = NULL;

    // 采样频率
    int sample_rate = 0;
    // 音频流索引
    int audioIndex = -1;
    // 输出音频缓冲区
    uint8_t *outBuffer = NULL;
    int data_size = 0;

    int duration = 0; // 总时长
    // 时间单位 总时间/帧数   单位时间 * 时间戳= pts  * 总时间/帧数
    AVRational time_base; // 时间基
    double now_time;  // 当前Frme的时间（解码时间）
    double clock;     // 当前播放的时间 （准确时间）

    double last_time; // 上一次调用时间

    jmethodID jmid_timeinfo;

    //立体声
    int mute = 2;

public:
    AndAudio(AndPlayStatus *playstatus, int sample_rate, AndCallJava *callJava);

    // 解码函数
    int resampleAudio();

    int getCurrentSampleRateForOpensles(int sample_rate);

    void initOpenSLES();

    void play();

    void pause();

    void resume();

    void setVolume(int percent);

    void setMute(int mute);
};


#endif //MUSICPLAYER_ANDAUDIO_H
