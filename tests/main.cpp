/* Copyright 2017 MaulingMonkey

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include <mmk/json/writer.hpp>
#include <iostream>

int main()
{
	MMK_JSON_WRITER_ROOT_OBJECT( overflow, 16 )
	{
		overflow("i", 42);
		overflow("n", 42.0);
		overflow("s", "\x01\x1f\x20\x7e\x7f\x80string");
	}

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
