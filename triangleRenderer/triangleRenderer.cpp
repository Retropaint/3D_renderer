#include <math.h>
#include "triangleClipper/triangleClipper.hpp"
#include "transformer/transformer.hpp"
#include "cameraPlanes/cameraPlanes.hpp"
#include "debugger/debugger.hpp"
#include "directives.hpp"
#include "externs.hpp"
#include "structs.hpp"

using namespace sf;
using namespace std;

Image *texImgs;

Vector2f texels[3];
float texZ[3];

void loadTextures() {
    delete[] texImgs;
    texImgs = new Image[1];
    texImgs[0].loadFromFile("tex.png");
}

float triCrossProduct(Vector3f verts[3]) {
    Vector3f line0 = verts[1] - verts[0];
    Vector3f line1 = verts[2] - verts[0];

    return line0.x * line1.y - line0.y * line1.x;
}

BaryTriArea baryCoords(Vector3f point, Vector3f verts[3], bool shouldCrossAreas = false, float areas[3] = 0) {
    // the 3 little triangles for area checking
    Vector3f tri0[3] = { verts[0], point, verts[1] };
    Vector3f tri1[3] = { verts[1], point, verts[2] };
    Vector3f tri2[3] = { verts[2], point, verts[0] };

    areas[0] = abs(triCrossProduct(tri0));
    areas[1] = abs(triCrossProduct(tri1));
    areas[2] = abs(triCrossProduct(tri2));

    float wholeArea = abs(triCrossProduct(verts));
    BaryTriArea bary = BaryTriArea(areas[0] / wholeArea, areas[1] / wholeArea, areas[2] / wholeArea);

    return bary;
}

void drawPixel(Vector3f point, Camera camera, Color color) {
    Vector3i p = Vector3i((int)point.x, (int)point.y, (int)point.z);

    // check if this point is closest to camera for this pixel
    depthZ[p.x][p.y] = point.z;
        depthColor[p.x][p.y] = color;
}

void drawHorizLine(float start, float end, float y, Color color, Vector3f triVerts[3]) {
    BaryTriArea startBary = baryCoords(Vector3f(start + 1, y, 0), triVerts);
    float startZ = 
        triVerts[0].z * startBary.tri2 +
        triVerts[1].z * startBary.tri3 +
        triVerts[2].z * startBary.tri1;

    BaryTriArea endBary = baryCoords(Vector3f(end, y, 0), triVerts);
    float endZ = 
        triVerts[0].z * endBary.tri2 +
        triVerts[1].z * endBary.tri3 +
        triVerts[2].z * endBary.tri1;

    Vector2f startTex = 
        texels[0] * startBary.tri2 +
        texels[1] * startBary.tri3 +
        texels[2] * startBary.tri1;
    float startTexZ = 
        texZ[0] * startBary.tri2 +
        texZ[1] * startBary.tri3 +
        texZ[2] * startBary.tri1;
    startTex *= (1/startTexZ);

    Vector2f endTex = 
        texels[0] * endBary.tri2 +
        texels[1] * endBary.tri3 +
        texels[2] * endBary.tri1;
    float endTexZ = 
        texZ[0] * endBary.tri2 +
        texZ[1] * endBary.tri3 +
        texZ[2] * endBary.tri1;
    endTex *= (1/endTexZ);

    for(int x = (int)start; x < (int)end + 2; x++) {
        if(x < 0 || x > screenWidth) {
            continue;
        }

        Vector3f pixel(x, y, 0);

        float interp = (x - start) / ((end + 2) - start);
        pixel.z = startZ + (endZ - startZ) * interp;
        
        Vector2f tex = startTex + (endTex - startTex) * interp;

        float lighting = max(20.0f, 255 - pixel.z/4);
        Color shading(
            min((float)color.r, lighting),
            min((float)color.g, lighting),
            min((float)color.b, lighting)
        );

        if(interp > 0) {
            drawPixel(pixel, camera, texImgs[0].getPixel(tex.x, tex.y));
        }
    }
}

void swapVerts(Vector3f *v1, Vector3f *v2) {
    Vector3f temp = *v1;
    
    v1->x = v2->x;
    v1->y = v2->y;

    v2->x = temp.x;
    v2->y = temp.y;
}

