#!/bin/sh

# The possible severities for messages are:
# error             used when bugs are found
# warning           suggestions about defensive programming to prevent bugs
# style             stylistic issues related to code cleanup (unused functions, redundant code, constness, and such)
# performance       Suggestions for making the code faster. These suggestions are only based on common knowledge.  It  is  not  certain  you'll  get  any  measurable  difference  in  speed  by  fixing these messages.
# portability       portability warnings. 64-bit portability. code might work different on different compilers. etc.
# information       Configuration problems. The recommendation is to only enable these during configuration

#cppcheck ./ --enable=warning 2> cpp_check_errors.txt
cppcheck ./ --enable=performance 2> cpp_check_performance.txt