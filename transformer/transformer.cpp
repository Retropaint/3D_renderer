#include <math.h>
#include "transformer/transformer.hpp"
#include "directives.hpp"

Vector3f worldToScreenPos(Vector3f worldPoint, Camera camera) {

    // on init, all pixels should be centered to the screen
    Vector3f screenPoint = Vector3f(screenWidth/2, screenHeight/2, worldPoint.z);

    if(worldPoint.z == 0) {
        worldPoint.z = 0.01;
    }

    screenPoint.x += camera.near * worldPoint.x / worldPoint.z;
    screenPoint.y += camera.near * worldPoint.y / worldPoint.z;
    
    return screenPoint;
}

Vector3f worldToLocalCameraPos(Vector3f worldPoint, Camera camera) {
    
    // position world points with respect to camera
    worldPoint -= camera.pos;

    worldPoint = rotateY(worldPoint, camera.angle.y);
    worldPoint = rotateX(worldPoint, camera.angle.x);

    return worldPoint;
}

Vector3f rotateX(Vector3f point, float angle) {
    Vector3f newPoint = point;

    newPoint.y = point.y * cos(angle) + point.z * -sin(angle);
    newPoint.z = point.y * sin(angle) + point.z * cos(angle);

    return newPoint;
}

Vector3f rotateY(Vector3f point, float angle) {
    Vector3f newPoint = point;

    newPoint.x = point.x * cos(angle) + point.z * sin(angle);
    newPoint.z = point.x * -sin(angle) + point.z * cos(angle);

    return newPoint;
}

Vector3f rotateZ(Vector3f point, float angle) {
    Vector3f newPoint = point;

    newPoint.x = point.x * cos(angle) + point.y * -sin(angle);
    newPoint.y = point.x * sin(angle) + point.y * cos(angle);

    return newPoint;
}

float clampAngle(float angle) {
    if(angle > M_PI/2) {
        angle = M_PI/2;
    } else if(angle < -M_PI/2) {
        angle = -M_PI/2;
    }

    return angle;
}