void fillBottomFlatTri(Vector3f verts[3], Vector3f midPoint, Color color, Triangle tri) {
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

void fillTopFlatTri(Vector3f verts[3], Vector3f midPoint, Color color, Triangle tri) {
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

void newFillTriangle(Triangle tri, Color color, bool canCull) {
    Vector2f minBounds(INFINITY, INFINITY);
    Vector2f maxBounds(0, 0);
    
    for(int i = 0; i < 3; i++) {
        tri.verts[i] = worldToScreenPos(tri.verts[i], camera);

        minBounds = Vector2f(
            min(minBounds.x, max(tri.verts[i].x, 1.0f)),
            min(minBounds.y, max(tri.verts[i].y, 1.0f))
        );

        maxBounds = Vector2f(
            max(maxBounds.x, min(tri.verts[i].x, (float)screenWidth)-1),
            max(maxBounds.y, min(tri.verts[i].y, (float)screenHeight)-1)
        );
    }

    bool hasPassedEdge = false;

    for(float y = minBounds.y; y < maxBounds.y; y++) {
        for(float x = minBounds.x; x < maxBounds.x; x++) {
            if(x == minBounds.x) {
                hasPassedEdge = false;
            }
            Vector3f line1[3] = { tri.verts[0], Vector3f(x, y, 0), tri.verts[1] };
            Vector3f line2[3] = { tri.verts[1], Vector3f(x, y, 0), tri.verts[2], };
            Vector3f line3[3] = { tri.verts[2], Vector3f(x, y, 0), tri.verts[0] };
            float areas[3] = { triCrossProduct(line1), triCrossProduct(line2), triCrossProduct(line3) };

            if((areas[0] > 0 || areas[1] > 0 || areas[2] > 0) && (areas[0] < 0 || areas[1] < 0 || areas[2] < 0)) {
                if(hasPassedEdge) {
                    hasPassedEdge = false;
                    break;
                } else {
                    continue;
                }
            }

            hasPassedEdge = true;
            
            BaryTriArea bary = baryCoords(Vector3f(x, y, 0), tri.verts, false, areas);

            float z = 
                texZ[0] * bary.tri2 +
                texZ[1] * bary.tri3 +
                texZ[2] * bary.tri1;
            
            if(z > depthZ[(int)x][(int)y]) {
                continue;
            }
            
            Vector2f texel = 
                texels[0] * bary.tri2 +
                texels[1] * bary.tri3 +
                texels[2] * bary.tri1;
            texel *= (1/z);

            drawPixel(Vector3f(x, y, z), camera, texImgs[0].getPixel(texel.x, texel.y));
        }
    }
}

void fillTriangle(Triangle tri, Color color, bool canCull) {
    for(int i = 0; i < 3; i++) {
        texels[i] = (Vector2f)tri.texels[i]/tri.verts[i].z;
        texZ[i] = 1/tri.verts[i].z;
    }
    
    newFillTriangle(tri, color, canCull);
    return;

    for(int i = 0; i < 3; i++) {
        tri.verts[i] = worldToScreenPos(tri.verts[i], camera);

        // truncate to prevent floating-point alignment issues with other triangle verts
        tri.verts[i] = Vector3f((int)tri.verts[i].x, (int)tri.verts[i].y, (int)tri.verts[i].z);
    }

    if(canCull && triCrossProduct(tri.verts) < 0) {
        return;
    }

    // separate sorted triangle verts, as bary coords will use original order
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

    float xInterp = (sortedVerts[1].y - sortedVerts[0].y) / (sortedVerts[2].y - sortedVerts[0].y);
    Vector3f midPoint = Vector3f(
        (int)(sortedVerts[0].x + ((sortedVerts[2].x - sortedVerts[0].x) * xInterp)),
        (int)sortedVerts[1].y,
        0
    );

    fillBottomFlatTri(sortedVerts, midPoint, color, tri);
    fillTopFlatTri(sortedVerts, midPoint, color, tri);

    dbg::drawnTris++;
}

void renderTriangle(Triangle tri, Color color) {

    // move triangle relative to camera
    for(int i = 0; i < 3; i++) {
        tri.verts[i] = worldToLocalCameraPos(tri.verts[i], camera);
    }

    if(isFullyCulled(tri.verts)) {
        return;
    }

    if(isFullyInView(tri.verts)) {
        fillTriangle(tri, color, dbg::backFaceCulling);
        return;
    }

    Triangle tris[15];
    int tLen;
    clipTriangle(tri, tris, &tLen);

    Color debugColors[10] = {
        Color(255, 0, 0),
        Color(0, 255, 0),
        Color(0, 0, 255),
        Color(255, 255, 0),
        Color(0, 255, 255),
        Color(255, 0, 255),
        Color(255, 255, 255)
    };

    for(int i = 0; i < tLen; i++) {
        fillTriangle(tris[i], (dbg::triClipColorCode) ? debugColors[i] : color, false);
    }
}