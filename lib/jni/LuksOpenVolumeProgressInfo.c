#include "cutil/jniutil.h"
#include "check.h"
#include "LuksOpenVolumeProgressInfo.h"

static jmethodID syncProgressMethodId;
static jmethodID isCancelledMethodId;

//#define CHECK

int initLuksOperationCallback(JNIEnv *env) {
    jclass cls = CHECK((*env)->FindClass(env, "com/sovworks/projecteds3/data/common/junction/luks/LuksImpl$LuksOperationCallback"));
    syncProgressMethodId = CHECK((*env)->GetMethodID(env, cls, "syncProgress", "(F)I"));
    (*env)->DeleteLocalRef(env, cls);
}

int reportLuksOperationProgress(jobject luksOperationCallback, float progress) {
    if (luksOperationCallback != NULL) {
        JNIEnv *env = c_get_env();
        int res;
        int r = call_jni_int_func(env, luksOperationCallback, syncProgressMethodId, &res,  progress);
        if (r < 0)
            return r;
        return res;
    }
    return 0;
}
