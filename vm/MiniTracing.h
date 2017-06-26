#ifndef DALVIK_MINITRACING_H_
#define DALVIK_MINITRACING_H_

struct ClassObject;
struct Method;

bool mtStart();


void mtPrintMethodCoverageData(void* fp, const ClassObject* clazz, Method* method);

void mtDumpCoverage(bool start=false);

void mtPostClassPrepare(ClassObject* clazz);

#endif
