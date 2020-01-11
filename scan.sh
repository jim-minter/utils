#!/bin/bash

# append -b for batch

scanimage -d 'brother4:net1;dev0' --format png -x 216 -y 280 "$@"
