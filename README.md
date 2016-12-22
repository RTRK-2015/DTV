# DTV App

## Configuration
DTV App uses CMake (version 3.0 at least) for configuration. To configure
DTV App, run the following:

```bash
cmake -G "Unix Makefiles" \
    -DCMAKE_TOOLCHAIN_FILE="</path/to/DTVApp/marvell-toolchain.cmake" \
    </path/to/DTVApp/>
```
Please note that in-source builds are not encouraged.

## Building
Once you configure the app successfully, run the following:

```bash
make
```
to build the app.

## Running
Once the app has been successfully built, you should have an executable file
named `Projekat` in your build folder.

Note that to run the app, you must have the `assets` directory in the same
directory that you will be running the app from. This `assets` directory is
provided with the app.

Also, the app expects a path to a config file as its first argument.

## Config file
An example configuration file is included with the app.

It consists of a list of assignments, separated by newlines. Spaces in the
assignments are optional.
