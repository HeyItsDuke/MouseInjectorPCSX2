#Mouse Injector Makefile
#Use with TDM-GCC 4.9.2-tdm-3 or MinGW
#For portable MinGW download Orwell Dev-C++
#mingw32-make.exe -f makefile to compile

#Compiler directories
MINGWDIR = C:/MinGW64/bin/
CC = $(MINGWDIR)gcc
WINDRES = $(MINGWDIR)windres

#Source directories
SRCDIR = ./
MANYMOUSEDIR = $(SRCDIR)manymouse/
GAMESDIR = $(SRCDIR)games/
OBJDIR = $(SRCDIR)obj/
DLLNAME = "$(SRCDIR)Mouse-Injector.dll"

#Compiler flags
CFLAGS = -ansi -O2 -m32 -std=c99 -Wall
WFLAGS = -Wextra -pedantic -Wno-parentheses -Wno-unused-parameter
RESFLAGS = -F pe-i386 --input-format=rc -O coff

#Linker flags
OBJS = $(OBJDIR)maindll.o $(OBJDIR)hook.o $(OBJDIR)mouse.o $(OBJDIR)manymouse.o $(OBJDIR)windows_wminput.o $(OBJDIR)dll.res
GAMEOBJS = $(patsubst $(GAMESDIR)%.c, $(OBJDIR)%.o, $(wildcard $(GAMESDIR)*.c))
LFLAGS = -shared $(OBJS) $(GAMEOBJS) -o $(DLLNAME) -static-libgcc -m32 -s -Wl,--add-stdcall-alias

#Main recipes
mouseinjector: $(OBJS) $(GAMEOBJS)
	$(CC) $(LFLAGS)

all: clean mouseinjector

#Individual recipes
$(OBJDIR)maindll.o: $(SRCDIR)maindll.c $(SRCDIR)hook.h $(SRCDIR)mouse.h $(GAMESDIR)game.h 
	$(CC) -c $(SRCDIR)maindll.c -o $(OBJDIR)maindll.o $(CFLAGS) $(WFLAGS)

$(OBJDIR)hook.o: $(SRCDIR)hook.c $(SRCDIR)hook.h
	$(CC) -c $(SRCDIR)hook.c -o $(OBJDIR)hook.o $(CFLAGS)

$(OBJDIR)mouse.o: $(SRCDIR)mouse.c $(SRCDIR)mouse.h $(MANYMOUSEDIR)manymouse.h
	$(CC) -c $(SRCDIR)mouse.c -o $(OBJDIR)mouse.o $(CFLAGS) $(WFLAGS)

$(OBJDIR)manymouse.o: $(MANYMOUSEDIR)manymouse.c $(MANYMOUSEDIR)manymouse.h
	$(CC) -c $(MANYMOUSEDIR)manymouse.c -o $(OBJDIR)manymouse.o $(CFLAGS) $(WFLAGS)

$(OBJDIR)windows_wminput.o: $(MANYMOUSEDIR)windows_wminput.c $(MANYMOUSEDIR)manymouse.h
	$(CC) -c $(MANYMOUSEDIR)windows_wminput.c -o $(OBJDIR)windows_wminput.o $(CFLAGS)

$(OBJDIR)dll.res: $(SRCDIR)dll.rc
	$(WINDRES) -i $(SRCDIR)dll.rc -o $(OBJDIR)dll.res $(RESFLAGS)

#Game drivers recipe
$(OBJDIR)%.o: $(GAMESDIR)%.c $(SRCDIR)maindll.h $(SRCDIR)mouse.h $(GAMESDIR)game.h $(GAMESDIR)memory.h
	$(CC) -c $< -o $@ $(CFLAGS) $(WFLAGS)

clean:
	rm -f $(SRCDIR)*.dll $(OBJDIR)*.o $(OBJDIR)*.res