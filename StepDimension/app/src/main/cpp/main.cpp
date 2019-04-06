#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Network.hpp>

// These headers are only needed for direct NDK/JDK interaction
#include <jni.h>
#include <android/native_activity.h>

// Since we want to get the native activity from SFML, we'll have to use an
// extra header here:
#include <SFML/System/NativeActivity.hpp>

#include "native-lib.hpp"

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
    
    // Bzzz!
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

// This is the actual Android example. You don't have to write any platform
// specific code, unless you want to use things not directly exposed.
// ('vibrate()' in this example; undefine 'USE_JNI' above to disable it)
int main(int argc, char *argv[])
{
    // Retrieve the JVM
    JavaVM* vm = getJvm();

    // Retrieve the JNI environment
    JNIEnv* env = getEnv();

    jint res = vm->AttachCurrentThread(&env, NULL);

    if (res == JNI_ERR)
        return EXIT_FAILURE;

    sf::VideoMode screen(sf::VideoMode::getDesktopMode());

    sf::RenderWindow window(screen, "");
    window.setFramerateLimit(30);

    sf::Texture texture;
    if(!texture.loadFromFile("image.png"))
        return EXIT_FAILURE;

    sf::Sprite image(texture);
    image.setPosition(screen.width / 2, screen.height / 2);
    image.setOrigin(texture.getSize().x/2, texture.getSize().y/2);

    sf::Font font;
    if (!font.loadFromFile("sansation.ttf"))
        return EXIT_FAILURE;

    sf::Text text("Tap anywhere to move the logo.", font, 64);
    text.setFillColor(sf::Color::Black);
    text.setPosition(10, 10);

    sf::Music music;
    if(!music.openFromFile("canary.wav"))
        return EXIT_FAILURE;

    music.play();

    sf::View view = window.getDefaultView();

    sf::Color background = sf::Color::White;

    // We shouldn't try drawing to the screen while in background
    // so we'll have to track that. You can do minor background
    // work, but keep battery life in mind.
    bool active = true;

    while (window.isOpen())
    {
        sf::Event event;

        while (active ? window.pollEvent(event) : window.waitEvent(event))
        {
            switch (event.type)
            {
                case sf::Event::Closed:
                    window.close();
                    break;
                case sf::Event::KeyPressed:
                    if (event.key.code == sf::Keyboard::Escape)
                        window.close();
                    break;
                case sf::Event::Resized:
                    view.setSize(event.size.width, event.size.height);
                    view.setCenter(event.size.width/2, event.size.height/2);
                    window.setView(view);
                    break;
                case sf::Event::LostFocus:
                    background = sf::Color::Black;
                    break;
                case sf::Event::GainedFocus:
                    background = sf::Color::White;
                    break;
                
                // On Android MouseLeft/MouseEntered are (for now) triggered,
                // whenever the app loses or gains focus.
                case sf::Event::MouseLeft:
                    active = false;
                    break;
                case sf::Event::MouseEntered:
                    active = true;
                    break;
                case sf::Event::TouchBegan:
                    if (event.touch.finger == 0)
                    {
                        image.setPosition(event.touch.x, event.touch.y);
                        vibrate(sf::milliseconds(10));
                    }
                    break;
            }
        }

        if (active)
        {
            window.clear(background);
            window.draw(image);
            window.draw(text);
            window.display();
        }
        else {
            sf::sleep(sf::milliseconds(100));
        }
    }

    // Detach thread again
    vm->DetachCurrentThread();

    return EXIT_SUCCESS;
}
