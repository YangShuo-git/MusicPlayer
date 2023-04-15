//
// Created by BaiYang on 2023-04-12.
//

#include "AndAudio.h"

AndAudio::AndAudio(AndPlayStatus *playStatus, int sample_rate, AndCallJava *callJava) {
    this->sample_rate = sample_rate;
    // 采样率 声道数 采样位数
    this->outBuffer = (uint8_t *)av_malloc(sample_rate * 2 * 2);
    this->callJava = callJava;
    this->playStatus = playStatus;
    this->queue = new AndQueue(playStatus);
}

// C的方法 传参 this就是这个data，把当前对象传进去，就可以调用该对象的方法了
// 用ffmpeg解码，并用opensl播放
void* decodePlay(void *data) {
    AndAudio *andAudio = (AndAudio *) data;

    // 使用OpenSL ES处理解码后的音频数据
    andAudio->initOpenSLES();
    pthread_exit(&andAudio->thread_play);
}

void AndAudio::play() {
    pthread_create(&thread_play, NULL, decodePlay, this);
}

// 获取采样率
int AndAudio::getCurrentSampleRateForOpensles(int sample_rate) {
    int rate = 0;
    switch (sample_rate)
    {
        case 8000:
            rate = SL_SAMPLINGRATE_8;
            break;
        case 11025:
            rate = SL_SAMPLINGRATE_11_025;
            break;
        case 12000:
            rate = SL_SAMPLINGRATE_12;
            break;
        case 16000:
            rate = SL_SAMPLINGRATE_16;
            break;
        case 22050:
            rate = SL_SAMPLINGRATE_22_05;
            break;
        case 24000:
            rate = SL_SAMPLINGRATE_24;
            break;
        case 32000:
            rate = SL_SAMPLINGRATE_32;
            break;
        case 44100:
            rate = SL_SAMPLINGRATE_44_1;
            break;
        case 48000:
            rate = SL_SAMPLINGRATE_48;
            break;
        case 64000:
            rate = SL_SAMPLINGRATE_64;
            break;
        case 88200:
            rate = SL_SAMPLINGRATE_88_2;
            break;
        case 96000:
            rate = SL_SAMPLINGRATE_96;
            break;
        case 192000:
            rate = SL_SAMPLINGRATE_192;
            break;
        default:
            rate =  SL_SAMPLINGRATE_44_1;
    }
    return rate;
}

// 喇叭需要主动取出数据  喇叭主动调用该函数  （重要）
void pcmBufferCallBack(SLAndroidSimpleBufferQueueItf bf, void * data)
{
    // AudioTrack 是被动 先解码，再送给AudioTrack
    // opengsl es 是主动 先触发opensl，再解码
    AndAudio *andAudio = (AndAudio *) data;
    if(andAudio != NULL) {
        // 喇叭配置 44100 2 2   数据量  44100*2*2 个字节
        int bufferSize = andAudio->resampleAudio();
        if(bufferSize > 0)
        {
            //  andAudio->clock  永远大于  帧 携带时间
//            andAudio->clock+=bufferSize /((double)(andAudio->sample_rate * 2 * 2));
//            if(andAudio->clock - andAudio->last_tiem >= 0.1){
//                andAudio->last_tiem = andAudio->clock;
//                andAudio->callJava->onCallTimeInfo(CHILD_THREAD,andAudio->clock,andAudio->duration);
//            }
            (*andAudio->pcmBufferQueue)->Enqueue(andAudio->pcmBufferQueue, andAudio->outBuffer, bufferSize );
        }
    }
}

// 解码一帧音频，并进行重采样
// 主要是获取解码后的pcm数据(outBuffer)及其实际大小(bufferSize)  因为opensl需要这两个
int AndAudio::resampleAudio() {
    while(playStatus != NULL && !playStatus->exit)
    {
        avPacket = av_packet_alloc();
        avFrame = av_frame_alloc();

        if(queue->getAvpacket(avPacket) != 0)
        {
            // 释放3步曲
            av_packet_free(&avPacket);  // 释放AVPacket里的容器 data
            av_free(avPacket);          // 释放对象avPacket
            avPacket = NULL;
            continue;
        }

        ret = avcodec_send_packet(codecCtx, avPacket);
        if(ret != 0)
        {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }
        ret = avcodec_receive_frame(codecCtx, avFrame);
        if(ret == 0) {
            // 转换器上下文
            SwrContext *swr_ctx = NULL;
            // 设值输入参数 和 输出参数  （swr_alloc_set_opts 通道布局、采样格式、采样率）
            swr_ctx = swr_alloc_set_opts(NULL,
                    AV_CH_LAYOUT_STEREO,AV_SAMPLE_FMT_S16,avFrame->sample_rate,
                    avFrame->channel_layout,(AVSampleFormat) avFrame->format,avFrame->sample_rate,
                    NULL, NULL);
            if(!swr_ctx || swr_init(swr_ctx) <0)
            {
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;

                av_frame_free(&avFrame);
                av_free(avFrame);
                avFrame = NULL;

                swr_free(&swr_ctx);
                continue;
            }

            // 开始转换 swr_convert  将一帧pcm转入outBuffer中
            int nb = swr_convert(swr_ctx,
                    &outBuffer,avFrame->nb_samples,
                    (const uint8_t **) avFrame->data,avFrame->nb_samples);
            int out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
            // 获取转换后的一帧pcm的真实大小 （AudioTrack不需要这个操作，但是opensl需要）
            data_size = nb * out_channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);

            // avFrame 等不等于  播放时 等于 1 不等于 2
            // 时间戳  单位时间  解码时间  =    244ms       100ms    344ms
            // now_time= avFrame->pts * (time_base.num / (double)  time_base.den);
//            now_time= avFrame->pts * av_q2d(time_base);
//            if(now_time < clock)
//            {
//                now_time = clock;
//            }
//            clock = now_time;

            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            swr_free(&swr_ctx);
            break;
        } else{
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;

            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            continue;
        }
    }
    return data_size;
}

