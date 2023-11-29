LIBVER ?=$(shell vtool -show-build $(shell brew --prefix)/lib/libgetargv.dylib | awk '/minos/{print $$2}')
export MACOSX_DEPLOYMENT_TARGET=$(LIBVER)
VERSION=0.1
COMPAT_VERSION := $(shell echo $(VERSION) | cut -f1 -d.).0

CODESIGN_PREFIX := cam.narzt.
KEYCHAIN := ~/Library/Keychains/login.keychain-db
CERT_IDENTITY := $(shell security find-identity -v -p codesigning | sed -Ee 's/.*"([^"]+)".*/\1/g' | grep -Fve ' valid identit' -e ' CA')

SRC_DIR = src
OBJ_DIR = obj
LIB_DIR = lib

PREFIX := /usr/local
CXX=clang++
CPPFLAGS += -MMD -MP

COMPILER_VERSION		:= $(shell $(CXX) --version | grep version | grep -o -m 1 "[0-9]\+\.[0-9]\+\.*[0-9]*" | head -n 1)
COMPILER_VERSION_NUMBER		:= $(shell echo $(COMPILER_VERSION) | sed -e 's/\.\([0-9][0-9]\)/\1/g' -e 's/\.\([0-9]\)/0\1/g' -e 's/^[0-9]\{3,4\}$$/&00/')
CLANG_13_OR_MORE		:= $(shell expr $(COMPILER_VERSION_NUMBER) \>= 130106)
ifneq ($(CLANG_13_OR_MORE),0)
# supported: c++11, c++14, c++17, c++20
# future: c++2b
CXXFLAGS := --std=c++20 -O3 -Iinclude
else
CXXFLAGS := --std=c++17 -O3 -Iinclude
endif

EXTRA_CXXFLAGS := -pedantic-errors -Weverything -Wno-c++98-compat -Wno-pre-c++20-compat-pedantic -Wno-poison-system-directories
LDFLAGS += -Llib -fvisibility=default -fPIC
LDLIBS += -lgetargv

LIB_SHORT_NAME = getargv++
DYLIB_FILENAME = lib$(LIB_SHORT_NAME).$(VERSION).dylib
DYLIB = lib/$(DYLIB_FILENAME)
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

.PHONY := db clean dylib install_dylib
.DEFAULT_GOAL := dylib

dylib: $(DYLIB)

install_dylib: $(DYLIB)
	install -d $(PREFIX)/$(LIB_DIR)
	install $(DYLIB) $(PREFIX)/$(DYLIB)
	install_name_tool -id $(PREFIX)/$(DYLIB) $(PREFIX)/$(DYLIB)
ifdef CERT_IDENTITY
	codesign --options runtime --prefix=$(CODESIGN_PREFIX) -s "$(CERT_IDENTITY)" --keychain $(KEYCHAIN) $(PREFIX)/$(DYLIB)
endif
	ln -sf ./$(DYLIB_FILENAME:%.$(VERSION).dylib=%.$(COMPAT_VERSION:%.0=%).dylib) $(PREFIX)/$(DYLIB:%.$(VERSION).dylib=%.dylib)
	ln -sf ./$(DYLIB_FILENAME) $(PREFIX)/$(DYLIB:%.$(VERSION).dylib=%.$(COMPAT_VERSION:%.0=%.dylib))
	install -d $(PREFIX)/include/$(LIB_SHORT_NAME)
	install -m 444 $(wildcard include/*.hpp) $(PREFIX)/include/$(LIB_SHORT_NAME)/
	sed -e "/Copyright: see LICENSE file/r LICENSE" -e "/Copyright: see LICENSE file/d" -i '' $(PREFIX)/include/$(LIB_SHORT_NAME)/*.hpp
	install -d $(PREFIX)/lib/pkgconfig/
	install -m 444 $(LIB_SHORT_NAME).pc $(PREFIX)/lib/pkgconfig/

$(DYLIB): $(OBJECTS) | $(LIB_DIR)
	$(CXX) $(EXTRA_CXXFLAGS) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS) -dynamiclib $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(EXTRA_CXXFLAGS) $(CXXFLAGS) $(CPPFLAGS) -c -fPIC $< -o $@

$(OBJ_DIR) $(LIB_DIR):
	mkdir -p $@

docs:
	doxygen -q doxygen.conf

db: compile_commands.json

compile_commands.json: Makefile
	$(shell brew --prefix bear)/bin/bear -- make -B $(OBJECTS)

clean:
	@$(RM) -rf $(OBJ_DIR) $(LIB_DIR) docs

lint: compile_commands.json
	for file in $(SOURCES); do /usr/bin/xcrun -r clangd --enable-config --clang-tidy --log=error --check=$$file; done
	$(shell brew --prefix llvm)/bin/scan-build -enable-checker security -enable-checker unix -enable-checker valist $(MAKE) -B dylib

-include $(OBJECTS:.o=.d)
