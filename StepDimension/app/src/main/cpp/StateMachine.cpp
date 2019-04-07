
// Author: Pierce Brooks

#include "StateMachine.hpp"

idc::StateMachine::Element::Element(StateMachine* owner, const std::string& path, const std::string& text) :
    visual(nullptr),
    texture(new sf::Texture()),
    label(new sf::Text())
{
    sf::FloatRect bounds;
    if (texture->loadFromFile(path))
    {
        visual = new sf::Sprite();
        visual->setTexture(*texture);
        visual->setOrigin(sf::Vector2f(texture->getSize())*0.5f);
    }
    else
    {
        delete texture;
        texture = nullptr;
    }
    label->setFont(*(owner->getFont()));
    label->setString(sf::String(text));
    bounds = label->getLocalBounds();
    label->setOrigin(sf::Vector2f(bounds.width, bounds.height)*0.5f);
    label->setFillColor(sf::Color::Black);
}

idc::StateMachine::Element::~Element()
{
    delete visual;
    delete texture;
    delete label;
}

idc::StateMachine::StateMachine() :
    state(INVALID_STATE),
    background(nullptr),
    backgroundTexture(nullptr),
    font(new sf::Font()),
    backer(new sf::RectangleShape())
{
    font->loadFromFile("sansation.ttf");
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

sf::Font* idc::StateMachine::getFont() const
{
    return font;
}

int idc::StateMachine::getState() const
{
    return state;
}

bool idc::StateMachine::setState(int state)
{
    if (state == INVALID_STATE)
    {
        LOGGER << "Invalid state!" << "\n";
        return false;
    }
    if (this->state == state)
    {
        LOGGER << "Same state!" << "\n";
        return false;
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
        if (this->state != INVALID_STATE)
        {
            backStack.push_back(this->state);
        }
        this->state = state;
    }
    else
    {
        LOGGER << "Used backStack!" << "\n";
    }
    for (int i = 0; i != backStack.size(); ++i)
    {
        LOGGER << "backStack[" << i << "] = " << backStack[i] << "\n";
    }
    update();
    return true;
}

void idc::StateMachine::select(int element)
{
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

void idc::StateMachine::select(sf::RenderWindow* window, const sf::Vector2f& cursor)
{
    if (!backer->getGlobalBounds().contains(cursor))
    {
        float height;
        float position;
        sf::Vector2f bounds = sf::Vector2f(window->getSize());
        if ((cursor.x > (bounds.x*0.5f)-(bounds.x*0.125f)) && (cursor.x < (bounds.x*0.5f)+(bounds.x*0.125f)))
        {
            for (int i = 0; i != elements.size(); ++i)
            {
                height = static_cast<float>(elements[i]->texture->getSize().y);
                position = getElementPosition(static_cast<float>(window->getSize().y), height, i, elements.size());
                if ((cursor.y > position-(height*0.5f)) && (cursor.y < position+(height*0.5f)))
                {
                    select(i);
                    return;
                }
            }
        }
        select(-1);
        return;
    }
    back();
}

void idc::StateMachine::back()
{
    if (!backStack.empty())
    {
        setState(backStack.back());
    }
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
        LOG(i);
        elements[i]->visual->setPosition(static_cast<float>(window->getSize().x)*0.5f, getElementPosition(static_cast<float>(window->getSize().y), static_cast<float>(elements[i]->texture->getSize().y), i, elements.size()));
        window->draw(*(elements[i]->visual));
        elements[i]->label->setPosition(elements[i]->visual->getPosition());
        window->draw(*(elements[i]->label));
    }
    backer->setPosition(0.0f, 0.0f);
    backer->setOrigin(0.0f, 0.0f);
    backer->setSize(sf::Vector2f(window->getSize())*0.125f);
    backer->setFillColor(sf::Color::Green);
    window->draw(*backer);
}

void idc::StateMachine::update()
{
    LOGGER << "State: " << state << "\n";
    delete background;
    delete backgroundTexture;
    background = nullptr;
    backgroundTexture = nullptr;
    for (int i = 0; i != elements.size(); ++i)
    {
        delete elements[i];
    }
    elements.clear();
    switch (state)
    {
        case LAUNCH_STATE:
            backgroundTexture = new sf::Texture();
            if (backgroundTexture->loadFromFile("backgrounds/game_bkg_1.png"))
            {
                background = new sf::Sprite();
                background->setTexture(*backgroundTexture);
            }
            elements.push_back(new Element(this, "image.png", "GO!"));
            break;
    }
    if ((backgroundTexture != nullptr) != (background != nullptr))
    {
        delete background;
        delete backgroundTexture;
        background = nullptr;
        backgroundTexture = nullptr;
    }
}

float idc::StateMachine::getElementPosition(float bounds, float height, int index, int total)
{
    return ((height*static_cast<float>(index))-(height*static_cast<float>(total)*0.5f))*1.5f;
}
