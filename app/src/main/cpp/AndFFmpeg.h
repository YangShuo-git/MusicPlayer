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

    AVFormatContext *formatCtx = NULL;

    AndAudio *andAudio = NULL;


public:
    int prepared();

};


#endif //MUSICPLAYER_ANDFFMPEG_H
