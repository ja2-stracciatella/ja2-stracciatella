## Build on Linux using GCC

Install GCC C/C++ compiler and SDL development library.  On Ubuntu it can be done like this:

```
$ sudo apt-get install gcc g++ libsdl1.2-dev
```

If you want to run the game from the directory with source codes:

```
$ make
```

If you want to install the game to some other directory, e.g. /usr/local:

```
$ ./configure
$ make
$ sudo make install
```

## Build for Windows on Linux using MinGW (cross build)

```
$ sudo apt-get install gcc-mingw-w64 g++-mingw-w64
$ make build-win-release-on-linux:
```

## Build on Windows

Open file _build\solution-vs10e\ja2.sln with Visual Studio 2010 Express and build the project.

or:
- install Cygwin enviroment (www.cygwin.com)
- install MinGW from http://sourceforge.net/projects/mingw to folder c:\MinGW
- from the Cygwin shell, execute: ```$ make build-on-win```


## Build on Mac OS

Install Xcode and Xcode command line tools.

```
$ make build-on-mac
```

## Build on FreeBSD 10

```
# pkg install gmake
# pkg install `pkg search sdl | grep '^sdl-1.2'`
$ gmake CXX=c++
```

## Build on OpenBSD 5.5

```
# pkg_add sdl-1.2.15p6
# pkg_add gmake
# pkg_add gcc-4.6.4p7
# pkg_add g++-4.6.4p4
$ gmake CC=egcc CXX=eg++
```
