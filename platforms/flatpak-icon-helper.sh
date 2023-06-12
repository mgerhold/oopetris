#!/usr/bin/env bash

APP_NAME="com.github.mgerhold.OOPetris"

function process_icon() {
  SRC=$1
  RES=$(echo "$SRC" | awk -F_ '{ print $2 }' | awk -F. '{ print $1 }' | awk '{ print $1 }')

  DEST_DIR="/app/share/icons/hicolor/${RES}x${RES}/apps"
  mkdir -p "$DEST_DIR"
  cp "$SRC" "$DEST_DIR/$APP_NAME.png"
}

export -f process_icon

find ../assets/icon/ -name "icon_*.png" -exec bash -c 'process_icon "$@";' -- {} \;

mkdir -p "/app/share/icons/hicolor/scalable/apps/"
cp "../assets/icon/OOPetris.svg" "/app/share/icons/hicolor/scalable/apps/$APP_NAME.svg"
