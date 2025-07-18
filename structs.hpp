#include <SFML/Graphics.hpp>

using namespace sf;

#ifndef STRUCTS
    #define STRUCTS 1
    struct Camera {
        int fov;

        float near;
        float far;
        Vector3f angle;
        Vector3f pos;
    };

    struct Triangle {
        Vector3f verts[3];
        Vector2f texels[3];
    };

    struct Object {
        Triangle *tris;
        int triLen;
    };

    struct BaryTriArea {
        float tri1;
        float tri2;
        float tri3;

        BaryTriArea(float ntri1, float ntri2, float ntri3) {
            tri1 = ntri1;
            tri2 = ntri2;
            tri3 = ntri3;
        }
    };
#endif

