# dstools
This is a small collection of programs to handle Dark Souls data files.

## dsview

Map viewer (based on the game collision data). Uses OpenGL, [glad](https://github.com/Dav1dde/glad) and [glfw](http://www.glfw.org/).

Controls:

- mouse to look around
- W/A/S/D keys to move
- ESC to exit
- hold SHIFT to boost movement speed

Any errors are written to the file `out.txt`.


## genmap

Tool to generate the map files for `dsview`. It reads `*.obj` model files, generates normals based on the geometry and writes `*.objc` files that will be read by `dsview`.

An `.objc` file is a simple binary format for vertices+normals+indices.


## extract

Tools to extract data files from Dark Souls:

- `dcxtool` inflates `dcx` files
- `bndtool` lists and extracts `bnd` archives
- `bhdtool` lists and extracts `bhd`/`bdt` archives (only `BHD3`/`BDT3` are currently supported)
- `hkxtool` lists and extracts geometry from `hkx` and `hkxbhd`/`hkxbdt` files
