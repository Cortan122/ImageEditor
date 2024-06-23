#!/bin/sh

CXX=clang++ CC=clang make --always-make --dry-run |
  grep -wE 'clang|clang\+\+' |
  grep -w '\-c' |
  jq -nR --arg PWD "$PWD" '[inputs|{directory:$PWD, command:., file: match(" [^ ]+$").string[1:]}]' \
  > compile_commands.json
