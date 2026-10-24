#include "../render/renderer.hpp"
#include <cstring>
#include <iomanip>
#include <sstream>
using namespace slide;
namespace
{
constexpr COLORREF mint = RGB(122, 242, 201);
constexpr COLORREF dim = RGB(139, 161, 177);
constexpr COLORREF panel = RGB(12, 23, 34);
} // namespace

void Renderer::ui(const Game &game, bool mute, float fps)
{
    std::memset(bits, 0, W * H * 4);

    rect(24, 24, 4, 37, mint);
    text(40, 22, 27, L"SLIDE", mint, true);
    text(40, 55, 12, L"WRONG CHAT. REAL CONSEQUENCES.", dim);
    text(640, 28, 15, wide(game.world.name), mint, true);
    if (game.mode == Mode::Playing || game.mode == Mode::Paused)
    {
        rect(24, 88, 218, 75, panel);
        text(38, 98, 14,
             L"BEACONS  " + std::to_wstring(game.collected) + L" / " +
                 std::to_wstring(game.world.checkpoints.size()),
             mint, true);
        text(38, 123, 16,
             time(game.elapsed) + L"    /    " + std::to_wstring(game.deaths) + L" resets");
        float speed =
            std::sqrt(game.velocity.x * game.velocity.x + game.velocity.z * game.velocity.z);
        rect(38, 150, 180, 3, RGB(35, 51, 65));
        rect(38, 150, int(speed / 8 * 180), 3, mint);
        text(24, 473, 14, wide(game.world.tip), RGB(203, 221, 226));
    }
    if (game.mode == Mode::Playing)
    {
        const Hazard *nearest = nullptr;
        float distance = 13;
        for (const auto &h : game.world.hazards)
        {
            float d = vec_magnitude(h.at(game.worldTime) - game.position);
            if (d < distance)
            {
                distance = d;
                nearest = &h;
            }
        }
        if (nearest)
        {
            const auto &m = game.world.messages.at(nearest->message);
            rect(600, 340, 336, 124, panel);
            rect(600, 340, 3, 124, RGB(255, 145, 177));
            text(614, 350, 12, L"INCOMING  /  " + wide(m.actual), RGB(255, 145, 177), true);
            wrapped(614, 374, 308, 58, 18, wide(m.body), RGB(240, 230, 235), true);
            text(614, 440, 12, L"Intended for: " + wide(m.intended), dim);
        }
    }
    text(24, 509, 13, L"WASD / ARROWS  Move     SPACE  Jump     SHIFT  Brake     R  Respawn", dim);
    text(680, 509, 13, L"ESC  Pause    M  " + std::wstring(mute ? L"Unmute" : L"Mute"), dim);
    switch (game.mode)
    {
    case Mode::Title:
        draw_title();
        break;
    case Mode::Paused:
        draw_pause();
        break;
    case Mode::Defeated:
        draw_defeat(game);
        break;
    case Mode::Won:
    case Mode::Complete:
        draw_results(game);
        break;
    case Mode::Playing:
        break;
    }
    if (software)
        text(620, 65, 13, L"SOFTWARE GPU FALLBACK - reduced performance", RGB(255, 180, 100));
    else
        text(800, 65, 12, std::to_wstring((int)fps) + L" FPS", dim);
    GdiFlush();
    context->UpdateSubresource(hud.p, 0, nullptr, bits, W * 4, 0);
}
void Renderer::draw_title()
{

    rect(45, 132, 450, 315, panel);
    rect(45, 132, 450, 3, mint);
    text(70, 155, 13, L"CHECK THE RECIPIENT BEFORE YOU SEND", mint, true);
    text(66, 181, 72, L"Find your", RGB(237, 240, 235), true);
    text(66, 255, 72, L"balance.", RGB(237, 240, 235), true);
    text(71, 343, 17, L"Dodge the drafts. Spare yourself the replies.", dim);
    rect(71, 388, 245, 37, RGB(37, 92, 83));
    text(86, 395, 18, L"ENTER  /  Start sliding", mint, true);
}

void Renderer::draw_pause()
{

    rect(260, 174, 440, 205, panel);
    text(288, 196, 42, L"Take a breath.", mint, true);
    text(290, 261, 17, L"ENTER / ESC   Resume");
    text(290, 294, 17, L"BACKSPACE  Restart / F5  Reload assets");
    text(290, 327, 17, L"Q   Quit");
}

void Renderer::draw_defeat(const Game &game)
{

    const COLORREF pink = RGB(255, 145, 177);
    rect(160, 113, 640, 374, panel);
    rect(160, 113, 640, 3, pink);
    text(185, 131, 32,
         game.lastMessage >= 0 ? L"Sent. To the wrong chat." : L"You slid out of the chat.", pink,
         true);
    if (game.lastMessage >= 0)
    {
        const auto &m = game.world.messages.at(size_t(game.lastMessage));
        text(188, 185, 16,
             L"MEANT FOR  " + wide(m.intended) + L"    /    SENT TO  " + wide(m.actual), dim);
        wrapped(188, 221, 580, 78, 27, L"\"" + wide(m.body) + L"\"", RGB(245, 235, 240), true);
        text(188, 312, 13, L"WHAT HAPPENS NEXT", pink, true);
        wrapped(188, 338, 580, 58, 20, wide(m.consequence), RGB(225, 231, 238));
    }
    else
    {
        wrapped(188, 213, 580, 75, 25,
                L"No message was sent. Your drafts are safe; your footing wasn't.",
                RGB(225, 231, 238));
        text(188, 320, 17, L"This fall does not add a wrong-chat consequence.", dim);
    }
    text(188, 419, 18, L"ENTER / R   Retry from checkpoint", mint, true);
    text(188, 453, 14, L"BACKSPACE   Restart course and clear its sends", dim);
}

void Renderer::draw_results(const Game &game)
{

    bool whole = game.mode == Mode::Complete;
    rect(145, 99, 670, 399, panel);
    rect(145, 99, 670, 3, mint);
    auto outcomes = game.outcomes(whole);
    int total = 0;
    for (const auto &outcome : outcomes)
        total += outcome.count;
    text(169, 113, 30,
         whole   ? L"The group chat aftermath"
         : total ? L"You made it. The chat noticed."
                 : L"Made it. Nothing leaked.",
         mint, true);
    text(171, 154, 14,
         time(float(game.elapsed + (whole ? game.journeyElapsed : 0))) + L" / " +
             std::to_wstring(total) + L" wrong sends",
         dim);
    size_t begin = std::min(game.recapPage * 6, outcomes.size());
    size_t end = std::min(begin + 6, outcomes.size());
    for (size_t i = begin; i < end; ++i)
    {
        const auto &outcome = outcomes[i];
        const auto &message = outcome.message;
        int y = 184 + int(i - begin) * 43;
        COLORREF accent = outcome.count ? RGB(255, 145, 177) : mint;
        auto status = outcome.count ? L"SENT " + std::to_wstring(outcome.count) + L"x" : L"AVOIDED";
        text(171, y, 13, status + L" / " + wide(message.actual), accent, true);
        wrapped(171, y + 18, 610, 24, 14,
                wide(outcome.count ? message.consequence : message.avoided), RGB(221, 231, 237));
    }

    text(171, 453, 17, whole ? L"ENTER   New journey" : L"ENTER   Continue", mint, true);
    text(171, 478, 12, L"BACKSPACE  Retry    PGUP / PGDN  Recap pages    F5  Reload assets", dim);
}
