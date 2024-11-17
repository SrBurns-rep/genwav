#!/usr/bin/env bash
gcc genwav.c -o genwav -lm -g -Og -fsanitize=address,undefined
