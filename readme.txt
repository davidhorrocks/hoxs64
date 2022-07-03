Building

1) Install Microsoft Visual Studio 2019.

2) git clone the hoxs64 source repository. The build process does not like spaces in the source directory path. Choose a path with no spaces.

3) Install Microsoft Vcpkg from here https://github.com/Microsoft/vcpkg

4) Use Vcpkg to download, compile and configure the dependent libraries.

Execute the following commands in a Visual Studio 2019 elevated command prompt. The "integrate install" commnand option requires an elevated command prompt.

.\vcpkg\vcpkg integrate install

.\vcpkg install DirectXTK:x86-windows
.\vcpkg install DirectXTK:x64-windows

.\vcpkg install libpng:x86-windows
.\vcpkg install libpng:x64-windows

.\vcpkg install assimp:x86-windows
.\vcpkg install assimp:x64-windows

The hoxs64_2019.sln solution should in theory now build in Microsoft Visual Studio 2019.

License

Hoxs64 Commodore 64 Emulator for Windows
Copyright (C) 1999  David Horrocks

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Exclusions
The Commodore 64 machine ROM images and Commodore 1541 machine ROM 
images are Copyright (C) by Commodore Business Machines and are 
present in the following files.
    Kernal.rom
    Basic.rom
    Char.rom
    C1541.rom
    C1541.rom.1541-c2.rom
End of Exclusions

Web: http://www.hoxs64.net
Email: support@hoxs64.net
