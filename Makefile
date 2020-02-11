release:
	cmake -H. -B.release -DCMAKE_BUILD_TYPE=Release
	cmake --build .release -- -j generate

debug:
	cmake -H. -B.debug -DCMAKE_BUILD_TYPE=Debug
	cmake --build .debug -- -j generate

run: release
	.release/generate

clean:
	rm -rf .release .debug
