//
// Created by BaiYang on 2023-04-12.
//

#include "AndAudio.h"

// C的方法 传参 this就是这个data，把当前对象传进去，就可以调用该对象的方法了
void *decodePlay(void *data) {
    AndAudio *andAudio = (AndAudio *) data;

    // OpenSL ES初始化
    andAudio->initOpenSLES();
    pthread_exit(&andAudio->thread_play);
}

int AndAudio::play() {
    pthread_create(&thread_play, NULL, decodePlay, this);
}


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
    LOGE("-------->initOpenSLES  engineObject");

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
    LOGE("-------->initOpenSLES  outputMixObject");


    // 流数据
    SLDataLocator_AndroidSimpleBufferQueue android_queue={SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,2};
    // pcm数据
    SLDataFormat_PCM pcm={
            SL_DATAFORMAT_PCM,   // 播放pcm格式的数据
            2,                   // 2个声道（立体声）
            static_cast<SLuint32>(getCurrentSampleRateForOpensles(sample_rate)), // 44100hz的频率
            SL_PCMSAMPLEFORMAT_FIXED_16,                    // 位数 16位
            SL_PCMSAMPLEFORMAT_FIXED_16,                    // 和位数一致就行
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, // 立体声（前左前右）
            SL_BYTEORDER_LITTLEENDIAN                       // 结束标志
    };

    // 配置混音器
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk ={&outputMix, 0};
    // 播放器跟混音器建立联系

    SLDataSource slDataSource = {&android_queue, &pcm};
    //SLDataSource *pAudioSrc,  音频源  音频配置
    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE,SL_IID_VOLUME,SL_IID_MUTESOLO};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE,SL_BOOLEAN_TRUE,SL_BOOLEAN_TRUE};

    // 3.创建播放器对象
    (*engineEngine)->CreateAudioPlayer(engineEngine, &pcmPlayerObject, &slDataSource, &audioSnk, 2, ids, req);
    // 初始化播放器
    (*pcmPlayerObject)->Realize(pcmPlayerObject, SL_BOOLEAN_FALSE);
    // 得到接口后调用  获取Player接口 //    得到接口后调用  获取Player接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_PLAY, &pcmPlayerPlay);
    //是否静音接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_MUTESOLO, &pcmMutePlay);

    //   拿控制  播放暂停恢复的句柄
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject,SL_IID_VOLUME,&pcmVolumePlay);
    //    注册回调缓冲区 获取缓冲队列接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_BUFFERQUEUE, &pcmBufferQueue);

    //喇叭  你该怎么拉数据
    (*pcmBufferQueue)->RegisterCallback(pcmBufferQueue, pcmBufferCallBack, this);
    //    操作播放器的接口 播放器 也不能 播放器播放

    //    获取播放状态接口
    (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PLAYING);
    LOGE("-------->initOpenSLES 5 ");
    //    播放接口
    //声道接口
    //暂停恢复 接口
    //注册 缓冲接口
    //注册回调缓冲接口  激活
    pcmBufferCallBack(pcmBufferQueue, this);
    LOGE("-------->initOpenSLES 6");
}
