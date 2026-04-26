.PHONY: build run-with-server

setup:
	cmake -S . -B build

build:
	cmake --build build

run: build
	./build/client/video_call_client.app/Contents/MacOS/video_call_client

run-with-server: build
	./build/server/video_call_server & server_pid=$$!; ./build/client/video_call_client.app/Contents/MacOS/video_call_client; kill "$$server_pid"

test: build
	cd ./build/tests/client && ctest

clean:
	rm -r build
