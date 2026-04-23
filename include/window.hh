#pragma once
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/window.h>


namespace kei::ui
{
    class window
    {
    public:
        window(const Glib::RefPtr<Gtk::Application> &app);


        auto set_workspace_amount(int amount) -> window &;
        auto set_focused_workspace(int index) -> window &;
        auto set_current_time(std::string_view time) -> window &;
        auto set_battery_level(int level) -> window &;
        void show();

    private:
        Gtk::Window m_window;

        Gtk::Box m_indicator_container;

        Gtk::Box                                       m_workspace_container;
        std::array<std::pair<Gtk::Box, Gtk::Label>, 9> m_workspaces;

        Gtk::Label m_time;
        Gtk::Label m_battery;

        int m_workspace_amount;
        int m_focused_workspace;

        sigc::connection m_hide_timeout;


        void mf_update_workspaces();
    };
}

