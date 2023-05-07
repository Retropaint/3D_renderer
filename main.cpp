#include <math.h>
#include "triangleRenderer/triangleRenderer.hpp"
#include "triangleClipper/triangleClipper.hpp"
#include "directives.hpp"
#include "externs.hpp"
#include "objLoader/objLoader.hpp"
#include "debugger/debugger.hpp"

Vector2i prevMouse;

Vector3f tri2Verts[3] = {
    Vector3f(0, 25, 0),
    Vector3f(0, 25, 40),
    Vector3f(10, 25, 0)
};

struct Triangle tri;
struct Triangle tri1;

Triangle drawTri;
int drawTriLen;
bool pressed = false;

RenderWindow window(VideoMode(screenWidth, screenHeight), "Triangle Clipping", Style::Default);
Camera camera;
RectangleShape pixel;

Object obj;
Object obj2;

bool looking = false;

void initTriangle(Triangle *tri, Vector3f triTemplate[3]) {
    tri->verts[0] = triTemplate[0];
    tri->verts[1] = triTemplate[1];
    tri->verts[2] = triTemplate[2];
}

void playerMove(Vector3f *pos, float angle, float speed) {
    pos->x += cos(angle) * speed;
    pos->z += sin(angle) * speed;
}

int main(int argc, char** argv)
{
    if(argc == 3) {
        readObjFile(argv[1], argv[2], &obj);
    } else if(argc == 2) {
        readObjFile(argv[1], (char*)"10", &obj);
    }
    
    //readObjFile("airship", (char*)"10", &obj2);
    for(int i = 0; i < obj2.triLen; i++) {
        for(int j = 0; j < 3; j++) {
            obj2.tris[i].verts[j].x += 100;
        }
    }

    // init camera
    camera.pos = Vector3f(0, 0, 0);
    camera.angle = Vector3f(0, 0, 0);
    camera.near = 300;

    initTriangle(&tri, tri2Verts);
    tri.pos = Vector3f(0, 25, 0);
    tri.rot = Vector3f(0, 0, 0);

    initTriangle(&tri1, tri2Verts);
    tri.pos = Vector3f(-10, 0, -10);
    tri.rot = Vector3f(0, M_PI, 0);

    window.setFramerateLimit(60);

    pixel.setSize(Vector2f(1, 1));

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

        if(Keyboard::isKeyPressed(Keyboard::Right)) {
            camera.angle.y -= 0.05;
        } else if(Keyboard::isKeyPressed(Keyboard::Left)) {
            camera.angle.y += 0.05;
        }
        if(Keyboard::isKeyPressed(Keyboard::Up)) {
            camera.pos.y -= 2;
        } else if(Keyboard::isKeyPressed(Keyboard::Down)) {
            camera.pos.y += 2;
        }

        float offset = M_PI/2;
        if(Keyboard::isKeyPressed(Keyboard::W)) {
            playerMove(&camera.pos, camera.angle.y + offset, 5);
        } else if(Keyboard::isKeyPressed(Keyboard::S)) {
            playerMove(&camera.pos, camera.angle.y - M_PI + offset, 5);
        }
        if(Keyboard::isKeyPressed(Keyboard::A)) {
            playerMove(&camera.pos, camera.angle.y + M_PI/2 + offset, 5);
        } else if(Keyboard::isKeyPressed(Keyboard::D)) {
            playerMove(&camera.pos, camera.angle.y - M_PI/2 + offset, 5);
        }

        if(Keyboard::isKeyPressed(Keyboard::Escape)) {
            window.close();
        }

        if(Keyboard::isKeyPressed(Keyboard::Space)) {
            looking = true;
        }
        if(Keyboard::isKeyPressed(Keyboard::Z)) {
            looking = false;
        }

        if(Keyboard::isKeyPressed(Keyboard::Num1)) {
            dbg::triClipColorCode = true;
        }
        if(Keyboard::isKeyPressed(Keyboard::Num2)) {
            dbg::triClipColorCode = false;
        }

        window.clear();

        if(argc == 1) {
            renderTriangle(tri, Color::Magenta);
            //renderTriangle(tri1, Color::Magenta);
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

        if(!looking) {
            window.setMouseCursorVisible(true);
        } else {

            // mouse look
            
            camera.angle.y -= ((float)Mouse::getPosition().x - (float)prevMouse.x) / 500;

            // clamp mouse Y to 360 deg (2PI rad)
            if(camera.angle.y < 0) {
                camera.angle.y = M_PI*2 + camera.angle.y;
            } else if(camera.angle.y > M_PI*2) {
                camera.angle.y -= M_PI*2;
            }

            camera.angle.x += ((float)Mouse::getPosition().y - (float)prevMouse.y) / 500;

            // clamp mouse X to 360 deg (2PI rad)
            if(camera.angle.x > M_PI/2) {
                camera.angle.x = M_PI/2;
            } else if(camera.angle.x < -M_PI/2) {
                camera.angle.x = -M_PI/2;
            }

            // hide and lock cursor
            window.setMouseCursorVisible(false);
            Mouse::setPosition(Vector2i(window.getPosition().x + screenWidth/2, window.getPosition().y + screenHeight/2));
        }
        prevMouse = Mouse::getPosition();

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
        
        Texture tex;
        tex.loadFromImage(image);

        Sprite spr;
        spr.setTexture(tex);
        window.draw(spr);

        printf("%d\n", dbg::drawnTris);

        window.display();
    }

    return 0;
}
