#=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
# GTCL TCL Interpreter, NotArtyom 2018
#=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

BINARY_NAME := gtcl

BASEDIR := $(PWD)
TESTDIR := $(BASEDIR)/tests

BINARY_NAME := $(BASEDIR)/$(BINARY_NAME)
TESTS := $(shell find $(TESTDIR) -name '*.tcl')
TESTS := $(TESTS:%.tcl=%)
TESTOUTS := $(TESTS:%=%.test)

CFLAGS := -w -fno-asynchronous-unwind-tables -fno-exceptions -fno-rtti -lm -O2

#=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

.PHONY: all run clean test test-outfiles
.SILENT: run clean

all: clean
	$(info [CC] -c $(shell find . -name '*.c') -o ./$(shell realpath -m --relative-to=$(PWD) $(BINARY_NAME)))
	@gcc $(CFLAGS) *.c -o $(BINARY_NAME)
	$(info [STRIP] ./$(shell realpath -m --relative-to=$(PWD) $(BINARY_NAME)))
	@strip $(BINARY_NAME)
	$(info [CHMOD] +x ./$(shell realpath -m --relative-to=$(PWD) $(BINARY_NAME)))
	@chmod +x $(BINARY_NAME)

run: clean all
	@$(BINARY_NAME) -i

clean:
	$(info [!] Cleaning up build files)
	@rm -f $(BINARY_NAME)

#=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
# Testing
#=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

# Phony skeletons for the test targets
test: $(BINARY_NAME)
	$(info [Running tests...])
	@make -s $(TESTS)
test-outfiles: all
	@rm -f $(TESTOUTS)
	@make -s $(TESTOUTS)

# Echo the correct output for each test into a .test file
$(TESTOUTS):
	$(info [Building Test] $(notdir $(basename $(@:%.test=%.tcl))).test)
	@$(BINARY_NAME) $(@:%.test=%.tcl) > $@

# Evaluate the tests and error on any issues
$(TESTS):
	$(eval TESTPASS := $(shell expr "$(shell $(BINARY_NAME) $@.tcl)" == "$(shell cat $@.test)"))
	$(eval TESTPASS := $(TESTPASS:1=))
	$(if $(TESTPASS), $(info [!]) && $(error Test FAILED: $(notdir $(basename $@))), $(info [Passed] $(notdir $(basename $@))))
