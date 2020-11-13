# windows-libc
windows-libc is a (mostly)POSIX compliant C library for Windows (10). It is designed to be modular. It is not a replacement for `msvcrt`, but designed to work alongside it.
Not all of the POSIX functions will be implemented. Only the functions which are absent in `msvcrt` or the ones which are not POSIX compliant are implemented currently. A few GNU extensions are also implemented. Wide character versions for the functions are also provided.

## Build Instructions
```
cmake -D<MOUDLE_NAME>=ON ..
```

## Usage Instructions
```
find_package(WLIBC)
...
target_link_libraries(your-target WLIBC::WLIBC)
```

See MODULES.md for the list of available modules.\
See NOTES.md for general information, caveats, differences about the functions.

## Requirements
* CMake
* Enable Developer mode on Windows 10 (for symlinks to work)
* MSVC or Clang compiler (MinGW is not supported)

## Bugs
If you find a bug please raise an issue here -> https://github.com/SibiSiddharthan/windows-libc/issues. 
