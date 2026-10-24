#pragma once
#include "platform/input.hpp"
#include "render/renderer.hpp"
#include "audio/audio.hpp"
#include <fstream>

class Application
{
  public:
    Application(HINSTANCE instance, int show, bool smoke);
    ~Application();
    int run();

  private:
    // One application owns the platform, game, renderer and audio lifetimes.
    WindowInput input;
    HWND window = nullptr;
    slide::Game game;
    Renderer renderer;
    slide::Audio audio;
    std::vector<fs::path> courseFolders;
    std::ofstream log;
    bool smokeTest = false;
    unsigned seed = 1;
    double accumulator = 0;
    bool pendingJump = false;
    float fps = 60;
    unsigned frameNumber = 0;

    void load_course(size_t index, bool newJourney = false);
    void handle_controls();
    void advance_simulation(double elapsed);
    void dispatch_sound_events();
    void draw_frame(double elapsed);
    void smoke_step();
};
