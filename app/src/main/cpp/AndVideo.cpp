//
// Created by BaiYang on 2023-04-20.
//

#include "AndVideo.h"

AndVideo::AndVideo(AndPlayStatus *playStatus, AndCallJava *callJava)
{
    this->playStatus = playStatus;
    this->callJava = callJava;
    this->queue = new AndQueue(playStatus);
    pthread_mutex_init(&codecMutex, NULL);
}

void * playVideo(void *handler)
{
    AndVideo *andVideo = static_cast<AndVideo *>(handler);

    //  死循环轮训
    while(andVideo->playStatus != NULL && !andVideo->playStatus->exit)
    {
        // 解码 seek   puase   队列没有数据
        if(andVideo->playStatus->seek)
        {
            av_usleep(1000 * 100);
            continue;
        }
        if(andVideo->playStatus->pause)
        {
            av_usleep(1000 * 100);
            continue;
        }
        if (andVideo->queue->getQueueSize() == 0) {
            // 队列中无数据，需要休眠  请慢慢等待  回调应用层
            if(!andVideo->playStatus->load)
            {
                andVideo->playStatus->load = true;
                andVideo->callJava->onCallLoad(CHILD_THREAD, true);
                av_usleep(1000 * 100);
                continue;
            }
        }

        AVPacket *avPacket = av_packet_alloc();
        if(andVideo->queue->getAvpacket(avPacket) != 0)
        {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }
        // 视频解码 比较耗时  多线程
        pthread_mutex_lock(&andVideo->codecMutex);
        // 解码操作
        if(avcodec_send_packet(andVideo->codecCtx, avPacket) != 0)
        {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            pthread_mutex_unlock(&andVideo->codecMutex);
            continue;
        }

        AVFrame *avFrame = av_frame_alloc();
        if(avcodec_receive_frame(andVideo->codecCtx, avFrame) != 0)
        {
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            pthread_mutex_unlock(&andVideo->codecMutex);
            continue;
        }
        //  此时解码成功了  如果 之前是yuv420  ----》   opengl
        if(avFrame->format == AV_PIX_FMT_YUV420P)
        {
            // 压缩1  原始数据2
            // avFrame->data[0];//y
            // avFrame->data[1];//u
            // avFrame->data[2];//v
            // 直接转换   yuv420     ---> yuv420
            //其他格式 --yuv420
            //休眠33ms  不可取33 * 1000
            //计算  音频 视频
            // av_usleep(33 * 1000);

            double diff = andVideo->getFrameDiffTime(avFrame);
            // 通过diff 计算休眠时间
            av_usleep(andVideo->getDelayTime(diff) * 1000000);
            andVideo->callJava->onCallRenderYUV(
                    andVideo->codecCtx->width,
                    andVideo->codecCtx->height,
                    avFrame->data[0],
                    avFrame->data[1],
                    avFrame->data[2]);
            LOGE("当前视频是YUV420P格式");
        }else{
            LOGE("当前视频不是YUV420P格式");
            AVFrame *pFrameYUV420P = av_frame_alloc();
            int num = av_image_get_buffer_size(
                    AV_PIX_FMT_YUV420P,
                    andVideo->codecCtx->width,
                    andVideo->codecCtx->height,
                    1);
            uint8_t *buffer = static_cast<uint8_t *>(av_malloc(num * sizeof(uint8_t)));
            av_image_fill_arrays(
                    pFrameYUV420P->data,
                    pFrameYUV420P->linesize,
                    buffer,
                    AV_PIX_FMT_YUV420P,
                    andVideo->codecCtx->width,
                    andVideo->codecCtx->height,
                    1);
            SwsContext *sws_ctx = sws_getContext(
                    andVideo->codecCtx->width,andVideo->codecCtx->height,andVideo->codecCtx->pix_fmt,
                    andVideo->codecCtx->width,andVideo->codecCtx->height,AV_PIX_FMT_YUV420P,
                    SWS_BICUBIC, NULL, NULL, NULL);

            if(!sws_ctx)
            {
                av_frame_free(&pFrameYUV420P);
                av_free(pFrameYUV420P);
                av_free(buffer);
                pthread_mutex_unlock(&andVideo->codecMutex);
                continue;
            }
            sws_scale(
                    sws_ctx,
                    reinterpret_cast<const uint8_t *const *>(avFrame->data),
                    avFrame->linesize,
                    0,
                    avFrame->height,
                    pFrameYUV420P->data,
                    pFrameYUV420P->linesize);
            // 渲染
            andVideo->callJava->onCallRenderYUV(
                    andVideo->codecCtx->width,
                    andVideo->codecCtx->height,
                    pFrameYUV420P->data[0],
                    pFrameYUV420P->data[1],
                    pFrameYUV420P->data[2]);

            av_frame_free(&pFrameYUV420P);
            av_free(pFrameYUV420P);
            av_free(buffer);
            sws_freeContext(sws_ctx);
        }
        av_frame_free(&avFrame);
        av_free(avFrame);
        avFrame = NULL;
        av_packet_free(&avPacket);
        av_free(avPacket);
        avPacket = NULL;
        pthread_mutex_unlock(&andVideo->codecMutex);
    }
    pthread_exit(&andVideo->thread_play);
}

void AndVideo::play() {
//    子线程 解码 播放
    pthread_create(&thread_play, NULL, playVideo, this);
}

double AndVideo::getFrameDiffTime(AVFrame *avFrame) {
//    先获取视频时间戳  处理之后
    double pts = av_frame_get_best_effort_timestamp(avFrame);
    if(pts == AV_NOPTS_VALUE)
    {
        pts = 0;
    }
//     1.001*40ms
//    pts=pts * time_base.num / time_base.den;
    pts *= av_q2d(time_base);

    if(pts > 0)
    {
        clock = pts;
    }

    double diff = vAudio->clock - clock;
    return diff;
}

// 33ms  -----》动态计算
double AndVideo::getDelayTime(double diff) {

//    返回秒数 3ms 以内
//音频超越视频  3ms   1

//视频超越  音频3ms   2
    if(diff > 0.003) {
//        视频休眠时间
        delayTime = delayTime * 2 / 3;// * 3/2;
        if (delayTime < defaultDelayTime / 2) {
//            用户有所察觉
            delayTime = defaultDelayTime * 2 / 3;
        }else if(delayTime > defaultDelayTime * 2) {
            delayTime = defaultDelayTime * 2;
        }
    } else if(diff < - 0.003)
    {
//视频超前    休眠时间 相比于以前大一些
        delayTime = delayTime * 3 / 2;
        if(delayTime < defaultDelayTime / 2)
        {
            delayTime = defaultDelayTime * 2 / 3;
        }
        else if(delayTime > defaultDelayTime * 2)
        {
            delayTime = defaultDelayTime * 2;
        }
    }
//感觉的 视频加速
    if (diff >= 0.5) {
        delayTime = 0;
    } else if(diff <= -0.5)
    {
        delayTime = defaultDelayTime * 2;
    }
//    音频太快了   视频怎么赶也赶不上        视频队列全部清空   直接解析最新的 最新鲜的
    if(diff>= 10)
    {
        queue->clearAvpacket();
        delayTime = defaultDelayTime;
    }
//视频太快了  音频赶不上
    if (diff <= -10) {
        vAudio->queue->clearAvpacket();
        delayTime = defaultDelayTime;
        LOGE("====================>视频太快了");
    }
    return delayTime;
}

void AndVideo::pause() {

}