#include <SFML/Graphics.hpp>
#include "structs.hpp"

Vector3f worldToScreenPos(Vector3f worldPoint, struct Camera camera);
Vector3f worldToLocalCameraPos(Vector3f worldPoint, Camera camera);
Vector3f rotateX(Vector3f point, float angle);
Vector3f rotateY(Vector3f point, float angle);
Vector3f rotateZ(Vector3f point, float angle);