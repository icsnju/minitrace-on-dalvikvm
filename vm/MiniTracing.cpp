#include "Dalvik.h"


bool mtStart() {
    char tempNameBuf[sizeof("/data/mini_trace_10032_config.in")];
    sprintf(tempNameBuf, "/data/mini_trace_%d_config.in", getuid());

    FILE* fp = fopen(tempNameBuf, "r");
    if (fp == NULL) {
        ALOGI("MiniTracing: Fail to open %s", tempNameBuf);
        return false;
    }

    ALOGI("MiniTracing: Succeed to open %s", tempNameBuf);

    mtDumpCoverage(true);
    gDvm.minitracingActive = true;
    return true;
}


void mtPrintMethodCoverageData(void* fp, const ClassObject* clazz, Method* method)
{
    char* desc;
    const DexCode* pCode;
    u4 insnsSize;
    u1* coverageData;
    u4* p;
    u4 len;
    u4 i;
    bool visited;
    FILE* f = (FILE*)fp;

    if (!IS_METHOD_FLAG_SET(method, METHOD_ISMINITRACEABLE)) {
        return;
    }

    coverageData = method->insnsCoverageData;
    if (coverageData == NULL) {
        return;
    }

    pCode = dvmGetMethodCode(method);
    insnsSize = pCode->insnsSize;

    if (insnsSize == 0) {
        return;
    }

    if (coverageData[0] == 0) {
        visited = false;
        len = insnsSize >> 2;
        p = (u4*) coverageData;
        for (i = 0; i <= len; i++) {
            if (p[i] != 0) {
                visited = true;
                break;
            }
        }
        if (!visited) {
            return;
        }
    }

    desc = dexProtoCopyMethodDescriptor(&method->prototype);
    fprintf(f, "%p\t%s\t%s\t%s\t\t%d\t", method, clazz->descriptor, method->name, desc, insnsSize);
    free(desc);

    for (i = 0; i < insnsSize; i++) {
        if (coverageData[i] == 1) {
            fprintf(f, "1");
            coverageData[i] = 0;
        } else if (coverageData[i] == 0) {
            fprintf(f, "0");
        } else {
            fprintf(f, "x");
        }
    }

    fprintf(f, "\n");
}

s8 mtGetNowMsec()
{
#ifdef HAVE_POSIX_CLOCKS
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return now.tv_sec * 1000LL + now.tv_nsec / 1000000LL;
#else
    struct timeval now;
    gettimeofday(&now, NULL);
    return now.tv_sec * 1000LL + now.tv_usec / 1000LL;
#endif
}

void mtDumpCoverage(bool start)
{
  char tempNameBuf[sizeof("/data/mini_trace_10032_coverage.dat")];
  sprintf(tempNameBuf, "/data/mini_trace_%d_coverage.dat", getuid());

  FILE* fp = fopen(tempNameBuf, "a");
  if (fp == NULL) {
    ALOGI("MiniTracing: Fail dump coverage into %s", tempNameBuf);
    return;
  }

  if (start) {
    ALOGI("MiniTracing: Begin start coverage into %s", tempNameBuf);
    fprintf(fp, "Start\t%d\t%lld\n", getpid(), mtGetNowMsec());
    fclose(fp);
    ALOGI("MiniTracing: End start coverage");
  } else {
    ALOGI("MiniTracing: Begin dump coverage into %s", tempNameBuf);
    fprintf(fp, "Dump\t%d\t%lld\n", getpid(), mtGetNowMsec());
    mtDumpAllClassesCoverage(fp);
    fclose(fp);
    ALOGI("MiniTracing: End dump coverage");
  }
}

void mtPostClassPrepare(ClassObject* clazz)
{
    int i;

    if (clazz == NULL) {
        return;
    }

    if (clazz->pDvmDex == NULL) {
        return;
    }

    if (!clazz->pDvmDex->isMiniTraceable) {
        return;
    }

    if (IS_CLASS_FLAG_SET(clazz, CLASS_ISMINITRACEABLE)) {
        return;
    }

    SET_CLASS_FLAG(clazz, CLASS_ISMINITRACEABLE);

    if (!dvmIsInterfaceClass(clazz)) {
        for (i = 0; i < clazz->virtualMethodCount; i++) {
            SET_METHOD_FLAG(&clazz->virtualMethods[i], METHOD_ISMINITRACEABLE);
        }
        for (i = 0; i < clazz->directMethodCount; i++) {
            SET_METHOD_FLAG(&clazz->directMethods[i], METHOD_ISMINITRACEABLE);
        }
    }
}
