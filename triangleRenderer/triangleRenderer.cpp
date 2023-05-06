#include <math.h>
#include "triangleClipper/triangleClipper.hpp"
#include "transformer/transformer.hpp"
#include "cameraPlanes.hpp"
#include "debugger/debugger.hpp"
#include "directives.hpp"
#include "externs.hpp"
#include "structs.hpp"

using namespace sf;
using namespace std;

float triCrossProduct(Vector3f verts[]) {
    Vector3f line0 = verts[1] - verts[0];
    Vector3f line1 = verts[2] - verts[0];

    return line0.x * line1.y - line0.y * line1.x;
}

BaryTriArea baryCoords(Vector3f point, Vector3f verts[3]) {

    // the 3 little triangles for area checking
    Vector3f tri0[3] = { verts[0], point, verts[1] };
    Vector3f tri1[3] = { verts[1], point, verts[2] };
    Vector3f tri2[3] = { verts[2], point, verts[0] };

    float area0 = abs(triCrossProduct(tri0));
    float area1 = abs(triCrossProduct(tri1));
    float area2 = abs(triCrossProduct(tri2));

    float wholeArea = abs(triCrossProduct(verts));
    BaryTriArea bary = BaryTriArea(area0 / wholeArea, area1 / wholeArea, area2 / wholeArea);

    return bary;
}

void drawPixel(Vector3f point, Camera camera, Color color) {
    Vector3i p = Vector3i((int)point.x, (int)point.y, (int)point.z);

    // check if this point is closest to camera for this pixel
    if(point.z <= depthZ[p.x][p.y]) {
        depthZ[p.x][p.y] = point.z;
        depthColor[p.x][p.y] = color;
    }
}

void drawHorizLine(float start, float end, float y, Color color, Vector3f *triVerts) {
    for(int x = (int)start; x < (int)end + 2; x++) {
        if(x < 0 || x > screenWidth) {
            continue;
        }

        Vector3f pixel(x, y, 0);
        
        BaryTriArea bary = baryCoords(pixel, triVerts);
        pixel.z = 
            triVerts[0].z * bary.tri2 +
            triVerts[1].z * bary.tri3 +
            triVerts[2].z * bary.tri1;

        Color shading(
            min((float)color.r, max(20.0f, 255 - pixel.z)),
            min((float)color.g, max(20.0f, 255 - pixel.z)),
            min((float)color.b, max(20.0f, 255 - pixel.z))
        );
        
        drawPixel(pixel, camera, shading);
    }
}

void swapVerts(Vector3f *v1, Vector3f *v2) {
    Vector3f temp = *v1;
    
    v1->x = v2->x;
    v1->y = v2->y;

    v2->x = temp.x;
    v2->y = temp.y;
}

void fillBottomFlatTri(Vector3f *verts, Vector3f midPoint, Color color, Triangle tri) {
    Vector3f rightPoint, leftPoint;
    Vector3f topPoint = verts[0];
    
    if(verts[1].x > midPoint.x) {
        rightPoint = verts[1];
        leftPoint = midPoint;
    } else {
        rightPoint = midPoint;
        leftPoint = verts[1];
    }

    float slopeLeft = (leftPoint.x - topPoint.x) / (leftPoint.y - topPoint.y);
    float slopeRight = (rightPoint.x - topPoint.x) / (rightPoint.y - topPoint.y);

    float lineStart = topPoint.x;
    float lineEnd = topPoint.x;

    for(int y = topPoint.y; y < verts[1].y + 1; y++) {
        if(y < screenHeight - 1 && y > 0) {
            drawHorizLine(lineStart, lineEnd, y, color, tri.verts);
        }

        lineStart += slopeLeft;
        lineEnd += slopeRight;

        // make sure lines do not overstep
        if(leftPoint.x < topPoint.x) {
            lineStart = max(lineStart, leftPoint.x);
        }
        if(rightPoint.x > topPoint.x) {
            lineEnd = min(lineEnd, rightPoint.x);
        }
    }
}

