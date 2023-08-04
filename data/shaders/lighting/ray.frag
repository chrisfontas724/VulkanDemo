// Copyright 2019 Sic Studios. All rights reserved.
// Use of this source code is governed by our license that can be
// found in the LICENSE file.

// Outputs
layout (location = 0) in vec4 light;
layout (location = 1) in vec4 weight;
layout(location = 0) out vec4 outColor;


// Simply write out the position with no transformation.
void main() {
    outColor = vec4(1.f, 0.0, 1.f, 1.f);//light*weight;
}