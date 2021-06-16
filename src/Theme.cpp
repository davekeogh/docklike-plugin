/*
 * Docklike Taskbar - A modern, minimalist taskbar for XFCE
 * Copyright (c) 2019-2020 Nicolas Szabo <nszabo@vivaldi.net>
 * gnu.org/licenses/gpl-3.0
 */

#include "Theme.hpp"

namespace Theme
{
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
		std::string cssStyle = setupColors();
		const gchar* filename;
		FILE* file_handle;
		int c;

        if (g_environ_getenv(g_get_environ(), "XDG_CONFIG_HOME") != NULL)
            filename = g_build_filename(g_environ_getenv(g_get_environ(), "XDG_CONFIG_HOME"), "xfce4-docklike-plugin/gtk.css", NULL);
        else
            filename = g_build_filename(g_environ_getenv(g_get_environ(), "HOME"), "xfce4-docklike-plugin/gtk.css", NULL);

		if (g_file_test(filename, G_FILE_TEST_IS_REGULAR))
			file_handle = fopen(filename, "r");
        else
        {
            // TODO: Update Makefile to install gtk.css and point this to it:
            filename = "/home/david/Projects/xfce4-docklike-plugin/src/gtk.css"; 
            if (g_file_test(filename, G_FILE_TEST_IS_REGULAR))
			    file_handle = fopen(filename, "r");
        }

		if (file_handle != NULL)
		{
			while ((c = getc(file_handle)) != EOF)
				cssStyle += c;
			fclose(file_handle);
		}

		if (gtk_css_provider_load_from_data(mCssProvider, cssStyle.c_str(), -1, NULL))
			gtk_style_context_add_provider_for_screen(mScreen, GTK_STYLE_PROVIDER(mCssProvider),
				GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
            
        printf(cssStyle.c_str(), NULL);
	}

	void applyDefault(GtkWidget* widget)
	{
		gtk_style_context_remove_provider(gtk_widget_get_style_context(widget),
            GTK_STYLE_PROVIDER(mCssProvider));
	}

	std::string setupColors()
	{
		GtkWidget* menu = gtk_menu_new();
		GtkStyleContext* sc = gtk_widget_get_style_context(menu);

		GValue gv = G_VALUE_INIT;
		gtk_style_context_get_property(sc, "background-color", GTK_STATE_FLAG_NORMAL, &gv);
		GdkRGBA* rgba = (GdkRGBA*)g_value_get_boxed(&gv);
		std::string menuBg = gdk_rgba_to_string(rgba);

		GtkWidget* item = gtk_menu_item_new();
		gtk_menu_attach(GTK_MENU(menu), item, 0, 1, 0, 1);

		sc = gtk_widget_get_style_context(item);

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

		return "@define-color dl_menu_bgcolor " + menuBg + ";\n@define-color dl_menu_item_color " + itemLabel + ";\n@define-color dl_menu_item_color_hover " + itemLabelHover + ";\n@define-color dl_menu_item_bgcolor_hover " + itemBgHover + ";\n";
	}
} // namespace Theme