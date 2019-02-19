all: build

build: image_processing.c bmp_header.h
	gcc image_processing.c -Wall -o image_processing

run:
	./image_processing

clean:
	rm -f image_processing
