/*Copyright 2017 Viktor Degtyarev

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.*/

#include <jni.h>
#include <dlfcn.h>
#include <stdio.h>
#include <pthread.h>
#include <malloc.h>
#include <android/log.h>
#include "audio.h"


#define TAG_NAME    "CallRecLib"

#define log_info(fmt, args...) __android_log_print(ANDROID_LOG_INFO, TAG_NAME, (const char *) fmt, ##args)
#define log_err(fmt, args...) __android_log_print(ANDROID_LOG_ERROR, TAG_NAME, (const char *) fmt, ##args)

#ifdef LOG_DBG
#define log_dbg log_info
#else
#define log_dbg(...)
#endif


extern "C" {
JNIEXPORT jint JNICALL
Java_net_callrec_library_fix_CallRecorderFix_stopFix(JNIEnv *env, jobject thiz);
JNIEXPORT jint JNICALL
Java_net_callrec_library_fix_CallRecorderFix_startFix(JNIEnv *env, jobject thiz, jint audioSessionId);
JNIEXPORT jint JNICALL
Java_net_callrec_library_fix_CallRecorderFix_startFix7(JNIEnv *env, jobject thiz);
JNIEXPORT jint JNICALL
Java_net_callrec_library_fix_CallRecorderFix_load(JNIEnv *env, jobject thiz);
}

int setParam(jint audioSessionId, const char *param) {
  pthread_mutex_lock(&mt_1);

  as = audioSessionId;
  string8(&paramAsString8, param);
  cm = CMD_GO;
  pthread_cond_signal(&cnd_1);
  pthread_mutex_unlock(&mt_1);

  return 0;
}

void *taskAudioSetParam(void *threadid) {
  /**
     Call AudioSystem->setParameters repeatidly in the loop for a period of time
     defined by java thread caling startParam() and stopParam().
     The point of doing this is to send our setting of input_source=4 after
     the regular setup sends it's setting of input_source=1.

     This way our setting is later and takes precendence.
  */
  while (1) {
    pthread_mutex_lock(&mt_1);
    if (cm == CMD_WAIT) {
      pthread_cond_wait(&cnd_1, &mt_1);
    } else if (setParameters != NULL) {
      log_info("audiosession = %i", as);
      log_info("setParameters() result=%i", setParameters(as, &paramAsString8));
    }
    pthread_mutex_unlock(&mt_1);
  }
}

int startParam(jint audioSessionId, const char *kvp) {
    LOGI("stop AndroidAudioRecord::check %i, %s", audioSessionId, kvp);
    return setParam(audioSessionId, kvp);
}

int stopParam() {
    pthread_mutex_lock(&mt_1);
    cm = CMD_WAIT;
    pthread_cond_signal(&cnd_1);
    pthread_mutex_unlock(&mt_1);
    return 0;
}

int load(JNIEnv *env, jobject thiz) {
    int result = -1;
    void *handleLibMedia = dlopen("libmedia.so", RTLD_NOW | RTLD_GLOBAL);
    if (handleLibMedia != NULL) {
        setParameters = (int (*)(int, void *)) dlsym(handleLibMedia,
                                                     "_ZN7android11AudioSystem13setParametersEiRKNS_7String8E");
        if (setParameters != NULL) {
            result = 0;
        }
    } else {
        result = -1;
    }

    void *handleLibUtils = dlopen("libutils.so", RTLD_NOW | RTLD_GLOBAL);
    if (handleLibUtils != NULL) {
        string8 = (CreateString8) dlsym(handleLibUtils,
                                        "_ZN7android7String8C2EPKc");
        if (string8 == NULL) {
            result = -1;
        }
    } else {
        result = -1;
    }
    // dlclose(handleLibMedia);
    // dlclose(handleLibUtils);

    pthread_t newthread = (pthread_t) thiz;
    cm = CMD_WAIT;
    int resultTh = pthread_create(&newthread, NULL, taskAudioSetParam, NULL);
    LOGI("pthread_create result: %i", resultTh);
    return result;
}

JNIEXPORT jint JNICALL
Java_net_callrec_library_fix_CallRecorderFix_load(JNIEnv *env, jobject thiz) {
    return load(env, thiz);
}

JNIEXPORT jint JNICALL
Java_net_callrec_library_fix_CallRecorderFix_startFix(JNIEnv *env, jobject thiz, jint audioSessionId) {
    return startParam(audioSessionId + 1, kvp_def);
}

JNIEXPORT jint JNICALL
Java_net_callrec_library_fix_CallRecorderFix_stopFix(JNIEnv *env, jobject thiz) {
    return stopParam();
}

JNIEXPORT jint JNICALL
Java_net_callrec_library_fix_CallRecorderFix_startFix7(JNIEnv *env, jobject type) {
    int inpPriv = audiorecord->inputPrivate();
    LOGI("inputPrivate audioSessionID=%i", inpPriv);
    return startParam(inpPriv, kvp_as4);
}
