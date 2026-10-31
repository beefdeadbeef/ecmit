#------------------------------------------ -*- tab-width: 8 -*-
TARGET		?= arm-none-eabi

CC		:= $(TARGET)-gcc
CXX		:= $(TARGET)-g++
LD		:= $(TARGET)-gcc
AR		:= $(TARGET)-ar
AS		:= $(TARGET)-as
OBJCOPY		:= $(TARGET)-objcopy
OBJDUMP		:= $(TARGET)-objdump
GDB		:= $(TARGET)-gdb
SIZE		:= $(TARGET)-size
