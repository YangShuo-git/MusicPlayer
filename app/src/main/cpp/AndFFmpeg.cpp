//
// Created by BaiYang on 2023-04-12.
//

//#include <pthread.h>
#include "AndFFmpeg.h"
#include "AndroidLog.h"


int AndFFmpeg::prepared() {
    avformat_network_init();

    /* ************************ 解封装3步曲 ************************ */
    // 1.分配解码器上下文
    formatCtx = avformat_alloc_context();
    // 2.打开文件并解析
    if(avformat_open_input(&formatCtx, url, NULL, NULL) != 0){
        LOGD("Couldn't open input stream.\n");
        return -1;
    }
    // 3.查找流的上下文信息，并填充Stream的MetaData信息
    if (avformat_find_stream_info(formatCtx, NULL) < 0) {
        LOGD("Couldn't find stream information\n");
        return -1;
    }

    // 遍历获取流索引
    for (int i = 0; i < formatCtx->nb_streams; ++i) {
        if (formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (andAudio) {
                andAudio = new AndAudio();
                andAudio->audioIndex = i;
                andAudio->codecpar = formatCtx->streams[i]->codecpar;
            }
            break;
        }
    }

    if (andAudio->audioIndex == -1) {
        LOGD("Couldn't find a audio stream.\n");
        return -1;
    }
    LOGD("成功找到音频频流.\n");
    /* ************************** 解封装结束 ************************** */

    /* ************************ 打开解码器4步曲 ************************ */
    andAudio->codecCtx = avcodec_alloc_context3(NULL);
    avcodec_parameters_to_context(andAudio->codecCtx, andAudio->codecpar);
    andAudio->avCodec = avcodec_find_decoder(andAudio->codecpar->codec_id);
    if (avcodec_open2(andAudio->codecCtx , andAudio->avCodec, NULL) < 0) {
        LOGD("Couldn't open audio codec.\n");
        return -1;
    }
    LOGD("成功打开解码器.\n");
    /* ************************ 打开解码器结束 ************************ */

    // 然后将状态回调到java层
    return 0;
}

