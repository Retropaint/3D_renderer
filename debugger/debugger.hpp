#include <SFML/Graphics.hpp>
#include "structs.hpp"

using namespace sf;

// the namespace is to let you hunt down any debugging functions and remove/comment them out

namespace dbg {
    void printVector3(char* prefix, Vector3f vector);
    void printMultiVector3(Vector3f *vectors, int len, char* separator);
    void drawTriClipPoints(Vector3f *points, int pLen, Camera camera, RenderWindow *window, bool shouldPrint, bool isAlreadyScreenPos);
}