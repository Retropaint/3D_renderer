#include <cmath>
#include <iostream>
#include <iomanip>
#include "cameraPlanes/cameraPlanes.hpp"
#include "debugger/debugger.hpp"
#include "externs.hpp"
#include "directives.hpp"

using namespace sf;
using namespace std;

float near = 5;
float far = 1000;

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

int clip(Vector3f* verts, Vector3f* points, Vector3f planeNormal, int offset) {
    int pointLen = 0;
    int inVertLen = 0;
    int outVertLen = 0;
    Vector3f inVerts[3];
    Vector3f outVerts[3];

    // identify which verts are in or out of plane
    for(int i = 0; i < 3; i++) {
        float distance = planeNormalDist(verts[i], planeNormal, offset);

        if(distance > 0) {
            points[pointLen++] = verts[i];
            inVerts[inVertLen++] = verts[i];
        } else {
            outVerts[outVertLen++] = verts[i];
        }
    }

    // get points of intersections from in to out verts
    for(int i = 0; i < inVertLen; i++) {
        for(int j = 0; j < outVertLen; j++) {
            if(offset == near) {
                offset = -near;
            }
            points[pointLen++] = linePlaneIntersection(inVerts[i], outVerts[j], planeNormal, offset);
        }
    }

    return pointLen;
}

void clipTriangle(Triangle mainTri, Triangle *tris, int *tLen) {
    tris[0] = mainTri;
    *tLen = 1;

    int planeLen = 6;
    Vector3f planes[planeLen] = { planeNear, planeFar, planeLeft, planeRight, planeTop, planeBottom };
    int offset[planeLen] = { near, far, 0, 0, 0, 0 }; 

    for(int p = 0; p < planeLen; p++) {
        for(int t = 0; t < *tLen; t++) {
            Triangle tri = tris[t];

            Vector3f points[4];
            int pointLen = clip(tri.verts, points, planes[p], offset[p]);
            
            Triangle newTri;
            newTri.verts[0] = points[0];
            newTri.verts[1] = points[1];
            newTri.verts[2] = points[2];
            tris[t] = newTri;

            if(pointLen == 4) {
                Triangle newTri2;
                newTri2.verts[0] = points[1];
                newTri2.verts[1] = points[2];
                newTri2.verts[2] = points[3];
                tris[*tLen] = newTri2;
                (*tLen)++;
                
                // only 1 triangle ever intersects at any moment
                break;
            }
        }
    }
}