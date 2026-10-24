#include "app.hpp"

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, LPWSTR command, int show)
{
    bool smoke = wcsstr(command, L"--smoke") != nullptr;
    try
    {
        SetProcessDPIAware();
        if (wcsstr(command, L"--validate"))
        {
            std::ofstream report(directory() / "validation.log");
            for (const auto &folder : slide::load_campaign(directory() / "assets"))
            {
                auto course = slide::load_course(folder);
                slide::load_sound_bank(course.audio);
                report << course.id << ": messages, layout and WAV files OK\n";
            }
            return 0;
        }
        Application application(instance, show, smoke);
        return application.run();
    }
    catch (const std::exception &error)
    {
        std::ofstream report(directory() / "error.log");
        report << error.what() << "\n";
        if (!smoke)
        {
            auto text = Renderer::wide(error.what());
            MessageBoxW(nullptr, text.c_str(), L"Slide could not continue", MB_OK | MB_ICONERROR);
        }
        return 1;
    }
}
