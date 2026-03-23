
#include <base/include.cpp>

#include "renderer.hpp"

PUSH_CLEAR(PushClear) { push_buffer->clear.color = clear.color; }

PUSH_CUBE(PushCube) {
    u32 idx = push_buffer->cube_buffer.count;

    if (idx < push_buffer->cube_buffer.size) {

        push_buffer->cube_buffer.cubes[idx] = cube;
        push_buffer->cube_buffer.count++;
    } else {
        Assert(false);
    }
}

PUSH_CUBE_WF(PushCubeWF) {
    u32 idx = push_buffer->cube_wf_buffer.count;

    if (idx < push_buffer->cube_wf_buffer.size) {

        push_buffer->cube_wf_buffer.cubes[idx] = cube_wf;
        push_buffer->cube_wf_buffer.count++;
    } else {
        Assert(false);
    }
}
