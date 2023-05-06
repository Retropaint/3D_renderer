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

Vector3f nearCamPlaneInt(Vector3f point) {
    float shortestDist = INFINITY;
    Vector3f nearestPlane;

    if(planeNormalDist(point, planeNear, near) < shortestDist) {
        shortestDist = planeNormalDist(point, planeNear, near);
        nearestPlane = planeNear;
    }
    if(planeNormalDist(point, planeFar, far-near) < shortestDist) {
        shortestDist = planeNormalDist(point, planeFar, far-near);
        nearestPlane = planeFar;
    }

    return nearestPlane;
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

int nearClip(Vector3f* verts, Vector3f* points) {
    int pLen = 0;
    int pointsInView = 0;

    float distances[3];

    for(int i = 0; i < 3; i++) {
        distances[i] = planeNormalDist(verts[i], planeNear, near);

        if(distances[i] > 0) {
            points[pLen++] = verts[i];
            pointsInView++;
        }
    }

    if(pointsInView == 3) {
        return -1;
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

/*
bool allTrisInView(Triangle tris[10], int tLen) {
    for(int i = 0; i < tLen; i++) {
        if(!isFullyInView(tris[i].verts)) {
            return true;
        }
    }

    return true;
}

void clipTriangle(Triangle mainTri, Triangle *tris, int *tLen) {
    tris[0] = mainTri;
    *tLen = 1;

    while(!allTrisInView(tris, *tLen)) {
        for(int t = 0; t < tLen; i++) {
            Triangle tri = tris[t];

            for(int v1 = 0; v1 < 3; v1++) {
                Vector3f vert = tri.verts[v1];

                for(int v2 = 0; v2 < 3; v2++) {
                    Vector3f otherVert = tri.verts[v2];
                
                    if(planeNormalDist(vert, planeNear, near) > 0 && planeNormalDist(otherVert, planeNear, near) < 0) {
                        
                    }
                }
            }
        }
    }
}
*/

// --------------------------------------------------
/*
    - everything beyond this point was created for clipping against top, bottom, right, and left planes
    - since the current algorithm only uses the near plane, these are all unused
*/

/*
// @returns The horizontal and vertical camera planes that the point is outside of, in binary.
int isPointOutOfView(Vector3f point) {
    int final = 0;

    if(planeNormalDist(point, planeRight) < -0.01) {
        final |= 1;
    }
    else if(planeNormalDist(point, planeLeft) < -0.01) {
        final |= 2;
    }

    if(planeNormalDist(point, planeTop) < -0.01) {
        final |= 4;
    }
    else if(planeNormalDist(point, planeBottom) < -0.01) {
        final |= 8;
    }

    return final;
}

// @returns The nearest camera plane that the point intersects with.
// @param discardedPlanes planes to ignore
Vector3f nearCamPlaneInt(Vector3f point) {
    float shortestDist = INFINITY;
    Vector3f nearestPlane;

    if(planeNormalDist(point, planeRight, 0) < shortestDist) {
        shortestDist = planeNormalDist(point, planeRight, 0);
        nearestPlane = planeRight;
    }
    if(planeNormalDist(point, planeLeft, 0) < shortestDist) {
        shortestDist = planeNormalDist(point, planeLeft, 0);
        nearestPlane = planeLeft;
    }
    if(planeNormalDist(point, planeTop, 0) < shortestDist) {
        shortestDist = planeNormalDist(point, planeTop, 0);
        nearestPlane = planeTop;
    }
    if(planeNormalDist(point, planeBottom, 0) < shortestDist) {
        shortestDist = planeNormalDist(point, planeBottom, 0);
        nearestPlane = planeBottom;
    }

    return nearestPlane;
}

// @returns The point of intersection in the camera planes from the given line segment.
Vector3f getClipPoint(Vector3f p1, Vector3f p2) {

    // store the planes that will be ignored on consecutive plane intersections
    bool discardedPlanes[5];
    discardedPlanes[0] = false;
    discardedPlanes[1] = false;
    discardedPlanes[2] = false;
    discardedPlanes[3] = false;
    discardedPlanes[4] = false;

    // get intersection with nearest plane
    Vector3f nearestPlane = nearCamPlaneInt(p2, discardedPlanes);
    Vector3f newPoint = linePlaneInt(p1, p2, nearestPlane);
    
    // keep searching for nearest plane with a visible intersection
    while(isPointOutOfView(newPoint)) {

        // ignore this wrongly-assumed nearest plane for future intersections
        if(nearestPlane == planeRight) {
            discardedPlanes[0] = true;
        } else if(nearestPlane == planeLeft) {
            discardedPlanes[1] = true;
        } else if(nearestPlane == planeTop) {
            discardedPlanes[2] = true;
        } else if(nearestPlane == planeBottom) {
            discardedPlanes[3] = true;
        }

        // get intersection with next nearest plane
        nearestPlane = nearCamPlaneInt(newPoint, discardedPlanes);
        newPoint = linePlaneInt(p1, p2, nearestPlane);
    }

    return newPoint;
}

// @returns The point that resides in any screen corner, from the given points
void checkCornerPoint(Vector3f *points, int *pLen, int pLenOffset, int *corners) {
    bool discardedPlanes[4];
    discardedPlanes[0] = false;
    discardedPlanes[1] = false;
    discardedPlanes[2] = false;
    discardedPlanes[3] = false;

    int planeRightInts = 0;
    int planeLeftInts = 0;
    int planeTopInts = 0;
    int planeBottomInts = 0;

    int horIdx;
    int verIdx;

    // keep track of how many intersections each plane has
    for(int i = pLenOffset; i < *pLen; i++) {
        if((isPointOutOfView(points[i])&16) == 16) {
            return;
        }
        Vector3f plane = nearCamPlaneInt(points[i], discardedPlanes);
        if(plane == planeRight) {
            planeRightInts++;
            horIdx = i;
        } else if(plane == planeLeft) {
            planeLeftInts++;
            horIdx = i;
        }
        if(plane == planeTop) {
            planeTopInts++;
            verIdx = i;
        } else if(plane == planeBottom) {
            planeBottomInts++;
            verIdx = i;
        }
    }

    // checking for 1 corner point (will apear if there's 1 point on a horizontal and vertical plane each)
    if(planeBottomInts == 1 && planeLeftInts == 1) {
        points[*pLen].x = -points[verIdx].y;
        points[*pLen].y = points[verIdx].y;
        points[*pLen].z = points[verIdx].y;
        (*pLen)++;
        *corners = 1;
    } else if(planeTopInts == 1 && planeRightInts == 1) {
        points[*pLen].x = points[verIdx].y;
        points[*pLen].y = -points[verIdx].y;
        points[*pLen].z = points[verIdx].y;
        (*pLen)++;
        *corners = 1;
    } else if(planeTopInts == 1 && planeLeftInts == 1) {
        points[*pLen].x = -points[verIdx].y;
        points[*pLen].y = -points[verIdx].y;
        points[*pLen].z = points[verIdx].y;
        (*pLen)++;
        *corners = 1;
    } else if(planeBottomInts == 1 && planeRightInts == 1) {
        points[*pLen].x = points[verIdx].y;
        points[*pLen].y = points[verIdx].y;
        points[*pLen].z = points[verIdx].y;
        (*pLen)++;
        *corners = 1;
    }

    // checking for 2 corners (unfinished)
    if(planeRightInts == 1 && planeLeftInts == 1) {
        points[*pLen].x = -points[horIdx].z;
        points[*pLen].y = points[horIdx].z;
        points[*pLen].z = points[horIdx].z;
        (*pLen)++;

        points[*pLen].x = points[horIdx].z;
        points[*pLen].y = points[horIdx].z;
        points[*pLen].z = points[horIdx].z;
        (*pLen)++;
        *corners = 2;
    }
}

bool pointExists(Vector3f* points, int pLen, Vector3f point) {
    for(int i = 0; i < pLen; i++) {
        if((Vector3i)points[i] == (Vector3i)point) {
            return true;
        }
    }
    return false;
}

// unused
// gets intersections across all planes except near and far
int allIntPoints(Vector3f *verts, Vector3f *points, int *corners) {
    int pLen = 0;
    int pointsInView = 0;

    // get points in view
    for(int i = 0; i < 3; i++) {
        if(!isPointOutOfView(verts[i])) {
            points[pLen++] = verts[i];
            pointsInView++;
        }
    }

    // get all intersections
    for(int i = 0; i < 3; i++) {
        for(int j = 0; j < 3; j++) {

            // don't check a vertex against itself
            if(i == j) {
                continue;
            }
            
            // ignore vertices that are outside of the same plane (they'll never intersect)
            if(
                (isPointOutOfView(verts[i])&1) == 1 && (isPointOutOfView(verts[j])&1) == 1 ||
                (isPointOutOfView(verts[i])&2) == 2 && (isPointOutOfView(verts[j])&2) == 2 ||
                (isPointOutOfView(verts[i])&4) == 4 && (isPointOutOfView(verts[j])&4) == 4 ||
                (isPointOutOfView(verts[i])&8) == 8 && (isPointOutOfView(verts[j])&8) == 8
            ) {
                continue;
            }

            if(isPointOutOfView(verts[i]) && !isPointOutOfView(verts[j]) || isPointOutOfView(verts[i]) && isPointOutOfView(verts[j])) {
                Vector3f newPoint = getClipPoint(verts[j], verts[i]);
                if(!isnan(newPoint.x)) {
                    points[pLen++] = newPoint;
                }
            }
        }
    }

    checkCornerPoint(points, &pLen, pointsInView, corners);

    return pLen;
}
*/
