//
// Created by BaiYang on 2023-04-12.
//

#include "AndCallJava.h"

AndCallJava::AndCallJava(_JavaVM *javaVM, JNIEnv *env, jobject obj) {
    this->javaVM = javaVM;
    this->jniEnv = env;
    this->jobj = env->NewGlobalRef(obj);

    // C++如何能调用java层的函数呢？通过如下签名规则即可：
    jclass jlz = jniEnv->GetObjectClass(jobj);
    jmid_prepared = env->GetMethodID(jlz, "onCallPrepared", "()V");
    jmid_timeinfo = env->GetMethodID(jlz, "onCallTimeInfo", "(II)V");
}

// java层的AndPlayer类有一个方法onCallPrepared()
void AndCallJava::onCallPrepared(int type) {
    // type用来区分是主线程，还是子线程
    if(type == MAIN_THREAD)
    {
        // 这里就实现了Native层调用java层的方法，需要准备好：jobject、MethodID、参数
        jniEnv->CallVoidMethod(jobj, jmid_prepared);
    } else if(type == CHILD_THREAD)
    {
        JNIEnv *jniEnv;
        if(javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK)
        {
            if(LOG_DEBUG) {
                LOGE("get child thread jnienv worng");
            }
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmid_prepared);
        javaVM->DetachCurrentThread();
    }
}

void AndCallJava::onCallTimeInfo(int type, int curr, int total) {
    if (type == MAIN_THREAD) {
        jniEnv->CallVoidMethod(jobj, jmid_timeinfo, curr, total);
    } else if (type == CHILD_THREAD) {
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            if (LOG_DEBUG) {
                LOGE("Fail to call onCallTimeInfo!");
            }
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmid_timeinfo, curr, total);
        javaVM->DetachCurrentThread();
    }
}