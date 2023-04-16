std	:= -std=c17

all: easy_process_patcher.exe

easy_process_patcher.exe: main.o
	gcc main.o -o easy_process_patcher.exe $(std)
	cp ./easy_process_patcher.exe "E:/never_code_the_same_again/snd-reversingwithlena-tutorials/snd-reversingwithlena-tutorial26.tutorial/files/easy_process_patcher.exe"
	cp ./patcher.conf "E:/never_code_the_same_again/snd-reversingwithlena-tutorials/snd-reversingwithlena-tutorial26.tutorial/files/patcher.conf"

main: main.c
	gcc -c main.c $(std)

clean:
	rm -rf *.o easy_process_patcher.exe