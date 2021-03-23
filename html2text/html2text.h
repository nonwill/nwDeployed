#ifndef HTML_TO_TEXT_EXP_H
#define HTML_TO_TEXT_EXP_H

#include <fstream>
#include <string>

#ifndef HTML2TEXT_EXE
#ifdef WIN32
#   if defined(HTML2TEXT_LIBS)
#       define HTML2TEXT_API __declspec(dllexport)
#   else
#       define HTML2TEXT_API __declspec(dllimport)
#   endif
#endif
#else
#   define HTML2TEXT_API
#endif

class HTML2TEXT_API h2t_iostream {
    public:
        virtual ~h2t_iostream() {}

        virtual int get() = 0;
        virtual int write(const char *inp, size_t len) = 0;

        h2t_iostream &operator<<(const char *inp);
        h2t_iostream &operator<<(const std::string &inp);
        h2t_iostream &operator<<(char inp);
};

extern HTML2TEXT_API bool html_to_text(h2t_iostream &h2tio, int width = 79);

#endif
