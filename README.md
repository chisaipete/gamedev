# gamedev

This is a collection of my personal efforts to learn about the unique challenges of game development, while refreshing my C/C++ skills.  It's a meandering journey, more about the process of learning than final product.

Primarily, everything is built using SDL2, compiling on Windows 10 using the latest gcc compilers as provided by the excellent nuwen.net MinGW Distro.  I've borrowed and lightly contributed to some headers (cute_headers via @randypgaul) I've just started to use in my work.  

Special thanks and Kudos to lazyfoo.net and countless other folks on the internet for their sharing of their game development experience.  This effort is definitely a true demonstration of source amnesia, I'm sure there are tons of engineers I've stolen things from in this journey.

I should probably slap a license on here, but if you use anything, please at least reach out to me and let me know @chisaipete

## Development Environment Setup
- clone repository and open as a CLion project
- download latest MinGW distro from nuwen.net and extract to `C:\`
- download SDL 2.0 mingw development libraries from https://www.libsdl.org/download-2.0.php
  + unzip them and add the libs to `C:\MingGW\lib`
  + add the dlls to the project repo `bin` directory
- download SDL_image 2.0 and SDL_ttf 2.0 mingw development libraries from https://www.libsdl.org/projects/SDL_image/ and https://www.libsdl.org/projects/SDL_ttf/
  + unzip them and add their respective files to `C:\MinGW\include\SDL2` and `C:\MinGW\lib`
  + add the dlls to the project repo `bin` directory
- configure the toolchain in CLion
  + Click the "Configure" toolchain prompt in the IDE console
  + Click the "MinGW" link to autodetect
- test build any of the pre-configured builds
