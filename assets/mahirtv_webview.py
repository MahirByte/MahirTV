#!/usr/bin/env python3
"""
MahirTV Embedded Browser — uses WebKit2GTK so websites run inside MahirTV.
Launch:  python3 assets/mahirtv_webview.py <url> [title]
"""
import gi, sys, os

try:
    gi.require_version('Gtk', '3.0')
    gi.require_version('WebKit2', '4.1')
    from gi.repository import WebKit2
    WK_VER = '4.1'
except:
    try:
        gi.require_version('Gtk', '3.0')
        gi.require_version('WebKit2', '4.0')
        from gi.repository import WebKit2
        WK_VER = '4.0'
    except Exception as e:
        print(f"WebKit2 not available: {e}")
        print("Install with: sudo apt install gir1.2-webkit2-4.0 python3-gi")
        sys.exit(1)

from gi.repository import Gtk, Gdk, GLib, Gio

URL   = sys.argv[1] if len(sys.argv) > 1 else 'https://www.google.com'
TITLE = sys.argv[2] if len(sys.argv) > 2 else 'MahirTV Browser'

CSS = b"""
window {
    background-color: #000820;
}

headerbar {
    background-color: #001050;
    border-bottom: 2px solid #1a4aaa;
    padding: 4px 10px;
    min-height: 52px;
}

headerbar label {
    color: #4090ff;
    font-size: 16px;
    font-weight: bold;
}

headerbar button {
    background-color: #001a60;
    border: 1px solid #1a3a7a;
    border-radius: 8px;
    color: #80b4ff;
    padding: 5px 12px;
    min-width: 32px;
    min-height: 32px;
}

headerbar button:hover {
    background-color: #002898;
    color: #c0d8ff;
}

headerbar button:active {
    background-color: #0a3cb4;
}

.url-bar {
    background-color: #000a38;
    border: 1px solid #1a3a7a;
    border-radius: 20px;
    color: #90c4ff;
    padding: 6px 18px;
    font-size: 14px;
    min-width: 460px;
}

.url-bar:focus {
    border-color: #2860dd;
    background-color: #000f48;
}

.home-btn {
    background-color: #0a3278;
    border-color: #2050a0;
    color: #a0c8ff;
    font-weight: bold;
}

.close-btn {
    background-color: #500018;
    border-color: #600020;
    color: #ff8090;
}

.close-btn:hover {
    background-color: #900028;
}

progressbar trough {
    background-color: #000f38;
    min-height: 3px;
}

progressbar progress {
    background-color: #1060ff;
    min-height: 3px;
}

scrollbar trough {
    background-color: #000f38;
}

scrollbar slider {
    background-color: #1e50b4;
    border-radius: 4px;
    min-width: 8px;
    min-height: 8px;
}
"""

