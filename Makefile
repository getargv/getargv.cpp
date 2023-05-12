CXX=clang++
CPPFLAGS += -MMD -MP
CXXFLAGS += --std=c++20 -pedantic-errors -Weverything -Wno-c++98-compat -Wno-pre-c++20-compat-pedantic
LDFLAGS += -Llib  -fvisibility=default
LDLIBS += -lgetargv

.PHONY := run db clean
.DEFAULT_GOAL := run

run: bin/main
	bin/main

lib/libgetargv++.dylib: obj/libgetargv++.o obj/argv.o obj/argvargc.o | lib
	$(CXX) -dynamiclib $(CXXFLAGS) $(LDFLAGS) $(LDLIBS) $^ -o $@

bin/main: lib/libgetargv++.dylib obj/main.o | bin
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS) -lgetargv++ $^ -o $@

obj/%.o: src/%.cpp | obj
	$(CXX) $(CPPFLAGS) -c $(CXXFLAGS) $< -o $@

bin lib obj:
	mkdir -p $@

db: compile_commands.json

compile_commands.json:
	bear -- make -B bin/main

clean:
	@$(RM) -rf obj bin lib

-include $(OBJ:.o=.d)
