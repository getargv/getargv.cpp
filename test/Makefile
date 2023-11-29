CXX                     := clang++
PRIMARY_SOURCEFILE      := $(wildcard ../src/*.cpp)
EXECUTABLE_NAME         := lib_unit_tests
OBJ_DIR			:= obj
LIB_DIR			:= lib
BIN_DIR			:= bin
SRC_DIR			:= src
LIB_SHORT_NAME          := getargv++
INCLUDE_DIR		:= include/$(LIB_SHORT_NAME)
COV_DIR			:= coverage
PREFIX			:= $(PWD)
BREW_PREFIX		:= $(shell /usr/bin/env -P /usr/local/bin:/opt/homebrew/bin brew --prefix)

COMPILER_VERSION		:= $(shell $(CXX) --version | grep version | grep -o -m 1 "[0-9]\+\.[0-9]\+\.*[0-9]*" | head -n 1)
COMPILER_VERSION_NUMBER		:= $(shell echo $(COMPILER_VERSION) | sed -e 's/\.\([0-9][0-9]\)/\1/g' -e 's/\.\([0-9]\)/0\1/g' -e 's/^[0-9]\{3,4\}$$/&00/')
CLANG_13_OR_MORE		:= $(shell expr $(COMPILER_VERSION_NUMBER) \>= 130106)
ifneq ($(CLANG_13_OR_MORE),0)
CXXFLAGS                := --std=c++20
# supported: c++11, c++14, c++17, c++20
# future: c++2b
else
CXXFLAGS                := --std=c++17
endif

LDFLAGS			:=
DYLIB			:= $(LIB_DIR)/lib$(LIB_SHORT_NAME).dylib

export MallocNanoZone = 0
SANI_FLAGS := -fno-omit-frame-pointer -fprofile-instr-generate -fsanitize=address -fsanitize=undefined -fsanitize=nullability -fsanitize=unsigned-integer-overflow -fsanitize=local-bounds -fsanitize=nullability -fsanitize=implicit-conversion
INNER_ASAN_OPTIONS := strict_string_checks=1:detect_stack_use_after_return=1:check_initialization_order=1:strict_init_order=1:detect_invalid_pointer_pairs=2:detect_stack_use_after_return=1

.PHONY: integration_tests lib_unit_tests pattern clean libtest1 libtest2 run_lib_unit_tests run_lib_unit_tests_verbose run_lib_unit_tests_debug run_lib_unit_tests_coverage

.DEFAULT_GOAL := run_lib_unit_tests

lib_unit_tests: $(BIN_DIR)/lib_unit_tests

#unit test library
$(BIN_DIR)/lib_unit_tests: CPPFLAGS := -MMD -MP -fvisibility=hidden
$(BIN_DIR)/lib_unit_tests: CXXFLAGS += -fblocks -I $(BREW_PREFIX)/include -O0 -g3 -fcoverage-mapping $(SANI_FLAGS)
$(BIN_DIR)/lib_unit_tests: EXE      := $(BIN_DIR)/lib_unit_tests
$(BIN_DIR)/lib_unit_tests: LDLIBS   := -l criterion -l getargv $(SANI_FLAGS)
$(BIN_DIR)/lib_unit_tests: LDFLAGS  := -L $(BREW_PREFIX)/lib

# test building with library, phony targets can't use % pattern when declaring deps, thus the non-DRY rules
libtest1: $(DYLIB) $(BIN_DIR)/libtest1
libtest2: $(DYLIB) $(BIN_DIR)/libtest2
$(BIN_DIR)/libtest%: EXE := $(BIN_DIR)/libtest%
$(BIN_DIR)/libtest%: CPPFLAGS :=
$(BIN_DIR)/libtest%: CXXFLAGS += -I $(PREFIX)/$(INCLUDE_DIR)
$(BIN_DIR)/libtest%: LDLIBS := -L $(PREFIX)/lib -l $(LIB_SHORT_NAME)

pattern:
	@echo 'CRITERION_TEST_PATTERN= make run'

run_lib_unit_tests: export ASAN_OPTIONS = $(INNER_ASAN_OPTIONS)
run_lib_unit_tests: $(BIN_DIR)/lib_unit_tests | $(BIN_DIR)/child
	$(BIN_DIR)/lib_unit_tests -j0

run_lib_unit_tests_verbose: export ASAN_OPTIONS = $(INNER_ASAN_OPTIONS)
run_lib_unit_tests_verbose: $(BIN_DIR)/lib_unit_tests | $(BIN_DIR)/child
	$(BIN_DIR)/lib_unit_tests --verbose 2 -j1

run_lib_unit_tests_debug: export ASAN_OPTIONS = $(INNER_ASAN_OPTIONS)
run_lib_unit_tests_debug: $(BIN_DIR)/lib_unit_tests | $(BIN_DIR)/child
	$(BIN_DIR)/lib_unit_tests --debug=lldb -j1

run_lib_unit_tests_coverage: $(COV_DIR)/lib_unit_tests.profdata

$(COV_DIR)/%.profdata: EXECUTABLE_NAME = $(basename $(notdir $@))
$(COV_DIR)/%.profdata: export ASAN_OPTIONS = $(INNER_ASAN_OPTIONS)
$(COV_DIR)/%.profdata: $(BIN_DIR)/% | $(COV_DIR) $(BIN_DIR)/child
	LLVM_PROFILE_FILE="$(COV_DIR)/%c$(EXECUTABLE_NAME)-%p.profraw" $< -j0
	/usr/bin/xcrun -r llvm-profdata merge -sparse $(COV_DIR)/$(EXECUTABLE_NAME)-*.profraw -o $@
	@$(RM) $(COV_DIR)/$(EXECUTABLE_NAME)-*.profraw
	/usr/bin/xcrun -r llvm-cov report --summary-only --instr-profile=$@ --ignore-filename-regex='(unit_test|criterion|.h)' $< $(PRIMARY_SOURCEFILE)

%_coverage_report: $(COV_DIR)/%.profdata | $(BIN_DIR)/%
	/usr/bin/xcrun -r llvm-cov show --instr-profile=$< --show-expansions --show-branches=count --show-line-counts-or-regions --ignore-filename-regex='(unit_test|criterion|.h)' $|
	/usr/bin/xcrun -r llvm-cov report --summary-only --instr-profile=$< --ignore-filename-regex='(unit_test|criterion|.h)' $| $(PRIMARY_SOURCEFILE)

$(LIB_DIR)/%.dylib:
	$(MAKE) -C .. PREFIX=$(PREFIX) dylib install_dylib

$(BIN_DIR)/%: $(OBJ_DIR)/%.o | $(BIN_DIR)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	echo $(COMPILER_VERSION) $(COMPILER_VERSION_NUMBER) $(CLANG_13_OR_MORE)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR) $(COV_DIR):
	mkdir -p $@

clean::
	@$(RM) -rv $(COV_DIR) $(LIB_DIR) $(INCLUDE_DIR) $(BIN_DIR) $(OBJ_DIR) share default.profraw

-include $($(wildcard $(OBJ_DIR)/*):.o=.d)

.SECONDARY: $(wildcard $(OBJ_DIR)/*)
