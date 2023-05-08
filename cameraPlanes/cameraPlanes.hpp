#include <SFML/Graphics.hpp>
#include <math.h>
#include "structs.hpp"

using namespace sf;

inline const Vector3f planeNear = Vector3f(0, 0, 1); // technically unused
inline const Vector3f planeFar = Vector3f(0, 0, -1); // technically unused
extern Vector3f planeBottom;
extern Vector3f planeTop;
extern Vector3f planeLeft;
extern Vector3f planeRight;

void setFOV(float fov, Camera *camera);