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
#include <stdio.h>
#include <string.h>
#ifdef _MSC_VER
#include <windows.h>
#endif

namespace mmk { namespace json
{
	void writer::error(const char* message)
	{
		fprintf(stderr, "mmk::json::writer::error(\"%s\")\n", message);
	}

	void writer::syntax(const char* s)
	{
		if (!locked) { error("writer::syntax(...) invoked without being locked by a scope!"); return; }
		if (!position) return;

		const size_t n = end-position;
		const size_t w = snprintf(position, end-position, "%s", s);
		position = (w >= n) ? 0 : (position + w);
	}

	void writer::syntax(char ch)
	{
		if (!locked) { error("writer::syntax(...) invoked without being locked by a scope!"); return; }
		if (!position) return;

		if (position == end) position = NULL;
		else                *position++ = ch;
	}

	void writer::operator()(         const char* value)
	{
		if (!locked) { error("writer::operator()(...) invoked without being locked by a scope!"); return; }
		if (!value ) { syntax("null"); return; }

		syntax('\"');

		while (position && position != end)
		{
			unsigned char next = *value++;
			if (!next) break;

			switch ((char)next)
			{
			case '\"': syntax("\\\""); break;
			case '\\': syntax("\\\\"); break;
			//case '/':  syntax("\\\/"); break;
			case '\b': syntax("\\\b"); break;
			case '\f': syntax("\\\f"); break;
			case '\n': syntax("\\\n"); break;
			case '\r': syntax("\\\r"); break;
			case '\t': syntax("\\\t"); break;
			default:
				if (0x20 <= next && next < 0x7F)
				{
					syntax(next);
				}
				else
				{
					const size_t n = end-position;
					const size_t w = snprintf(position, n, "\\u%04x", next);
					position = (w >= n) ? 0 : (position + w);
				}
				break;
			}
		}

		syntax('\"');
	}

	void writer::operator()(         bool        value) { if (!locked) { error("writer::operator()(...) invoked without being locked by a scope!"); return; } syntax(value ? "true" : "false"); }
	void writer::operator()(unsigned int         value) { if (!locked) { error("writer::operator()(...) invoked without being locked by a scope!"); return; } const size_t n = end-position; const size_t w = snprintf(position, n, "%u",   value); position = (w >= n) ? 0 : (position + w); }
	void writer::operator()(  signed int         value) { if (!locked) { error("writer::operator()(...) invoked without being locked by a scope!"); return; } const size_t n = end-position; const size_t w = snprintf(position, n, "%i",   value); position = (w >= n) ? 0 : (position + w); }
	void writer::operator()(unsigned long        value) { if (!locked) { error("writer::operator()(...) invoked without being locked by a scope!"); return; } const size_t n = end-position; const size_t w = snprintf(position, n, "%lu",  value); position = (w >= n) ? 0 : (position + w); }
	void writer::operator()(  signed long        value) { if (!locked) { error("writer::operator()(...) invoked without being locked by a scope!"); return; } const size_t n = end-position; const size_t w = snprintf(position, n, "%li",  value); position = (w >= n) ? 0 : (position + w); }
	void writer::operator()(unsigned long long   value) { if (!locked) { error("writer::operator()(...) invoked without being locked by a scope!"); return; } const size_t n = end-position; const size_t w = snprintf(position, n, "%llu", value); position = (w >= n) ? 0 : (position + w); }
	void writer::operator()(  signed long long   value) { if (!locked) { error("writer::operator()(...) invoked without being locked by a scope!"); return; } const size_t n = end-position; const size_t w = snprintf(position, n, "%lli", value); position = (w >= n) ? 0 : (position + w); }
	void writer::operator()(         float       value) { if (!locked) { error("writer::operator()(...) invoked without being locked by a scope!"); return; } const size_t n = end-position; const size_t w = snprintf(position, n, "%f",   value); position = (w >= n) ? 0 : (position + w); }
	void writer::operator()(         double      value) { if (!locked) { error("writer::operator()(...) invoked without being locked by a scope!"); return; } const size_t n = end-position; const size_t w = snprintf(position, n, "%f",   value); position = (w >= n) ? 0 : (position + w); }
	void writer::operator()(         long double value) { if (!locked) { error("writer::operator()(...) invoked without being locked by a scope!"); return; } const size_t n = end-position; const size_t w = snprintf(position, n, "%Lf",  value); position = (w >= n) ? 0 : (position + w); }



	objectWriter::objectWriter(json::writer& w, noKeyTag)
		: w         (w)
		, parentLock(w.locked)
		, locked    (false)
		, needsComma(false)
	{
		if (parentLock) { w.error("objectWriter constructed against already locked writer!"); return; }
		parentLock = true;
		w.syntax("{");
	}

	objectWriter::objectWriter(arrayWriter& parent, noKeyTag)
		: w         (parent.w)
		, parentLock(parent.locked)
		, locked    (false)
		, needsComma(false)
	{
		if (parentLock) { w.error("objectWriter constructed against already locked arrayWriter!"); return; }
		parentLock = true;
		if (parent.needsComma) w.syntax(",");
		parent.needsComma = true;
		w.syntax("{");
	}

	objectWriter::objectWriter(objectWriter& parent, const char* key)
		: w         (parent.w)
		, parentLock(parent.locked)
		, locked    (false)
		, needsComma(false)
	{
		if (parentLock) { w.error("objectWriter constructed against already locked objectWriter!"); return; }
		parentLock = true;
		if (parent.needsComma) w.syntax(",");
		parent.needsComma = true;
		w(key);
		w.syntax(":{");
	}

	objectWriter::~objectWriter()
	{
		w.syntax("}");
		parentLock = false;
	}

	arrayWriter::arrayWriter(json::writer& w, noKeyTag)
		: w         (w)
		, parentLock(w.locked)
		, locked    (false)
		, needsComma(false)
	{
		if (parentLock) { w.error("objectWriter constructed against already locked writer!"); return; }
		parentLock = true;
		w.syntax("[");
	}

	arrayWriter::arrayWriter(arrayWriter& parent, noKeyTag)
		: w         (parent.w)
		, parentLock(parent.locked)
		, locked    (false)
		, needsComma(false)
	{
		if (parentLock) { w.error("objectWriter constructed against already locked arrayWriter!"); return; }
		parentLock = true;
		if (parent.needsComma) w.syntax(",");
		parent.needsComma = true;
		w.syntax("[");
	}

	arrayWriter::arrayWriter(objectWriter& parent, const char* key)
		: w         (parent.w)
		, parentLock(parent.locked)
		, locked    (false)
		, needsComma(false)
	{
		if (parentLock) { w.error("objectWriter constructed against already locked objectWriter!"); return; }
		parentLock = true;
		if (parent.needsComma) w.syntax(",");
		parent.needsComma = true;
		w(key);
		w.syntax(":[");
	}

	arrayWriter::~arrayWriter()
	{
		w.syntax("]");
		parentLock = false;
	}

}} // namespace mmk::json
