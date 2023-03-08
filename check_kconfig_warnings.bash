#!/usr/bin/env bash

if [[ $(grep -P "warning: .*?([A-Z_]+) \(defined" $1) ]]; then
    echo "KConfig warning detected!"
    exit 1
fi
