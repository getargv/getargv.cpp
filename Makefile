include Makefile-variables

.PHONY := db clean dylib install install_dylib lint dmg bump tag
.DEFAULT_GOAL := dylib

dylib: $(DYLIB)

install: install_dylib

install_dylib: $(DYLIB)
	install -d $(PREFIX)/$(LIB_DIR)
	install $(DYLIB) $(PREFIX)/$(DYLIB)
	install_name_tool -id $(PREFIX)/$(DYLIB) $(PREFIX)/$(DYLIB)
ifdef CERT_IDENTITY
ifneq ($(CERT_IDENTITY),)
	codesign --options runtime --prefix=$(CODESIGN_PREFIX) -s "$(CERT_IDENTITY)" --keychain $(KEYCHAIN) $(PREFIX)/$(DYLIB)
endif
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

$(OBJ_DIR) $(LIB_DIR) $(FAKE_ROOT) $(PKG_DIR):
	mkdir -p $@

bump: SHELL:=$(shell which bash)
bump:
	@ruby -e 'path = STDIN.read.strip;File.write(path, File.read(path).sub(/^(Version: +)([0-9]+)(?:(?:\.)([0-9]+)){1,}/){|s|"#{$$1}#{$$2}.#{$$3.to_i + 1}"})' <<< getargv++.pc
	@ruby -e 'path = STDIN.read.strip;File.write(path, File.read(path).sub(/^(VERSION=)([0-9]+)(?:(?:\.)([0-9]+)){1,}/){|s|"#{$$1}#{$$2}.#{$$3.to_i + 1}"})' <<< Makefile-variables
	@ruby -e 'path = STDIN.read.strip;File.write(path, File.read(path).sub(/^(PROJECT_NUMBER += +)([0-9]+)(?:(?:\.)([0-9]+)){1,}/){|s|"#{$$1}#{$$2}.#{$$3.to_i + 1}"})' <<< doxygen.conf
	@env PKG_CONFIG_PATH=. pkg-config --define-variable=dep="''" --modversion getargv++

tag:
	git add -A
	git commit -m 'version $(VERSION)'
	git tag '$(VERSION)'
	git push origin HEAD:main
	git push origin tag '$(VERSION)'

docs: doxygen.conf $(SOURCES) $(HEADERS)
	doxygen -q doxygen.conf

db: compile_commands.json

compile_commands.json: Makefile
	$(shell brew --prefix bear)/bin/bear -- make -B $(OBJECTS)

clean:
	@$(RM) -rf $(OBJ_DIR) $(LIB_DIR) docs $(FAKE_ROOT) $(PKG_DIR)

lint: compile_commands.json
	for file in $(SOURCES); do /usr/bin/xcrun -r clangd --enable-config --clang-tidy --log=error --check=$$file; done
	$(shell brew --prefix llvm)/bin/scan-build -enable-checker security -enable-checker unix -enable-checker valist $(MAKE) -B dylib

dmg: $(DMG)

$(DMG): $(PRODUCT)
	hdiutil create -fs "$(DMG_FS)" -volname "$(DMG_VOLUME_NAME)" -srcfolder "$(PKG_DIR)" -ov -format "$(DMG_FORMAT)" "$@"

$(DISTRIBUTION): $(SRC_DIR)/dist.xml
	< $< > $@ sed \
	-e 's/OS_VERSION/$(MACOSX_DEPLOYMENT_TARGET)/g' \
	-e 's/ARCH/$(ARCH)/g' \
	-e 's/VERSION/$(VERSION)/g' \
	-e 's/LIB_PKG_NAME/$(LIB_PKG:$(PKG_DIR)/%=%)/g' \
	-e 's/LIB_ID/$(LIB_BUNDLE_IDENTIFIER)/g' \
	-e 's/LIB_NAME/$(LIB_NAME)/g'

$(PRODUCT): $(LIB_PKG) $(DISTRIBUTION)
	productbuild \
	--identifier $(PRODUCT_BUNDLE_IDENTIFIER) \
	--version $(VERSION) \
	--package-path $(PKG_DIR) \
	--resources ./ \
	--distribution $(DISTRIBUTION) \
	$(SIGN_PACKAGE_FLAG) \
	$@
	@$(RM) $^

$(LIB_PKG): $(FAKE_ROOT) | $(PKG_DIR)
	@$(RM) -rf $(FAKE_ROOT)/*
	$(MAKE) PREFIX=$(FAKE_ROOT) install_dylib
	pkgbuild --root $(FAKE_ROOT) \
	--identifier "$(LIB_BUNDLE_IDENTIFIER)" \
	--version "$(VERSION)" \
	$(PKG_VERSION_FLAG) \
	--install-location "$(PREFIX)" \
	$(SIGN_PACKAGE_FLAG) \
	$@

-include $(OBJECTS:.o=.d)
