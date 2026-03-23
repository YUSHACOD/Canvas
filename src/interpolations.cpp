#ifndef INTERPOLANTS_CPP
#define INTERPOLANTS_CPP
//  interpolants of various kind : --------------------------------------------------- (section)  //

#include <base/include.cpp>

Enum(Interpolation_Kind,
    IK_InSine,
    IK_OutSine,
    IK_InOutSine,
    IK_InQuad,
	IK_InOutQuad,
    IK_OutQuad,
    IK_InCubic,
    IK_OutCubic,
    IK_InOutCubic,
    IK_InQuart,
    IK_OutQuart,
    IK_InOutQuart,
    IK_InQuint,
    IK_OutQuint,
    IK_InOutQuint,
    IK_OutExpo,
    IK_InExpo,
    IK_InOutExpo,
    IK_InCirc,
    IK_OutCirc,
    IK_InOutCirc,
    IK_InBack,
    IK_OutBack,
    IK_InOutBack,
    IK_InElastic,
    IK_OutElastic,
    IK_InOutElastic,
    IK_OutBounce,
    IK_InBounce,
    IK_InOutBounce);

typedef f32 interpolator_type(f32);


f32 easeInSine(f32 x) { return 1 - cos((x * Pi32) / 2); }

f32 easeOutSine(f32 x) { return sin((x * Pi32) / 2); }

f32 easeInOutSine(f32 x) { return -(cos(Pi32 * x) - 1) / 2; }

f32 easeInQuad(f32 x) { return x * x; }

f32 easeOutQuad(f32 x) { return 1 - (1 - x) * (1 - x); }

f32 easeInOutQuad(f32 x) {
    if (x < 0.5f) {
        return 2.0f * x * x;
    } else {
        f32 t = -2.0f * x + 2.0f;
        return 1.0f - (t * t) / 2.0f;
    }
}

f32 easeInCubic(f32 x) { return x * x * x; }

f32 easeOutCubic(f32 x) {
    f32 t = 1 - x;
    t     = t * t * t;
    return 1 - t;
}

f32 easeInOutCubic(f32 x) {
    if (x < 0.5f) {
        return 4.0f * x * x * x;
    } else {
        f32 t = -2.0f * x + 2.0f;
        return 1.0f - (t * t * t) / 2.0f;
    }
}


f32 easeInQuart(f32 x) { return x * x * x * x; }

f32 easeOutQuart(f32 x) {
    f32 t  = 1.0f - x;
    f32 t2 = t * t;
    return 1.0f - (t2 * t2); // (1 - x)^4
}

f32 easeInOutQuart(f32 x) {
    if (x < 0.5f) {
        return 8.0f * x * x * x * x;
    } else {
        f32 t  = -2.0f * x + 2.0f;
        f32 t2 = t * t;
        return 1.0f - (t2 * t2) / 2.0f; // t^4
    }
}

f32 easeInQuint(f32 x) {
    f32 x2 = x * x;
    f32 x4 = x2 * x2;
    return x4 * x; // x^5
}

f32 easeOutQuint(f32 x) {
    f32 t  = 1.0f - x;
    f32 t2 = t * t;
    f32 t4 = t2 * t2;
    return 1.0f - (t4 * t); // (1 - x)^5
}

f32 easeInOutQuint(f32 x) {
    if (x < 0.5f) {
        return 16.0f * x * x * x * x * x;
    } else {
        f32 t  = -2.0f * x + 2.0f;
        f32 t2 = t * t;
        f32 t4 = t2 * t2;
        return 1.0f - (t4 * t) / 2.0f; // t^5
    }
}


f32 easeOutExpo(f32 x) { return (x == 1.0f) ? 1.0f : 1.0f - powf(2.0f, -10.0f * x); }



f32 easeInExpo(f32 x) { return (x == 0.0f) ? 0.0f : powf(2.0f, 10.0f * x - 10.0f); }

f32 easeInOutExpo(f32 x) {
    if (x == 0.0f)
        return 0.0f;
    if (x == 1.0f)
        return 1.0f;

    if (x < 0.5f) {
        return powf(2.0f, 20.0f * x - 10.0f) / 2.0f;
    } else {
        return (2.0f - powf(2.0f, -20.0f * x + 10.0f)) / 2.0f;
    }
}

f32 easeInCirc(f32 x) { return 1.0f - sqrtf(1.0f - x * x); }

f32 easeOutCirc(f32 x) { return sqrtf(1.0f - (x - 1.0f) * (x - 1.0f)); }

