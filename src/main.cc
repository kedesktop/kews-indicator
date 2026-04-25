#include <iostream>
#include <thread>

#include <gtk4-layer-shell.h>
#include <gtkmm.h>
#include <json/value.h>

#include "modules.hh"
#include "window.hh"


namespace kei
{
    static class controller *g_controller = nullptr;


    class controller
    {
    public:
        controller(const Glib::RefPtr<Gtk::Application> &app) : m_window { app }
        {
            g_controller = this;

            m_show_dispatcher.connect([this] { mf_show(); });
            m_hide_dispatcher.connect([this] { m_window.hide(); });

            m_niri.signal_on_workspace_event = [](int amount, int focused)
            {
                if (g_controller->m_workspace_amount == amount
                    && g_controller->m_focused_workspace == focused)
                    return;

                g_controller->m_workspace_amount  = amount;
                g_controller->m_focused_workspace = focused;

                g_controller->m_show_dispatcher.emit();
            };

            m_niri.signal_on_overview_toggle = [](bool in_overview)
            {
                g_controller->m_in_overview = in_overview;

                if (in_overview)
                    g_controller->m_show_dispatcher.emit();
                else
                    g_controller->m_hide_dispatcher.emit();
            };

            app->signal_startup().connect([this]
                                          { std::thread { [this] { m_niri.run(); } }.detach(); });
        }

    private:
        ui::window    m_window;
        modules::niri m_niri;

        int  m_workspace_amount;
        int  m_focused_workspace;
        bool m_in_overview = false;

        Glib::Dispatcher m_show_dispatcher;
        Glib::Dispatcher m_hide_dispatcher;


        void
        mf_show()
        {
            m_window.set_current_time(modules::get_current_time())
                .set_battery_level(modules::get_battery_level())
                .set_workspace_info(m_workspace_amount, m_focused_workspace)
                .show(!m_in_overview);
        }
    };
}


auto
main() -> int
try
{
    auto app = Gtk::Application::create("org.kedesktop.kews-indicator");

    kei::controller controller { app };

    return app->run();
}
catch (const std::exception &e)
{
    std::cerr << e.what() << '\n';
}
