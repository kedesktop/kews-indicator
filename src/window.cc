#include <gtk4-layer-shell.h>
#include <gtkmm.h>

#include "window.hh"

using kei::ui::window;

namespace
{
    void
    load_css()
    {
        auto css = Gtk::CssProvider::create();
        css->load_from_string("window {"
                              "    background-color: transparent;"
                              "}"
                              ".indicator-container {"
                              "    font-family: \"Maple Font NF\";"
                              "    border: 2px solid #202020;"
                              "    background-color: #0e0e0e;"
                              "    padding: 5px;"
                              "}"
                              ".workspace-container {"
                              "    background-color: #202020;"
                              "    border-radius: 5px;"
                              "}"
                              ".workspace-number {"
                              "    margin: 5px;"
                              "}"
                              ".workspace-number-container.begin-active {"
                              "    background-color: #768fa2;"
                              "    border-radius: 5px 5px 0px 0px;"
                              "}"
                              ".workspace-number-container.center-active {"
                              "    background-color: #768fa2;"
                              "    border-radius: 0px;"
                              "}"
                              ".workspace-number-container.end-active {"
                              "    background-color: #768fa2;"
                              "    border-radius: 0px 0px 5px 5px;"
                              "}"
                              ".time-label {"
                              "    background-color: #202020;"
                              "    border-radius: 5px;"
                              "    margin-top: 5px;"
                              "    padding: 5px;"
                              "}"
                              ".battery-label {"
                              "    background-color: #202020;"
                              "    border-radius: 5px;"
                              "    margin-top: 5px;"
                              "    padding: 5px;"
                              "}");

        Gtk::StyleContext::add_provider_for_display(Gdk::Display::get_default(), css,
                                                    GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }
}


window::window(const Glib::RefPtr<Gtk::Application> &app)
{
    app->signal_startup().connect(sigc::ptr_fun(&load_css));
    app->signal_activate().connect([&] { app->hold(); });

    m_window.set_default_size(50, 50);

    gtk_layer_init_for_window(m_window.gobj());
    gtk_layer_set_layer(m_window.gobj(), GTK_LAYER_SHELL_LAYER_OVERLAY);
    gtk_layer_set_anchor(m_window.gobj(), GTK_LAYER_SHELL_EDGE_LEFT, TRUE);
    gtk_layer_set_margin(m_window.gobj(), GTK_LAYER_SHELL_EDGE_LEFT, 7);

    m_window.set_child(m_indicator_container);

    m_indicator_container.set_orientation(Gtk::Orientation::VERTICAL);
    m_indicator_container.set_vexpand(false);
    m_indicator_container.add_css_class("indicator-container");
    m_indicator_container.set_visible(true);

    m_indicator_container.append(m_workspace_container);
    m_indicator_container.append(m_time);
    m_indicator_container.append(m_battery);

    m_workspace_container.set_orientation(Gtk::Orientation::VERTICAL);
    m_workspace_container.set_spacing(5);
    m_workspace_container.add_css_class("workspace-container");
    m_workspace_container.set_visible(true);

    for (const auto &[i, ws] : std::views::enumerate(m_workspaces))
    {
        auto &[box, label] = ws;

        m_workspace_container.append(box);

        box.set_orientation(Gtk::Orientation::VERTICAL);
        box.set_visible(false);
        box.add_css_class("workspace-number-container");
        box.append(label);

        label.set_markup(std::format("<b>{}</b>", i + 1));
        label.add_css_class("workspace-number");
        label.set_visible(true);
    }

    m_time.set_valign(Gtk::Align::START);
    m_time.set_vexpand(false);
    m_time.add_css_class("time-label");
    m_time.set_visible(true);

    m_battery.set_valign(Gtk::Align::START);
    m_battery.set_vexpand(false);
    m_battery.add_css_class("battery-label");
    m_battery.set_visible(true);
}


void
window::mf_update_workspaces()
{
    static constexpr std::array positional_classes {
        "begin", "center", "end", "begin-active", "center-active", "end-active"
    };

    for (const auto &[i, ws] : std::views::enumerate(m_workspaces))
    {
        auto &[box, label] = ws;
        const int ws_num   = static_cast<int>(i) + 1;

        box.set_visible(ws_num <= m_workspace_amount);

        for (const auto *cls : positional_classes) box.remove_css_class(cls);
        label.remove_css_class("active");

        if (ws_num > m_workspace_amount) continue;

        std::string pos_class;
        if (ws_num == 1)
            pos_class = "begin";
        else if (ws_num == m_workspace_amount)
            pos_class = "end";
        else
            pos_class = "center";

        if (ws_num == m_focused_workspace)
        {
            pos_class += "-active";
            label.add_css_class("active");
        }

        box.add_css_class(pos_class);
    }
}


auto
window::set_workspace_amount(int amount) -> window &
{
    m_workspace_amount = amount;
    mf_update_workspaces();
    return *this;
}


auto
window::set_focused_workspace(int index) -> window &
{
    m_focused_workspace = index;
    mf_update_workspaces();
    return *this;
}


auto
window::set_current_time(std::string_view time) -> window &
{
    m_time.set_markup(std::format("<b>{}</b>", time));
    return *this;
}


auto
window::set_battery_level(int level) -> window &
{
    m_battery.set_markup(std::format("<b>{}</b>", level));
    return *this;
}


void
window::show(bool timed)
{
    m_window.set_visible(true);

    if (m_hide_timeout) m_hide_timeout.disconnect();
    if (!timed) return;

    m_hide_timeout = Glib::signal_timeout().connect(
        [this]
        {
            hide();
            return false;
        },
        1500);
}


void
window::hide()
{ m_window.set_visible(false); }
