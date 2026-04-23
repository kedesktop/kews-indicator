#include <thread>

#include <gtk4-layer-shell.h>
#include <gtkmm.h>
#include <json/value.h>

#include "modules.hh"
#include "window.hh"


namespace kei
{
    class controller
    {
    public:
        controller(const Glib::RefPtr<Gtk::Application> &app) : m_window { app }
        {
            m_show_dispatcher.connect([this] { mf_show(); });
            m_hide_dispatcher.connect([this] { m_window.hide(); });

            m_niri.signal_on_workspace_changed.connect(
                [this](int amount)
                {
                    m_workspace_amount = amount;
                    m_show_dispatcher.emit();
                });

            m_niri.signal_on_workspace_activated.connect(
                [this](int index)
                {
                    m_focused_workspace = index;
                    m_show_dispatcher.emit();
                });

            m_niri.signal_on_overview_toggle.connect(
                [this](bool in_overview)
                {
                    m_in_overview = in_overview;

                    if (in_overview)
                        m_show_dispatcher.emit();
                    else
                        m_hide_dispatcher.emit();
                });

            m_niri.connect();

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
            m_window.set_workspace_amount(m_workspace_amount)
                .set_focused_workspace(m_focused_workspace)
                .set_current_time(modules::get_current_time())
                .set_battery_level(modules::get_battery_level())
                .show(!m_in_overview);
        }
    };
}


auto
main() -> int
{
    auto app = Gtk::Application::create("org.kedesktop.kews-indicator");

    kei::controller controller { app };

    return app->run();
}
