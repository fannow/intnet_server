make_dir:=$(shell pwd)
make_dir2:=$(pwd) 



.PHONY:all
all:
	make clean;\
	mkdir build; \
	cd build; \
	cmake ..; \
	make; \
	cp $(make_dir)/build/cgi/cgi $(make_dir)/webroot/;
.PHONY:clean	
clean:
	rm -rf build;\
	rm ./webroot/cgi;

.PHONY:run
run:
	make clean;\
	make;\
	source $(make_dir)/run.sh;
