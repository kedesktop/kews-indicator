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
            m_dispatcher.connect([this]() { mf_show(); });

            m_niri.signal_on_workspace_changed.connect(
                [this](int amount)
                {
                    m_workspace_amount = amount;
                    m_dispatcher.emit();
                });

            m_niri.signal_on_workspace_activated.connect(
                [this](int index)
                {
                    m_focused_workspace = index;
                    m_dispatcher.emit();
                });

            m_niri.connect();

            app->signal_startup().connect([this]()
                                          { std::thread { [this]() { m_niri.run(); } }.detach(); });
        }

    private:
        ui::window    m_window;
        modules::niri m_niri;

        int m_workspace_amount;
        int m_focused_workspace;

        Glib::Dispatcher m_dispatcher;


        void
        mf_show()
        {
            m_window.set_workspace_amount(m_workspace_amount)
                .set_focused_workspace(m_focused_workspace)
                .set_current_time(modules::get_current_time())
                .set_battery_level(modules::get_battery_level())
                .show();
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
