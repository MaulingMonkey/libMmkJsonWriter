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

#ifndef ZMMK_IG_JSON_WRITER_HPP
#define ZMMK_IG_JSON_WRITER_HPP

#include <vector>
#include <stddef.h>

#define ZMMK_JSON_WRITER_SAFE_BOOL(type)              \
private:                                              \
    typedef void (type::*bool_type)() const;          \
    void truthy_value() const {}                      \
public:                                               \
    operator bool_type() const {                      \
        return safe_bool_implementation() ?           \
            &type::truthy_value : 0;                  \
    }                                                 \
private:                                              \
    bool safe_bool_implementation() const /* { ... } */

namespace mmk { namespace json
{
	class writer;
	class arrayWriter;
	class objectWriter;

	struct noKeyTag {};

	class writer
	{
		char* const     begin;
		char* const     end;
		char*           position;
		bool            locked;

		friend class objectWriter;
		friend class arrayWriter;

		void error(const char*);
		void syntax(const char*);
		void syntax(char);

		void operator()(         const char* value);
		// TODO: Wide string support

		void operator()(         bool        value);

		// Values smaller than int should auto-promote unambiguously
		void operator()(unsigned int         value);
		void operator()(  signed int         value);
		void operator()(unsigned long        value);
		void operator()(  signed long        value);
		void operator()(unsigned long long   value);
		void operator()(  signed long long   value);

		void operator()(         float       value);
		void operator()(         double      value);
		void operator()(         long double value);

		ZMMK_JSON_WRITER_SAFE_BOOL(writer) { return !!position; }

	public:
		explicit writer(char* buffer, size_t bufferSize)
			: begin   (buffer+0)
			, end     (buffer+bufferSize)
			, position(bufferSize ? buffer+0 : 0)
			, locked  (false)
		{
			if (position) position[0] = '\0';
		}

		template < size_t bufferSize > explicit writer(char (&buffer)[bufferSize])
			: begin   (buffer+0)
			, end     (buffer+bufferSize)
			, position(bufferSize ? buffer+0 : 0)
			, locked  (false)
		{
			if (position) position[0] = '\0';
		}

		explicit writer(std::vector<char>& buffer)
			: begin   (buffer.data())
			, end     (buffer.data()+buffer.size())
			, position(buffer.size() ? buffer.data() : 0)
			, locked  (false)
		{
			if (position) position[0] = '\0';
		}

		const char* c_str() const { return locked ? 0 : position ? begin : 0; }
		size_t      size()  const { return locked ? 0 : position ? (position-begin) : 0; }
	};

	class objectWriter
	{
		writer& w;
		bool&   parentLock;
		bool    locked;
		bool    needsComma;

		friend class arrayWriter;

		ZMMK_JSON_WRITER_SAFE_BOOL(objectWriter) { return !!w.position; }

	public:
		objectWriter(writer& w, noKeyTag);
		objectWriter(arrayWriter& parent, noKeyTag);
		objectWriter(objectWriter& parent, const char* key);
		~objectWriter();

		template < typename Value > void operator()(const char* key, Value value)
		{
			if (locked) { w.error("objectWriter::operator()(...) invoked while locked by child writer!"); return; }
			if (needsComma) w.syntax(",");
			needsComma = true;
			w(key);
			w.syntax(":");
			w(value);
		}

		template < typename Container > void array(const char* key, const Container& container);
	};

	class arrayWriter
	{
		writer& w;
		bool&   parentLock;
		bool    locked;
		bool    needsComma;

		friend class objectWriter;

		ZMMK_JSON_WRITER_SAFE_BOOL(arrayWriter) { return !!w.position; }

	public:
		explicit arrayWriter(writer& w, noKeyTag);
		explicit arrayWriter(arrayWriter& parent, noKeyTag);
		arrayWriter(objectWriter& parent, const char* key);
		~arrayWriter();

		template < typename Value > void operator()(Value value)
		{
			if (locked) { w.error("arrayWriter::operator()(...) invoked while locked by child writer!"); return; }
			if (needsComma) w.syntax(",");
			needsComma = true;
			w(value);
		}

		template < typename Container > void array(const Container& container);
	};

