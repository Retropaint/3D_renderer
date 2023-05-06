#include "debugger/debugger.hpp"
#include "transformer/transformer.hpp"
#include "directives.hpp"
#include "externs.hpp"

using namespace sf;

void dbg::printVector3(char* prefix, Vector3f vector) {
    printf("%s: %.1f, %.1f, %.1f\n", prefix, vector.x, vector.y, vector.z);
}

void dbg::drawTriClipPoints(Vector3f *points, int pLen, Camera camera, RenderWindow *window, bool shouldPrint, bool isAlreadyScreenPos) {
    if(shouldPrint) {
        printf("\nclipPoints\n");
    }

    Color colors[] = {
        Color(255, 0, 0),
        Color(0, 255, 0),
        Color(0, 0, 255),
        Color(255, 255, 0),
        Color(0, 255, 255),
        Color(255, 255, 255)
    };

    CircleShape clipPoint;
    clipPoint.setRadius(2);
    clipPoint.setOrigin(2, 2);
    clipPoint.setFillColor(colors[0]);

    for(int i = 0; i < pLen; i++) {
        clipPoint.setFillColor(colors[i]);
        if(!isAlreadyScreenPos) {
            clipPoint.setPosition(worldToScreenPos(points[i], camera).x, worldToScreenPos(points[i], camera).y);
        } else {
            clipPoint.setPosition(points[i].x, points[i].y);
        }
        window->draw(clipPoint);
        if(shouldPrint) {
            printf("%.3f %.3f\n", clipPoint.getPosition().x, clipPoint.getPosition().y);
        }
    }
}

void dbg::printMultiVector3(Vector3f *vectors, int len, char* separator) {
    if(len == 0) {
        return;
    }
    printf("%s\n", separator);
    for(int l = 0; l < len; l++) {
        printVector3("p", *vectors++);
    }
}