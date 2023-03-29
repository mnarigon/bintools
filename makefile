#
# -- makefile
#

# -- general configuration
SHELL=/bin/sh

# -- program extension
ifeq ($(OS),Windows_NT)
    EXE=".exe"
else
    EXE=
endif

#
# -- programs
BIN2HEX= bin2hex$(EXE)
HEX2BIN= hex2bin$(EXE)

#
# -- directory
INSTALL_DIR= /usr/local/bin

#
# -- compiler flags
CC= cc
CCFLAGS= -O

#
# -- linker flags
LDFLAGS=

#
# -- make all target
all: $(BIN2HEX) $(HEX2BIN)

#
# -- compile file rule
.c.o:
	-@echo "Compiling $< ..."
	$(CC) $(CCFLAGS) -c $< -o $@

#
# -- link bin2hex
$(BIN2HEX): bin2hex.o intel_format.o
	@echo "Linking $@ ..."
	$(CC) $(LDFLAGS) $^ -o $@

#
# -- link hex2bin
$(HEX2BIN): hex2bin.o intel_format.o
	@echo "Linking $@ ..."
	$(CC) $(LDFLAGS) $^ -o $@

#
# -- install target
install: $(BIN2HEX) $(HEX2BIN)
	@echo "Installing $(BIN2HEX) and $(HEX2BIN) ..."
	@/bin/cp -f $(BIN2HEX) $(INSTALL_DIR)/$(BIN2HEX)
	@/bin/chmod 755 $(INSTALL_DIR)/$(BIN2HEX)
	@/bin/cp -f $(HEX2BIN) $(INSTALL_DIR)/$(HEX2BIN)
	@/bin/chmod 755 $(INSTALL_DIR)/$(HEX2BIN)

#
# -- clean target
clean:
	rm -f $(BIN2HEX) $(HEX2BIN) bin2hex.o hex2bin.o intel_format.o

#
# -- run test files
check: $(BIN2HEX) $(HEX2BIN)
	@echo "Checking $(BIN2HEX) ..."
	./$(BIN2HEX) -a 0F000h test/test.bin > test/test.hex.out0
	./$(BIN2HEX) -a 0F000h -o test/test.hex.out1 test/test.bin
	@echo "Checking $(HEX2BIN) ..."
	./$(HEX2BIN) test/test.hex > test/test.bin.out0
	./$(HEX2BIN)  -o test/test.bin.out1 test/test.hex

#
# -- dependencies
bin2hex.o: intel_format.h types.h

hex2bin.o: intel_format.h types.h

intel_format.o: intel_format.h
