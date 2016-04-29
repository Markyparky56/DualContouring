This solution is intended to be compiled for x64, x86 configurations may be missing certain options.

This application makes use of the [Libnoise Library](http://libnoise.sourceforge.net/) in a modified form.
The modified version can be found at https://github.com/Markyparky56/libnoise. The modified version is configured to 
produce a static .lib file as opposed to a .dll.

This program also makes use of GLEW and GLFW3. These .lib and .dll files must be present when compiling and executing. 
Within the folder External is the folder lib containing folders for each build configuration. These folders contain a 
lib.txt specificy the required .lib files which must be present for that configuration.
Libnoise should output its own .lib file to the appropriate folder during compilation.

[GLEW](http://glew.sourceforge.net/) binaries can be found at: https://sourceforge.net/projects/glew/files/glew/1.13.0/glew-1.13.0-win32.zip/download 
or you may compile them from source. 

[GLFW3](http://www.glfw.org/docs/latest/) binaries can be found at: https://github.com/glfw/glfw/releases/download/3.1.2/glfw-3.1.2.bin.WIN64.zip 
or you may compile them from source.

Libnoise is released under the [GNU General Public License](http://libnoise.sourceforge.net/downloads/gpl/COPYING.txt)
[GLEW licensing](https://github.com/nigels-com/glew#copyright-and-licensing)
[GLFW licensing](http://www.glfw.org/license.html)