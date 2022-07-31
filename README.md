# windows-libc
`windows-libc` is a modular and (mostly)POSIX compliant C library for Windows (10, 11) designed to work alongside `msvcrt`.

## Build Instructions
```
cmake -DENABLE_<MOUDLE_NAME>=ON ..
```

## Usage Instructions
```
find_package(WLIBC)
...
target_link_libraries(your-target WLIBC::WLIBC)
```

See MODULES.md for more information regarding the available modules.

## Requirements
* CMake
* Enable Developer mode on Windows 10 (for symlinks to work)
* MSVC or Clang compiler (MinGW is not supported)
* (Optional) Add privilege for increase scheduling priority (SE_INC_BASE_PRIORITY_NAME)

## Bugs
If you find a bug please raise an issue here -> https://github.com/SibiSiddharthan/windows-libc/issues. 
