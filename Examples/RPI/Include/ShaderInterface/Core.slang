#pragma once
#include "ShaderInterface/ShaderMacros.h"

namespace SI
{
    re_struct ViewCB
    {
        re_mat4 worldToViewMatrix;
        re_mat4 viewToWorldMatrix;
        re_mat4 viewToClipMatrix;
        re_mat4 worldToClipMatrix;
        re_mat4 clipToWorldMatrix;
    };

    re_struct TransformUB
    {
        re_mat4 modelToWorldMatrix;
    };
};