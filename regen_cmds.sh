#!/bin/bash

# look through the engine code for implementations of console commands,
# then register all those so that they are available

egrep -hr '^(COMMAND_ROUTINE|COMMAND_SET_BOOL)\s+\(.*\);?' src \
	| sed -e "s/^.*(\([[:alnum:]_]\+\).*$/COMMAND (\1)/" \
	> src/input/cmds.inc
touch src/input/input.cpp
