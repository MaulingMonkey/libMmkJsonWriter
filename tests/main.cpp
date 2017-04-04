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
#include <mmk/test/unit.hpp>
#include <cstring>

MMK_UNIT_TEST_CATEGORY("Demos")
{
	MMK_UNIT_TEST("Original Example")
	{
		MMK_JSON_WRITER_ROOT_OBJECT( example, 1024 )
		{
			const char* const null = 0; // C++03 hack for demo

			// example is now an objectWriter - trying to invoke array-style writing methods (e.g. "example(42)" with no key) will be a compile time error.
			example("i", 42);
			example("n", 42.0);
			example("s", "\x01\x1f\x20\x7e\x7f\x80string");
			example("p", null);

			MMK_JSON_WRITER_OBJECT_ARRAY(example, "a")
			{
				// example is now an arrayWriter - trying to invoke object-style writing methods (e.g. "example("i",42)" with a key) will be a compile time error.
				example(42);
				example(42.0);
				example("string");
				example(null);
			}

			MMK_JSON_WRITER_OBJECT_OBJECT(example, "o")
			{
				// example is now an objectWriter - trying to invoke array-style writing methods (e.g. "example(42)" with no key) will be a compile time error.
				example("i", 42);
				example("n", 42.0);
				example("s", "string");
				example("p", null);
			}
		}

		// example is now simply a writer - you can now query various properties about the final result without causing compile time errors:
		ASSERT_MSG(!!example,                 "Example safe-bools to true if the JSON fit within the buffer, false otherwise.");
		ASSERT_MSG(example.c_str(),           ".c_str() will return the non-null string if the JSON fit within the buffer, null otherwise.");
		ASSERT_CMP_MSG(example.size(), >, 32, ".size() will return the string length (excluding '\0') if the JSON fit within the buffer, 0 otherwise.");
	}
}

MMK_UNIT_TEST_CATEGORY("Edge Conditions")
{
	MMK_UNIT_TEST("Empty root overflows")
	{
		MMK_JSON_WRITER_ROOT_OBJECT(o1, 1) { (void)o1; }
		MMK_JSON_WRITER_ROOT_ARRAY( a1, 1) { (void)a1; }
		MMK_JSON_WRITER_ROOT_OBJECT(o2, 2) { (void)o2; }
		MMK_JSON_WRITER_ROOT_ARRAY( a2, 2) { (void)a2; }

		ASSERT_MSG(!o1, "Empty root object should overflow 1-byte buffer (no space for \"}\0\")");
		ASSERT_MSG(!a1, "Empty root array should overflow 1-byte buffer (no space for \"]\0\")");
		ASSERT_MSG(!o2, "Empty root object should overflow 2-byte buffer (no space for '\0')");
		ASSERT_MSG(!a2, "Empty root array should overflow 2-byte buffer (no space for '\0')");

		ASSERT_MSG(!o1.c_str(), "JSON overflows should result in null-string results");
		ASSERT_MSG(!a1.c_str(), "JSON overflows should result in null-string results");
		ASSERT_MSG(!o2.c_str(), "JSON overflows should result in null-string results");
		ASSERT_MSG(!a2.c_str(), "JSON overflows should result in null-string results");

		ASSERT_CMP_MSG(o1.size(), ==, 0, "JSON overflows should result in 0-length results");
		ASSERT_CMP_MSG(a1.size(), ==, 0, "JSON overflows should result in 0-length results");
		ASSERT_CMP_MSG(o2.size(), ==, 0, "JSON overflows should result in 0-length results");
		ASSERT_CMP_MSG(a2.size(), ==, 0, "JSON overflows should result in 0-length results");

		ASSERT_CMP_MSG(o1_buf[0],==,'\0', "JSON overflow shouldn't result in C++ level overflows");
		ASSERT_CMP_MSG(a1_buf[0],==,'\0', "JSON overflow shouldn't result in C++ level overflows");
		ASSERT_MSG(!o2_buf[0] || !o2_buf[1], "JSON root object overflow shouldn't result in C++ level overflows");
		ASSERT_MSG(!a2_buf[0] || !a2_buf[1], "JSON root array overflow shouldn't result in C++ level overflows");
	}

	MMK_UNIT_TEST("Empty root objects barely within buffers")
	{
		MMK_JSON_WRITER_ROOT_OBJECT(o, 3) { (void)o; }
		ASSERT_MSG(!!o, "Empty root object should not overflow 3-byte buffer");
		ASSERT_CMP(strcmp(o.c_str(), "{}"), ==, 0);
		// TODO: Add ASSERT_CMP_STR or similar (and perhaps regex options?) to libMmkUnitTest

		MMK_JSON_WRITER_ROOT_ARRAY(a, 3) { (void)a; }
		ASSERT_MSG(!!a, "Empty root array should not overflow 3-byte buffer");
		ASSERT_CMP(strcmp(a.c_str(), "[]"), ==, 0);
	}

	MMK_UNIT_TEST("Other overflowing examples")
	{
		MMK_JSON_WRITER_ROOT_OBJECT(o, 16)
		{
			o("i", 42);
			o("s", "\x01\x1f\x20\x7e\x7f\x80string");
			o("n", 42.0);
		}
		ASSERT_MSG(!o, "Overflow should have occured while writing string out");

		MMK_JSON_WRITER_ROOT_ARRAY(a, 16)
		{
			a(42);
			a("\x01\x1f\x20\x7e\x7f\x80string");
			a(42.0);
		}
		ASSERT_MSG(!a, "Overflow should have occured while writing string out");
	}

	MMK_UNIT_TEST("Overflows vs 0-byte regions")
	{
		// To avoid nonstandard 0-length arrays, use the low level API
		using namespace mmk::json;

		const char original[] = "    ";
		char       buffer  [] = "    ";

		{
			writer w(buffer, 0);
			ASSERT_MSG(!w, "Empty writer should've already failed to write to array");

			{
				objectWriter ow(w, noKeyTag());
				ASSERT_MSG(!ow, "Empty object writer should've already failed to write to array");
				ow("a", 42);
			}
			{
				arrayWriter aw(w, noKeyTag());
				ASSERT_MSG(!aw, "Empty object writer should've already failed to write to array");
				aw(42);
			}
		}

		static_assert(sizeof(original) == sizeof(buffer), "The below memcpy assumes these are the same size");
		ASSERT_CMP_MSG(memcmp(original, buffer, sizeof(original)), ==, 0, "writer/objectWriter/arrayWriter should've all left the buffer alone");
		// TODO: Add ASSERT_CMP_MEM or similar to libMmkUnitTest
	}
}

