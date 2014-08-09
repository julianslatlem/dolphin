// Copyright 2014 Dolphin Emulator Project
// Licensed under GPLv2
// Refer to the license.txt file included.

#include "Common/Thread.h"
#include "Core/Host.h"
#include "DolphinWX/GLInterface/X11_Util.h"
#include "VideoBackends/OGL/GLInterfaceBase.h"
#include "VideoCommon/VideoConfig.h"

void cX11Window::Initialize(Display *_dpy)
{
	dpy = _dpy;
}

Window cX11Window::CreateXWindow(Window parent, XVisualInfo *vi)
{
	XSetWindowAttributes attr;

	colormap = XCreateColormap(dpy, parent, vi->visual, AllocNone);

	// Setup window attributes
	attr.colormap = colormap;

	// Create the window
	win = XCreateWindow(dpy, parent,
			    0, 0, 1, 1, 0,
			    vi->depth, InputOutput, vi->visual,
			    CWColormap, &attr);
	XSelectInput(dpy, parent, StructureNotifyMask);
	XMapWindow(dpy, win);
	XSync(dpy, True);

	xEventThread = std::thread(&cX11Window::XEventThread, this);

	return win;
}

void cX11Window::DestroyXWindow(void)
{
	XUnmapWindow(dpy, win);
	if (xEventThread.joinable())
		xEventThread.join();
	XFreeColormap(dpy, colormap);
}

void cX11Window::XEventThread()
{
	while (win)
	{
		XEvent event;
		for (int num_events = XPending(dpy); num_events > 0; num_events--)
		{
			XNextEvent(dpy, &event);
			switch (event.type) {
				case ConfigureNotify:
					XResizeWindow(dpy, win, event.xconfigure.width, event.xconfigure.height);
					GLInterface->SetBackBufferDimensions(event.xconfigure.width, event.xconfigure.height);
					break;
				default:
					break;
			}
		}
		Common::SleepCurrentThread(20);
	}
}
