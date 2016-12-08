all:
	$(MAKE) -C atk-cocoa libatkcocoa

install:
	@echo "Use make install-md to install into Monodevelop"

install-md:
	mkdir -p ../monodevelop/main/build/lib/gtk-2.0
	cp atk-cocoa/libatkcocoa.so ../monodevelop/main/build/lib/gtk-2.0/
