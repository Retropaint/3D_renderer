#include <math.h>
#include "triangleRenderer/triangleRenderer.hpp"
#include "triangleClipper/triangleClipper.hpp"
#include "directives.hpp"
#include "externs.hpp"
#include "objLoader/objLoader.hpp"
#include "debugger/debugger.hpp"
#include "cameraPlanes/cameraPlanes.hpp"
#include "transformer/transformer.hpp"
#include <stdlib.h>
#include <stdio.h>

Vector2i prevMouse;

Vector3f tri1Verts[3] = {
    Vector3f(0, 0, 0),
    Vector3f(52, 0, 0),
    Vector3f(52, 22, 0)
};

Vector2i tri1Texels[3] = {
    Vector2i(0, 0),
    Vector2i(27, 0),
    Vector2i(27, 11)
};

Vector3f tri2Verts[3] = {
    Vector3f(0, 0, 0),
    Vector3f(52, 22, 0),
    Vector3f(0, 22, 0)
};

Vector2i tri2Texels[3] = {
    Vector2i(0, 0),
    Vector2i(27, 11),
    Vector2i(0, 11)
};

struct Triangle tri1;
struct Triangle tri2;

RenderWindow window(VideoMode(screenWidth, screenHeight), "Triangle Clipping", Style::Default);
Camera camera;

Object obj;
Object obj2;

using namespace std;

bool looking = false;
float shipAngle = 0;

void initTriangle(Triangle *tri, Vector3f triVertsTemplate[3], Vector2i triTexelsTemplate[3]) {
    for(int i = 0; i < 3; i++) {
        tri->verts[i] = triVertsTemplate[i];
        tri->texels[i] = triTexelsTemplate[i];
    }
}

Vector3f playerMove(Vector3f pos, float angle, float speed) {
    pos.x += cos(angle) * speed;
    pos.z += sin(angle) * speed;

    return pos;
}

void playerMoveControls() {
    float offset = M_PI/2;
    if(Keyboard::isKeyPressed(Keyboard::W)) {
        camera.pos = playerMove(camera.pos, camera.angle.y + offset, 5);
    } else if(Keyboard::isKeyPressed(Keyboard::S)) {
        camera.pos = playerMove(camera.pos, camera.angle.y - M_PI + offset, 5);
    }
    if(Keyboard::isKeyPressed(Keyboard::A)) {
        camera.pos = playerMove(camera.pos, camera.angle.y + M_PI/2 + offset, 5);
    } else if(Keyboard::isKeyPressed(Keyboard::D)) {
        camera.pos = playerMove(camera.pos, camera.angle.y - M_PI/2 + offset, 5);
    }
}

int main(int argc, char** argv) {
    if(argc >= 3) {
        readObjFile(argv[1], argv[2], &obj);
    } else if(argc == 2) {
        readObjFile(argv[1], (char*)"10", &obj);
    }

    if(argc == 5) {
        window.setSize(Vector2u(atoi(argv[3]), atoi(argv[4])));
    }

    // init camera
    camera.pos = Vector3f(0, 0, 0);
    camera.angle = Vector3f(0, 0, 0);

    int fov = 90;

    initTriangle(&tri1, tri1Verts, tri1Texels);
    initTriangle(&tri2, tri2Verts, tri2Texels);


    while (window.isOpen())
    {
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed) {
                window.close();
            }
        }

        // reset frame buffer
        for(int x = 0; x < screenWidth; x++) {
            for(int y = 0; y < screenHeight; y++) {
                depthZ[x][y] = INFINITY;
            }
        }
        dbg::drawnTris = 0;

        /*
        if(Keyboard::isKeyPressed(Keyboard::W)) {
            for(int i = 0; i < obj.triLen; i++) {
                for(int j = 0; j < 3; j++) {
                    obj.tris[i].verts[j] = playerMove(obj.tris[i].verts[j], shipAngle + M_PI/2, 5);
                }
            }
        } else if(Keyboard::isKeyPressed(Keyboard::S)) {
            for(int i = 0; i < obj.triLen; i++) {
                for(int j = 0; j < 3; j++) {
                    obj.tris[i].verts[j] = playerMove(obj.tris[i].verts[j], shipAngle - M_PI/2, 5);
                }
            }
        }
        */

       camera.pos = obj.tris[10].verts[0];
        
        if(Keyboard::isKeyPressed(Keyboard::Up)) {
            camera.pos.y -= 2;
        } else if(Keyboard::isKeyPressed(Keyboard::Down)) {
            camera.pos.y += 2;
        }
        
        if(Keyboard::isKeyPressed(Keyboard::Num1)) {
            dbg::triClipColorCode = true;
        }
        if(Keyboard::isKeyPressed(Keyboard::Num2)) {
            dbg::triClipColorCode = false;
        }

        playerMoveControls();

        if(Keyboard::isKeyPressed(Keyboard::Num3)) {
            fov++;
        }
        if(Keyboard::isKeyPressed(Keyboard::Num4)) {
            fov--;
        }
        fov = max(90, min(fov, 135));
        setFOV(fov, &camera);

       if(Keyboard::isKeyPressed(Keyboard::Space)) {
            looking = true;
        }
        if(Keyboard::isKeyPressed(Keyboard::Z)) {
            looking = false;
        }

        if(Keyboard::isKeyPressed(Keyboard::Escape)) {
            window.close();
        }

        if(!looking) {
            window.setMouseCursorVisible(true);
        } else {

            // mouse look
            
            camera.angle.y -= ((float)Mouse::getPosition().x - (float)prevMouse.x) / 500;

            camera.angle.x += ((float)Mouse::getPosition().y - (float)prevMouse.y) / 500;
            camera.angle.x = clampAngle(camera.angle.x);

            Vector3f offset = rotateX(Vector3f(0, 0, -100), -camera.angle.x);
            offset = rotateY(offset, -camera.angle.y);
            camera.pos += offset;

            // hide and lock cursor
            window.setMouseCursorVisible(false);
            Mouse::setPosition(Vector2i(window.getPosition().x + screenWidth/2, window.getPosition().y + screenHeight/2));
        }
        prevMouse = Mouse::getPosition();

        if(argc == 1) {
            renderTriangle(tri1, Color::Magenta);
            renderTriangle(tri2, Color::Magenta);
        } else {
            for(int i = 0; i < obj.triLen; i++) {
                renderTriangle(obj.tris[i], Color::White);
            }
            for(int i = 0; i < obj2.triLen; i++) {
                renderTriangle(obj2.tris[i], Color::White);
            }
        }

        Image image;
        image.create(screenWidth, screenHeight);

        for(int x = 0; x < screenWidth; x++) {
            for(int y = 0; y < screenHeight; y++) {
                if(depthZ[x][y] != INFINITY) {
                    //pixel.setPosition(x, y);
                    //pixel.setFillColor(depthColor[x][y]);
                    //window.draw(pixel);
                    image.setPixel(x, y, depthColor[x][y]);
                }
            }
        }

        window.clear();

        Texture tex;
        tex.loadFromImage(image);

        Sprite spr;
        spr.setTexture(tex);
        window.draw(spr);

        //printf("%d\n", dbg::drawnTris);

        window.display();
    }

    return 0;
}
