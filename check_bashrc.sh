#!/bin/bash

# check if .bashrc exists
if [ -f ~/.bashrc ]; then
    echo "export HELLO=\$HOSTNAME" >> ~/.bashrc
    echo "LOCAL=\$(whoami)" >> ~/.bashrc
else
    echo ".bashrc not found in home directory!"
fi

# open new terminal
gnome-terminal &
