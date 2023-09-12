#!/bin/bash

tmux new-session -d -s nitro_dev
tmux send-keys -t nitro_dev 'nvim .' C-m
tmux -u attach -t nitro_dev
