#include <algorithm>
#include <chrono>
#include <fstream>

#include <json/json.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "modules.hh"

using kei::modules::niri;

namespace kei::modules
{
    auto
    get_current_time() noexcept -> std::string
    {
        namespace chrono = std::chrono;

        return std::format(
            "{:%H:%M}", chrono::zoned_time { chrono::current_zone(), chrono::system_clock::now() });
    }


    auto
    get_battery_level() -> std::uint8_t
    {
        std::ifstream bat0_capacity { "/sys/class/power_supply/BAT0/capacity" };

        int level = 0;
        bat0_capacity >> level;
        return level;
    }
}


niri::niri()
{
    memset(&m_socket_address, 0, sizeof(sockaddr_un));

    if (const char *path = std::getenv("NIRI_SOCKET"); path == nullptr)
        throw std::runtime_error { "environment variable $NIRI_SOCKET is not defined" };
    else /* NOLINT */
        std::strcpy(m_socket_address.sun_path, path);
    m_socket_address.sun_family = AF_UNIX;

    if (m_fd = socket(AF_UNIX, SOCK_STREAM, 0); m_fd == -1)
        throw std::runtime_error { std::format("failed to create a socket: {}",
                                               std::strerror(errno)) };
}


niri::~niri() { close(m_fd); }


auto
niri::connect() -> niri &
{
    if (::connect(m_fd, reinterpret_cast<sockaddr *>(&m_socket_address), sizeof(sockaddr_un)) < 0)
        throw std::runtime_error { std::format("failed to connect to a socket: {}",
                                               std::strerror(errno)) };
    return *this;
}


auto
niri::run() -> niri &
{
    if (write(m_fd, "\"EventStream\"\n", 14) < 0)
        throw std::runtime_error { std::format("failed to write to a socket: {}",
                                               std::strerror(errno)) };

    std::unique_ptr<Json::CharReader> reader { Json::CharReaderBuilder {}.newCharReader() };

    std::string message;
    Json::Value event;

    for (char c; read(m_fd, &c, 1) > 0;)
    {
        if (c != '\n')
        {
            message += c;
            continue;
        }

        if (!reader->parse(message.begin().base(), message.end().base(), &event, nullptr)) continue;

        mf_on_event(event);

        event.clear();
        message.clear();
    }

    return *this;
}


void
niri::mf_on_event(const Json::Value &event)
{
    if (event.isMember("Ok")) return;
    if (event.isMember("Err")) throw std::runtime_error { event["Err"].asCString() };

    if (event.isMember("WorkspacesChanged"))
    {
        std::ranges::fill(m_workspaces, -1);

        for (const auto &ws : event["WorkspacesChanged"]["workspaces"])
            m_workspaces[ws["idx"].asUInt() - 1] = ws["id"].asUInt();

        signal_on_workspace_changed.emit(9 - std::ranges::count(m_workspaces, -1));
        return;
    }

    if (event.isMember("WorkspaceActivated"))
    {
        const auto &wa = event["WorkspaceActivated"];
        if (!wa["focused"].asBool()) return;

        signal_on_workspace_activated.emit(
            std::distance(m_workspaces.begin(), std::ranges::find(m_workspaces, wa["id"].asUInt()))
            + 1);
    }
}
