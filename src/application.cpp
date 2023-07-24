// Copyright 2019 Sic Studios. All rights reserved.
// Use of this source code is governed by our license that can be
// found in the LICENSE file.

#include "application.hpp"
#include <FileStreaming/file_stream.hpp>
#include <UsefulUtils/logging.hpp>

namespace christalz {


bool Application::linkEngine(std::shared_ptr<dali::Engine> engine) {
    // Recreate scene graph.
    CXL_DCHECK(engine);

    // cxl::FileStream json_file;
    // if (!json_file.load(fs_, package_name_ + ".json")) {
    //     CXL_LOG(WARNING) << "Could not find JSON file for application " << package_name_;
    //     return false;
    // }

    // std::string json_string = json_file.text();
    // if (json_string.length() > 0) {
    //     try {
    //         ruby::Deserializer deserializer(json_file.text());
    //         scene_graph_->accept(&deserializer);
    //     } catch(std::runtime_error& e) {
    //         CXL_LOG(WARNING) << e.what();
    //         return false;
    //     }
    // }

    return true;
}

// bool Application::writeToDisk() {
//     if (!fs_ || package_name_.length() <= 0) {
//         return false;
//     }

//     // ruby::Serializer serializer;
//     // scene_graph_->accept(&serializer);

//     // cxl::FileStream json_file;
//     // if (!json_file.load(fs_, package_name_ + ".json")) {
//     //     CXL_LOG(WARNING) << "Could not open json file for application " << package_name_;
//     //     return false;
//     // }

//     // TODO
//     return true;
// }

} // christalz