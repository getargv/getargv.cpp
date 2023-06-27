LIBVER=$(shell vtool -show-build $(shell brew --prefix)/lib/libgetargv.dylib | awk '/minos/{print $$2}')
export MACOSX_DEPLOYMENT_TARGET=$(LIBVER)
VERSION=0.1
COMPAT_VERSION := $(shell echo $(VERSION) | cut -f1 -d.).0

CODESIGN_PREFIX := cam.narzt.
KEYCHAIN := ~/Library/Keychains/login.keychain-db
CERT_IDENTITY := $(shell security find-identity -v -p codesigning | sed -Ee 's/.*"([^"]+)".*/\1/g' | grep -Fve ' valid identit' -e ' CA')

SRC_DIR = src
OBJ_DIR = obj
LIB_DIR = lib
MAN_DIR = man

PREFIX := /usr/local
CXX=clang++
CPPFLAGS += -MMD -MP
CXXFLAGS := --std=c++20 -O3 -Iinclude
# must be c++20 or greater, before that c++ cannot correctly represent the semantics of this library, due to a change in copy/move semantics
EXTRA_CXXFLAGS := -pedantic-errors -Weverything -Wno-c++98-compat -Wno-pre-c++20-compat-pedantic -Wno-poison-system-directories
LDFLAGS += -Llib -fvisibility=default -fPIC
LDLIBS += -lgetargv

LIB_SHORT_NAME = getargv++
DYLIB_FILENAME = lib$(LIB_SHORT_NAME).$(VERSION).dylib
DYLIB = lib/$(DYLIB_FILENAME)
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
MANPAGE_3 := $(wildcard $(MAN_DIR)/*.3)
TMP_MANPAGE_3 := $(MANPAGE_3:$(MAN_DIR)/%=/tmp/%)

.PHONY := db clean dylib install_dylib
.DEFAULT_GOAL := dylib

dylib: $(DYLIB)


$(DYLIB): $(OBJECTS) | $(LIB_DIR)
	$(CXX) $(EXTRA_CXXFLAGS) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS) -dynamiclib $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(EXTRA_CXXFLAGS) $(CXXFLAGS) $(CPPFLAGS) -c -fPIC $< -o $@

$(OBJ_DIR) $(LIB_DIR):
	mkdir -p $@

db: compile_commands.json

compile_commands.json: Makefile
	bear -- make -B dylib

clean:
	@$(RM) -rf $(OBJ_DIR) $(LIB_DIR)

-include $(OBJECTS:.o=.d)