class MahirBrowser(Gtk.Window):
    def __init__(self, url, title):
        super().__init__(title="MahirTV")
        self.set_default_size(1920, 1080)
        self.maximize()
        self.set_decorated(True)

        # Apply CSS theme
        css_provider = Gtk.CssProvider()
        css_provider.load_from_data(CSS)
        Gtk.StyleContext.add_provider_for_screen(
            Gdk.Screen.get_default(),
            css_provider,
            Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION
        )

        self._build_ui(url, title)
        self.connect('destroy', Gtk.main_quit)
        self.connect('key-press-event', self._on_key)

    def _build_ui(self, url, title):
        # ── Header bar ────────────────────────────────────────────
        header = Gtk.HeaderBar()
        header.props.show_close_button = False   # we add our own
        header.set_title("MahirTV")
        header.set_subtitle(title)

        # Back / Forward / Reload
        self.back_btn = Gtk.Button.new_from_icon_name("go-previous-symbolic", Gtk.IconSize.BUTTON)
        self.fwd_btn  = Gtk.Button.new_from_icon_name("go-next-symbolic",     Gtk.IconSize.BUTTON)
        self.rel_btn  = Gtk.Button.new_from_icon_name("view-refresh-symbolic",Gtk.IconSize.BUTTON)
        for b in (self.back_btn, self.fwd_btn, self.rel_btn):
            b.get_style_context().add_class('nav-btn')

        header.pack_start(self.back_btn)
        header.pack_start(self.fwd_btn)
        header.pack_start(self.rel_btn)

        # URL bar (centre)
        self.url_entry = Gtk.Entry()
        self.url_entry.set_text(url)
        self.url_entry.get_style_context().add_class('url-bar')
        header.set_custom_title(self.url_entry)

        # Home (back to MahirTV) / Close buttons
        home_btn  = Gtk.Button(label="⌂  MahirTV")
        close_btn = Gtk.Button(label="✕")
        home_btn.get_style_context().add_class('home-btn')
        close_btn.get_style_context().add_class('close-btn')

        header.pack_end(close_btn)
        header.pack_end(home_btn)

        self.set_titlebar(header)

        # ── Progress bar ──────────────────────────────────────────
        self.progress = Gtk.ProgressBar()
        self.progress.set_fraction(0)
        self.progress.get_style_context().add_class('progress')

        # ── WebView ───────────────────────────────────────────────
        settings = WebKit2.Settings()
        settings.set_enable_javascript(True)
        settings.set_enable_media(True)
        settings.set_enable_mediasource(True)
        settings.set_hardware_acceleration_policy(
            WebKit2.HardwareAccelerationPolicy.ALWAYS)
        settings.set_user_agent_with_application_details("MahirTV", "1.0")

        self.wv = WebKit2.WebView()
        self.wv.set_settings(settings)
        self.wv.load_uri(url)

        # ── Layout ────────────────────────────────────────────────
        vbox = Gtk.Box(orientation=Gtk.Orientation.VERTICAL)
        vbox.pack_start(self.progress, False, False, 0)
        vbox.pack_start(self.wv, True, True, 0)
        self.add(vbox)

        # ── Signals ───────────────────────────────────────────────
        self.back_btn.connect('clicked', lambda *a: self.wv.go_back())
        self.fwd_btn.connect('clicked',  lambda *a: self.wv.go_forward())
        self.rel_btn.connect('clicked',  lambda *a: self.wv.reload())
        home_btn.connect('clicked',  lambda *a: self.destroy())
        close_btn.connect('clicked', lambda *a: self.destroy())
        self.url_entry.connect('activate', self._on_url_activate)
        self.wv.connect('load-changed', self._on_load_changed)
        self.wv.connect('load-failed',  self._on_load_failed)
        self.wv.connect('notify::estimated-load-progress', self._on_progress)
        self.wv.connect('notify::title', self._on_title)

    def _on_url_activate(self, entry):
        url = entry.get_text().strip()
        if url and not url.startswith('http'):
            # If it looks like a search query, use Google
            if ' ' in url or '.' not in url:
                url = 'https://www.google.com/search?q=' + url.replace(' ', '+')
            else:
                url = 'https://' + url
        self.wv.load_uri(url)

    def _on_load_changed(self, wv, event):
        uri = wv.get_uri() or ''
        self.url_entry.set_text(uri)
        self.back_btn.set_sensitive(wv.can_go_back())
        self.fwd_btn.set_sensitive(wv.can_go_forward())
        if event == WebKit2.LoadEvent.FINISHED:
            self.progress.set_fraction(0)
            self.progress.set_visible(False)
        else:
            self.progress.set_visible(True)

    def _on_load_failed(self, wv, event, uri, err):
        pass

    def _on_progress(self, wv, param):
        p = wv.get_estimated_load_progress()
        self.progress.set_fraction(p)

    def _on_title(self, wv, param):
        t = wv.get_title()
        if t:
            self.set_title("MahirTV — " + t)

    def _on_key(self, widget, event):
        if event.keyval == Gdk.KEY_Escape:
            self.destroy()
            return True
        if event.keyval == Gdk.KEY_F5:
            self.wv.reload()
            return True
        if event.keyval == Gdk.KEY_F12:
            if self.is_maximized():
                self.unmaximize()
            else:
                self.maximize()
            return True
        return False


def main():
    Gtk.init(sys.argv[:1])
    win = MahirBrowser(URL, TITLE)
    win.show_all()
    Gtk.main()


if __name__ == '__main__':
    main()
