#pragma once
#include <vector>
#include "SceneObject.h"

struct Scene
{
    std::vector<SceneObject> objects;
    float globalScale = 1.0f;
};
