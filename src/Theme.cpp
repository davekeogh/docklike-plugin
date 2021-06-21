/*
 * Docklike Taskbar - A modern, minimalist taskbar for XFCE
 * Copyright (c) 2019-2020 Nicolas Szabo <nszabo@vivaldi.net>
 * gnu.org/licenses/gpl-3.0
 */

#include "Theme.hpp"

namespace Theme
{
	// This needs work to survive porting to GTK4.
	// GdkScreen is removed, use GdkDisplay instead.

	GdkScreen* mScreen;
	GtkCssProvider* mCssProvider;
	GtkStyleContext* mStyleContext;

	std::string setupColors();

	void init()
	{
		mScreen = gdk_screen_get_default();
		mCssProvider = gtk_css_provider_new();
		mStyleContext = gtk_widget_get_style_context(Dock::mBox);

		load();

		g_signal_connect(G_OBJECT(mStyleContext), "changed",
			G_CALLBACK(+[](GtkStyleContext* stylecontext) { load(); }), NULL);
	}

	void load()
	{
		std::string css = setupColors();
		const gchar* filename;
		FILE* f;

		if (g_environ_getenv(g_get_environ(), "XDG_CONFIG_HOME") != NULL)
			filename = g_build_filename(g_environ_getenv(g_get_environ(), "XDG_CONFIG_HOME"),
				"xfce4-docklike-plugin/gtk.css", NULL);
		else
			filename = g_build_filename(g_environ_getenv(g_get_environ(), "HOME"),
				".config/xfce4-docklike-plugin/gtk.css", NULL);

		if (g_file_test(filename, G_FILE_TEST_IS_REGULAR))
			f = fopen(filename, "r");
		else
			return;

		if (f != NULL)
		{
			int read_char;
			while ((read_char = getc(f)) != EOF)
				css += read_char;
			fclose(f);
		}

		if (gtk_css_provider_load_from_data(mCssProvider, css.c_str(), -1, NULL))
			gtk_style_context_add_provider_for_screen(mScreen, GTK_STYLE_PROVIDER(mCssProvider),
				GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	}

	std::string setupColors()
	{
		GtkWidget* menu = gtk_menu_new();
		GtkWidget* item = gtk_menu_item_new();
		GtkStyleContext* sc = gtk_widget_get_style_context(menu);

		GValue gv = G_VALUE_INIT;
		gtk_style_context_get_property(sc, "background-color", GTK_STATE_FLAG_NORMAL, &gv);
		GdkRGBA* rgba = (GdkRGBA*)g_value_get_boxed(&gv);
		std::string menuBg = gdk_rgba_to_string(rgba);
		gtk_menu_attach(GTK_MENU(menu), item, 0, 1, 0, 1);

		gv = G_VALUE_INIT;
		gtk_style_context_get_property(sc, "color", GTK_STATE_FLAG_NORMAL, &gv);
		rgba = (GdkRGBA*)g_value_get_boxed(&gv);
		std::string itemLabel = gdk_rgba_to_string(rgba);

		gv = G_VALUE_INIT;
		gtk_style_context_get_property(sc, "color", GTK_STATE_FLAG_PRELIGHT, &gv);
		rgba = (GdkRGBA*)g_value_get_boxed(&gv);
		std::string itemLabelHover = gdk_rgba_to_string(rgba);

		gv = G_VALUE_INIT;
		gtk_style_context_get_property(sc, "background-color", GTK_STATE_FLAG_PRELIGHT, &gv);
		rgba = (GdkRGBA*)g_value_get_boxed(&gv);
		std::string itemBgHover = gdk_rgba_to_string(rgba);

		gtk_widget_destroy(item);
		gtk_widget_destroy(menu);

		return "@define-color menu_bgcolor " + menuBg + ";\n@define-color menu_item_color " + itemLabel + ";\n@define-color menu_item_color_hover " + itemLabelHover + ";\n@define-color menu_item_bgcolor_hover " + itemBgHover + ";\n";
	}
} // namespace Theme
