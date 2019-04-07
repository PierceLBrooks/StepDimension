#include "native-lib.hpp"
#include "StateMachine.hpp"

int main(int argc, char *argv[])
{
    idc::StateMachine* menu = new idc::StateMachine();
    LOGGER << "State: " << menu->setState(LAUNCH_STATE) << "\n";

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

    sf::View view = window.getDefaultView();

    sf::Color background = sf::Color::White;

    // We shouldn't try drawing to the screen while in background
    // so we'll have to track that. You can do minor background
    // work, but keep battery life in mind.
    bool active = true;

    jobject backFlag = env->NewStringUTF("BACK");

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
                        vibrate(sf::milliseconds(10));
                        menu->select(&window, sf::Vector2f(sf::Vector2i(event.touch.x, event.touch.y)));
                    }
                    break;
            }
        }

        if (active) {
            window.clear(background);
            menu->render(&window);
            window.display();
        } else {
            sf::sleep(sf::milliseconds(100));
        }

        if (checkFlag(backFlag)) {
            menu->back();
        }
    }

    // Detach thread again
    vm->DetachCurrentThread();

    delete menu;

    return EXIT_SUCCESS;
}
