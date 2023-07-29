#include <SFML/Graphics.hpp>
#include "structs.hpp"

using namespace sf;

// the namespace is to let you hunt down any debugging functions and remove/comment them out

namespace dbg {
    void printVector3(char* prefix, Vector3f vector);
    void printMultiVector3(Vector3f *vectors, int len, char* separator);

    inline bool triClipColorCode = false;
    inline bool backFaceCulling = true;
    inline int drawnTris = 0;
}