#include <jni.h>
#include <string>
#include "AndFFmpeg.h"
#include "AndCallJava.h"

_JavaVM *javaVM = NULL;
AndFFmpeg *ffmpeg = NULL;
AndCallJava *callJava = NULL;
AndPlayStatus *playStatus = NULL;

extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{
    jint result = -1;
    javaVM = vm;
    JNIEnv *env;
    if(vm->GetEnv((void **)&env, JNI_VERSION_1_4) != JNI_OK)
    {
        return result;
    }
    return JNI_VERSION_1_4;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_musicplayer_service_AndPlayer_n_1prepared(JNIEnv *env, jobject thiz, jstring source_) {
    // 获取const char*
    const char *source = env->GetStringUTFChars(source_,0);

    if (ffmpeg == NULL) {
        if (callJava == NULL) {
            callJava = new AndCallJava(javaVM,env,thiz);
        }
        playStatus = new AndPlayStatus();
        ffmpeg = new AndFFmpeg(playStatus, callJava,source);
        ffmpeg->callJava = callJava;
        ffmpeg->prepared();
    }

//    env->ReleaseStringUTFChars(source_,source);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_musicplayer_service_AndPlayer_n_1start(JNIEnv *env, jobject thiz) {
    if(ffmpeg != NULL)
    {
        ffmpeg->start();
    }
}