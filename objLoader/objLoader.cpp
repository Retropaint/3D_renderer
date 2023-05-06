#include <fstream>
#include <iostream>
#include <strstream>
#include <string>
#include <stdlib.h>
#include "externs.hpp"
#include "debugger/debugger.hpp"

using namespace std;

void readObjFile(char* path, char* mult, Object *obj) {
    ifstream file((string)path + ".obj");

    Vector3f verts[6000];
    int vLen = 0;
    int tLen = 0;

    obj->triLen = 0;

    while(!file.eof()) {
        char line[128];

        file.getline(line, 128);
        strstream stream;
        
        stream << line;
        char junk;
        float test;

        if(line[0] == 'v') {
            stream >> junk >> verts[vLen].x >> verts[vLen].y >> verts[vLen].z;
            verts[vLen].x *= atoi(mult);
            verts[vLen].y *= -atoi(mult);
            verts[vLen++].z *= atoi(mult);
        }
        if(line[0] == 'f') {
            int vIdx[3];
            stream >> junk >> vIdx[0] >> vIdx[1] >> vIdx[2];
            obj->tris[tLen].verts[0] = verts[vIdx[0] - 1];
            obj->tris[tLen].verts[1] = verts[vIdx[1] - 1];
            obj->tris[tLen++].verts[2] = verts[vIdx[2] - 1];
        }
    }
    obj->triLen = tLen;
}