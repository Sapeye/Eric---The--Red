# Eric---The--Red
IPA checker


You will need to use Codeblocks IDE, For Windows use this link http://sourceforge.net/projects/codeblocks/files/Binaries/12.11/Windows/codeblocks-12.11mingw-setup.exe/download
This also installs MinGW that is required for Windows. You may also need MSYS, see http://www.mingw.org/wiki/msys for this.
 
For Linux your Linux distribution should have CodeBlocks as an installable option.

In the folder “ShopCalc13” is Win-ShopCalc.cbp,  “fluid” is win-fluid.cbp. And for Linux  Linux-ShopCalc.cbp  and  Linux-fluid.cbp
The *.cbp files are the project files for Codeblocks.

The first step is to make fltk-1.3.3.  In the fltk-1.3.3 folder is README that gives directions on how to build FLTK, do what it says or you will be banging your head against a brick wall.

When you build fltk-1.3.3 a program called fluid will be built, do not use this program with ShopCalc as fluid has some short comings.  There is also a folder named “fluid”, build that with Codeblocks as that version and use it as I have fixed that version. Fluid is the layout editor used for building the GUI and generating the GUI source code.

Fltk-1.3.3 should be installed, use “make install” to do that, on Windows MinGW and MSYS must be installed first.

Now you can compile EricTheRedConfig in windows (and some work) inside CodeBlocks. There will be problems with the configuration of the *.cbp files that have to be resolved caused by your computer setup being different than mine, all the resolution is made within CodeBlocks. 

In the files for eric-the-red there is a folder named lib, this contains the lib files for FLTK

