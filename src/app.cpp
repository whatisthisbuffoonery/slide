#include "app.hpp"
#include <chrono>
using namespace slide;

Application::Application(HINSTANCE instance, int show, bool smoke)
    : log(directory() / "slide.log"), smokeTest(smoke)
{
    courseFolders = load_campaign(directory() / "assets");
    window = create_window(instance, input);
    renderer.init(window);
    load_course(0, true);
    audio.muted = smoke;
    audio.start();
    log << "Adapter: " << std::string(renderer.adapter.begin(), renderer.adapter.end()) << "\n";
    log << "Software fallback: " << renderer.software << "\n";
    log << "Audio device: " << audio.available() << "\n";
    if (!smoke)
        ShowWindow(window, show);
}
Application::~Application()
{
    if (window)
        DestroyWindow(window);
}
void Application::load_course(size_t index, bool newJourney)
{
    // Parse and decode first: a malformed content edit must not discard the current game.
    Course course = slide::load_course(courseFolders.at(index), seed + unsigned(index));
    auto sounds = load_sound_bank(course.audio);
    renderer.level(course);
    audio.set_bank(std::move(sounds));
    if (newJourney)
        game.new_journey(std::move(course));
    else
        game.load(std::move(course), int(index));
    accumulator = 0;
    pendingJump = false;
}
void Application::handle_controls()
{
    if (input.lostFocus && game.mode == Mode::Playing)
        game.mode = Mode::Paused;
    if (input.pressed['M'])
        audio.muted = !audio.muted.load();
    if (input.pressed['Q'] && game.mode != Mode::Playing)
        input.quit = true;
    if (input.pressed[VK_ESCAPE])
    {
        if (game.mode == Mode::Playing)
            game.mode = Mode::Paused;
        else if (game.mode == Mode::Paused)
            game.mode = Mode::Playing;
    }
    if (input.pressed[VK_F5] || (input.pressed[VK_BACK] && game.mode != Mode::Playing))
    {
        try
        {
            load_course(size_t(game.level));
            game.mode = Mode::Playing;
        }
        catch (const std::exception &error)
        {
            game.mode = Mode::Paused;
            auto text = Renderer::wide(error.what());
            MessageBoxW(window, text.c_str(), L"Course reload failed", MB_OK | MB_ICONERROR);
        }
        return;
    }
    if (input.pressed[VK_RETURN])
    {
        switch (game.mode)
        {
        case Mode::Playing:
            break;
        case Mode::Title:
        case Mode::Paused:
            game.mode = Mode::Playing;
            break;
        case Mode::Defeated:
            game.retry();
            break;
        case Mode::Won:
            if (size_t(game.level + 1) == courseFolders.size())
                game.mode = Mode::Complete;
            else
            {
                // Content validation happens before the prior course is committed.
                auto next = slide::load_course(courseFolders.at(size_t(game.level + 1)),
                                               seed + unsigned(game.level + 1));
                auto sounds = load_sound_bank(next.audio);
                renderer.level(next);
                game.commit_course();
                game.load(std::move(next), game.level + 1);
                audio.set_bank(std::move(sounds));
                game.mode = Mode::Playing;
            }
            break;
        case Mode::Complete:
            ++seed;
            load_course(0, true);
            game.mode = Mode::Playing;
            break;
        }
    }
    if (input.pressed['R'])
    {
        if (game.mode == Mode::Playing)
            game.reset();
        else if (game.mode == Mode::Defeated)
            game.retry();
    }
    if (input.pressed[VK_SPACE] && game.mode == Mode::Playing)
        pendingJump = true;
    if (game.mode == Mode::Won || game.mode == Mode::Complete)
    {
        size_t pages = (game.outcomes(game.mode == Mode::Complete).size() + 5) / 6;
        if (input.pressed[VK_NEXT] && game.recapPage + 1 < pages)
            ++game.recapPage;
        if (input.pressed[VK_PRIOR] && game.recapPage > 0)
            --game.recapPage;
    }
}
void Application::advance_simulation(double elapsed)
{
    constexpr double fixedStep = 1.0 / 120.0;
    if (game.mode != Mode::Playing)
    {
        accumulator = 0;
        pendingJump = false;
        return;
    }
    accumulator += std::min(elapsed, 0.1);
    int steps = 0;
    while (accumulator >= fixedStep && steps < 12)
    {
        game.tick(input.movement(pendingJump), fixedStep);
        pendingJump = false;
        accumulator -= fixedStep;
        ++steps;
    }
}
void Application::dispatch_sound_events()
{
    for (SoundEvent event : game.events)
        audio.play(event);
    game.events.clear();
}
void Application::draw_frame(double elapsed)
{
    renderer.ui(game, audio.muted, fps);
    renderer.draw_frame(game, float(std::min(elapsed, 0.1)));
}
int Application::run()
{
    auto previous = std::chrono::steady_clock::now();
    double fpsTime = 0;
    unsigned fpsFrames = 0;
    while (!input.quit)
    {
        input.poll();
        if (input.quit)
            break;
        auto now = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration<double>(now - previous).count();
        previous = now;
        fpsTime += elapsed;
        if (++fpsFrames > 15 && fpsTime > 0.25)
        {
            fps = float(fpsFrames / fpsTime);
            fpsFrames = 0;
            fpsTime = 0;
        }

        handle_controls();
        advance_simulation(elapsed);
        dispatch_sound_events();
        if (smokeTest)
            smoke_step();
        draw_frame(smokeTest ? 1.0 / 60.0 : elapsed);

        if (smokeTest)
        {
            renderer.screenshot(directory() / ("smoke-" + std::to_string(frameNumber) + ".bmp"));
            log << "Rendered state " << int(game.mode) << ", course " << game.level + 1 << "\n";
            log.flush();
            if (frameNumber >= 9)
                input.quit = true;
        }
        ++frameNumber;
        input.finish_frame();
        if (!input.active && !smokeTest)
            Sleep(30);
    }
    log << "Clean shutdown.\n";
    return 0;
}