void fillTopFlatTri(Vector3f *verts, Vector3f midPoint, Color color, Triangle tri) {
    Vector3f rightPoint, leftPoint;
    Vector3f bottomPoint = verts[2];
    
    if(verts[1].x > midPoint.x) {
        rightPoint = verts[1];
        leftPoint = midPoint;
    } else {
        rightPoint = midPoint;
        leftPoint = verts[1];
    }

    float slopeLeft = (bottomPoint.x - leftPoint.x) / (bottomPoint.y - leftPoint.y);
    float slopeRight = (bottomPoint.x - rightPoint.x) / (bottomPoint.y - rightPoint.y);

    float lineStart = bottomPoint.x;
    float lineEnd = bottomPoint.x;

    for(int y = bottomPoint.y; y > verts[1].y - 1; y--) {
        if(y < screenHeight - 1 && y > 0) {
            drawHorizLine(lineStart, lineEnd, y, color, tri.verts);
        }
        
        lineStart -= slopeLeft;
        lineEnd -= slopeRight;

        // make sure lines do not overstep
        if(leftPoint.x < bottomPoint.x) {
            lineStart = max(lineStart, leftPoint.x);
        }
        if(rightPoint.x > bottomPoint.x) {
            lineEnd = min(lineEnd, rightPoint.x);
        }
    }
}

void fillTriangle(Triangle tri, Color color, bool baryShading, int modType, bool canCull) {
    for(int i = 0; i < 3; i++) {

        // position the verts in relation to screen space
        if(modType == 1) {
            tri.verts[i] += tri.pos;
            tri.verts[i] = rotateX(tri.verts[i], tri.rot.x);
            tri.verts[i] = rotateY(tri.verts[i], tri.rot.y);
            tri.verts[i] = rotateZ(tri.verts[i], tri.rot.z);
            tri.verts[i] = worldToLocalCameraPos(tri.verts[i], camera);
            tri.verts[i] = worldToScreenPos(tri.verts[i], camera);
        } else if(modType == 2) {
            tri.verts[i] = worldToScreenPos(tri.verts[i], camera);
        }

        // truncate to prevent floating-point alignment issues with other tri verts
        tri.verts[i] = Vector3f((int)tri.verts[i].x, (int)tri.verts[i].y, (int)tri.verts[i].z);
    }

    if(canCull && triCrossProduct(tri.verts) < 0) {
        return;
    }

    tris++;

    // separate sorted tri verts (by descending y), so the original order can be sent and make bary coords easier to do
    Vector3f sortedVerts[3];
    sortedVerts[0] = Vector3f(tri.verts[0].x, tri.verts[0].y, tri.verts[0].z);
    sortedVerts[1] = Vector3f(tri.verts[1].x, tri.verts[1].y, tri.verts[1].z);
    sortedVerts[2] = Vector3f(tri.verts[2].x, tri.verts[2].y, tri.verts[2].z);

    if(sortedVerts[0].y > sortedVerts[1].y) {
        swapVerts(&sortedVerts[0], &sortedVerts[1]);
    }
    if(sortedVerts[0].y > sortedVerts[2].y) {
        swapVerts(&sortedVerts[0], &sortedVerts[2]);
    }
    if(sortedVerts[1].y > sortedVerts[2].y) {
        swapVerts(&sortedVerts[1], &sortedVerts[2]);
    }

    float t = (sortedVerts[1].y - sortedVerts[0].y) / (sortedVerts[2].y - sortedVerts[0].y);
    Vector3f midPoint = Vector3f(
        (int)(sortedVerts[0].x + ((sortedVerts[2].x - sortedVerts[0].x) * t)), 
        (int)sortedVerts[1].y,
        0
    );

    fillBottomFlatTri(sortedVerts, midPoint, color, tri);
    fillTopFlatTri(sortedVerts, midPoint, color, tri);
}

