HOST_GCC=g++
TARGET_GCC=gcc
PLUGIN_SOURCE_FILES= main.cpp
PLUGIN_TARGET=plugin.so
GCCPLUGINS_DIR:= $(shell $(TARGET_GCC) -print-file-name=plugin)
CXXFLAGS+= -I$(GCCPLUGINS_DIR)/include -fPIC -fno-rtti -O2

plugin.so: $(PLUGIN_SOURCE_FILES)
	$(HOST_GCC) -shared $(CXXFLAGS) $^ -o $@

.PHONY: test clean
test: $(PLUGIN_TARGET)
	gcc -v -fplugin=./$(PLUGIN_TARGET) ./test.c -o test.elf -fno-builtin -O0 -ffunction-sections
	objdump -h test.elf

clean:
	rm -rf $(PLUGIN_TARGET) test.elf
