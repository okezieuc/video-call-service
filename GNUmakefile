.PHONY: build

setup:
	cmake -S . -B build

build:
	cmake --build build

run:
	./build/client/video_call_client.app/Contents/MacOS/video_call_client

test:
	cd ./build/tests/client && ctest

clean:
	rm -r build
