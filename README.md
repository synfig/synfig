Synfig Studio
=============

About
-----

Synfig Studio is a free and open-source 2D animation software, designed as
powerful industrial-strength solution for creating film-quality animation using
a vector and bitmap artwork. It eliminates the need to create animation
frame-by frame, allowing you to produce 2D animation of a higher quality with
fewer people and resources. Synfig Studio is available for Windows, Linux and
MacOS X.

https://synfig.org/

[![Build Status](https://travis-ci.com/synfig/synfig.svg?branch=master)](https://travis-ci.com/synfig/synfig)


Installing
----------

#### Debian/Ubuntu
1. Download the appimage file from the website.
2. Left-click on the file and select `properties`. Navigate to `permissions` and check the box that says `Allow executing file as program`. Finally, run the  file.

For more information see https://discourse.appimage.org/t/how-to-make-an-appimage-executable/80.

#### Windows
1. Download the `.exe` file from the website.
2. Run the file. A wizard will then guide you through the rest of the installation.

#### OSX
1. Download the `.dmg` file from the website.
2. Drag the file into your applications directory, then run it.

If you get an error message like "can't be opened because it comes from a non identified developer", then do the fllowing:

1. Locate the Synfig app in Finder (don’t use Launchpad to do this - Launchpad doesn’t allow you to access the shortcut menu)
2. Control-click the app icon, choose "Open" from the shortcut menu.
3. Click Open.

The app is saved as an exception to your security settings, and you can open it in the future by double-clicking it just as you can any registered app.

Note: You can also grant an exception for a blocked app by clicking the “Open Anyway” button in the General pane of Security & Privacy preferences. This button is available for about an hour after you try to open the app. To open this pane, choose Apple menu > System Preferences, click Security & Privacy, then click General.

For more details please refer to this page - https://support.apple.com/kb/PH25088?locale=en_US

#### Building from source

See instructions here - https://synfig-docs-dev.readthedocs.io/en/latest/common/building.html

If you have previous synfig build installed in system path (e.g. `/usr/local/`),
you are recommended to uninstall that.
