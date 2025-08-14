#!/bin/bash

create-dmg \
  --volname "Synfig Studio Installer" \
  --volicon "SynfigStudio.icns" \
  --background "abstract-lines2.png" \
  --window-pos 200 120 \
  --window-size 660 404 \
  --icon-size 100 \
  --icon "SynfigStudio.app" 200 190 \
  --hide-extension "SynfigStudio.app" \
  --app-drop-link 480 190 \
  "SynfigStudio-1.4.5.dmg" \
  "SynfigStudio.app/"