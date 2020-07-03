# GHOST.DLL

This is a DLL for [DIV Games Studio 2](https://github.com/vii1/div). You can use it to 
give an additive transparency effect to your games.

The project URL is https://github.com/vii1/ghost.dll

![Demo](https://github.com/vii1/ghost.dll/blob/master/demo/ghosdemo.gif)

## How to use it

Copy the compiled GHOST.DLL to the DLL folder inside your DIV2 installation directory.

In your game source, add the following line just below your `PROGRAM` statement:
```div
IMPORT "ghost.dll";
```

And then add the following line wherever you want to activate the effect (p.e. near the start of your game):
```div
ghost_select(1);
```

From then on, your game will apply additive blending instead of standard transparency whenever you use `flags = 4`.

If you want to retain the effect between palette loads, add the following line:
```div
ghost_set_auto(1);
```

See below for a detailed explanation of what each function does.

### Functions provided

* `ghost_select(<mode>)`

  Selects the desired transparency effect and applies it immediately.

  You can select between 2 modes:
  * **0** : standard transparency (the default in DIV).
  * **1** : additive blending.

  The selected mode will be active until the next time a new palette is loaded, unless you set automatic mode with `ghost_set_auto()` (see below).

  **WARNING**: every time you use this function, your game will freeze for a few seconds (see **Issues and limitations** below).

* `ghost_set_auto(<flag>)`
  
  Enables/disables automatic overwriting of ghost table.

  Whenever a new palette is loaded, DIV generates a new ghost table automatically, which means the transparency will be set to normal. If you want to retain the additive blending effect between palette loads, you can use this function to tell GHOST.DLL to activate the effect every time a new palette is loaded.

## How to run the demo

Copy GHOSDEMO.PRG to your PRG folder and GHOST.FPG to your FPG folder inside your DIV2 installation directory. Then open and run GHOSDEMO.PRG.

At the top left you'll see the current ghost table for reference. The rest of the screen will show you various transparent sprites to demonstrate the effect.

To compare between the additive and standard blending, press **SPACE** to toggle the additive transparency effect.

You can exit the demo by pressing **ALT+X**.

## How to compile the DLL

A compiled GHOST.DLL is already provided in this repo for your convenience, so you don't need to compile it yourself unless you want to modify it.

To compile GHOST.DLL yourself, you will need the **Watcom 10.6** C++ compiler. Either the Windows or DOS version is fine. Sorry, Open Watcom is not supported currently.

### Set up your WLSYSTEM.LNK

DIV DLL's require a special system configuration for the linker. If you did this already in the past, you can skip this section. If you think you did, but are getting an error like ` undefined system name: div_dll` or end up with a GHOST.EXE instead of a GHOST.DLL, keep reading.

1. Open your WLSYSTEM.LNK file in a text editor. You will find it in the BINW subdirectory of your Watcom installation directory. P.e.: `C:\WATCOM\BINW\WLSYSTEM.LNK`
2. Go to the end of the file and paste the contents of the DIV_DLL.LNK file provided in this repository.

Alternatively, you can use a command like `type DIV_DLL.LNK >> C:\WATCOM\BINW\WLSYSTEM.LNK`.

### Compile the DLL
1. Access a console or DOS prompt with the Watcom environment configured, i.e. the Watcom binaries must be in your PATH and the WATCOM environment variable must be correctly configured. Usually the installer does this for you. In the Windows versions, you should have a shortcut in your Start menu to open a Watcom console. Check the Watcom documentation if you need help on this.
2. Enter the directory where you have cloned this repository.
3. Type the command: `wmake`
4. If all goes well, the newly created GHOST.DLL should be in the current directory.

### Install the DLL

Remember to copy the DLL to your DIV2\DLL subdirectory whenever you compile a new version. You can copy it manually or use the command `wmake install`. Warning: the makefile assumes you have DIV2 installed in `C:\DIV2`. If you have it installed in a different directory, edit the makefile first.

## Issues and limitations

### The effect is global
Whenever you activate the additive blending effect, the global ghost table of the DIV2 runtime will be overwritten. This means that **you lose the capability of using standard transparency**, at least until you switch back with `ghost_select(0)`.

It would be good to have an additional flag to use both simultaneously, but this would require rewriting the DIV2 graphics engine.

### Delay when switching the effect
GHOST.DLL will freeze your game for a few seconds every time you switch between transparency modes. More exactly:
* Every time you use `ghost_select()`.
* If you enabled **automatic mode** with `ghost_set_auto()`, every time a new palette is loaded (with `load_pal()`, `load_fpg()`, etc). DIV2 does a fade-out every time you load a new palette, so this just means that the screen will stay black a few seconds more.

Be mindful of this while coding your game, so the freezes don't harm your users' experience. If you use `ghost_select()` to switch between modes, place the call whenever the screen is not expected to change (p.e. a loading screen, between a `fade_off()` and a `fade_on()`, etc).

### Beware of runtime palette modifications
I haven't tested this DLL with palette rotations or `set_color()`, but be warned they may produce funny results.

## How does it work? The technical details!

This section is provided for the curious. You don't need to read it to use GHOST.DLL.

How does DIV2 achieve the transparency effect having just 256 colors? It uses a lookup table called the **ghost table** which tells the graphic engine what color results when you blend color A with color B. This table, of course, is 256x256 in size and each entry is 1 byte storing the index of the resultant color. DIV exposes this table through the `ghost` pointer, declared in `div.h`. You can read it or, if you want, overwrite it with any data you see fit, which is what the function `make_ghost_additive` does (see the file `ghost.cpp`).

Normally, to calculate the mix of two colors in RGB space, you just get the mean for each component, like this:
```c
r = (ColorA.r + ColorB.r) / 2;
g = (ColorA.g + ColorB.g) / 2;
b = (ColorA.b + ColorB.b) / 2;
```

This is what DIV does by default. But we want an *additive* mix, to emulate *bright* objects, and not just *transparent* objects. Well, the formula for this can't be simpler:
```c
r = ColorA.r + ColorB.r;
g = ColorA.g + ColorB.g;
b = ColorA.b + ColorB.b;
```

*But wait*. Our RGB space is limited to values 0 to 63. What if the addition yields too high a value? Shouldn't we clamp it? Well, if we were working in a *packed RGB* mode (as opposed to *palletized*), we would calculate the resulting color just by clamping these values and packing them in a 32, 24 or 16-bit value. But we are in a palletized mode, so we still have some work to do. Imagine our RGB space as a cube. Each axis in the 3-D space represents a component: red, green or blue. We have a palette that contains just 256 points inside the cube, but then we have our result point, which may or may not coincide with one of those points (most probably not, especially if our point is *outside* the cube!). We need to find which one of those 256 points has the smallest distance to our result point.

To do this, we use the RGB components as coordinates and calculate the Euclidean three-dimensional distance to each of the 256 colors. The one that yields the smallest distance is the final approximated color. Said distance is usually calculated with a variant of the Pythagorean teorem:
```c
dr = ColorA.r - ColorB.r;
dg = ColorA.g - ColorB.g;
db = ColorA.b - ColorB.b;
dist = sqrt( dr*dr + dg*dg + db*db );
```

We just want to *compare* the distances and find the smallest one, so for the sake of efficiency, we can compare the *squared* distances instead and get rid of the slow square root:
```c
squared_dist = dr*dr + dg*dg + db*db;
```

If you refer to the `ghost.cpp` file, this algorithm is contained in the `find_color` function.

## History

### 2020-07-03
* Added README.
* Removed `tab_cuad.h` dependency from makefile.
* Disabled auto mode by default.
* Added the functions `ghost_select()` and `ghost_set_auto()`.

### 2020-07-01
* First version published on GitHub.

### 2020-02-07
* First version of source and demo.