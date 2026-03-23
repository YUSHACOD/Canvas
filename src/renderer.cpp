
#include <base/include.cpp>

#include "renderer.hpp"

R_PUSH_CLEAR(R_PushClear) { push_buffer->clear.color = clear.color; }

R_PUSH_CUBE(R_PushCube) {
    u32 idx = push_buffer->cube_buffer.count;

    if (idx < push_buffer->cube_buffer.size) {

        push_buffer->cube_buffer.cubes[idx] = cube;
        push_buffer->cube_buffer.count++;
    } else {
        Assert(false);
    }
}

R_PUSH_CUBE_WF(R_PushCubeWF) {
    u32 idx = push_buffer->cube_wf_buffer.count;

    if (idx < push_buffer->cube_wf_buffer.size) {

        push_buffer->cube_wf_buffer.cubes[idx] = cube_wf;
        push_buffer->cube_wf_buffer.count++;
    } else {
        Assert(false);
    }
}
