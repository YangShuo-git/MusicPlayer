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
    int audioIndex = -1;
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
    // 采样频率
    int sample_rate = 0;

public:
    AndAudio();

    int getCurrentSampleRateForOpensles(int sample_rate);

    void initOpenSLES();

    int play();

};


#endif //MUSICPLAYER_ANDAUDIO_H
