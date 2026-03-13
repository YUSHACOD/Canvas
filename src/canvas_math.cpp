
#include "base/include.cpp"

v3 quad_bezier(v3 a, v3 b, v3 c, f32 t) {

    v3 d = lerp(a, b, t);
    v3 e = lerp(b, c, t);

    v3 res = lerp(d, e, t);

    return res;
}
