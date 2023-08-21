#include "debugger/debugger.hpp"
#include "transformer/transformer.hpp"
#include "directives.hpp"
#include "externs.hpp"

using namespace sf;

void dbg::printVector3(char* prefix, Vector3f vector) {
    printf("%s: %.1f, %.1f, %.1f\n", prefix, vector.x, vector.y, vector.z);
}