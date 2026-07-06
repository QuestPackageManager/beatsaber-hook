# Beat Saber Quest (il2cpp) C++ modding framework

_known by literally no one as pineappl_

## Information

This library offers a wide realm of C++20 features for modding il2cpp games on Android devices (specifically meant for the Oculus Quest 1/2/3).

Do not be put off by the name: `beatsaber-hook`, this is entirely game agnostic.

The only dependencies it requires are a way of determining the application ID, currently done by calling a modloader exposed function from: [scotland2](https://github.com/sc2ad/scotland2), a [logging library](https://github.com/Fernthedev/paperlog), and a [hooking api](https://github.com/sc2ad/Flamingo).

It also requires that the game is of a unity version >= 6000. Some other versions are theoretically supported with compile defines, but are currently not tested.

It is also required that il2cpp functionality is not stripped, ex: exports from the `libil2cpp.so` match most of the symbol names here (with an `il2cpp_` prefix): [shared/api.hpp](./shared/api.hpp#L30).

## High Level Functionality

This framework provides a good deal of functionality, including, but not limited to, the following:

- `i2c` API, for accessing the exported API in a more ergonomic and type safe manner
- Runtime type checking
- Detailed exceptions with backtraces
- Method hooking
- Helpers for parsing many ARM64 instructions (`capstone.hpp`)
- Interop with [the modloader](https://github.com/sc2ad/scotland2)
- Exposal of many non-exported il2cpp API functions
- Wrappers for commonly used types
- [Rapidjson](https://github.com/Tencent/rapidjson)

## Usage

`beatsaber-hook` is what I consider to be a _library_. Thus, it does not do anything on ELF load, instead only when functions are called from mods does it do anything. (This is technically not quite true, as it does initialize capstone, but nothing else.)

The most important thing to note is timing: **Most beatsaber-hook functions require `il2cpp_init` to be called!**

This means that if you have a modloader that performs things _before_ that call (as mine does), you will need to delay calling any of these functions (in `i2c`) until that point.

Typically, the workflow for a mod involves hooking into some existing game functionality on `load` (called after `il2cpp_init`) and initializing the bs-hook API, which will cache most of its accesses.

This is done simply by calling `i2c::functions::initialize()`, or by calling any `i2c` API function that uses `i2c::functions` internally.

## Building + Contributing

All contributions are welcome, [license available here](./LICENSE).

In order to build, the recommended way is to use [QPM](https://github.com/QuestPackageManager/QPM.CLI). Run `qpm restore` and `qpm qmod zip` to create `beatsaber-hook.qmod`, which can be installed by any quest mod installer.

This library now also statically links against [capstone](https://github.com/aquynh/capstone) in order to perform _better_ instruction parsing. This CAN be removed, but note that many features will be broken or otherwise damaged.

It also includes [rapidjson](https://github.com/Tencent/rapidjson).

There are a few defines that can control the built code:

- `UNITY_2019`/`UNITY_2021`/`UNITY_6`: Sets the target unity version. Exactly one should be defined; currently set in `CMakeLists.txt`.
- `HAS_CODEGEN`: Assumes a codegen-like library is present that provides headers for all C# types.
- `BS_HOOK_MATCH_UNSAFE`: Disables checks for method size and existence in `MAKE_HOOK_MATCH`.
- `SUPPRESS_MACRO_LOGS`: Disables a number of primarily error log messages.
- `PERSISTENT_DIR`: Override the base directory for mod data storage.
- `CONFIG_PATH_FORMAT`: Override the directory for mod config files.
- `NO_EVENT_CALLBACK_INVOKE_SAFETY`: Disables safety for events being added to a `basic_event_callback` during an invoke.
- `EXCEPTION_MESSAGE_SIZE`: Maximum size for il2cpp exception message formatting.

## Running tests

Tests are automatically compiled but not run. To run the tests, create and install the qmod by running `qpm qmod zip --import .\mod.tests.json test-beatsaber-hook`, then inspect the log. Note that tests failing will not necessarily cause the game to crash, and in fact rarely should.

## Acknowledgements

This wouldn't have been possible without a few people who have helped immensely.

- [emulamer](https://github.com/emulamer/): Created [BeatOn](https://github.com/emulamer/BeatOn), provided support, tested, and in general just a great person to talk to. This wouldn't exist at all if not for him motivating me to keep working.
- [jakibaki](https://github.com/jakibaki/): Created the library that actually supports all of the mod loading, as well as a lot of support when developing (and bearing with my annoying questions).
- [leo60228](https://github.com/leo60228): Created the custom colors mod using jakibaki's mod loader. The first mod that used her library.
- Trueavid#8335 (Discord): Helped test various things.
