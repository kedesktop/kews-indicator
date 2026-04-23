#pragma once
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

        auto connect() -> niri &;
        auto run() -> niri &;


        sigc::signal<void(int)> signal_on_workspace_changed;
        sigc::signal<void(int)> signal_on_workspace_activated;
        sigc::signal<void(bool)> signal_on_overview_toggle;

    private:
        sockaddr_un m_socket_address;
        int         m_fd;

        std::array<std::int8_t, 9> m_workspaces;

        void mf_on_event(const Json::Value &event);
    };


    [[nodiscard]] auto get_current_time() noexcept -> std::string;
    [[nodiscard]] auto get_battery_level() -> std::uint8_t;
}