void renderTriangle(Triangle tri, Color color) {

    // transform triangle into camera space
    for(int i = 0; i < 3; i++) {
        tri.verts[i] += tri.pos;
        tri.verts[i] = rotateX(tri.verts[i], tri.rot.x);
        tri.verts[i] = rotateY(tri.verts[i], tri.rot.y);
        tri.verts[i] = rotateZ(tri.verts[i], tri.rot.z);
        tri.verts[i] = worldToLocalCameraPos(tri.verts[i], camera);
    }

    if(isFullyCulled(tri.verts)) {
        return;
    }

    Vector3f points[4];
    int pLen = nearClip(tri.verts, points);

    Triangle newTri = tri;
    newTri.verts[0] = points[0];
    newTri.verts[1] = points[1];
    newTri.verts[2] = points[2];

    if(pLen == -1 && backCulling) {
        fillTriangle(newTri, Color::White, false, 2, true);
    } else {
        fillTriangle(newTri, Color::White, false, 2, false);
    }

    printf("%d\n", pLen);

    if(pLen >= 4) {
        newTri.verts[0] = points[1];
        newTri.verts[1] = points[2];
        newTri.verts[2] = points[3];
        fillTriangle(newTri, Color::White, false, 2, false);
    }
}

// unused
// meant to be used with triangleClipper's allIntPoints()

/*
float clockDirection(Vector3f p1, Vector3f p2, Vector3f p3) {
    return  
        (p2.x - p1.x) * (p3.y - p1.y) - 
        (p3.x - p1.x) * (p2.y - p1.y);
}

void renderClippedTriangles(Vector3f *points, int pLen, Triangle tri, Color color) {
    int cutLen = 0;

    Color colors[] = {
        Color(255, 0, 0),
        Color(0, 255, 0),
        Color(0, 0, 255),
        Color(255, 255, 0),
        Color(0, 255, 255),
        Color(255, 255, 255)
    };

    int triLen = 0;
    
    while(cutLen < pLen - 3) {
        Vector3f mainPoint = points[cutLen];
        Vector3f neighbour1 = Vector3f(NAN, NAN, NAN);
        Vector3f neighbour2;

        // check if intersection exists in current segment
        for(int i = cutLen+1; i < pLen; i++) {
            Vector3f neighbourPoint = points[i];
            bool isIntersecting = false;
            
            for(int j = cutLen+1; j < pLen; j++) {
                Vector3f otherPoint1 = points[j];
                if(otherPoint1 == neighbourPoint || otherPoint1 == neighbour1) {
                    continue;
                }
                
                for(int k = cutLen+1; k < pLen; k++) {
                    Vector3f otherPoint2 = points[k];
                    if(otherPoint2 == neighbourPoint || otherPoint2 == otherPoint1) {
                        continue;
                    }

                    // do cross product clockwise and counter-clockwise checks
                    if(
                        clockDirection(mainPoint, neighbourPoint, otherPoint1) < 0 && clockDirection(mainPoint, neighbourPoint, otherPoint2) > 0 ||
                        clockDirection(mainPoint, neighbourPoint, otherPoint1) > 0 && clockDirection(mainPoint, neighbourPoint, otherPoint2) < 0
                    ) {
                        isIntersecting = true;
                        break;
                    } 
                }

                if(isIntersecting) {
                    break;
                }
            }

            if(!isIntersecting) {
                if(isnan(neighbour1.x)) {
                    neighbour1 = neighbourPoint;
                } else {
                    neighbour2 = neighbourPoint;
                    break;
                }
            }
        }

        Triangle newTri = tri;
        newTri.verts[0] = mainPoint;
        newTri.verts[1] = neighbour1;
        newTri.verts[2] = neighbour2;
        fillTriangle(newTri, colors[triLen], false, 2);
        triLen++;

        cutLen++;
    }

    // connect the 3 remaining points
    Triangle newTri = tri;
    newTri.verts[0] = points[pLen-1];
    newTri.verts[1] = points[pLen-2];
    newTri.verts[2] = points[pLen-3];
    fillTriangle(newTri, colors[triLen], false, 2);
}
*/