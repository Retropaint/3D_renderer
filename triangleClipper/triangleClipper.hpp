#include <SFML/Graphics.hpp>
#include "structs.hpp"

using namespace sf;

bool isFullyCulled(Vector3f* verts);
void clipTriangle(Triangle mainTri, Triangle *tris, int *tLen, int *offset);
int nearClip(Vector3f* verts, Vector3f* points);
bool isFullyInView(Vector3f verts[3]);