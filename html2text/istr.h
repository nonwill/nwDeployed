/*
 * Copyright 2020 Fabian Groffen
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License in the file COPYING for more details.
 */

#ifndef ISTR_H
#define ISTR_H 1

#include <vector>
#include <memory>
#include <cstring>

/* crude temp hack until we properly use wchar, glibc isspace crashes on
 * too large values */
#define isspace(X) ((X) > 0 && (X) < 256 && isspace(X))

class istr {
	public:
		istr():
			elems({})
		{
		}
		istr(const char *p):
			elems({})
		{
			for (; *p != '\0'; p++)
				elems.push_back(((int)*p) & 0xFF);
		}
		istr(const string &p):
			elems({})
		{
			int i;
			for (i = 0; i < p.length(); i++)
				elems.push_back(((int)p.at(i)) & 0xFF);
		}

		bool empty(void)
		{
			return elems.empty();
		}
		int get(size_t pos)
		{
			return elems.size() < pos ? -1 : elems[pos];
		}
		string::size_type length(void) const
		{
			return (string::size_type)elems.size();
		}
		istr &erase(size_t pos = 0, size_t len = string::npos)
		{
			if (len == string::npos)
				len = elems.size();
			/* we assume pos within range here (should throw out_of_range) */
			if (pos + len >= elems.size())
				len = elems.size() - pos;
			elems.erase(elems.begin() + pos, elems.begin() + pos + len);
			return *this;
		}
		istr &replace(size_t pos, size_t len, const char *s)
		{
			erase(pos, len);
			for (int i = 0; *s != '\0'; s++, i++)
				elems.insert(elems.begin() + pos + i, ((int)*s) & 0xFF);
			return *this;
		}
		istr &replace(size_t pos, size_t len, const int i)
		{
			erase(pos, len);
			elems.insert(elems.begin() + pos, i);
			return *this;
		}
		istr slice(size_t pos = 0, size_t len = string::npos)
		{
			istr ret = istr();

			if (len == string::npos)
				len = elems.size();
			/* we assume pos within range here (should throw out_of_range) */
			if (pos + len >= elems.size())
				len = elems.size() - pos;

			for (len += pos; pos < len; pos++)
				ret += elems[pos];

			return ret;
		}
		int compare(size_t pos, size_t len, const char *s) const
		{
			int ret;
			int elm;

			for (size_t i = 0; i < len; i++) {
				elm = (pos + i) < elems.size() ? elems[pos + i] : 0;
				if ((ret = s[i] - elm) != 0)
					break;
			}

			return ret;
		}
		istr &operator+=(const int inp)
		{
			elems.push_back(inp);
			return *this;
		}
		istr &operator+=(const char *p)
		{
			for (; *p != '\0'; p++)
				elems.push_back(((int)*p) & 0xFF);
			return *this;
		}
		istr &operator+=(const string &p)
		{
			for (const char c : p)
				elems.push_back(((int)c) & 0xFF);
			return *this;
		}
		istr &operator<<=(const int inp)
		{
			elems.push_back(inp);
			return *this;
		}
		istr &operator<<=(const char inp)
		{
			return *this <<= (((const int)inp) & 0xFF);
		}
		istr &operator<<=(const char *inp)
		{
			return *this += inp;
		}
		istr &operator>>=(const char inp)
		{
			elems.insert(elems.begin(), ((int)inp) & 0xFF);
			return *this;
		}
		istr &operator>>=(const char *inp)
		{
			for (int i = 0; *inp != '\0'; i++, inp++)
				elems.insert(elems.begin() + i, ((int)*inp) & 0xFF);
			return *this;
		}
		int operator[](const int pos) const
		{
			return elems[pos];
		}
		bool operator==(const char *inp)
		{
			return this->compare(0, strlen(inp), inp) == 0;
		}
		bool operator!=(const char *inp)
		{
			return !(*this == inp);
		}
		const char *c_str(void) const
		{
			std::unique_ptr<string> s(new std::string);

			for (int c : elems) {
				*s += c & 0xFF;
				if ((c >> 7) & 1) {
					unsigned int d = c;
					unsigned char point = 1;
					while ((c >> (7 - point++)) & 1) {
						d >>= 8;
						*s += d & 0xFF;
					};
				}
			}

			return s->c_str();
		}

	private:
		std::vector<int> elems;
};

#endif
