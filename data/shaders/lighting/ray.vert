// Copyright 2019 Sic Studios. All rights reserved.
// Use of this source code is governed by our license that can be
// found in the LICENSE file.

#include "types/ray.comp"
#include "types/intersection.comp"

layout(std430, set = 0, binding = 0) buffer buf {
    Ray rays[];
};


// Outputs
layout(location = 0) out vec4 accumulation;


// Simply write out the position with no transformation.
void main() {
    Ray ray = rays[gl_VertexIndex];

    accumulation = ray.accumulation;

    gl_Position = vec4(ray.coord, 1.0, 1.0);
    gl_PointSize = 1.0;
}
