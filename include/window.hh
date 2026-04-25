#pragma once
#include <chrono>

#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/window.h>


namespace kei::ui
{
    class window
    {
    public:
        window(const Glib::RefPtr<Gtk::Application> &app);


        auto set_workspace_info(int amount, int focused) -> window &;
        auto set_current_time(std::chrono::system_clock::time_point time) -> window &;
        auto set_battery_level(int level) -> window &;

        void show(bool timed);
        void hide();

    private:
        Gtk::Window m_window;

        Gtk::Box m_indicator_container;

        Gtk::Box                                       m_workspace_container;
        std::array<std::pair<Gtk::Box, Gtk::Label>, 9> m_workspaces;

        Gtk::Label m_time;
        Gtk::Label m_battery;

        sigc::connection m_hide_timeout;


        void mf_update_workspaces(int amount, int focused);
    };
}

