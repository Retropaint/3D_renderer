#include <cmath>
#include <iostream>
#include <iomanip>
#include "cameraPlanes.hpp"
#include "debugger/debugger.hpp"
#include "externs.hpp"

using namespace sf;
using namespace std;

float near = 5;
float far = 1000;
float cullTolerance = 50;

float planeNormalDist(Vector3f point, Vector3f planeNormal, float offset) {
    if(planeNormal == planeNear) {
        return point.z - offset;
    } else if(planeNormal == planeFar) {
        return -point.z + offset;
    }
    return planeNormal.x * point.x + planeNormal.y * point.y + planeNormal.z * point.z;
}

Vector3f linePlaneIntersection(Vector3f p1, Vector3f p2, Vector3f planeNormal, float offset) {
    float interp = (-offset - (planeNormalDist(p1, planeNormal, 0))) / planeNormalDist(p2 - p1, planeNormal, 0);
    return p1 + (p2 - p1) * interp;
}

bool isFullyCulled(Vector3f* verts) {
    for(int i = 0; i < 3; i++) {
        if(
            planeNormalDist(verts[i], planeNear, near) > -cullTolerance &&
            planeNormalDist(verts[i], planeFar, far) > -cullTolerance &&
            planeNormalDist(verts[i], planeTop, 0) > -cullTolerance &&
            planeNormalDist(verts[i], planeBottom, 0) > -cullTolerance &&
            planeNormalDist(verts[i], planeLeft, 0) > -cullTolerance &&
            planeNormalDist(verts[i], planeRight, 0) > -cullTolerance
        ) {
            return false;
        }
    }

    return true;
}

bool isFullyInView(Vector3f verts[3]) {
    for(int i = 0; i < 3; i++) {
        if(
            planeNormalDist(verts[i], planeNear, near) < -0 ||
            planeNormalDist(verts[i], planeFar, far) < -0 ||
            planeNormalDist(verts[i], planeLeft, 0) < -0 ||
            planeNormalDist(verts[i], planeRight, 0) < -0 ||
            planeNormalDist(verts[i], planeBottom, 0) < -0 ||
            planeNormalDist(verts[i], planeTop, 0) < -0
        ) {
            return false;
        }
    }

    return true;
}

int clip(Vector3f* verts, Vector3f* points, Vector3f plane) {
    int pLen = 0;
    int offset = 0;

    if(plane == planeNear) {
        offset = near;
    } else if(plane == planeFar) {
        offset = far;
    }

    float distances[3];

    for(int i = 0; i < 3; i++) {
        distances[i] = planeNormalDist(verts[i], plane, offset);

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
                if(plane == planeFar) {
                    points[pLen++] = linePlaneIntersection(verts[i], verts[j], plane, far);
                } else if(plane == planeNear) {
                    points[pLen++] = linePlaneIntersection(verts[i], verts[j], plane, -near);
                } else {
                    points[pLen++] = linePlaneIntersection(verts[i], verts[j], plane, 0);
                }
            }
        }
    }

    return pLen;
}

void clipTriangle(Triangle mainTri, Triangle *tris, int *tLen, int *offset) {
    tris[0] = mainTri;
    *tLen = 1;
    
    int planeLen = 6;
    Vector3f planes[planeLen] = { planeNear, planeFar, planeLeft, planeRight, planeTop, planeBottom };
    float offsets[planeLen] = { near, far, 0, 0, 0, 0 };

    for(int p = 0; p < planeLen; p++) {
        int currLen = *tLen;
        
        for(int t = 0; t < currLen; t++) {
            Triangle tri = tris[t];

            Vector3f points[4];
            int pLen = clip(tri.verts, points, planes[p]);

            if(pLen == 2) {
                continue;
            }

            Triangle newTri;
            newTri.verts[0] = points[0];
            newTri.verts[1] = points[1];
            newTri.verts[2] = points[2];
            tris[t] = newTri;

            if(pLen == 4) {
                Triangle newTri2;
                newTri2.verts[0] = points[1];
                newTri2.verts[1] = points[2];
                newTri2.verts[2] = points[3];
                tris[*tLen] = newTri2;
                (*tLen)++;

            }
        }
    }
}