#!/bin/sh

sudo pkg install -y gmake
sudo pkg install -y `pkg search sdl | grep '^sdl-1.2'`
