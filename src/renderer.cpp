
#include "base/sugars.hpp"

#include "renderer.hpp"

PUSH_CLEAR(PushClear) { push_buffer->clear.color = clear.color; }

PUSH_CUBE(PushCube) {
    u32 idx = push_buffer->cube.count;
    if (idx < push_buffer->cube.size) {
        push_buffer->cube.cubes[idx] = cube;
        push_buffer->cube.count++;
    } else {
        Assert(false);
    }
}

PUSH_CUBE_WF(PushCubeWF) {
    u32 idx = push_buffer->cube_wf.count;
    if (idx < push_buffer->cube_wf.size) {
        push_buffer->cube_wf.cubes[idx] = cube_wf;
        push_buffer->cube_wf.count++;
    } else {
        Assert(false);
    }
}
