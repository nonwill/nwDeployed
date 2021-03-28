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
#include <cstring>

/* crude temp hack until we properly use wchar, glibc isspace crashes on
 * too large values */
#define isspace_char(X) ((X) > 0 && (X) < 256 && isspace(X))

class istr {
	public:
		istr():
			elems({})
		{
		}
		istr(const char *p):
            elems(std::move(istr::from_chars(p)))
		{
		}
		istr(const string &p):
            elems(std::move(istr::from_chars(p.c_str())))
		{
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
            std::vector<int> elems_ = from_chars(s);
            elems.insert(elems.begin() + pos, elems_.begin(), elems_.end());
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

            std::vector<int> elems_ = from_chars(s, true);

			for (size_t i = 0; i < len; i++) {
				elm = (pos + i) < elems.size() ? elems[pos + i] : 0;
                if ((ret = elems_[i] - elm) != 0)
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
            std::vector<int> elems_ = from_chars(p);
            elems.insert(elems.end(), elems_.begin(), elems_.end());
			return *this;
		}
		istr &operator+=(const string &p)
		{
            return operator+=(p.c_str());
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
            std::vector<int> elems_ = from_chars(inp);
            elems.insert(elems.begin(), elems_.begin(), elems_.end());
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
        const std::string c_str(void) const
		{
            std::string s;

			for (int c : elems) {
                append_utf8_codec(s, c);
			}

            return s;
		}

        static void append_utf8_codec(std::string &s, int c)
        {
            s += char(c & 0xFF);
            if ((c >> 7) & 1) {
                unsigned int d = c;
                unsigned char point = 1;
                while ((c >> (7 - point++)) & 1) {
                    d >>= 8;
                    s += char(d & 0xFF);
                };
            }
        }

    protected:
        static std::vector<int> from_chars(const char *p, bool endchar = false)
        {
            std::vector<int> elems_;
            while( *p )
            {
                unsigned int op = *p++;
                if ((op >> 7) & 1) {
                    unsigned char nextpoint = 1;

                    /* we assume iconv produced valid UTF-8 here */
                    while ( *p && ((op >> (7 - nextpoint)) & 1) && nextpoint < 4 )
                        op |= ((*p++ & 0xFF) << (8 * nextpoint++));
                }
                elems_.push_back(op);
            }

            if ( endchar )
                elems_.push_back(0);

            return elems_;
        }

	private:
		std::vector<int> elems;
};

#endif
