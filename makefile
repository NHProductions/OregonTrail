# ----------------------------
# Makefile Options
# ----------------------------

NAME = OREGON
ICON = icon.png
DESCRIPTION = "1988 Oregon Trail Port"
COMPRESSED = YES

CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz

# ----------------------------

include $(shell cedev-config --makefile)
