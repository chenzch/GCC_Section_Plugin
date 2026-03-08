HOST_GCC=g++
TARGET_PREFIX=arm-none-eabi-
TARGET_GCC=$(TARGET_PREFIX)gcc
PLUGIN_SOURCE_FILES= main.cpp
PLUGIN_NAME=gccsection
GCC_VERSION:=$(shell $(TARGET_GCC) -dumpversion | cut -d. -f1)
PLUGIN_TARGET=$(PLUGIN_NAME)$(GCC_VERSION).so
GCCPLUGINS_DIR:= $(shell $(TARGET_GCC) -print-file-name=plugin)
CXXFLAGS+= -I$(GCCPLUGINS_DIR)/include -fPIC -fno-rtti -O2

$(PLUGIN_TARGET): $(PLUGIN_SOURCE_FILES)
	$(HOST_GCC) -shared $(CXXFLAGS) $^ -o $@

.PHONY: install uninstall test clean
install: $(PLUGIN_TARGET)
	sudo cp $(PLUGIN_TARGET) $(GCCPLUGINS_DIR)

uninstall:
	sudo rm -f $(GCCPLUGINS_DIR)/$(PLUGIN_TARGET)

test: $(PLUGIN_TARGET)
	$(TARGET_GCC) -c -fplugin=./$(PLUGIN_TARGET) ./test.c -o test.o -fno-builtin -Os -ffunction-sections -fdata-sections -flto -v
	$(TARGET_GCC) ./test.o -o test.elf -Xlinker --gc-sections -Wl,-Map,test.map
#	$(TARGET_PREFIX)objdump -h test.elf

clean:
	rm -rf $(PLUGIN_TARGET) test.o test.elf test.map
