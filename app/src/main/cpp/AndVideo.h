//
// Created by BaiYang on 2023-04-20.
//

#ifndef MUSICPLAYER_ANDVIDEO_H
#define MUSICPLAYER_ANDVIDEO_H

#include "AndQueue.h"
#include "AndCallJava.h"
#include "AndPlayStatus.h"
#include "AndAudio.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/time.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
};

class AndVideo {
public:
    pthread_t thread_play;
    pthread_mutex_t codecMutex;

    AndAudio *vAudio = NULL;  // 视频里必须要有audio，要做到向audio同步
    AndQueue *queue = NULL;
    AndPlayStatus *playStatus = NULL;
    AndCallJava *callJava = NULL;

    int streamIndex = -1;
    AVCodecContext *codecCtx = NULL;
    AVCodecParameters *codecpar = NULL;

    double clock = 0;
    // 实时计算出来   主要与音频的差值
    double delayTime = 0;
    // 默认休眠时间   40ms  0.04s    帧率 25帧
    double defaultDelayTime = 0.04;
    AVRational time_base;

public:
    AndVideo(AndPlayStatus *playStatus, AndCallJava *callJava);
    ~AndVideo();

    void play();
    void pause();
    void resume();

    double getDelayTime(double diff);
    double getFrameDiffTime(AVFrame *avFrame);
};

#endif //MUSICPLAYER_ANDVIDEO_H
