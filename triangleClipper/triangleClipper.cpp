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

float lineToPlaneIntersectInterp(Vector3f p1, Vector3f p2, Vector3f planeNormal, float offset) {
    return (-offset - (planeNormalDist(p1, planeNormal, 0))) / planeNormalDist(p2 - p1, planeNormal, 0);
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

int clip(Triangle tri, Vector3f* points, Vector2f *texels, Vector3f planeNormal, int offset) {
    int pointLen = 0;
    int inVertLen = 0;
    int outVertLen = 0;
    int ivIdx[3];
    int ovIdx[3];
    
    // identify which verts are in or out of plane
    for(int i = 0; i < 3; i++) {
        float distance = planeNormalDist(tri.verts[i], planeNormal, offset);

        if(distance > 0) {
            points[pointLen] = tri.verts[i];
            texels[pointLen++] = tri.texels[i];
            ivIdx[inVertLen++] = i;
        } else {
            ovIdx[outVertLen++] = i;
        }
    }

    // get points and texels of intersections from in to out verts
    for(int i = 0; i < inVertLen; i++) {
        for(int j = 0; j < outVertLen; j++) {
            if(offset == near) {
                offset = -near;
            }
            float interp = lineToPlaneIntersectInterp(tri.verts[ivIdx[i]], tri.verts[ovIdx[j]], planeNormal, offset);

            Vector3f iv = tri.verts[ivIdx[i]];
            points[pointLen] = iv + (tri.verts[ovIdx[j]] - iv) * interp;
            
            Vector2f it = tri.texels[ivIdx[i]];
            texels[pointLen++] = it + (tri.texels[ovIdx[j]] - it) * interp;
        }
    }

    return pointLen;
}

void clipTriangle(Triangle mainTri, Triangle *tris, int *tLen) {
    tris[0] = mainTri;
    *tLen = 1;

	int planeLen = 6;
    Vector3f planes[6] = { planeNear, planeFar, planeLeft, planeRight, planeTop, planeBottom };
    float offset[6] = { near, far, 0, 0, 0, 0 };

    for(int p = 0; p < planeLen; p++) {
        for(int t = 0; t < *tLen; t++) {
            Triangle tri = tris[t];

            Vector3f points[4];
            Vector2f texels[4];
            int pointLen = clip(tri, points, texels, planes[p], offset[p]);
            
            Triangle newTri;
            for(int i = 0; i < 3; i++) {
                newTri.verts[i] = points[i];
                newTri.texels[i] = texels[i];
            }
            tris[t] = newTri;

            if(pointLen == 4) {
                Triangle newTri2;
                for(int i = 0; i < 3; i++) {
                    newTri2.verts[i] = points[i+1];
                    newTri2.texels[i] = texels[i+1];
                }
                tris[*tLen] = newTri2;
                (*tLen)++;
                
                // only 1 triangle ever intersects at any moment
                break;
            }
        }
    }
}
