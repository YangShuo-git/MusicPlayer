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
    const char *url = NULL;
    pthread_t decodeThread;
    pthread_mutex_t seek_mutex;

    AVFormatContext *formatCtx = NULL;

    AndAudio *andAudio = NULL;

    AndCallJava *callJava = NULL;
    AndPlayStatus *playStatus = NULL;

public:
    AndFFmpeg(AndCallJava *callJava, const char *url);

    int prepared();

    int start();
};


#endif //MUSICPLAYER_ANDFFMPEG_H