void AndAudio::initOpenSLES() {
    // 1.创建引擎对象 engineObject
    SLresult result;
    result= slCreateEngine(&engineObject, 0, 0,
                           0, 0, 0);
    if(result != SL_RESULT_SUCCESS) {
        return;
    }
    // 初始化引擎  false 同步  true 异步
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    // 获取引擎接口 engineEngine，之后通过接口来操作引擎
    (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    if(result!=SL_RESULT_SUCCESS) {
        return;
    }
    LOGD("-------->initOpenSLES  engineObject");

    // 2.创建混音器 outputMixObject
    const SLInterfaceID mids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean mreq[1] = {SL_BOOLEAN_FALSE};
    result =(*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, mids, mreq);
    if(result!=SL_RESULT_SUCCESS) {
        return;
    }
    // 初始化混音器
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if(result!=SL_RESULT_SUCCESS) {
        return;
    }
    // 获取混音器接口 outputMixEnvironmentalReverb
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB, &outputMixEnvironmentalReverb);
    if(result!=SL_RESULT_SUCCESS) {
        return;
    }
    LOGD("-------->initOpenSLES  outputMixObject");


    // 传递的是流数据 2代表一个声音两个字节
    SLDataLocator_AndroidSimpleBufferQueue android_queue={SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,2};
    // pcm数据
    SLDataFormat_PCM pcm={
            SL_DATAFORMAT_PCM,   // 播放数据的格式是pcm格式
            2,       // 2个声道（立体声）
            static_cast<SLuint32>(getCurrentSampleRateForOpensles(sample_rate)), // 默认是44100hz的频率
            SL_PCMSAMPLEFORMAT_FIXED_16,                    // 位数 16位
            SL_PCMSAMPLEFORMAT_FIXED_16,                    // 和位数一致就行
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, // 立体声（前左前右）
            SL_BYTEORDER_LITTLEENDIAN                       // 结束标志
    };
    SLDataSource slDataSource = {&android_queue, &pcm};

    // 配置混音器 audioSnk  播放器跟混音器建立联系
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk ={&outputMix, 0};

    // SLDataSource *pAudioSrc,  音频源  音频配置
    // 各种配置与对应的开关
    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE,SL_IID_VOLUME,SL_IID_MUTESOLO};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE,SL_BOOLEAN_TRUE,SL_BOOLEAN_TRUE};

    // 3.创建播放器对象 pcmPlayerObject
    (*engineEngine)->CreateAudioPlayer(engineEngine, &pcmPlayerObject, &slDataSource, &audioSnk, 2, ids, req);
    // 初始化播放器
    (*pcmPlayerObject)->Realize(pcmPlayerObject, SL_BOOLEAN_FALSE);
    // 获取播放器接口 pcmPlayerPlay
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_PLAY, &pcmPlayerPlay);
    // 获取静音接口 pcmMutePlay
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_MUTESOLO, &pcmMutePlay);
    // 获取控制播放暂停恢复、音量的接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject,SL_IID_VOLUME,&pcmVolumePlay);
    // 注册回调缓冲区 获取缓冲队列接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_BUFFERQUEUE, &pcmBufferQueue);
    // 通知喇叭怎么拉数据
    (*pcmBufferQueue)->RegisterCallback(pcmBufferQueue, pcmBufferCallBack, this);
    // 获取播放状态接口
    (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PLAYING);

    // 播放接口
    // 声道接口
    // 暂停恢复 接口
    // 注册 缓冲接口
    // 注册回调缓冲接口  激活
    pcmBufferCallBack(pcmBufferQueue, this);
    LOGD("-------->initOpenSLES pcmPlayerObject");
}
