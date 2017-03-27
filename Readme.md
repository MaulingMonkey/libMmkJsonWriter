# libMmkJsonWriter

MaulingMonKey's Json Writer API.  No, the world didn't really need another JSON writer library.  Yes, I wrote one anyways.

Project Goals:
- Zero Allocations - I should be able to use this to report heap corruptions.  Payload limits are a feature (tm).
- Cross Platform - any C++03 compiler should work (tm).
- Less Bugs - via the abuse of macros & variable shadowing, to avoid common errors.
- No Config - aiming to have "perfect" NuGet integration, with .libs, sane defaults, and source compilation fallback.

License: [Apache 2.0](LICENSE.txt)

# Example ([tests/main.cpp](tests/main.cpp))

```cpp
#include <mmk/json/writer.hpp>
#include <iostream>

int main()
{
	MMK_JSON_WRITER_ROOT_OBJECT( example, 1024 )
	{
		const char* const null = 0; // C++03 hack for demo

		// example is now an objectWriter
		example("i", 42);
		example("n", 42.0);
		example("s", "\x01\x1f\x20\x7e\x7f\x80string");
		example("p", null);

		MMK_JSON_WRITER_OBJECT_ARRAY(example, "a")
		{
			// example is now an arrayWriter
			example(42);
			example(42.0);
			example("string");
			example(null);
		}

		MMK_JSON_WRITER_OBJECT_OBJECT(example, "o")
		{
			// example is now an objectWriter
			example("i", 42);
			example("n", 42.0);
			example("s", "string");
			example("p", null);
		}
	}

	// example is now a writer
	if (example) std::cout << example.c_str() << "\n";
}
```

Output:
```json
{"i":42,"n":42.000000,"s":"\u0001\u001f ~\u007f\u0080string","p":null,"a":[42,42.000000,"string",null],"o":{"i":42,"n":42.000000,"s":"string","p":null}}
```

# Installation

## Via NuGet
<strike>Add [libMmkJsonWriter](https://www.nuget.org/packages/libMmkJsonWriter/) to your project via nuget.  Done!</strike> **Soon(tm)**

## From Source (Windows)
- Clone the repository
- Open libMmkJsonWriter.sln, uild whatever combinations you like
- Add [libMmkJsonWriter/include/](libMmkJsonWriter/include/) to your #include paths.
- Add [libMmkJsonWriter/build/native/libs/](libMmkJsonWriter/build/native/libs/) to your library paths.
- Link libMmkJsonWriter.lib

## From Source (Linux)
- Clone the repository
- Invoke make
- Add [`-IlibMmkJsonWriter/include/`](libMmkJsonWriter/include/) to your `CCFLAGS`.
- Add [`-LlibMmkJsonWriter/build/native/libs/`](libMmkJsonWriter/build/native/libs/) to your `LDFLAGS`.
- Add `-lMmkJsonWriter` to your `LDFLAGS`.

# Compatability

Supported compilers:
- MSVC 2005+
- GCC 3.0+
- Clang

Requirements:
- A C++03 compatable compiler

# TODO

- Public CI
- Support ICC
- 'nix friendly packaging?

# Why?

- I plan to write an SDK for sentry.io using this.
- Also I want to experiment with better native nuget packaging.
