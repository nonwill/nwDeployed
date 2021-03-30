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

const char endl_char = '\n';
const char flush_char = '\0';

class istr {
public:
	public:
        typedef std::vector<unsigned int> utf8_string;

		istr():
			elems({})
		{
		}
        istr(const istr &is):
            elems(is.elems)
        {
        }
        istr(const char *p):
            elems(std::move(istr::from_stdstring(p)))
		{
		}
        istr(const string &s):
            elems(std::move(istr::from_stdstring(s)))
		{
		}
        virtual ~istr(){}

        inline bool empty(void)
		{
			return elems.empty();
		}
        inline int get(size_t pos)
        {
            return elems.size() < pos ? -1 : elems[pos];
        }
        inline string::size_type length(void) const
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
            utf8_string elems_ = from_stdstring(s);
            elems.insert(elems.begin() + pos, elems_.begin(), elems_.end());
			return *this;
		}
        istr &replace(size_t pos, size_t len, const unsigned int i)
		{
			erase(pos, len);
			elems.insert(elems.begin() + pos, i);
			return *this;
		}
        std::string slice(size_t pos = 0, size_t len = string::npos) const
		{
            if (len == string::npos)
                len = elems.size();
            /* we assume pos within range here (should throw out_of_range) */
            if (pos + len >= elems.size())
                len = elems.size() - pos;

            std::string s;

            for (len += pos; pos < len; ++pos)
                append_utf8char(s, elems[pos]);

            return s;
		}
        int compare(size_t pos, size_t len, const char *s) const
        {
            int ret;
            unsigned int elm;

            utf8_string elems_ = from_stdstring(s, true);

            for (size_t i = 0; i < len && i < elems_.size(); ++i) {
                elm = (pos + i) < elems.size() ? elems[pos + i] : 0;
                if ((ret = elems_[i] - elm) != 0)
                    break;
            }

            return ret;
        }
        istr &operator+=(const istr &inp)
        {
            elems.insert(elems.end(), inp.elems.begin(), inp.elems.end());
            return *this;
        }
        istr &operator+=(const int inp)
        {
            elems.push_back((unsigned int)inp);
            return *this;
        }
#if 0
        istr &operator+=(const unsigned int inp)
        {
            elems.push_back(inp);
            return *this;
        }
#endif
        istr &operator+=(const char *p)
		{
            utf8_string elems_ = from_stdstring(p);
            elems.insert(elems.end(), elems_.begin(), elems_.end());
			return *this;
		}
        inline istr &operator+=(const string &p)
		{
            utf8_string elems_ = from_stdstring(p);
            elems.insert(elems.end(), elems_.begin(), elems_.end());
            return *this;
        }
#if 0
        istr &operator<<=(const istr &inp)
		{
            elems.insert(elems.end(), inp.elems.begin(), inp.elems.end());
			return *this;
		}
        istr &operator<<=(const unsigned int inp)
        {
            elems.push_back(inp);
            return *this;
        }
#endif
        istr &operator<<=(const int inp)
		{
            elems.push_back((unsigned int)inp);
            return *this;
        }
        inline istr &operator<<=(const char *inp)
		{
			return *this += inp;
		}
#if 0
        istr &operator>>=(const istr &inp)
        {
            elems.insert(elems.begin(), inp.elems.begin(), inp.elems.end());
            return *this;
        }
#endif
        istr &operator>>=(const char inp)
		{
            elems.insert(elems.begin(), inp & 0xFF);
			return *this;
		}
        istr &operator>>=(const char *inp)
		{
            utf8_string elems_ = from_stdstring(inp);
            elems.insert(elems.begin(), elems_.begin(), elems_.end());
            return *this;
		}
        inline unsigned int operator[](const size_t pos) const
		{
			return elems[pos];
		}
        inline bool operator==(const char *inp)
        {
            return this->compare(0, strlen(inp), inp) == 0;
        }
        inline bool operator!=(const char *inp)
		{
			return !(*this == inp);
		}
        const std::string to_stdstring(void) const
		{
            std::string s;
            s.reserve(elems.size() * 3);

            for ( size_t b = 0; b < elems.size(); ++b)
                append_utf8char(s, elems[b]);

            return s;
		}

        static void append_utf8char(std::string &s, unsigned int inp)
        {
            if( inp < 0xff)
            {
                s += inp;
                return;
            }

            unsigned char point = 0;
            char fc = 0;
            do {
                fc = (inp >> (24 - 8 * point)) & 0xff;
                ++point;
            }while(!fc && point < 4);

            s += fc;

            if ((fc >> 7) & 1) {
                char single = 0;
                while((single = (inp >> (24 - 8 * point))) && point++ < 4)
                    s += single;
            }
        }

        static utf8_string from_stdstring(const char *p, bool endchar = false, long long reserve = -1)
        {
            utf8_string elems_;

            if( p && *p != '\0')
            {
              if ( reserve > 8)
                elems_.reserve(reserve);

              do {
                unsigned int op = *p++;
                unsigned char fc = op & 0xFF;
                if ((op >> 7) & 1) {
                    unsigned char nextpoint = 1;

                    /* we assume iconv produced valid UTF-8 here */
                    while ( ((fc >> (7 - nextpoint)) & 1) && nextpoint++ < 4 )
                    {
                        op <<= 8;
                        op |= (*p++ & 0xFF);
                    }
                }
                elems_.push_back(op);
              }while( *p != '\0' );
            }

            if ( endchar )
                elems_.push_back(0);

            return elems_;
        }

        static utf8_string from_stdstring(const std::string &ins, bool endchar = false)
        {
            return from_stdstring(ins.c_str(), endchar, ins.size());
        }

    private:
        utf8_string elems;
};

#endif
