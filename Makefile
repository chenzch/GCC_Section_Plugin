HOST_GCC=g++
TARGET_PREFIX=arm-none-eabi-
TARGET_GCC=$(TARGET_PREFIX)gcc
PLUGIN_SOURCE_FILES= main.cpp
PLUGIN_TARGET=gccsection.so
GCCPLUGINS_DIR:= $(shell $(TARGET_GCC) -print-file-name=plugin)
CXXFLAGS+= -I$(GCCPLUGINS_DIR)/include -fPIC -fno-rtti -O2

$(PLUGIN_TARGET): $(PLUGIN_SOURCE_FILES)
	$(HOST_GCC) -shared $(CXXFLAGS) $^ -o $@
#-Wl,--export-all-symbols for windows
.PHONY: install test clean uninstall
install: $(PLUGIN_TARGET)
	sudo cp $(PLUGIN_TARGET) $(GCCPLUGINS_DIR)

uninstall:
	sudo rm -f $(GCCPLUGINS_DIR)/$(PLUGIN_TARGET)

test: install
	$(TARGET_PREFIX)gcc -c -fplugin=$(basename $(PLUGIN_TARGET)) ./test.c -o test.o -fno-builtin -Os -ffunction-sections -fdata-sections -flto -v
	$(TARGET_PREFIX)gcc ./test.o -o test.elf -Xlinker --gc-sections -Wl,-Map,test.map
#	$(TARGET_PREFIX)objdump -h test.elf

clean:
	rm -rf $(PLUGIN_TARGET) test.o test.elf test.map
