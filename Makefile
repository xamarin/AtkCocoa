INSTALL_ROOT=../monodevelop/main/build/lib/gtk-2.0/

all:
	xcodebuild DSTROOT="Build/Release"

install:
	mkdir -p $(INSTALL_ROOT)
	cp build/Release/libAtkCocoa.dylib $(INSTALL_ROOT)/libatkcocoa.so
