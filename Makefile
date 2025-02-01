HOST_GCC=g++
TARGET_PREFIX=arm-none-eabi-
TARGET_GCC=$(TARGET_PREFIX)gcc
PLUGIN_SOURCE_FILES= main.cpp
PLUGIN_TARGET=plugin.so
GCCPLUGINS_DIR:= $(shell $(TARGET_GCC) -print-file-name=plugin)
CXXFLAGS+= -I$(GCCPLUGINS_DIR)/include -fPIC -fno-rtti -O2

plugin.so: $(PLUGIN_SOURCE_FILES)
	$(HOST_GCC) -shared $(CXXFLAGS) $^ -o $@

.PHONY: test clean
test: $(PLUGIN_TARGET)
	$(TARGET_PREFIX)gcc -c -fplugin=./$(PLUGIN_TARGET) ./test.c -o test.o -fno-builtin -Os -ffunction-sections -fdata-sections -flto
	$(TARGET_PREFIX)gcc ./test.o -o test.elf -Xlinker --gc-sections -Wl,-Map,test.map
#	$(TARGET_PREFIX)objdump -h test.elf

clean:
	rm -rf $(PLUGIN_TARGET) test.elf
