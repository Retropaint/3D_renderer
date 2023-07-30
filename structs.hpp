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
        Vector2i texels[3];
    };

    struct Object {
        Triangle *tris;
        int triLen;
    };

    struct BaryTriArea {
        float tri1;
        float tri2;
        float tri3;
    };
#endif

