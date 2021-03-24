#ifndef HTML_TO_TEXT_EXP_H
#define HTML_TO_TEXT_EXP_H

#include <string>

#if !defined(HTML2TEXT_EXE) && defined(WIN32)
#   if defined(HTML2TEXT_LIBS)
#       define HTML2TEXT_API __declspec(dllexport)
#   else
#       define HTML2TEXT_API __declspec(dllimport)
#   endif
#else
#   define HTML2TEXT_API
#endif

class HTML2TEXT_API h2t_iostream {
    public:
        virtual ~h2t_iostream() {}

        virtual bool useBackspaces() { return false; }
        virtual int get() = 0;
        virtual size_t write(const char *inp, size_t len) = 0;

        h2t_iostream &operator<<(const char *inp);
        h2t_iostream &operator<<(const std::string &inp);
        h2t_iostream &operator<<(char inp);
};

extern HTML2TEXT_API bool html_to_text(h2t_iostream &h2tio, int width = 79);

#if !defined(HTML2TEXT_EXE) && !defined(HTML2TEXT_LIBS)

class h2t_utf8buffer : public h2t_iostream {
public:
    h2t_utf8buffer(const char *in, size_t inlen, char *out, size_t &outlen)
        : inbuffer(in), inbuffer_len(inlen), inbuffer_pos(0),
          outbuffer(out), outbuffer_len(outlen), outbuffer_cap(outlen)
    {
        outbuffer_len = 0;
    }
    virtual ~h2t_utf8buffer() {}

    int get()
    {
        return (inbuffer_pos < inbuffer_len) ? (inbuffer[inbuffer_pos++] & 0xFF) : EOF;
    }

    size_t write(const char *inp, size_t len)
    {
        int ret = EOF;
        if ( outbuffer_len < outbuffer_cap )
        {
            size_t buffer_cap = outbuffer_cap - outbuffer_len;
            if(buffer_cap > len)
                ret = len;
            else
                ret = buffer_cap;

            memcpy(outbuffer + outbuffer_len, inp, ret);
            outbuffer_len += ret;
        }

        return ret;
    }

private:
    const char *inbuffer;
    const size_t inbuffer_len;
    size_t inbuffer_pos;

    char *const outbuffer;
    size_t &outbuffer_len;
    const size_t outbuffer_cap;
};


class h2t_utf8string : public h2t_iostream {
public:
    h2t_utf8string(const std::string &in_str, std::string &out_str) :
        in_string(in_str),
        out_string(out_str),
        inbuffer_pos(0)
    {
        out_str.reserve(in_str.size());
    }
    virtual ~h2t_utf8string() {}

    int get()
    {
        return (inbuffer_pos < in_string.size()) ? (in_string[inbuffer_pos++] & 0xFF) : EOF;
    }

    size_t write(const char *inp, size_t len)
    {
        out_string.append(inp, len);
        return len;
    }

private:
    const std::string &in_string;
    std::string &out_string;

    size_t inbuffer_pos;
};


class h2t_utf8file : public h2t_iostream
{
public:
    h2t_utf8file(const char *filename, std::string &out_str) :
        in_file(::fopen(filename, "r")),
        out_string(out_str)
    {
        if ( in_file )
        {
            fseek( in_file, 0L, SEEK_END );
            out_str.reserve( ftell( in_file ) );
            fseek( in_file, 0L, SEEK_SET );
        }
    }
    virtual ~h2t_utf8file()
    {
        if( in_file )
            fclose(in_file);
    }

    int get()
    {
        return ::feof( in_file ) ? EOF : ::fgetc( in_file );
    }

    size_t write(const char *inp, size_t len)
    {
        out_string.append(inp, len);
        return len;
    }

private:
    FILE *in_file;
    std::string &out_string;
};

#endif

#endif
