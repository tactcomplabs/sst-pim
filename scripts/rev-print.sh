#!/bin/bash

awk 'BEGIN { on=0; off=0}  \
  /<rev-print>/ { on=1;}   \
  /<\/rev-print>/ {off=1;} \
  { if (on==1) {print($0);} if (off==1) {on=0; off=0;} }' $1 \
| sed 's/<\/rev-print>//g' | sed 's/<rev-print>//g' 

