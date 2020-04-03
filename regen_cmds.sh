#!/bin/bash

# look through the engine code for implementations of console commands,
# then register all those so that they are available

egrep -hr '^(COMMAND_ROUTINE|COMMAND_SET_BOOL)\s+\(.*\);?' src/ \
| sed -e "s/^.*(\([[:alnum:]_]\+\).*$/COMMAND (\1)/" \
> /tmp/churn-cmds-inc
if ! diff -q /tmp/churn-cmds-inc src/input/_cmds.inc
then
	cp /tmp/churn-cmds-inc src/input/_cmds.inc
	touch src/input/input.cpp
fi
rm /tmp/churn-cmds-inc
