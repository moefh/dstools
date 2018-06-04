
.PHONY: all all-message clean maps extract tools genmap dsview

all: extract tools genmap dsview
	make -C extract
	make -C tools
	make -C genmap
	make -C dsview
	make all-message

clean:
	make clean -C extract
	make clean -C tools
	make clean -C genmap
	make clean -C dsview

all-message:
	@echo ====================================================
	@echo ==
	@echo ==  Build complete.
	@echo ==
	@echo ==  To generate the maps for dsview, use:
	@echo ==
	@echo ==     make maps
	@echo ==
	@echo ====================================================

maps:
	make -C genmap maps
