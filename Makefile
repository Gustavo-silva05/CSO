all: osinfo
	./osinfo

osinfo: osinfo.c
	gcc osinfo.c -o osinfo


clean:
	rm -f osinfo
	clear