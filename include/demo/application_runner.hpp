// Copyright 2019 Sic Studios. All rights reserved.
// Use of this source code is governed by our license that can be
// found in the LICENSE file.

#ifndef APPLICATION_APPLICATION_RUNNER_HPP_
#define APPLICATION_APPLICATION_RUNNER_HPP_

#include "application.hpp"
#include "application_config.hpp"
#include "engine_interface.hpp"

#include <windowing/window_delegate.hpp>

namespace christalz {
class ApplicationRunner : public display::WindowDelegate {
public:

    enum class Mode {
        kPlayback
    };

    static std::shared_ptr<ApplicationRunner> create(std::shared_ptr<dali::Engine> engine);
    ApplicationRunner();
    ~ApplicationRunner();

    void set_mode(Mode mode){ mode_ = mode; }
    Mode mode() const { return mode_; }

    void setTicksPerSecond(uint32_t ticks_per_second);
    uint32_t ticks_per_second() const { return ticks_per_second_; }

    void set_max_frame_skips(uint32_t max_frame_skips){ max_frame_skips_ = max_frame_skips; }
    uint32_t max_frame_skips() const { return max_frame_skips_; }

    int32_t run(std::weak_ptr<Application> application);

    ApplicationConfig& config() {
        return config_;
    }

protected:

    bool setApplication(std::weak_ptr<Application> application);
    void update();
    void drawFrame(float delta);
    void calculateFrameRate();
    
    // |WindowDelegate|
    void onUpdate() override;
    void onResize(int32_t width, int32_t height) override;
    void onWindowMove(int32_t x, int32_t y) override;
    void onStart(display::Window*) override;
    void onClose() override;

    ApplicationConfig config_;

    std::weak_ptr<Application> application_;
    std::shared_ptr<dali::Engine> engine_;
    Mode mode_;

    //timeval last_time_;
    uint64_t iteration_;
    uint32_t ticks_per_second_;
    uint32_t skip_ticks_;
    uint32_t max_frame_skips_;
    uint64_t next_game_tick_;
};
}

#endif // APPLICATION_APPLICATION_RUNNER_HPP_