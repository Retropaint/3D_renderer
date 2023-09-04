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
    texImgs[0].loadFromFile("tex.jpg");
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

void fillTriangle(Triangle tri, Color color, bool canCull) {
    // cache perspective-correct texel coords
    for(int i = 0; i < 3; i++) {
        texels[i] = (Vector2f)tri.texels[i]/tri.verts[i].z;
        texZ[i] = 1/tri.verts[i].z;
    }

    Vector2f minBounds(INFINITY, INFINITY);
    Vector2f maxBounds(0, 0);
    for(int i = 0; i < 3; i++) {
        tri.verts[i] = worldToScreenPos(tri.verts[i], camera);
        minBounds.x = min(minBounds.x, max(tri.verts[i].x, 1.0f));
        minBounds.y = min(minBounds.y, max(tri.verts[i].y, 1.0f));
        maxBounds.x = max(maxBounds.x, min(tri.verts[i].x, (float)screenWidth)-1);
        maxBounds.y = max(maxBounds.y, min(tri.verts[i].y, (float)screenHeight)-1);
    }

    float wholeArea = triCrossProduct(tri.verts);
    if(canCull && wholeArea < 0) {
        return;
    }
    wholeArea = abs(wholeArea);

    // rounding bounds prevents misalignment on connecting triangles
    for(float y = floor(minBounds.y); y < ceil(maxBounds.y)+1; y++) {
        Vector3f leftLine1[3] = { tri.verts[0], Vector3f(floor(minBounds.x), y, 0), tri.verts[1] };
        Vector3f leftLine2[3] = { tri.verts[1], Vector3f(floor(minBounds.x), y, 0), tri.verts[2] };
        Vector3f leftLine3[3] = { tri.verts[2], Vector3f(floor(minBounds.x), y, 0), tri.verts[0] };
        float leftAreas[3] = { 
            triCrossProduct(leftLine1),
            triCrossProduct(leftLine2),
            triCrossProduct(leftLine3)
        };

        Vector3f rightLine1[3] = { tri.verts[0], Vector3f(ceil(maxBounds.x), y, 0), tri.verts[1] };
        Vector3f rightLine2[3] = { tri.verts[1], Vector3f(ceil(maxBounds.x), y, 0), tri.verts[2] };
        Vector3f rightLine3[3] = { tri.verts[2], Vector3f(ceil(maxBounds.x), y, 0), tri.verts[0] };
        float rightAreas[3] = { 
            triCrossProduct(rightLine1),
            triCrossProduct(rightLine2),
            triCrossProduct(rightLine3)
        };

        for(float x = floor(minBounds.x); x < ceil(maxBounds.x)+1; x++) {
            float interp = (x - floor(minBounds.x)) / (ceil(maxBounds.x) - floor(minBounds.x));
            float areas[3] = {
                leftAreas[0] + ((rightAreas[0] - leftAreas[0]) * interp),
                leftAreas[1] + ((rightAreas[1] - leftAreas[1]) * interp),
                leftAreas[2] + ((rightAreas[2] - leftAreas[2]) * interp)
            };

            // discard if point is not in triangle (checked with winding)
            if(
                (areas[0] > 0 || areas[1] > 0 || areas[2] > 0) &&
                (areas[0] < 0 || areas[1] < 0 || areas[2] < 0)) {
                    continue;
            }

            BaryTriArea bary = BaryTriArea(
                abs(areas[0])/wholeArea, 
                abs(areas[1])/wholeArea, 
                abs(areas[2])/wholeArea
            );

            float realZ = 
                tri.verts[0].z * bary.tri2 +
                tri.verts[1].z * bary.tri3 +
                tri.verts[2].z * bary.tri1;
            if(realZ > depthZ[(int)x][(int)y]) {
                continue;
            }

            float finalTexZ = 
                texZ[0] * bary.tri2 +
                texZ[1] * bary.tri3 +
                texZ[2] * bary.tri1;
            Vector2f texel = 
                texels[0] * bary.tri2 +
                texels[1] * bary.tri3 +
                texels[2] * bary.tri1;
            texel *= (1/finalTexZ);

            drawPixel(Vector3f(x, y, realZ), camera, texImgs[0].getPixel(texel.x, texel.y));
        }
    }
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