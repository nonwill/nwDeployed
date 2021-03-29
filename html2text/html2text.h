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

/* streaming method for input and output UTF-8 encoded data.
 * if other codec is needed, just derive from h2t_iostream, and:
 * override get() to encode the input's codec to UTF-8,
 * overide write() to decode UTF-8 to the output's codec.
 * overide useBackspaces() to Render boldface and underlining or not.
*/
class HTML2TEXT_API h2t_iostream {
public:
    virtual ~h2t_iostream() {}

    /* Render boldface and underlining (using backspaces) or not,
     * default not. -nobs by html2text exec to identify this.
    */
    virtual bool useBackspaces() { return false; }

protected:
    /* get one char: return EOF at end, otherwise one char(<0xff) */
    virtual int getc() = 0;
    /* write output[UTF-8]: return EOF for overflow, otherwise size of really written */
    virtual size_t write(const char *inp, size_t len) = 0;

public:
    /* really using write to output the transformed content, called by the engine */
    h2t_iostream &operator<<(const char *inp);
    h2t_iostream &operator<<(const std::string &inp);
    h2t_iostream &operator<<(char inp);

    /* write one UTF-8 codec wchar data to output the transformed content */
    h2t_iostream &operator<<(unsigned int inp);
    /* get one UTF-8 codec wchar data, so load bytes as required */
    h2t_iostream &operator>>(unsigned int &op);
};

#ifndef HTML2TEXT_EXE

/* h2t_iostream, for input and output streams.
 * enable_links, Generate reference list with link targets.
 * width, Optimize for screen widths other than 79.
 * always used PRINT_AS_ASCII - ASCII output
*/
extern HTML2TEXT_API bool html_to_text(h2t_iostream &h2tio, int width = 79, bool enable_links = false);

#endif

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

    int getc()
    {
        return (inbuffer_pos < inbuffer_len) ? (inbuffer[inbuffer_pos++] & 0xFF) : EOF;
    }

    size_t write(const char *inp, size_t len)
    {
        if ( outbuffer_len >= outbuffer_cap )
            return EOF;

        if( !len )
            return len;

        size_t buffer_cap = outbuffer_cap - outbuffer_len;
        if(buffer_cap < len)
            len = buffer_cap;

        memcpy(outbuffer + outbuffer_len, inp, len);
        outbuffer_len += len;

        return len;
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

    int getc()
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

    int getc()
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
