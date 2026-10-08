.PHONY: all addin test checksums install clean

all: test addin checksums

addin:
	./tools/build-addin

test:
	./tools/test

checksums:
	./tools/checksums

install:
	./tools/install-calculator

clean:
	$(MAKE) -C calculator clean FXCGSDK=$${FXCGSDK:-/opt/prizmsdk-linux}
	rm -rf .build
