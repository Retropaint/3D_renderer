#include <SFML/Graphics.hpp>
#include <math.h>

using namespace sf;

inline const Vector3f planeNear = Vector3f(0, 0, 1); // technically unused
inline const Vector3f planeFar = Vector3f(0, 0, -1); // technically unused

inline Vector3f planeBottom = Vector3f(0, -1/sqrt(2), 1/sqrt(2));
inline Vector3f planeTop = Vector3f(0, 1/sqrt(2), 1/sqrt(2));
inline Vector3f planeLeft = Vector3f(1/sqrt(2), 0, 1/sqrt(2));
inline Vector3f planeRight = Vector3f(-1/sqrt(2), 0, 1/sqrt(2));
