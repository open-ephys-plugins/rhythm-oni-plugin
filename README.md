# Rhythm Plugins

![acquisition-board-plugin-screenshot](Resources/acquisition-board-plugin.png)

A set of plugins for streaming data from devices running Intan's Rhythm firmware. 

These devices are currently supported:

- [Open Ephys Acquisition Board](https://open-ephys.org/acq-board)
- [Intan RHD USB Interface Board](https://intantech.com/RHD_USB_interface_board.html)

Plugins for the following devices are coming soon:

- [Intan 512 and 1024 Recording Controllers](https://intantech.com/RHD_controllers.html)
- [Intan RHS Recording/Stimulation Controller](https://intantech.com/stim_record_controller.html)

## Installation

These plugins can be added via the Open Ephys GUI's built-in Plugin Installer. Press **ctrl-P** or **⌘P** to open the Plugin Installer, browse to "Rhythm Plugins," and click the "Install" button. The Acquisition Board, Intan RHD USB, Acq. Board Output plugins should now be available to use.

## Usage

(link to documentation)


## Building from source

First, follow the instructions on [this page](https://open-ephys.github.io/gui-docs/Developer-Guide/Compiling-the-GUI.html) to build the Open Ephys GUI.

**Important:** This plugin is intended for use with the pre-release core application, version 0.6.0. The GUI should be compiled from the [`development-juce6`](https://github.com/open-ephys/plugin-gui/tree/development-juce6) branch, rather than the `master` branch.

Then, clone this repository into a directory at the same level as the `plugin-GUI`, e.g.:
 
```
Code
├── plugin-GUI
│   ├── Build
│   ├── Source
│   └── ...
├── OEPlugins
│   └── rhythm-plugins
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

Next, launch Visual Studio and open the `OE_PLUGIN_rhythm-plugins.sln` file that was just created. Select the appropriate configuration (Debug/Release) and build the solution.

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

This will build the plugin and copy the `.so` file into the GUI's `plugins` directory. The next time you launch the GUI compiled version of the GUI, the new plugins should be available.


### macOS

**Requirements:** [Xcode](https://developer.apple.com/xcode/) and [CMake](https://cmake.org/install/)

From the `Build` directory, enter:

```bash
cmake -G "Xcode" ..
```

Next, launch Xcode and open the `rhythm-plugins.xcodeproj` file that now lives in the “Build” directory.

Running the `ALL_BUILD` scheme will compile the plugin; running the `INSTALL` scheme will install the `.bundle` file to `/Users/<username>/Library/Application Support/open-ephys/plugins-api`. The new plugins should be available the next time you launch the GUI from Xcode.



## Attribution

Developed by Jonathan Newman ([@jonnew](https://github.com/jonnew)).
