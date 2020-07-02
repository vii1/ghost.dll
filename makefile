all: ghost.dll .SYMBOLIC

ghost.dll: ghost.cpp div.h
	wcl386 ghost.cpp -ox -s -l=div_dll

.SILENT
clean: .SYMBOLIC
	-del ghost.err
	-del ghost.dll
	-del ghost.obj

install: ghost.dll .SYMBOLIC
	copy /y ghost.dll c:\div2\dll
