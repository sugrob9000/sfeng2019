#!/bin/bash

# look through the engine code for implementations of console commands,
# then register all those so that they are available

[ -z "$1" ] && src=src/ || src=$@

egrep -hr '^(COMMAND_ROUTINE|COMMAND_SET_BOOL)\s*\(.*\);?' $src \
| sed -e "s/^.*(\([[:alnum:]_]\+\).*$/COMMAND (\1)/" | sort | uniq \
> /tmp/churn-cmds-inc

if ! diff -q /tmp/churn-cmds-inc src/input/_cmds.inc > /dev/null
then
	echo Rebuilding command list
	cp /tmp/churn-cmds-inc src/input/_cmds.inc
	touch src/input/input.cpp
fi

rm /tmp/churn-cmds-inc