	template < typename Container >
	void objectWriter::array(const char* key, const Container& container)
	{
		arrayWriter child(*this, key);
		const typename Container::const_iterator end=container.end();
		for (typename Container::const_iterator i=container.begin(); i != end; ++i) child(*i);
	}

	template < typename Container >
	void arrayWriter::array(const Container& container)
	{
		arrayWriter child(*this, noKeyTag());
		const typename Container::const_iterator end=container.end();
		for (typename Container::const_iterator i=container.begin(); i != end; ++i) child(*i);
	}
}} // namespace mmk::json


// Because I'm a horrible person, I intentionally use variable shadowing.  And suppress warnings about it.
//
// ...okay, I actually have a reason:  All access to the 'parent' writer while a child object is in scope is an error.
// I have some assert-like stuff going on, but let's be honest, compile time errors are way better than run time errors.
// Worse, keeping child writers and the parent writer straight has proved annoying - the intended scope is obvious from context, but the correct name is not always.
//
// Another way to look at this would be that I expose a single writer object that handles writing to the stream at all scopes,
// but instead of providing (key,value) and (value) overloads for objects and arrays, I abuse a proxy object to hide whichever
// set of APIs is incorrect for your current scope.

#ifdef _MSC_VER
	#define ZMMK_JSON_WARN_SUPRESS()   __pragma(warning(push)) __pragma(warning(disable: 4456))
	#define ZMMK_JSON_WARN_UNSUPRESS() __pragma(warning(pop))
#elif defined __GNUC__
	#define ZMMK_JSON_WARN_SUPRESS()   _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wshadow\"")
	#define ZMMK_JSON_WARN_UNSUPRESS() _Pragma("GCC diagnostic pop")
#else // TODO: Your compiler's warning supression pragmas here.
	#define ZMMK_JSON_WARN_SUPRESS()
	#define ZMMK_JSON_WARN_UNSUPRESS()
#endif

#define MMK_JSON_WRITER_ARRAY_ARRAY(   name       ) ZMMK_JSON_WARN_SUPRESS() if (::mmk::json::arrayWriter  name ## _array_w  = ::mmk::json::arrayWriter (name, ::mmk::json::noKeyTag() )) if (::mmk::json::arrayWriter&  name = name ## _array_w ) ZMMK_JSON_WARN_UNSUPRESS() /* { ... } */
#define MMK_JSON_WRITER_OBJECT_ARRAY(  name, key  ) ZMMK_JSON_WARN_SUPRESS() if (::mmk::json::arrayWriter  name ## _array_w  = ::mmk::json::arrayWriter (name, key                     )) if (::mmk::json::arrayWriter&  name = name ## _array_w ) ZMMK_JSON_WARN_UNSUPRESS() /* { ... } */
#define MMK_JSON_WRITER_ARRAY_OBJECT(  name       ) ZMMK_JSON_WARN_SUPRESS() if (::mmk::json::objectWriter name ## _object_w = ::mmk::json::objectWriter(name, ::mmk::json::noKeyTag() )) if (::mmk::json::objectWriter& name = name ## _object_w) ZMMK_JSON_WARN_UNSUPRESS() /* { ... } */
#define MMK_JSON_WRITER_OBJECT_OBJECT( name, key  ) ZMMK_JSON_WARN_SUPRESS() if (::mmk::json::objectWriter name ## _object_w = ::mmk::json::objectWriter(name, key                     )) if (::mmk::json::objectWriter& name = name ## _object_w) ZMMK_JSON_WARN_UNSUPRESS() /* { ... } */
#define MMK_JSON_WRITER_ROOT(          name, size ) char name ## _buf [size]; ::mmk::json::writer name( name ## _buf )
#define MMK_JSON_WRITER_ROOT_OBJECT(   name, size ) MMK_JSON_WRITER_ROOT(name, size); MMK_JSON_WRITER_ARRAY_OBJECT(name) /* { ... } */
#define MMK_JSON_WRITER_ROOT_ARRAY(    name, size ) MMK_JSON_WRITER_ROOT(name, size); MMK_JSON_WRITER_ARRAY_ARRAY(name)  /* { ... } */

#endif /* ndef ZMMK_IG_JSON_WRITER_HPP */
