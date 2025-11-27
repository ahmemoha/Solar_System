#pragma once

#include "Geometry.h"

namespace ShapeGenerator
{
    [[nodiscard]]
    CPU_Geometry Sphere(float radius, int slices, int stacks);

    CPU_Geometry UnitCube();
    CPU_Geometry Ring(float innerRadius, float outerRadius, int segments);
};