
// Author: Pierce Brooks

#ifndef STATE_MACHINE_HPP
#define STATE_MACHINE_HPP

#include "native-lib.hpp"

#define INVALID_STATE -1
#define LAUNCH_STATE 0
#define SONGS_STATE 1
#define SONG_1_STATE 2
#define SONG_2_STATE 3
#define SONG_3_STATE 4
#define VICTORY_STATE 5
#define LOSS_STATE 6

namespace idc
{
    class StateMachine
    {
        public:
            class Element
            {
                public:
                    Element(StateMachine* owner, const std::string& path, const std::string& text);
                    virtual ~Element();
                    sf::Sprite* visual;
                    sf::Texture* texture;
                    sf::Text* label;
            };
            StateMachine();
            virtual ~StateMachine();
            sf::Font* getFont() const;
            int getState() const;
            bool setState(int state);
            void select(int element);
            void select(sf::RenderWindow* window, const sf::Vector2f& cursor);
            void back();
            void render(sf::RenderWindow* window);
        private:
            void update();
            static float getElementPosition(float bounds, float height, int index, int total);
            int state;
            std::vector<int> backStack;
            std::vector<Element*> elements;
            sf::Sprite* background;
            sf::Texture* backgroundTexture;
            sf::Font* font;
            sf::RectangleShape* backer;
    };
}

#endif
