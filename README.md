# Raylib Image Editor

This is a project i wrote for my 2020/2021 coursework at my old university, but now i'm trying to use it on linux as my screenshot editor.

There are some memory leaks that i haven't fixed yet, apparently...

## Dependencies

* raylib v5.0
* glfw
* xclip
* imagemagick

On arch linux you an install them as

```console
$ sudo pacman -S raylib xclip imagemagick
```

`xclip` and `imagemagick` (specifically `import`) are used for taking screenshots on x11, because i couldn't be bothered to implement it manually.

To compile this on windows, if it still even works there, you'll need `tcc.exe`.

## Build & Run

```console
$ make
$ ./main
```

This makes the debug version. If you want to install it on your system, you can use:

```console
$ sudo make install
$ raylid
```

## Keybinds

Press <kbd>F1</kbd> inside the editor to get it to display (most) of the controls.

## Configuration

You can change some values in `config.h` and then recompile the program.
