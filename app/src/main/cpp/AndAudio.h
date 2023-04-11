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
};


#endif //MUSICPLAYER_ANDAUDIO_H
