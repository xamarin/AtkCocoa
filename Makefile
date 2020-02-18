INSTALL_PATH=main/build/lib/gtk-2.0
MD_ADDINS_INSTALL_ROOT=../../../$(INSTALL_PATH)
STANDALONE_INSTALL_ROOT=../vsmac/$(INSTALL_PATH)

all:
	xcodebuild DSTROOT="Build/Release"

install:
	mkdir -p $(MD_ADDINS_INSTALL_ROOT)
	cp build/Release/libAtkCocoa.dylib $(MD_ADDINS_INSTALL_ROOT)/libatkcocoa.so

install-standalone:
	mkdir -p $(STANDALONE_INSTALL_ROOT)
	cp build/Release/libAtkCocoa.dylib $(STANDALONE_INSTALL_ROOT)/libatkcocoa.so
	
clean:
	xcodebuild -alltargets clean
