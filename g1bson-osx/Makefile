build/xinput2-test: xinput2_test.c build
	gcc -I/opt/X11/include -L/opt/X11/lib -lX11 -lXi -lXtst xinput2_test.c -o build/xinput2-test

build:
	mkdir -p build

clean:
	rm -Rf build