MMK_UNIT_TEST_CATEGORY("Formatting tests")
{
	MMK_UNIT_TEST("A simple object")
	{
		const char oExpected[] =
		"{"
			"\"i\":"      "42"    ","
			"\"s\":\""    "string"    "\""
		"}";

		MMK_JSON_WRITER_ROOT_OBJECT( o, 32 )
		{
			o("i", 42);
			o("s", "string");

			// Since exact floating point formatting can vary by platform, I've excluded it from the tests here.
		}

		ASSERT_MSG(!!o,                                     "Simple object shouldn't overflow 256-byte buffer");
		ASSERT_MSG(o.c_str(),                               "Simple object shouldn't overflow 256-byte buffer");
		ASSERT_CMP_MSG(o.size(), ==, sizeof(oExpected)-1,   "Simple object shouldn't overflow 256-byte buffer and should match expected results length");
		ASSERT_CMP_FMT(strcmp(o.c_str(), oExpected), ==, 0, "Simple object %s should match expected result %s", o.c_str(), oExpected);
	}

	MMK_UNIT_TEST("A simple array")
	{
		const char aExpected[] =
		"["
			"42"    ","
			"\""    "string"    "\""
		"]";

		MMK_JSON_WRITER_ROOT_ARRAY( a, 16 )
		{
			a(42);
			a("string");
		}

		ASSERT_MSG(!!a,                                     "Simple array shouldn't overflow 256-byte buffer");
		ASSERT_MSG(a.c_str(),                               "Simple array shouldn't overflow 256-byte buffer");
		ASSERT_CMP_MSG(a.size(), ==, sizeof(aExpected)-1,   "Simple array shouldn't overflow 256-byte buffer and should match expected results length");
		ASSERT_CMP_FMT(strcmp(a.c_str(), aExpected), ==, 0, "Simple array %s should match expected result %s", a.c_str(), aExpected);
	}

	MMK_UNIT_TEST("String encoding")
	{
		const char aExpected[] =
		"["
			"\""    "\\u0001\\u001f ~\\u007f\\u0080string"    "\""    ","
			"\""    "\\\" \\\\ / \\b \\f \\n \\r \\t"         "\""
		"]";

		MMK_JSON_WRITER_ROOT_ARRAY( a, 256 )
		{
			// Input is treated as ASCII.  Note JSON requires:
			//    Known control codes to use their special JSON literals (e.g. \" \\ \b \f \n \r \t)
			//    Unknown control codes to use the \uNNNN unicode escape syntax.
			// Additionally:
			//    Other low ASCII values (<128) will be copied literally
			//    High ASCII values (>=128) will be translated to \uNNNN unicode escape syntax - leaving the output pure 7-bit ASCII, avoiding any potential encoding issues.
			// In the future, I'll probably want to add:
			//    o("s", utf8(...));  // to treat >=128 values as utf8 encoding instead of high ASCII, allowing multi-character inputs.
			//    o("s", utf16(...)); // for wchar_t support
			//    o(utf8("s"), ...);  // Because why limit encoding to values?
			a("\x01\x1f\x20\x7e\x7f\x80string");
			a("\" \\ / \b \f \n \r \t");
		}

		ASSERT_MSG(!!a,                                     "Strings example shouldn't overflow 256-byte buffer");
		ASSERT_MSG(a.c_str(),                               "Strings example shouldn't overflow 256-byte buffer");
		ASSERT_CMP_MSG(a.size(), ==, sizeof(aExpected)-1,   "Strings example shouldn't overflow 256-byte buffer and should match expected results length");
		ASSERT_CMP_FMT(strcmp(a.c_str(), aExpected), ==, 0, "Strings example %s should match expected result %s", a.c_str(), aExpected);
	}
}

int main(int argc, char** argv)
{
	return mmk::test::unit::run(argc, argv);
}
