#pragma once
#include "ShaderInterface/ShaderMacros.h"

namespace SI
{
    struct ViewCB
    {
        re_vec4 color;
        re_mat4 worldToViewMatrix;
        re_mat4 viewToWorldMatrix;
        re_mat4 viewToClipMatrix;
        re_mat4 worldToClipMatrix;
        re_mat4 clipToWorldMatrix;
    };
};