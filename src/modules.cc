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
    get_current_time() noexcept -> std::chrono::system_clock::time_point
    { return std::chrono::system_clock::now(); }


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
    memset(&m_address, 0, sizeof(sockaddr_un));

    if (const char *path = std::getenv("NIRI_SOCKET"); path == nullptr)
        throw std::runtime_error { "environment variable $NIRI_SOCKET is not defined" };
    else /* NOLINT */
        std::strcpy(m_address.sun_path, path);
    m_address.sun_family = AF_UNIX;

    for (int *fd : { &m_eventstream_fd, &m_workspaces_fd })
    {
        if (*fd = socket(AF_UNIX, SOCK_STREAM, 0); *fd == -1)
            throw std::runtime_error { std::format("failed to create a socket: {}",
                                                   std::strerror(errno)) };

        if (::connect(*fd, reinterpret_cast<sockaddr *>(&m_address), sizeof(sockaddr_un)) < 0)
            throw std::runtime_error { std::format("failed to connect to a socket: {}",
                                                   std::strerror(errno)) };
    }
}


niri::~niri()
{
    for (int *fd : { &m_eventstream_fd, &m_workspaces_fd }) close(*fd);
}


void
niri::run()
{
    if (write(m_eventstream_fd, "\"EventStream\"\n", 14) != 14)
        throw std::runtime_error { std::format("failed to write to a socket: {}",
                                               std::strerror(errno)) };

    std::unique_ptr<Json::CharReader> reader { Json::CharReaderBuilder {}.newCharReader() };

    std::string message;
    Json::Value event;

    for (char c0; read(m_eventstream_fd, &c0, 1) > 0;)
    {
        message += c0;
        if (c0 != '\n') continue;

        if (!reader->parse(message.begin().base(), message.end().base(), &event, nullptr)) continue;
        message.clear();

        mf_on_event(event);

        if (write(m_workspaces_fd, "\"Workspaces\"\n", 13) != 13)
            throw std::runtime_error { std::format("failed to write to a socket: {}",
                                                   std::strerror(errno)) };

        for (char c1; read(m_workspaces_fd, &c1, 1) > 0 && c1 != '\n';) message += c1;

        if (!reader->parse(message.begin().base(), message.end().base(), &event, nullptr))
        {
            message.clear();
            continue;
        }

        message.clear();
        mf_on_event(event);
    }
}


void
niri::mf_on_event(const Json::Value &event)
{
    if (event.isMember("Err"))
        throw std::runtime_error { std::format("niri error: {}", event["Err"].asString()) };
    if (event.isMember("OverviewOpenedOrClosed"))
    {
        if (signal_on_overview_toggle != nullptr)
            signal_on_overview_toggle(event["OverviewOpenedOrClosed"]["is_open"].asBool());
        return;
    }

    const Json::Value *workspaces = nullptr;
    bool               changed    = false;

    if (event.isMember("WorkspacesChanged"))
    {
        std::ranges::fill(m_workspaces, -1);
        changed = true;

        workspaces = &event["WorkspacesChanged"]["workspaces"];
    }
    else if (event.isMember("Ok") && event["Ok"].isObject() && event["Ok"].isMember("Workspaces"))
        workspaces = &event["Ok"]["Workspaces"];

    if (workspaces == nullptr) return;

    for (const auto &ws : *workspaces)
    {
        if (changed) m_workspaces[ws["idx"].asUInt() - 1] = ws["id"].asUInt();
        if (!ws["is_focused"].asBool()) continue;

        if (signal_on_workspace_event != nullptr)
            signal_on_workspace_event(workspaces->size(), ws["idx"].asUInt());
    }
}
