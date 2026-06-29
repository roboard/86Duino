@echo off
if '' == '%1' dns -q16 -s3 version.bind
if not '' == '%1' dns -q16 -s3 version.bind -n%1