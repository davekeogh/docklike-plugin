/*
 * Docklike Taskbar - A modern, minimalist taskbar for XFCE
 * Copyright (c) 2019-2020 Nicolas Szabo <nszabo@vivaldi.net>
 * gnu.org/licenses/gpl-3.0
 */

#include "GroupWindow.hpp"

GroupWindow::GroupWindow(WnckWindow* wnckWindow)
{
	mWnckWindow = wnckWindow;
	mGroupMenuItem = new GroupMenuItem(this);
	mGroupAssociated = false;

	// TODO: check here for exotic group association (like libreoffice)
	std::string groupName = Wnck::getGroupName(this);
	AppInfo* appInfo = AppInfos::search(groupName);

	mGroup = Dock::prepareGroup(appInfo);

	//--------------------------------------------------

	g_signal_connect(G_OBJECT(mWnckWindow), "name-changed",
		G_CALLBACK(+[](WnckWindow* window, GroupWindow* me) {
			me->mGroupMenuItem->updateLabel();
		}),
		this);

	g_signal_connect(G_OBJECT(mWnckWindow), "icon-changed",
		G_CALLBACK(+[](WnckWindow* window, GroupWindow* me) {
			me->mGroupMenuItem->updateIcon();
		}),
		this);

	g_signal_connect(G_OBJECT(mWnckWindow), "state-changed",
		G_CALLBACK(+[](WnckWindow* window, WnckWindowState changed_mask,
						WnckWindowState new_state, GroupWindow* me) {
			me->updateState();
		}),
		this);

	g_signal_connect(G_OBJECT(mWnckWindow), "workspace-changed",
		G_CALLBACK(+[](WnckWindow* window, GroupWindow* me) {
			me->updateState();
		}),
		this);

	g_signal_connect(G_OBJECT(mWnckWindow), "geometry-changed",
		G_CALLBACK(+[](WnckWindow* window, GroupWindow* me) {
			me->updateState();
		}),
		this);

	g_signal_connect(G_OBJECT(mWnckWindow), "class-changed",
		G_CALLBACK(+[](WnckWindow* window, GroupWindow* me) {
			std::string groupName = Wnck::getGroupName(me);
			Group* group = Dock::prepareGroup(AppInfos::search(groupName));
			if (group != me->mGroup)
			{
				me->leaveGroup();
				me->mGroup = group;
				me->getInGroup();
				Wnck::setActiveWindow();
			}
		}),
		this);

	updateState();
	mGroupMenuItem->updateIcon();
	mGroupMenuItem->updateLabel();
}

bool GroupWindow::getState(WnckWindowState flagMask) { return (mState & flagMask) != 0; }
void GroupWindow::activate(guint32 timestamp) { Wnck::activate(this, timestamp); }
void GroupWindow::minimize() { Wnck::minimize(this); }

GroupWindow::~GroupWindow()
{
	leaveGroup();
	delete mGroupMenuItem;
}

void GroupWindow::getInGroup()
{
	if (mGroupAssociated)
		return;
	mGroup->add(this);
	mGroupAssociated = true;
}

void GroupWindow::leaveGroup()
{
	if (!mGroupAssociated)
		return;
	mGroup->remove(this);
	mGroup->onWindowUnactivate();
	mGroupAssociated = false;
}

void GroupWindow::onActivate()
{
	Help::Gtk::cssClassAdd(GTK_WIDGET(mGroupMenuItem->mItem), "active");
	gtk_widget_queue_draw(GTK_WIDGET(mGroupMenuItem->mItem));
	mGroupMenuItem->updateLabel();

	if (mGroupAssociated)
		mGroup->onWindowActivate(this);
}

void GroupWindow::onUnactivate()
{
	Help::Gtk::cssClassRemove(GTK_WIDGET(mGroupMenuItem->mItem), "active");
	gtk_widget_queue_draw(GTK_WIDGET(mGroupMenuItem->mItem));
	mGroupMenuItem->updateLabel();

	if (mGroupAssociated)
		mGroup->onWindowUnactivate();
}

void GroupWindow::updateState()
{
	bool onScreen = true;
	bool monitorChanged = false;
	bool onWorkspace = true;
	bool onTasklist = !(mState & WnckWindowState::WNCK_WINDOW_STATE_SKIP_TASKLIST);
	mState = Wnck::getState(this);

	if (Settings::onlyDisplayVisible)
	{
		WnckWorkspace* windowWorkspace = wnck_window_get_workspace(mWnckWindow);

		if (windowWorkspace != NULL)
		{
			WnckWorkspace* activeWorkspace = wnck_screen_get_active_workspace(Wnck::mWnckScreen);

			if (windowWorkspace != activeWorkspace)
				onWorkspace = false;
		}
	}

	if (Settings::onlyDisplayScreen)
	{
		// Adapted from Xfce panel's tasklist-widget.c
		gint x, y, w, h;

		wnck_window_get_geometry(mWnckWindow, &x, &y, &w, &h);

		GdkWindow* pluginWindow = gtk_widget_get_window(GTK_WIDGET(Plugin::mXfPlugin));
		GdkMonitor* currentMonitor = gdk_display_get_monitor_at_point(Plugin::display, x + (w / 2), y + (h / 2));

		if (gdk_display_get_monitor_at_window(Plugin::display, pluginWindow) != currentMonitor)
			onScreen = false;

		if (mMonitor != currentMonitor)
		{
			if (mMonitor != NULL)
				monitorChanged = true;
			mMonitor = currentMonitor;
		}
		else
			monitorChanged = false;

		g_free(pluginWindow);
		g_free(currentMonitor);
	}

	if (onWorkspace && onTasklist && onScreen)
	{
		getInGroup();
		if (monitorChanged)
			Wnck::setActiveWindow();
	}
	else
		leaveGroup();

	gtk_widget_show(GTK_WIDGET(mGroupMenuItem->mItem));
}
