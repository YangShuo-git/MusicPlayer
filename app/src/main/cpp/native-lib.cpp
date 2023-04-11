#include <jni.h>
#include <string>
#include "AndFFmpeg.h"
#include "AndCallJava.h"

AndFFmpeg *ffmpeg = NULL;
AndCallJava *callJava = NULL;

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_musicplayer_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_musicplayer_service_AndPlayer_n_1parpared(JNIEnv *env, jobject thiz, jstring source_) {
    // 获取const char*
    const char *source = env->GetStringUTFChars(source_,0);

    if (ffmpeg == NULL) {
        ffmpeg = new AndFFmpeg();
    }

    env->ReleaseStringUTFChars(source_,source);
}