f32 easeInOutCirc(f32 x) {
    if (x < 0.5f) {
        return (1.0f - sqrtf(1.0f - (2.0f * x) * (2.0f * x))) / 2.0f;
    } else {
        f32 t = -2.0f * x + 2.0f;
        return (sqrtf(1.0f - t * t) + 1.0f) / 2.0f;
    }
}

f32 easeInBack(f32 x) {
    const f32 c1 = 1.70158f;
    const f32 c3 = c1 + 1.0f;

    return c3 * x * x * x - c1 * x * x;
}

f32 easeOutBack(f32 x) {
    const f32 c1 = 1.70158f;
    const f32 c3 = c1 + 1.0f;

    f32 t = x - 1.0f;
    return 1.0f + c3 * powf(t, 3.0f) + c1 * powf(t, 2.0f);
}

f32 easeInOutBack(f32 x) {
    const f32 c1 = 1.70158f;
    const f32 c2 = c1 * 1.525f;

    if (x < 0.5f) {
        f32 t = 2.0f * x;
        return (t * t * ((c2 + 1.0f) * t - c2)) / 2.0f;
    } else {
        f32 t = 2.0f * x - 2.0f;
        return (t * t * ((c2 + 1.0f) * t + c2) + 2.0f) / 2.0f;
    }
}

f32 easeInElastic(f32 x) {
    const f32 c4 = (2.0f * Pi32) / 3.0f;

    if (x == 0.0f)
        return 0.0f;
    if (x == 1.0f)
        return 1.0f;

    return -powf(2.0f, 10.0f * x - 10.0f) * sinf((x * 10.0f - 10.75f) * c4);
}

f32 easeOutElastic(f32 x) {
    const f32 c4 = (2.0f * Pi32) / 3.0f;

    if (x == 0.0f)
        return 0.0f;
    if (x == 1.0f)
        return 1.0f;

    return powf(2.0f, -10.0f * x) * sinf((x * 10.0f - 0.75f) * c4) + 1.0f;
}



f32 easeInOutElastic(f32 x) {
    const f32 c5 = (2.0f * Pi32) / 4.5f;

    if (x == 0.0f)
        return 0.0f;
    if (x == 1.0f)
        return 1.0f;

    if (x < 0.5f) {
        return -(powf(2.0f, 20.0f * x - 10.0f) * sinf((20.0f * x - 11.125f) * c5)) / 2.0f;
    } else {
        return (powf(2.0f, -20.0f * x + 10.0f) * sinf((20.0f * x - 11.125f) * c5)) / 2.0f + 1.0f;
    }
}

f32 easeOutBounce(f32 x) {
    const f32 n1 = 7.5625f;
    const f32 d1 = 2.75f;

    if (x < 1.0f / d1) {
        return n1 * x * x;
    } else if (x < 2.0f / d1) {
        x -= 1.5f / d1;
        return n1 * x * x + 0.75f;
    } else if (x < 2.5f / d1) {
        x -= 2.25f / d1;
        return n1 * x * x + 0.9375f;
    } else {
        x -= 2.625f / d1;
        return n1 * x * x + 0.984375f;
    }
}

f32 easeInBounce(f32 x) { return 1.0f - easeOutBounce(1.0f - x); }

f32 easeInOutBounce(f32 x) {
    return (x < 0.5f) ? (1.0f - easeOutBounce(1.0f - 2.0f * x)) / 2.0f
                      : (1.0f + easeOutBounce(2.0f * x - 1.0f)) / 2.0f;
}


// clang-format off
global interpolator_type* interps[EnumCount(Interpolation_Kind)] = {
    easeInSine,
    easeOutSine,
    easeInOutSine,
    easeInQuad,
    easeOutQuad,
	easeInOutQuad,
    easeInCubic,
    easeOutCubic,
    easeInOutCubic,
    easeInQuart,
    easeOutQuart,
    easeInOutQuart,
    easeInQuint,
    easeOutQuint,
    easeInOutQuint,
    easeOutExpo,
    easeInExpo,
    easeInOutExpo,
    easeInCirc,
    easeOutCirc,
    easeInOutCirc,
    easeInBack,
    easeOutBack,
    easeInOutBack,
    easeInElastic,
    easeOutElastic,
    easeInOutElastic,
    easeOutBounce,
    easeInBounce,
    easeInOutBounce
};
// clang-format on

//  (section) --------------------------------------------------- : interpolants of various kind  //
#endif
