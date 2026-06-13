#!/bin/bash

sudo pacman -Syu --noconfirm
sudo pacman -S --needed base-devel cmake git clang pkgconf python python-virtualenv libwayland glxinfo --noconfirm
sudo pacman -S --needed gcc13 --noconfirm
sudo archlinux-java status >/dev/null 2>&1
sudo ln -sf /usr/bin/gcc-13 /usr/bin/gcc
sudo ln -sf /usr/bin/g++-13 /usr/bin/g++
python -m venv .venv
source ".venv/bin/activate" && pip install --upgrade pip
pip install libclang
glxinfo | grep OpenGL
