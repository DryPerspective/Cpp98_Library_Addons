# Library addons for the C++98 standard library project

A small collection of tools made for my [C++98 standard library project](https://github.com/DryPerspective/Cpp98_Library) which are outside of the specification of the standard library and the constraints imposed on that project. These have largely been relegated here to avoid the scope of that project from growing out of control, and serve as addons to that library.

Note that all tools in this repo require the C++98 standard library project to function, and are in `namespace dp`. 

## Features

A full writeup of every tool in this library can be found on this repo's wiki, but a brief summary of the tools included is.

* `cow_ptr` - A copy-on-write smart pointer.
* `value_ptr` - A smart pointer which provides value semantics for the held object.