###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_delta_dsc100_INCLUDES := -I $(THIS_DIR)inc
x86_64_delta_dsc100_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_delta_dsc100_DEPENDMODULE_ENTRIES := init:x86_64_delta_dsc100 ucli:x86_64_delta_dsc100
