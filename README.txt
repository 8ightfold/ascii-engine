If you are attempting to build this program, make sure you run the exe as admin (VERY IMPORTANT!!!).
Since Windows updated their console api, some functions may not work otherwise.

The SPEC file is a format I made to define project specifications. I have yet to standardize the system.
Hopefully I will have a parser finished soon.

The compiler command can be found in "compile.bat". If you wish to compile by hand,
make sure you include "include" with -I, and link to libgdi32.