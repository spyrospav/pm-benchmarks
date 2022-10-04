.PHONY: clean install default

install:
# Possibly download GenMC
	chmod +x pmdriver.sh

clean:
	rm -rf out/*
