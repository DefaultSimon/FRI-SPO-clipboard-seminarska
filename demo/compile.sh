#!/usr/bin/env bash

echo "Compiling inspectclipboard.c"
gcc inspectclipboard.c -o inspectclipboard -lX11
echo "Done!"
