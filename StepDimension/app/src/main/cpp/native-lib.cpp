#include "native-lib.hpp"

#include <android/native_activity.h>

// Since we want to get the native activity from SFML, we'll have to use an
// extra header here:
#include <SFML/System/NativeActivity.hpp>

#ifdef __cplusplus
extern "C" {
#endif

std::map<std::string, jobject>* strings = nullptr;
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

jobject newString(JNIEnv *env, const char *string) {
    if (strings == nullptr) {
        strings = new std::map<std::string, jobject>();
    }
    if (strings->find(std::string(string)) == strings->end()) {
        (*strings)[std::string(string)] = env->NewStringUTF(string);
    }
    return (*strings)[std::string(string)];
}

jclass findClass(const char *name) {
    return findClassWithEnv(getEnv(), name);
}

jclass findClassWithEnv(JNIEnv *env, const char *name) {
    return static_cast<jclass>(env->CallObjectMethod(gClassLoader, gFindClassMethod, newString(env, name)));
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

int attach(JavaVM** jvm, JNIEnv** env)
{
    (*env)->GetJavaVM(jvm);

    JNIEnv* myNewEnv; // as the code to run might be in a different thread (connections to signals for example) we will have a 'new one'
    JavaVMAttachArgs jvmArgs;
    jvmArgs.version = JNI_VERSION_1_6;

    int attachedHere = 0; // know if detaching at the end is necessary
    jint res = (*jvm)->GetEnv((void**)&myNewEnv, JNI_VERSION_1_6); // checks if current env needs attaching or it is already attached
    if (JNI_EDETACHED == res) {
        // Supported but not attached yet, needs to call AttachCurrentThread
        res = (*jvm)->AttachCurrentThread(reinterpret_cast<JNIEnv **>(&myNewEnv), &jvmArgs);
        if (JNI_OK == res) {
            attachedHere = 1;
        } else {
            // Failed to attach, cancel
            return attachedHere;
        }
    } else if (JNI_OK == res) {
        // Current thread already attached, do not attach 'again' (just to save the attachedHere flag)
        // We make sure to keep attachedHere = 0
    } else {
        // JNI_EVERSION, specified version is not supported cancel this..
        return attachedHere;
    }

    *env = myNewEnv;

    return attachedHere;
}

int detach(JavaVM* jvm, int attachedHere)
{
    if (attachedHere) { // Key check
        jvm->DetachCurrentThread(); // Done only when attachment was done here
    }
    return EXIT_SUCCESS;
}

int vibrate(sf::Time duration)
{
    // First we'll need the native activity handle
    ANativeActivity *activity = sf::getNativeActivity();

    JavaVM* jvm;
    JNIEnv* env = getEnv();
    int attachedHere = attach(&jvm, &env);

    // Retrieve class information
    jclass natact = findClassWithEnv(env, "com/ssugamejam/stepdimension/SFMLActivity");
    jclass context = findClassWithEnv(env, "com/ssugamejam/stepdimension/SFMLActivity");

    // Get the value of a constant
    jfieldID fid = env->GetStaticFieldID(context, "VIBRATOR_SERVICE", "Ljava/lang/String;");
    jobject svcstr = env->GetStaticObjectField(context, fid);

    // Get the method 'getSystemService' and call it
    jmethodID getss = env->GetMethodID(natact, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
    jobject vib_obj = env->CallObjectMethod(activity->clazz, getss, svcstr);

    // Get the object's class and retrieve the member name
    jclass vib_cls = env->GetObjectClass(vib_obj);
    jmethodID vibrate = env->GetMethodID(vib_cls, "vibrate", "(J)V");

    // Determine the timeframe
    jlong length = duration.asMilliseconds();

    env->CallVoidMethod(vib_obj, vibrate, length);

    // Free references
    env->DeleteLocalRef(vib_obj);
    env->DeleteLocalRef(vib_cls);
    env->DeleteLocalRef(svcstr);
    env->DeleteLocalRef(context);
    env->DeleteLocalRef(natact);

    detach(jvm, attachedHere);

    return EXIT_SUCCESS;
}

int play(const std::string& song)
{
    // First we'll need the native activity handle
    ANativeActivity *activity = sf::getNativeActivity();

    JavaVM* jvm;
    JNIEnv* env = getEnv();
    int attachedHere = attach(&jvm, &env);

    // Retrieve class information
    jclass natact = findClassWithEnv(env, "com/ssugamejam/stepdimension/SFMLActivity");

    // Get the method 'getSelf' and call it
    jmethodID getss = env->GetStaticMethodID(natact, "getSelf", "()Lcom/ssugamejam/stepdimension/SFMLActivity;");
    jobject play_obj = env->CallStaticObjectMethod(natact, getss);

    // Get the object's class and retrieve the member name
    jclass play_cls = env->GetObjectClass(play_obj);
    jmethodID play = env->GetMethodID(play_cls, "play", "(Ljava/lang/String;)V");

    env->CallVoidMethod(play_obj, play, env->NewStringUTF(song.c_str()));

    // Free references
    env->DeleteLocalRef(play_obj);
    env->DeleteLocalRef(play_cls);
    env->DeleteLocalRef(natact);

    detach(jvm, attachedHere);

    return EXIT_SUCCESS;
}

int pause()
{
    // First we'll need the native activity handle
    ANativeActivity *activity = sf::getNativeActivity();

    JavaVM* jvm;
    JNIEnv* env = getEnv();
    int attachedHere = attach(&jvm, &env);

    // Retrieve class information
    jclass natact = findClassWithEnv(env, "com/ssugamejam/stepdimension/SFMLActivity");

    // Get the method 'getSelf' and call it
    jmethodID getss = env->GetStaticMethodID(natact, "getSelf", "()Lcom/ssugamejam/stepdimension/SFMLActivity;");
    jobject pause_obj = env->CallStaticObjectMethod(natact, getss);

    // Get the object's class and retrieve the member name
    jclass pause_cls = env->GetObjectClass(pause_obj);
    jmethodID pause = env->GetMethodID(pause_cls, "pause", "()V");

    env->CallVoidMethod(pause_obj, pause);

    // Free references
    env->DeleteLocalRef(pause_obj);
    env->DeleteLocalRef(pause_cls);
    env->DeleteLocalRef(natact);

    detach(jvm, attachedHere);

    return EXIT_SUCCESS;
}

bool checkFlag(jobject name)
{
    jboolean flag;

    // First we'll need the native activity handle
    ANativeActivity *activity = sf::getNativeActivity();

    JavaVM* jvm;
    JNIEnv* env = getEnv();
    int attachedHere = attach(&jvm, &env);

    // Retrieve class information
    jclass natact = findClassWithEnv(env, "com/ssugamejam/stepdimension/SFMLActivity");

    // Get the method 'getSelf' and call it
    jmethodID getss = env->GetStaticMethodID(natact, "getSelf", "()Lcom/ssugamejam/stepdimension/SFMLActivity;");
    jobject check_obj = env->CallStaticObjectMethod(natact, getss);

    // Get the object's class and retrieve the member name
    jclass check_cls = env->GetObjectClass(check_obj);
    jmethodID check = env->GetMethodID(check_cls, "checkFlag", "(Ljava/lang/Object;)Z");

    flag = env->CallBooleanMethod(check_obj, check, name);

    // Free references
    env->DeleteLocalRef(check_obj);
    env->DeleteLocalRef(check_cls);
    env->DeleteLocalRef(natact);

    detach(jvm, attachedHere);

    return (bool)flag;
}

#ifdef __cplusplus
};
#endif
