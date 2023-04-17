//
// Created by BaiYang on 2023-04-12.
//

//#include <pthread.h>
#include "AndFFmpeg.h"
#include "AndroidLog.h"

AndFFmpeg::AndFFmpeg(AndPlayStatus *playStatus, AndCallJava *callJava, const char *url) {
    this->playStatus = playStatus;
    this->callJava = callJava;
    this->url = url;

    pthread_mutex_init(&seek_mutex, NULL);
}

void *demuxFFmpeg(void *data)
{
    AndFFmpeg *andFFmpeg = (AndFFmpeg *) data;
    andFFmpeg->demuxFFmpegThead();
    pthread_exit(&andFFmpeg->demuxThead);
}

void AndFFmpeg::prepared() {
    pthread_create(&demuxThead, NULL, demuxFFmpeg, this);
}

int AndFFmpeg::demuxFFmpegThead() {
    // 初始化网络库
//    avformat_network_init();

    /* ************************ 解封装3步曲 ************************ */
    // 1.分配解码器上下文
    formatCtx = avformat_alloc_context();
    // 2.打开文件并解析
    if(avformat_open_input(&formatCtx, url, NULL, NULL) != 0){
        LOGE("Couldn't open input stream.\n");
        return -1;
    }
    // 3.查找流的上下文信息，并填充Stream的MetaData信息
    if (avformat_find_stream_info(formatCtx, NULL) < 0) {
        LOGE("Couldn't find stream information\n");
        return -1;
    }

    // 遍历获取流索引、解码器参数
    for (int i = 0; i < formatCtx->nb_streams; ++i) {
        if (formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (andAudio == NULL) {
                andAudio = new AndAudio(playStatus, formatCtx->streams[i]->codecpar->sample_rate, callJava);
                andAudio->audioIndex = i;
                andAudio->codecpar = formatCtx->streams[i]->codecpar;

                andAudio->time_base = formatCtx->streams[i]->time_base;
                andAudio->duration = formatCtx->duration / AV_TIME_BASE;

                duration = andAudio->duration;
            }
            break;
        }
    }
    if (andAudio->audioIndex == -1) {
        LOGE("Couldn't find a andAudio stream.\n");
        return -1;
    }
    LOGD("成功找到音频流.\n");
    /* ************************** 解封装结束 ************************** */

    /* ************************ 打开解码器4步曲 ************************ */
    andAudio->codecCtx = avcodec_alloc_context3(NULL);
    avcodec_parameters_to_context(andAudio->codecCtx, andAudio->codecpar);
    andAudio->avCodec = avcodec_find_decoder(andAudio->codecpar->codec_id);
    if (avcodec_open2(andAudio->codecCtx , andAudio->avCodec, NULL) < 0) {
        LOGE("Couldn't open andAudio codec.\n");
        return -1;
    }
    LOGD("成功打开音频解码器.\n");
    /* ************************ 打开解码器结束 ************************ */

    // 回调java层函数，可以将一些状态回调到java层  使用子线程
    // prepared()结束，就调用onCallPrepared()
    callJava->onCallPrepared(CHILD_THREAD);

    return 0;
}

int AndFFmpeg::start() {
    if(andAudio == NULL) {
        if(LOG_DEBUG) {
            LOGE("andAudio is null");
            return -1;
        }
    }
    andAudio->play();

    int count = 0;
    while(playStatus != NULL && !playStatus->exit)
    {
        if(playStatus->seek)
        {
            continue;
        }
        // 放入队列
        if(andAudio->queue->getQueueSize() > 40){
            continue;
        }

        AVPacket *avPacket = av_packet_alloc();
        // 用户停止 或者最后一帧 文件末尾  要有清空队列的操作
        if(av_read_frame(formatCtx, avPacket) == 0)
        {
            if(avPacket->stream_index == andAudio->audioIndex)
            {
                andAudio->queue->putAvpacket(avPacket);
            } else{
                av_packet_free(&avPacket);
                av_free(avPacket);
            }
        } else {
            av_packet_free(&avPacket);
            av_free(avPacket);

            //特殊情况
            while(playStatus != NULL && !playStatus->exit)
            {
                if(andAudio->queue->getQueueSize() > 0)
                {
                    continue;
                } else {
                    playStatus->exit = true;
                    break;
                }
            }
        }

        if(playStatus != NULL && playStatus->exit)
        {
            andAudio->queue->clearAvpacket();
            playStatus->exit = true;
        }
    }
    return 0;
}

void AndFFmpeg::pause() {
    if(andAudio != NULL)
    {
        andAudio->pause();
    }
}


void AndFFmpeg::seek(jint secds) {
    if (duration <= 0) {
        return;
    }

    if (secds >= 0 && secds <= duration) {
        pthread_mutex_lock(&seek_mutex);
        if (andAudio != NULL) {
            playStatus->seek = true;
            andAudio->queue->clearAvpacket();
            andAudio->clock = 0;
            andAudio->last_time = 0;

            // s    *  us
            int64_t rel = secds * AV_TIME_BASE;
            avformat_seek_file(formatCtx, -1, INT64_MIN, rel, INT64_MAX, 0);

            playStatus->seek = false;
        }
        pthread_mutex_unlock(&seek_mutex);
    }
}

void AndFFmpeg::resume() {
    if(andAudio != NULL)
    {
        andAudio->resume();
    }
}

void AndFFmpeg::setMute(jint mute) {
    if(andAudio != NULL)
    {
        andAudio->setMute(mute);
    }
}

