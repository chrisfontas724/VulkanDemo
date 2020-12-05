// Copyright 2019 Sic Studios. All rights reserved.
// Use of this source code is governed by our license that can be
// found in the LICENSE file.

#ifndef APPLICATIONS_APPLICATION_HPP_
#define APPLICATIONS_APPLICATION_HPP_

#include "streaming/file_system.hpp"
#include "engine_interface.hpp"

namespace christalz {
class Application {
public:

    bool linkEngine(std::shared_ptr<dali::Engine> engine);

    const std::string& package_name() const { return package_name_; }

private:

    const cxl::FileSystem* fs_;
    std::string package_name_;

};
} // christalz

#endif // APPLICATIONS_APPLICATION_HPP_