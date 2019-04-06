
#ifndef NATIVE_LIB_HPP
#define NATIVE_LIB_HPP

#include <jni.h>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

jclass findClass(const char *name);
jclass findClassWithEnv(JNIEnv *env, const char *name);
JNIEnv *getEnv();
JavaVM *getJvm();

#ifdef __cplusplus
};
#endif

#endif