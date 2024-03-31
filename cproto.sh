#!/bin/sh

awk '/=/ {print "extern "$1" "$2" "$3";"}' "$1"
