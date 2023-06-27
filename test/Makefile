CXX                     := clang++
PRIMARY_SOURCEFILE	:= $(wildcard ../src/*.cpp)
OBJ_DIR			:= obj
LIB_DIR			:= lib
BIN_DIR			:= bin
SRC_DIR			:= src
LIB_SHORT_NAME          := getargv++
INCLUDE_DIR		:= include/$(LIB_SHORT_NAME)
COV_DIR			:= coverage
PREFIX			:= $(PWD)
BREW_PREFIX		:= $(shell brew --prefix criterion)
CXXFLAGS                := --std=c++20
LDFLAGS			:=
DYLIB			:= $(LIB_DIR)/lib$(LIB_SHORT_NAME).dylib

export MallocNanoZone = 0
SANI_FLAGS := -fno-omit-frame-pointer -fprofile-instr-generate -fsanitize=address -fsanitize=undefined -fsanitize=nullability -fsanitize=unsigned-integer-overflow -fsanitize=local-bounds -fsanitize=nullability -fsanitize=implicit-conversion
INNER_ASAN_OPTIONS := strict_string_checks=1:detect_stack_use_after_return=1:check_initialization_order=1:strict_init_order=1:detect_invalid_pointer_pairs=2:detect_stack_use_after_return=1

.PHONY: integration_tests runner unit_tests lib_unit_tests pattern benchmark clean libtest1 libtest2 run_unit_tests run_unit_tests_verbose run_unit_tests_debug run_unit_tests_coverage unit_tests_coverage_report run_lib_unit_tests run_lib_unit_tests_verbose run_lib_unit_tests_debug run_lib_unit_tests_coverage lib_unit_tests_coverage_report

runner: $(BIN_DIR)/runner
benchmark: $(BIN_DIR)/benchmark
lib_unit_tests: $(BIN_DIR)/lib_unit_tests

$(BIN_DIR)/benchmark: EXE := $(BIN_DIR)/benchmark
$(BIN_DIR)/benchmark: CXXFLAGS += -I ../src/ -fblocks

#unit test library
$(BIN_DIR)/lib_unit_tests: CPPFLAGS := -MMD -MP -D_PRINT_METHOD=$(PRINT_METHOD) -D_SEARCH_METHOD=$(SEARCH_METHOD) -D_PID_MAX=$(PID_MAX) -DMAKE_DYLIB -fvisibility=hidden
$(BIN_DIR)/lib_unit_tests: CXXFLAGS += -I ../src/ -fblocks -I $(BREW_PREFIX)/include -g -fcoverage-mapping $(SANI_FLAGS)
$(BIN_DIR)/lib_unit_tests: EXE      := $(BIN_DIR)/lib_unit_tests
$(BIN_DIR)/lib_unit_tests: LDLIBS   := -l criterion $(SANI_FLAGS)
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

run_lib_unit_tests_coverage: export ASAN_OPTIONS = $(INNER_ASAN_OPTIONS)
run_lib_unit_tests_coverage: $(BIN_DIR)/lib_unit_tests | $(COV_DIR) $(BIN_DIR)/child
	LLVM_PROFILE_FILE="$(COV_DIR)/%c$(EXECUTABLE_NAME)-%p.profraw" $(BIN_DIR)/lib_unit_tests -j0
	/usr/bin/xcrun -r llvm-profdata merge -sparse $(COV_DIR)/$(EXECUTABLE_NAME)-*.profraw -o $(COV_DIR)/$(EXECUTABLE_NAME).profdata
	@$(RM) $(COV_DIR)/$(EXECUTABLE_NAME)-*.profraw
	/usr/bin/xcrun -r llvm-cov report --summary-only --instr-profile=$(COV_DIR)/$(EXECUTABLE_NAME).profdata $(BIN_DIR)/lib_unit_tests $(PRIMARY_SOURCEFILE)

lib_unit_tests_coverage_report:
	/usr/bin/xcrun -r llvm-cov show --instr-profile=$(COV_DIR)/$(EXECUTABLE_NAME).profdata --show-expansions --show-branches=count --show-line-counts-or-regions $(BIN_DIR)/lib_unit_tests $(PRIMARY_SOURCEFILE)
	/usr/bin/xcrun -r llvm-cov report --summary-only --instr-profile=$(COV_DIR)/$(EXECUTABLE_NAME).profdata $(BIN_DIR)/lib_unit_tests $(PRIMARY_SOURCEFILE)

$(LIB_DIR)/%.dylib:
	$(MAKE) -C .. PREFIX=$(PREFIX) dylib install_dylib

$(BIN_DIR)/%: $(OBJ_DIR)/%.o | $(BIN_DIR)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR) $(COV_DIR):
	mkdir -p $@

clean::
	@$(RM) -rv $(COV_DIR) $(LIB_DIR) $(INCLUDE_DIR) default.profraw

-include $(OBJ:.o=.d)
