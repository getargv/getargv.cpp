LIBVER=$(shell vtool -show-build /usr/local/lib/libgetargv.dylib | awk '/minos/{print $$2}')
export MACOSX_DEPLOYMENT_TARGET=$(LIBVER)

CXX=clang++
CPPFLAGS += -MMD -MP
CXXFLAGS += --std=c++20 -pedantic-errors -Weverything -Wno-c++98-compat -Wno-pre-c++20-compat-pedantic -Wno-poison-system-directories
LDFLAGS += -Llib -fvisibility=default -fPIC
LDLIBS += -lgetargv

.PHONY := run db clean
.DEFAULT_GOAL := run

run: bin/main
	bin/main

lib/libgetargv++.dylib: obj/argv.o obj/argvargc.o | lib
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS) -dynamiclib $^ -o $@

bin/main: lib/libgetargv++.dylib obj/main.o | bin
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS) -lgetargv++ -fPIE $^ -o $@

obj/%.o: src/%.cpp | obj
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $^ -o $@

bin lib obj:
	mkdir -p $@

db: compile_commands.json

compile_commands.json: Makefile
	bear -- make -B bin/main

clean:
	@$(RM) -rf obj bin lib

-include $(OBJ:.o=.d)
