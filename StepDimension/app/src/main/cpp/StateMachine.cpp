
// Author: Pierce Brooks

#include "StateMachine.hpp"

idc::StateMachine::StateMachine() :
    state(INVALID_STATE)
{
    background = nullptr;
    backgroundTexture = nullptr;
}

idc::StateMachine::~StateMachine()
{
    backStack.clear();
    for (int i = 0; i != elements.size(); ++i)
    {
        delete elements[i];
    }
    elements.clear();
    delete background;
    delete backgroundTexture;
}

int idc::StateMachine::getState() const
{
    return state;
}

void idc::StateMachine::setState(int state)
{
    if (state == INVALID_STATE)
    {
        return;
    }
    if (this->state == state)
    {
        return;
    }
    bool check = false;
    for (int i = backStack.size(); --i >= 0;)
    {
        if (backStack[i] == state)
        {
            check = true;
            while (backStack.size() > i)
            {
                backStack.erase(backStack.begin()+i);
            }
            break;
        }
    }
    if (!check)
    {
        backStack.push_back(this->state);
        this->state = state;
    }
    for (int i = 0; i != backStack.size(); ++i)
    {
        std::cout << "backStack[" << i << "] = " << backStack[i] << std::endl;
    }
    update();
}

void idc::StateMachine::select(int element)
{
    if (element < 0)
    {
        if (!backStack.empty())
        {
            setState(backStack.back());
        }
        return;
    }
    switch (state)
    {
        case LAUNCH_STATE:
            setState(SONGS_STATE);
            break;
        case SONGS_STATE:
            switch (element)
            {
                case 0:
                    setState(SONG_1_STATE);
                    break;
                case 1:
                    setState(SONG_2_STATE);
                    break;
                case 2:
                    setState(SONG_3_STATE);
                    break;
                case 3:
                    setState(LAUNCH_STATE);
                    break;
            }
            break;
        case SONG_1_STATE:
        case SONG_2_STATE:
        case SONG_3_STATE:
            switch (element)
            {
                case 0:
                    setState(VICTORY_STATE);
                    break;
                case 1:
                    setState(LOSS_STATE);
                    break;
                case 2:
                    setState(LAUNCH_STATE);
                    break;
            }
            break;
        case LOSS_STATE:
        case VICTORY_STATE:
            setState(LAUNCH_STATE);
            break;
    }
}

void idc::StateMachine::back()
{
    select(-1);
}

void idc::StateMachine::render(sf::RenderWindow* window)
{
    if (window == nullptr)
    {
        return;
    }
    if (background != nullptr)
    {
        background->setOrigin(sf::Vector2f(backgroundTexture->getSize())*0.5f);
        background->setPosition(sf::Vector2f(window->getSize())*0.5f);
        window->draw(*background);
    }
    switch (state)
    {
        case LAUNCH_STATE:
            break;
    }
    for (int i = 0; i != elements.size(); ++i)
    {
        window->draw(*(elements[i]->visual));
    }
}

void idc::StateMachine::update()
{
    std::cout << "State: " << state << std::endl;
    delete background;
    delete backgroundTexture;
    background = nullptr;
    backgroundTexture = nullptr;
    switch (state)
    {
        case LAUNCH_STATE:
            backgroundTexture = new sf::Texture();
            backgroundTexture->loadFromFile("backgrounds/game_bkg_1.png");
            background = new sf::Sprite();
            background->setTexture(*backgroundTexture);
            break;
    }

}
