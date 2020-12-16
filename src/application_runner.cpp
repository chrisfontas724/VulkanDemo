// Copyright 2019 Sic Studios. All rights reserved.
// Use of this source code is governed by our license that can be
// found in the LICENSE file.

#include "application_runner.hpp"
#include <core/logging.hpp>
#include <chrono>

namespace christalz {

namespace {
// Default values.
const int32_t kTicksPerSecond = 80;
const int32_t kSkipTicks = 1000 / kTicksPerSecond;
const int32_t kMaxFrameSkips = 5;

double time_to_double(timeval* t) {
    return (t->tv_sec + (t->tv_usec/1000000.0));
}

double time_diff(timeval* t1, timeval* t2) {
    return time_to_double(t2) - time_to_double(t1);
}

uint64_t get_time_milliseconds() {
    using namespace std::chrono;
    return duration_cast< milliseconds >(
    system_clock::now().time_since_epoch()).count();
}

} // anonymous namespace


std::shared_ptr<ApplicationRunner> 
ApplicationRunner::create(std::shared_ptr<display::Window> window,
                          std::shared_ptr<dali::Engine> engine) {
    auto result = std::make_shared<ApplicationRunner>();
    CXL_DCHECK(result);

    if (!engine) {
        CXL_LOG(ERROR) << "Invalid Engine!";
        return nullptr;
    }


    engine->linkToWindow(window.get());

    result->window_ = window;
    result->engine_ = engine;
    window->set_delegate(result);
    return std::move(result);
}

ApplicationRunner::ApplicationRunner()
    : ticks_per_second_(kTicksPerSecond)
    , skip_ticks_(kSkipTicks)
    , max_frame_skips_(kMaxFrameSkips)  {
    config_.start_resolution_x = 1024;
    config_.start_resolution_y = 768;
}

ApplicationRunner::~ApplicationRunner() {
     engine_.reset();
}

void ApplicationRunner::setTicksPerSecond(uint32_t ticks_per_second) {
    ticks_per_second_ = ticks_per_second;
    skip_ticks_ = 1000 / ticks_per_second_;
}


bool ApplicationRunner::setApplication(std::weak_ptr<Application> application) {
    // if (auto app = application.lock()) {
    //     application_ = application;
    //     return true;
    // }

    // CXL_LOG(ERROR) << "Application pointer has expired.";
    return false;
}

int32_t ApplicationRunner::run(std::weak_ptr<Application> application) {
    // if (!setApplication(application)) {
    //     CXL_LOG(ERROR) << "Could not set application to runner";
    //     return 1;
    // }

    // auto app = application_.lock();
    // CXL_DCHECK(app);

    // Begin looping.
    next_game_tick_ = get_time_milliseconds();
    while (!window_->shouldClose()) {
       window_->poll();
    }
    return 0;
}

void ApplicationRunner::onUpdate() {
    uint32_t loops = 0;
    while (get_time_milliseconds() > next_game_tick_ && loops < max_frame_skips_) {
        update();

        calculateFrameRate();

        next_game_tick_ += skip_ticks_;
        loops++;
    }

    float interpolation = float( get_time_milliseconds()  + skip_ticks_ - next_game_tick_ )
                        / float( skip_ticks_ );

    drawFrame(interpolation); 
}

void ApplicationRunner::onResize(int32_t width, int32_t height) {
    // CXL_VLOG(9) << "Window resized: " << width << ", " << height;
    // if (auto app = application_.lock()) {
    //     // TODO: Update viewport.
    //     engine_->resizeFramebuffer(width, height);
    // }
}

void ApplicationRunner::onWindowMove(int32_t x, int32_t y) {
 //   CXL_VLOG(9) << "Window moved: " << x << ", " << y;
}

void ApplicationRunner::onClose() {
    // CXL_VLOG(9) << "Window closed!";
    // // Cleanup all assets.
    // cxl::Locator::resetServices();
    // if (auto app = application_.lock()) {

    //     cxl::Locator::resetServices();
    //     // TODO: Cleanup scene graph
    //     // TODO: Block any further rendering from happening.
    // }
}

void ApplicationRunner::onStart(display::Window* window) {
    // cxl::Locator::startupServices();
    // if (auto app = application_.lock()) {
    //     engine_->linkToWindow(window);
    //     app->linkEngine(engine_);
    //     auto graph = app->scene_graph();
    //     ruby::ScriptBindings::set_scene_graph(graph);
    //     ruby::ScriptBindings::set_engine(engine_.get());
    //     ruby::ScriptBindings::set_input_manager(window->input_manager());
    //     graph->start();
    // }
}

void ApplicationRunner::update() {
    // cxl::Locator::updateServices();
    // if (auto app = application_.lock()) {
    //     auto graph = app->scene_graph();
    //     CXL_DCHECK(graph);
    //     graph->update();
    // }
}

void ApplicationRunner::drawFrame(float delta) {
    engine_->render(delta, true, nullptr);
}


void ApplicationRunner::calculateFrameRate() {
    // if (VLOG_IS_ON(4)) {
    //     iteration_++;
    //     if (iteration_ % kTicksPerSecond == 0) {
    //         timeval current_time;
    //         gettimeofday(&current_time, NULL);
    //         double seconds = time_diff(&last_time_, &current_time);
    //         double frame_rate = kTicksPerSecond / seconds;
    //         last_time_ = current_time;
    //         CXL_LOG(INFO) << "FrameRate is " << frame_rate << "fps";
    //     }
    // }
}


} // christalz