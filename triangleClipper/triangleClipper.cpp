#include <cmath>
#include <iostream>
#include <iomanip>
#include "cameraPlanes.hpp"
#include "debugger/debugger.hpp"
#include "externs.hpp"

using namespace sf;
using namespace std;

float near = 5;
float far = 30;

float planeNormalDist(Vector3f point, Vector3f planeNormal, float offset) {
    if(planeNormal == planeNear) {
        return point.z - offset;
    } else if(planeNormal == planeFar) {
        return -(point.z) + offset;
    }
    return planeNormal.x * point.x + planeNormal.y * point.y + planeNormal.z * point.z;
}

Vector3f linePlaneIntersection(Vector3f p1, Vector3f p2, Vector3f planeNormal, float offset) {
    float interp = (offset - (planeNormalDist(p1, planeNormal, 0))) / planeNormalDist(p2 - p1, planeNormal, 0);
    return p1 + (p2 - p1) * interp;
}

bool isFullyCulled(Vector3f* verts) {
    for(int i = 0; i < 3; i++) {
        if(
            planeNormalDist(verts[i], planeNear, near) > 0 &&
            planeNormalDist(verts[i], planeRight, 0) > 0 &&
            planeNormalDist(verts[i], planeLeft, 0) > 0 &&
            planeNormalDist(verts[i], planeTop, 0) > 0 &&
            planeNormalDist(verts[i], planeBottom, 0) > 0
        ) {
            return false;
        }
    }

    for(int i = 0; i < 3; i++) {
        for(int j = 0; j < 3; j++) {
            if(i == j) {
                continue;
            }
            if(nearCamPlaneInt(verts[i]) != nearCamPlaneInt(verts[j])) {
                return false;
            }
        }
    }

    return true;
}

int clip(Vector3f* verts, Vector3f* points) {
    int pLen = 0;

    float distances[3];

    for(int i = 0; i < 3; i++) {
        distances[i] = planeNormalDist(verts[i], planeNear, near);
        if(distances[i] > 0) {
            points[pLen++] = verts[i];
        }
    }

    for(int i = 0; i < 3; i++) {
        for(int j = 0; j < 3; j++) {

            // don't check a vertex against itself
            if(i == j) {
                continue;
            }

            if(distances[i] > 0 && distances[j] < 0) {
                points[pLen++] = linePlaneIntersection(verts[i], verts[j], planeNear, near);
            }
        }
    }

    return pLen;
}

bool isFullyInView(Vector3f verts[3]) {
    for(int i = 0; i < 3; i++) {
        if(
            planeNormalDist(verts[i], planeNear, near) < -0.01
        ) {
            return false;
        }
    }

    return true;
}

void clipTriangle(Triangle mainTri, Triangle *tris, int *tLen) {
    tris[0] = mainTri;
    *tLen = 1;

    Vector3f planes[2] = { planeNear, planeFar };

    for(int t = offset; t < *tLen; t++) {
        Triangle tri = tris[t];

        if(isFullyInView(tri.verts)) {
            offset++;
            continue;
        }

        Vector3f points[4];
        int pLen = clip(tri.verts, points);
        
        tris[(*tLen) - 1].verts[0] = points[0];
        tris[(*tLen) - 1].verts[1] = points[1];
        tris[(*tLen) - 1].verts[2] = points[2];
        
        if(pLen == 4) {
            tris[*tLen].verts[0] = points[1];
            tris[*tLen].verts[1] = points[2];
            tris[*tLen].verts[2] = points[3];
            (*tLen)++;
        }
    }
}