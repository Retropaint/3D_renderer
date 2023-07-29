#include <fstream>
#include <cstdlib>
#include <iostream>
#include <strstream>
#include <string>
#include <stdlib.h>
#include "externs.hpp"
#include "debugger/debugger.hpp"

using namespace std;

void readObjFile(char* path, char* mult, Object *obj) {

    // remove existing tris in-case this is a re-used obj
    delete[] obj->tris;

    ifstream file((string)path + ".obj");

    // get vertex and triangle count of mesh so that the appropriate sizes can be allocated
    int vertLen = 0;
    int triLen = 0;
    while(!file.eof()) {
        char line[64];

        file.getline(line, 64);

        if(line[0] == 'v') {
            vertLen++;
        }
        if(line[0] == 'f') {
            triLen++;
        }
    }

    Vector3f *verts = new Vector3f[vertLen];
    obj->tris = new Triangle[triLen];
    obj->triLen = triLen;

    int v = 0;
    int t = 0;

    int multInt = atoi(mult);

    file.clear();
    file.seekg(0);

    while(!file.eof()) {
        char line[128];

        file.getline(line, 128);
        strstream stream;
        
        stream << line;
        char junk;
        float test;

        if(line[0] == 'v') {
            stream >> junk >> verts[v].x >> verts[v].y >> verts[v].z;
            verts[v].x *= multInt;
            verts[v].y *= -multInt;
            verts[v++].z *= multInt;
        }
        
        if(line[0] == 'f') {
            int vIdx[3];
            stream >> junk >> vIdx[0] >> vIdx[1] >> vIdx[2];
            obj->tris[t].verts[0] = verts[vIdx[0] - 1];
            obj->tris[t].verts[1] = verts[vIdx[1] - 1];
            obj->tris[t++].verts[2] = verts[vIdx[2] - 1];
        }
    }

    delete[] verts;
    file.close();
}