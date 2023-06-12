#!/usr/bin/env bash

for RES in 24 64 96 160 192 256 512; do

    inkscape "--export-width=$RES" "--export-height=$RES" --export-type=png "--export-filename=../assets/icon/icon_$RES.png" "../assets/OOPetris.svg"

done

MAX_RES="4096"
inkscape "--export-width=$MAX_RES" "--export-height=$MAX_RES" --export-type=svg "--export-filename=../assets/icon/OOPetris.svg" "../assets/OOPetris.svg"
