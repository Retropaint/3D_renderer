#include "directives.hpp"
#include "cameraPlanes.hpp"

Vector3f planeBottom;
Vector3f planeTop;
Vector3f planeLeft;
Vector3f planeRight;

void setFOV(float fov, Camera *camera) {
    camera->near = (180-fov)/180 * screenWidth;

    float rad = (180-fov)/2 * M_PI/180;
    planeBottom = Vector3f(0, -sin(rad), cos(rad));
    planeTop = Vector3f(0, sin(rad), cos(rad));
    planeLeft = Vector3f(sin(rad), 0, cos(rad));
    planeRight = Vector3f(-sin(rad), 0, cos(rad));
}