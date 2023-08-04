// Copyright 2019 Sic Studios. All rights reserved.
// Use of this source code is governed by our license that can be
// found in the LICENSE file.

#include "types/ray.comp"
#include "types/intersection.comp"

layout(std430, set = 0, binding = 0) buffer buf {
    Ray rays[];
};

layout(std140, set = 0, binding = 1) buffer buf2 {
    HitPoint intersections[];
};

// Outputs
layout(location = 0) out vec4 light;
layout(location = 1) out vec4 weight;


// Simply write out the position with no transformation.
void main() {
    Ray ray = rays[gl_VertexIndex];
    HitPoint intersection = intersections[gl_VertexIndex];

    light = intersection.emission;
    weight = ray.weight;

    gl_Position = vec4(ray.coord, 1.0, 1.0);
    gl_PointSize = 1.0;
}
