# Homebrew Navigator

### Download

[![GitHub latest release version](https://img.shields.io/github/v/release/danielesteban/navigator.svg?style=flat)](https://navigator.gatunes.com)

### Development

##### Dependencies

 * [Build Tools for Visual Studio 2022](https://aka.ms/vs/17/release/vs_BuildTools.exe)
   * Select "Desktop development with C++"
   * Install the MSVC v143 and the Windows 11 SDK
 * [CMake](https://cmake.org/download)
 * [Conan](https://conan.io/downloads)
    * Run `conan profile detect` after install

##### Build

```bash
# clone repo:
git clone --recurse-submodules https://github.com/danielesteban/navigator.git
cd navigator
# build:
./build.sh
```

##### Optional dependencies

 * [Inno Setup](https://jrsoftware.org/isinfo.php)
   * Only required if you want to generate the setup with: [setup.iss](setup.iss)
 * [UPX](https://upx.github.io/)
   * Only required if you want compress the binary with: `PACKAGE=1 ./build.sh`
