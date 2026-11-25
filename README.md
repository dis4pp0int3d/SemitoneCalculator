# Semitone Calculator

Choose a source key and a target key to see how many semitones to move up or down. Turn on **Audio pitch** to hear that shift on the incoming audio. Minor keys use their relative majors for the calculation. For example, A minor to D minor gives **+5 semitones**.

## Build

You need a C++17 compiler, CMake and [JUCE 8.0.12](https://github.com/juce-framework/JUCE/tree/8.0.12). Download JUCE separately and point `JUCE_PATH` to its top-level folder. SoundTouch is included. Run the commands from the directory containing this README.

**Windows x64** — install Visual Studio 2026 with the Desktop development with C++ workload and CMake 4.2 or later.

```bat
cmake -S . -B build -G "Visual Studio 18 2026" -A x64 -DJUCE_PATH="C:/JUCE"
cmake --build build --config Release --target SemitoneCalculator_All
```

**macOS** — install Xcode and CMake 3.22 or later. This builds for Intel and Apple silicon.

```sh
cmake -S . -B build -G Xcode -DJUCE_PATH="$HOME/JUCE" -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0
cmake --build build --config Release --target SemitoneCalculator_All
```

Find the VST3 and standalone application under `build/SemitoneCalculator_artefacts/Release/`. The macOS build also produces an Audio Unit for Logic Pro. Copy the entire plug-in bundle when installing it. For the Windows standalone app, keep the EXE, DLL and Licenses folder together.

## License

The Semitone Calculator plug-in code and build scripts in this release are available under the [GNU Affero General Public License, version 3](LICENSE) (AGPL-3.0-only). JUCE 8 provides an AGPLv3 licensing option. [JUCE license](https://github.com/juce-framework/JUCE/blob/8.0.12/LICENSE.md)

SoundTouch 2.4.0 by Olli Parviainen handles pitch shifting and builds as a separate shared library. Its source files carry their original notices and use the [GNU Lesser General Public License, version 2.1 or later](Source/SoundTouch/COPYING.TXT).
