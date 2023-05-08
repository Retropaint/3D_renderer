#include <SFML/Graphics.hpp>
#include "structs.hpp"

using namespace sf;

bool isFullyCulled(Vector3f* verts);
void clipTriangle(Triangle mainTri, Triangle *tris, int *tLen);
bool isFullyInView(Vector3f verts[3]);