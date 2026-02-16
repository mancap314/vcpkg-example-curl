# VCPKG Example Curl

This repo implement a project in C leveraging the [vcpkg](https://vcpkg.io) package manager. It builds a program that fetches the title of a website given its url, similar to the [example](https://doc.rust-lang.org/stable/book/ch17-01-futures-and-syntax.html) of [rust book](https://doc.rust-lang.org/stable/book/)

## dependencies
The dependencies used by this project are
- [curl](https://vcpkg.io/en/package/curl)
- [lexbor](https://vcpkg.io/en/package/lexbor), to parse html strings
- [check](https://vcpkg.io/en/package/check), to perform unit tests

## Prerequisites
First you need to install:
- llvm (for the clang compiler)
- Ninja (the build system)
- cmake (the meta build sytem)
- last but not least: vcpkg

On Windows, the easiest way is to [install chocolatey](https://chocolatey.org/install) and then in Powershell (admin mode): `choco install llvm`, `choco install ninja` and `choco install cmake`.

On Linux (Ubuntu / Debian), same with `sudo apt` instead of `choco`.

Not tested on Mac, but it should be the same with `brew`.

To install vcpkg:
1. Clone their [repo](https://github.com/microsoft/vcpkg) and go to the directory
2. Run the `bootstrap-vcpkg` script 

## Setup
1. Clone the repo and go to the project directory
2. execute `vcpkg install`
3. execute `cmake --preset your-preset`, where `your-preset` is for example `windows-release-static` if you are on Windows and wants to build a static executable in release mode
4. `cmake --build --preset your-preset`
5. (Optional) Run tests: `ctest --preset your-preset`

## Presets
### Definitions
a *preset* defines a binary output. The file [CMakePresets.json](./CMakePresets.json) defines builds for three platforms:
1. Windows
2. Linux
3. MacOS

For each platform, there is a target for release vs. debug builds, and for static vs. dynamic builds, in all combinations.
For example, there is a `windows-release-static` preset, `linux-debug-dynamic` etc.

The presets are defined by *heritage*: for example the preset `windows-release-static` inherits three presets:
1. `vcpkg-base`
2. `windows-release`
3. `release-build`

### Composing presets
The preset `windows-release` itself inherits `vcpkg-windows` (the preset `windows-dynamic` also inherits `vcpkg-windows`).

The presets inherited from are *hidden*: they can't be used directly, but only indirectly in final presets that inherit from them. This allows the possibility to *compose* various presets to create new presets with a short definition, just combining several targets through inheritance.


## Performance
The compiler is set to *clang* (see *configurePresets->cacheVariables->CMAKE_C_COMPILER* in [cmakePresets.json](./CMakePresets.json)), which normally [compiles faster than gcc](https://github.com/nordlow/compiler-benchmark).

Also used Ninja instead of Make as build system, since it has a [shorter build time](https://mesonbuild.com/Simple-comparison.html).

Additionally, the build is performed by 8 threads in parallel (see *buildPresets[0]->jobs* in [cmakePresets.json](./CMakePresets.json))).

## Adding dependencies
On the [vcpkg website](https://vcpkg.io/en/packages?query=) you can browse for packages to install in your project. By entering `vcpkg port install package-name` in the terminal, it will add an entry in *vcpkg.json*. However, it will be a single line containing the package name. Better is to have a full entry within *dependencies* containing the *name* and *version* fields (see [vcpkg.json](./vcpkg.json) file in this project).

Then you have to insert this package in [CMakeLists.txt](./CMakeLists.txt) for the targets needing it.

**Nice case**: the library is well behaved. All you have to do is 
```
find_package(ThePackage REQUIRED)
target_link_libraries(your_target PRIVATE ThePackage::thelibrary)
```

where `ThePackage` is the name of the package you installed with `vcpkg` and `thelibrary` is the name of the library it contains that you need for your target.

**Bad case**: the vcpkg port of your library does not contain a `<PackageName>Config.cmake` file. In this case, `find_package()` does not work (see [cmake documentation](https://cmake.org/cmake/help/latest/command/find_package.html)) and thus you have to include the library in three steps:
1. Find the `include/` path of the library. For `lexbor`, I did it through:
```
```
```
find_path(LEXBOR_INCLUDE_DIR NAMES lexbor/core/base.h REQUIRED)
```
Basically it's telling cmake: "the include directory of lexbor is thee directory containing `lexbor/core/base.h`".

2. Find the library object itself, distinguishing the static and the dynamic case through the `BUILD_SHARED_LIB` variable:
```
if (BUILD_SHARED_LIBS)
  find_library(LEXBOR_LIBRARY NAMES lexbor REQUIRED)
else()
  target_compile_definitions(curl_zero PRIVATE LEXBOR_STATIC)
  find_library(LEXBOR_LIBRARY NAMES lexbor_static REQUIRED)
endif()
```

3. Add the include directory and the library of the package to the target (my target here is called `curl-zero`):
```
target_include_directories(curl_zero PRIVATE ${LEXBOR_INCLUDE_DIR})
target_link_libraries(curl_zero PRIVATE ${LEXBOR_LIBRARY})
```
Ooof!!

You have to do the last step for each target using this "misbehaved" package. The first to steps, you just have to perform them for the first target using this library.


## Miscellanous
In order to use the clang LSP in neovim, with nice stuffs like syntax highlighting, auto-completion, compile checks, you need a file called *compile_commands.json* at the root of your project. How to obtain it? 
In [CMakePresets.json](./CMakePresets.json), set *cacheVariables->CMAKE_EXPORT_COMPILE_COMMANDS* to *ON*. Then after executing `cmake --preset your-preset-name` in the terminal, this file will be generated under *build/your-preset-name/*. Just copy it at the root of your project.

You also have to declare the vcpkg toolchain at the top of the *CMakeLists.txt* file:
```
set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
```

And also for avoiding dealing with relative file paths of artefacts required by the program, like in this case the [cacert.pem](./ca-certificates/cacert.pem) file required by curl, you can fix it through:
```
  target_compile_definitions(your-target PRIVATE
      YOUR_FILE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/your-file")
```

This way, you can use the macro variable `YOUR_FILE_PATH` in your code to get a path that works.

## Opinion
This combination vcpkg + CMake is quite convenient to deal with external dependencies. However it feels somehow quite clunky to first define the package dependencies in *vcpkg.json* and then integrate them in *CMakeLists.txt*





