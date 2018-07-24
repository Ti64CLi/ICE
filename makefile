# ----------------------------
# Set NAME to the program name
# Set ICON to the png icon file name
# Set DESCRIPTION to display within a compatible shell
# Set COMPRESSED to "YES" to create a compressed program
# ----------------------------

NAME        ?= ICE
COMPRESSED  ?= YES
ICON        ?= icon.png
DESCRIPTION ?= "ICE Compiler v2.2.1.0 (scroll by Ti64CLi++)"

# ----------------------------

include $(CEDEV)/include/.makefile

