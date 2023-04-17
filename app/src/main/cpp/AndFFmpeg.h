//
// Created by BaiYang on 2023-04-12.
//

#ifndef MUSICPLAYER_ANDFFMPEG_H
#define MUSICPLAYER_ANDFFMPEG_H

#include "AndAudio.h"
#include "AndCallJava.h"
#include "AndPlayStatus.h"


extern "C" {
#include "include/libavformat/avformat.h"
}

class AndFFmpeg {
public:
    pthread_t demuxThead;
    pthread_mutex_t seek_mutex;

    const char *url = NULL;
    AVFormatContext *formatCtx = NULL;

    AndAudio *andAudio = NULL;
    AndCallJava *callJava = NULL;
    AndPlayStatus *playStatus = NULL;

    int duration = 0;

public:
    AndFFmpeg(AndPlayStatus *playStatus, AndCallJava *callJava, const char *url);

    int demuxFFmpegThead();

    // 解封装
    void prepared();
    // 解码
    int start();

    void pause();

    void seek(jint i);

    void resume();

    void setMute(jint i);
};


#endif //MUSICPLAYER_ANDFFMPEG_H
