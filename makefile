include setup.mk

all:
	make -C src

setup:
	cp -r $(HTML_ROOT) $(DEST)

clean:
	rm -rf $(DEST)/$(HTML_ROOT)
	make clean -C src
