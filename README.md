# Build Instructions

Make a new build directory

```
mkdir build
```
Please make sure that the build directory is not commited to git tree. To do
this add a line with the build directory name/path to `.gitignore` or
`.git/info/exclude`. Example:

```
echo "build" >> .git/info/exclude
```

Change to `build` and run CMake with path to your local ABC directory (please
make sure that a shared ABC library - `libabc.so` or eqivalent is present)

```
cd build
cmake .. -DABC_DIR=<path_to_abc>
```

Run `make` (or `make -j4`) to compile. The executable `pimc` will be generated
in the `build` directory.
