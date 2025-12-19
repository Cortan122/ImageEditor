# Raylib Image Editor

This is a project i wrote for my 2020/2021 coursework at my old university, but now i'm trying to use it on linux as my screenshot editor.
Some comments are in russian, sorry ☹️

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
The windows compilation is currently very broken, i suggest you use the v1.0 commit with raylib 3.5.

### Some additional make dependencies

Those are mostly common sense coreutils.

* gnu make
* xxd
* find
* akw
* a C compiler that supports the `-MMD` option

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

## Bugs

- [x] When cropping an image open as a file, the program segfaults

## Acknowledgements

- Some icons are taken from [aha-soft.com](https://www.small-icons.com/packs/16x16-free-application-icons.htm), licensed under CC SA 3.0
- The font used for added text is stolen from Minecraft
