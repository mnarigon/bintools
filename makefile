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
# -- directory
INSTALL_DIR=    /usr/local/bin

#
# -- compiler flags
CC= cc
CCFLAGS= -O

#
# -- linker flags
LDFLAGS=

#
# -- make all target
all: hex2bin$(EXE) bin2hex$(EXE)

#
# -- compile a file rule
.c.o:
	-@echo "Compiling $< ..."
	$(CC) $(CCFLAGS) -c $< -o $@

#
# -- link hex2bin
hex2bin$(EXE): hex2bin.o intel_format.o
	@echo "Linking $@ ..."
	$(CC) $(LDFLAGS) $^ -o $@

#
# -- link bin2hex
bin2hex$(EXE): bin2hex.o intel_format.o
	@echo "Linking $@ ..."
	$(CC) $(LDFLAGS) $^ -o $@

#
# -- install target
install: hex2bin$(EXE) bin2hex$(EXE)
	@echo "Installing hex2bin$(EXE) and bin2hex$(EXE) ..."
	@/bin/cp -f bin2hex$(EXE) $(INSTALL_DIR)/bin2hex$(EXE)
	@/bin/chmod 755 $(INSTALL_DIR)/bin2hex$(EXE)
	@/bin/cp -f hex2bin$(EXE) $(INSTALL_DIR)/hex2bin$(EXE)
	@/bin/chmod 755 $(INSTALL_DIR)/hex2bin$(EXE)

clean:
	rm -f bin2hex.o hex2bin.o intel_format.o

#
# -- dependencies
bin2hex.o: intel_format.h types.h

hex2bin.o: intel_format.h types.h

intel_format.o: intel_format.h
