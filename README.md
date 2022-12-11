# Rhythm ONI Plugin

![acquisition-board-plugin-screenshot](Resources/acquisition-board-plugin.png)

A set of plugins for streaming data from a device running ONI compatible version of Intan's Rhythm firmware. 

This device is currently supported:

- [Open Ephys Acquisition Board with custom FPGA (available Dec 2022)](https://open-ephys.org/acq-board); for all other versions, please install the [Rhythm Plugins](https://open-ephys.github.io/gui-docs/User-Manual/Plugins/Rhythm-Plugins.html)

## Installation

This plugins can be added via the Open Ephys GUI's built-in Plugin Installer. Press **ctrl-P** or **⌘P** to open the Plugin Installer, browse to "Acquisition Board (OE FPGA)", and click the "Install" button. The Acquisition Board (OE FPGA) plugin should now be available to use.

## Usage

Instructions for using the Rhythm ONI Plugin are available [here](https://open-ephys.github.io/gui-docs/User-Manual/Plugins/Rhythm-Plugins.html).

## Building from source

First, follow the instructions on [this page](https://open-ephys.github.io/gui-docs/Developer-Guide/Compiling-the-GUI.html) to build the Open Ephys GUI.

**Important:** This plugin is intended for use with the latest version of the GUI (0.6.0 and higher). The GUI should be compiled from the [`main`](https://github.com/open-ephys/plugin-gui/tree/main) branch, rather than the former `master` branch.

Then, clone this repository into a directory at the same level as the `plugin-GUI`, e.g.:
 
```
Code
├── plugin-GUI
│   ├── Build
│   ├── Source
│   └── ...
├── OEPlugins
│   └── rhythm-oni-plugin
│       ├── Build
│       ├── Source
│       └── ...
```

### Windows

**Requirements:** [Visual Studio](https://visualstudio.microsoft.com/) and [CMake](https://cmake.org/install/)

From the `Build` directory, enter:

```bash
cmake -G "Visual Studio 17 2022" -A x64 ..
```

Next, launch Visual Studio and open the `OE_PLUGIN_rhythm-oni-plugin.sln` file that was just created. Select the appropriate configuration (Debug/Release) and build the solution.

Selecting the `INSTALL` project and manually building it will copy the `.dll` and any other required files into the GUI's `plugins` directory. The next time you launch the GUI from Visual Studio, the new plugins should be available.


### Linux

**Requirements:** [CMake](https://cmake.org/install/)

From the `Build` directory, enter:

```bash
cmake -G "Unix Makefiles" ..
cd Debug
make -j
make install
```

This will build the plugin and copy the `.so` file into the GUI's `plugins` directory. The next time you launch the compiled version of the GUI, the new plugins should be available.


### macOS

**Requirements:** [Xcode](https://developer.apple.com/xcode/) and [CMake](https://cmake.org/install/)

From the `Build` directory, enter:

```bash
cmake -G "Xcode" ..
```

Next, launch Xcode and open the `rhythm-plugins.xcodeproj` file that now lives in the “Build” directory.

Running the `ALL_BUILD` scheme will compile the plugin; running the `INSTALL` scheme will install the `.bundle` file to `/Users/<username>/Library/Application Support/open-ephys/plugins-api8`. The new plugins should be available the next time you launch the GUI from Xcode.



## Attribution

This plugin has been collaboratively developed by Josh Siegle and Aarón Cuevas López.
