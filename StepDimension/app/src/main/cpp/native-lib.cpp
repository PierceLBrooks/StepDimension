#include "native-lib.hpp"

#ifdef __cplusplus
extern "C" {
#endif

JavaVM *gJvm = nullptr;
static jobject gClassLoader;
static jmethodID gFindClassMethod;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *pjvm, void *reserved) {
    gJvm = pjvm;  // cache the JavaVM pointer
    auto env = getEnv();
    //replace with one of your classes in the line below
    auto randomClass = env->FindClass("com/ssugamejam/stepdimension/SFMLActivity");
    jclass classClass = env->GetObjectClass(randomClass);
    auto classLoaderClass = env->FindClass("java/lang/ClassLoader");
    auto getClassLoaderMethod = env->GetMethodID(classClass, "getClassLoader", "()Ljava/lang/ClassLoader;");
    gClassLoader = env->NewGlobalRef(env->CallObjectMethod(randomClass, getClassLoaderMethod));
    gFindClassMethod = env->GetMethodID(classLoaderClass, "findClass", "(Ljava/lang/String;)Ljava/lang/Class;");

    //gJvm->DetachCurrentThread();

    return JNI_VERSION_1_6;
}

jclass findClass(const char *name) {
    return findClassWithEnv(getEnv(), name);
}

jclass findClassWithEnv(JNIEnv *env, const char *name) {
    return static_cast<jclass>(env->CallObjectMethod(gClassLoader, gFindClassMethod, env->NewStringUTF(name)));
}

JNIEnv *getEnv() {
    JNIEnv *env;
    int status = gJvm->GetEnv((void **) &env, JNI_VERSION_1_6);
    if (status < 0) {
        status = gJvm->AttachCurrentThread(&env, NULL);
        if (status < 0) {
            return nullptr;
        }
    }
    return env;
}

JavaVM *getJvm() {
    return gJvm;
}

JNIEXPORT jstring JNICALL Java_com_ssugamejam_stepdimension_SFMLActivity_stringFromJNI(JNIEnv *env, jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

#ifdef __cplusplus
};
#endif
