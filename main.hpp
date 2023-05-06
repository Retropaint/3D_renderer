#include <SFML/Graphics.hpp>
#include "structs.hpp"

using namespace sf;
using namespace std;

void renderLine(Vector3f p1, Vector3f p2, Color color, bool world);
void playerMove(Vector3f *pos, float angle, float speed);
void initTriangle(Triangle *tri, Vector3f triTemplate[3]);
