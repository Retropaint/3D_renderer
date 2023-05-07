#include <SFML/Graphics.hpp>
#include "directives.hpp"
#include "structs.hpp"

using namespace sf;

#ifndef EXTERNS
    #define EXTERNS 1
    inline float depthZ[screenWidth][screenHeight];
    inline Color depthColor[screenWidth][screenHeight];
    
    extern RenderWindow window;
    extern Camera camera;
    extern RectangleShape pixel;
#endif