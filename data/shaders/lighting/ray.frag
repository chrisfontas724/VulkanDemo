// Copyright 2019 Sic Studios. All rights reserved.
// Use of this source code is governed by our license that can be
// found in the LICENSE file.

// Outputs
layout (location = 0) in vec4 accumulation;

layout(location = 0) out vec4 outColor;


// Simply write out the position with no transformation.
void main() {
    outColor = accumulation;
}