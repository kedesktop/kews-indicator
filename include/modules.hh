#pragma once
#include <chrono>

#include <sigc++/signal.h>
#include <sys/un.h>

namespace Json { class Value; }


namespace kei::modules
{
    class niri
    {
    public:
        niri();
        ~niri();

        void run();


        void (*signal_on_workspace_event)(int, int) = nullptr;
        void (*signal_on_overview_toggle)(bool)     = nullptr;

    private:
        sockaddr_un m_address;

        int m_eventstream_fd;
        int m_workspaces_fd;

        std::array<std::int8_t, 10> m_workspaces;

        void mf_on_event(const Json::Value &event);
    };


    [[nodiscard]] auto get_current_time() noexcept -> std::chrono::system_clock::time_point;
    [[nodiscard]] auto get_battery_level() -> std::uint8_t;
}
