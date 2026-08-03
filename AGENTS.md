# AGENTS.md

Guidance for coding agents working in `beatsaber-hook`, a C++23 modding framework for il2cpp games on
Android/Quest (game-agnostic despite the name). See [README.md](./README.md) for build instructions,
compile-time defines, and the error-handling convention (throwing vs. `i2c::result<T>`) shared across
most of the API. This file is a map of what the library exposes, not a design doc.

## Layout

- `shared/*.hpp` — the public, header-heavy API surface (what mods actually include).
- `src/*.cpp` — implementations backing the headers above.
- `src/tests/*.cpp` — runtime tests, registered via the `TEST(name)` macro (`src/tests/tests.hpp`) and
  run as an installed qmod (see README "Running tests"); they execute against a real running game.

## Feature areas

- **`api.hpp` / `src/api.cpp`** — `i2c::functions`, raw function pointers to (almost) every exported
  il2cpp C API function, resolved/cached via `i2c::functions::initialize()`. Also home to higher-level
  helpers: GC-aware allocation, `i2c::resolve_icall<R, TArgs...>` for resolving internal calls (icalls)
  by name, and assembly/image enumeration.
- **`types.hpp` / `src/types.cpp`** — Runtime type checking and conversion: `i2c::cast`/`try_cast`
  between il2cpp types, `class_of<T>()`, `class_from_name`, C#-type reflection helpers, and the
  `type_check`/`type_markers` concepts used throughout the library to distinguish reference types,
  value types, wrapper types, etc. at compile time.
- **`members.hpp`** — The main "do stuff with C# objects" API: `run_method`, `get_field`/`set_field`,
  `get_property`/`set_property`, `new_ctor`. All follow the shared throw-vs-`i2c::result<T>` convention
  and accept flexible ways of identifying a class/method/field (name strings, `MethodInfo*`, generics).
- **`hooking.hpp`** — Method hooking macros: `MAKE_HOOK`/`MAKE_HOOK_MATCH` (define a hook, with or
  without matching against a real `MethodInfo` for safety), `INSTALL_HOOK`/`UNINSTALL_HOOK`,
  `HOOK_ORIG`/`HOOK_BEFORE`/`HOOK_AFTER`. Built on top of the Flamingo hooking backend.
- **`callback.hpp`** — `basic_event_callback` and friends: type-erased C++ callback/event containers
  usable as backing storage for hooked C# events/delegates.
- **`safeptr.hpp` / `src/safeptr.cpp`** — `safe_ptr`/`count_ptr` smart pointers that keep a GC handle
  alive (reference counted) across garbage collections, so a held C# object isn't collected out from
  under a mod. Includes `try_cast()` between wrapped types.
- **`stringw.hpp` / `src/stringw.cpp`** — `StringW`, a wrapper around `Il2CppString*` with
  C++/UTF-8-friendly construction, comparison, and conversion, plus `ConstString` for compile-time
  interned strings.
- **`arrayw.hpp` / `listw.hpp`** — `ArrayW<T>`/`ListW<T>` wrappers giving C++ iterator/array semantics
  (`[]`, `begin()`/`end()`, `try_get`) over `Il2CppArray*`/`System.Collections.Generic.List<T>`.
- **`valuew.hpp`** — `ValueW<Size, Namespace, Name>`, an (unsafely) sized stand-in for C# value types
  when no codegen type is available.
- **`byref.hpp`** — `by_ref<T>`, marks a parameter as a C# `ref`/`out` byref for `run_method` and
  friends (needed since plain C++ references are ambiguous with byref at compile time).
- **`threading.hpp`** — `i2c::threading::il2cpp_thread` and helpers for spinning up threads that are
  properly attached/detached from the il2cpp runtime.
- **`exceptions.hpp` / `src/exceptions.cpp`** — `i2c::trace_exception` (exceptions carrying a captured
  backtrace), `exception_to_string` for `Il2CppException*`, and `result_or_throw<T>`, the shared
  primitive behind the throw-vs-`i2c::result<T>` convention.
- **`utils.hpp` / `src/utils.cpp`** — Foundational utilities used everywhere else: `i2c::result<T>`
  (`std::expected<T, std::string>` alias) and its `is_result_v`/`remove_result_t`/`change_result_t`
  helpers, `function_ptr_t`, misc concepts, and logging glue.
- **`find.hpp` / `src/find.cpp`** — Internal helpers for resolving classes/methods/fields/properties
  from name-based lookup info structs, shared by `members.hpp`.
- **`config.hpp`** — Mod config path conventions and small config-related macros/defines
  (`PERSISTENT_DIR`, `CONFIG_PATH_FORMAT`, visibility/inline attribute macros).
- **`debug.hpp` / `src/debug.cpp`** — Debug logging helpers for dumping il2cpp class/method metadata
  (slow, intended for one-off investigation, not production paths).
- **`binary.hpp` / `src/binary.cpp`** — Raw memory inspection: prints bytes at a pointer and heuristically
  follows nested pointers, for low-level debugging.
- **`capstone.hpp` / `src/capstone.cpp`** — ARM64 instruction parsing (via statically-linked Capstone),
  used by hook-safety checks (e.g. `MAKE_HOOK_MATCH`) to detect if a target method's compiled code looks
  as expected.
- **`alphanum.hpp`** — A natural-order ("alphanum") string comparator.
- **`rapidjson.hpp`** — Bundled [RapidJSON](https://github.com/Tencent/rapidjson) plus small helpers for
  JSON (de)serialization.

## Testing requirement

**Every new feature or bug fix must come with a test in `src/tests/`.** Add a `TEST(name)` (see
`src/tests/tests.hpp`) in the file matching the area you touched (e.g. `api.cpp` for `api.hpp` changes),
covering both the success path and the failure/edge case your change was about. Since these tests run
against a real il2cpp process rather than in a desktop unit-test harness, always also verify the change
compiles cleanly for the real target, even when the test itself can't be executed in this environment.

Use the qpm scripts (defined in `qpm.json`'s `workspace.scripts`), not raw `cmake` invocations:

- `qpm s build` — configures and builds everything (both `beatsaber-hook` and `test-beatsaber-hook`)
  against the real Android NDK toolchain with `-Wall -Wextra -Werror`.
- `qpm s tests` — builds, then packages `test-beatsaber-hook.qmod`, the qmod that gets installed on a
  Quest to actually run the `TEST(...)` cases (see README "Running tests"). Building it is the
  compile-verification step available here; running it requires a real device.

## Conventions worth knowing before editing

- Most functions require `i2c::functions::initialize()` (implicitly called by most `i2c` API entry
  points) which in turn requires `il2cpp_init` to have already run — see README "Usage".
- The throw vs. `i2c::result<T>` split (README "Error Handling") is selected via the template
  return-type argument itself, not a separate bool flag or overload — follow this convention for any
  new API rather than inventing a new error-handling shape.
- `type_check`/`type_markers` concepts (in `types.hpp`) are the vocabulary used across `members.hpp`,
  `byref.hpp`, `valuew.hpp`, etc. to describe "is this a C# reference type / value type / wrapper type";
  reuse them rather than re-deriving similar checks.
