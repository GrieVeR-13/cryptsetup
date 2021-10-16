#ifndef EDS3_LUKSOPENVOLUMEPROGRESSINFO_H
#define EDS3_LUKSOPENVOLUMEPROGRESSINFO_H

#include <jni.h>
#include "check.h"

int initLuksOperationCallback(JNIEnv *env);

int reportLuksOperationProgress(jobject luksOperationCallback, float progress);

#define reportLuksOperationProgressWithCheck(LUKSOPERATIONCALLBACK, PROGRESS) \
    CHECK_RES(reportLuksOperationProgress(LUKSOPERATIONCALLBACK, PROGRESS))


#endif //EDS3_LUKSOPENVOLUMEPROGRESSINFO_H
