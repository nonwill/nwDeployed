#ifndef BREAK_AFTER_PUNCTUATIONS_HH
#define BREAK_AFTER_PUNCTUATIONS_HH

#if 1 /* HTMLControl.cpp:667 */
inline int utf8char_codec(unsigned int utf8char) {
    unsigned int ichar = utf8char & 0xff;
    while( (utf8char >>= 8) )
    {
        ichar <<= 8;
        ichar |= ( utf8char & 0xff );
    }
    return ichar;
}
#else
#define utf8char_codec(x) x
#endif

inline bool break_after_punctuations(int utf8char)
{
    switch (utf8char)
    {
    /* begin chinese punctuation */
    case 0xE280A6:
    case 0xE280BA:
    case 0xE38081:
    case 0xE38082:
    case 0xE38089:
    case 0xE3808B:
    case 0xE3808D:
    case 0xE3808F:
    case 0xE38091:
    case 0xE38095:
    case 0xE38097:
    case 0xEFB990:
    case 0xEFB994:
    case 0xEFB996:
    case 0xEFB99A:
    case 0xEFB99C:
    case 0xEFB99E:
    case 0xEFBC81:
    case 0xEFBC89:
    case 0xEFBC8C:
    case 0xEFBC9A:
    case 0xEFBC9B:
    case 0xEFBC9E:
    case 0xEFBC9F:
    case 0xEFBCBD:
    case 0xEFBD9D:
    /* end chinese punctuation */
        return true;
    default:
        break;
    }

    return false;
}

#endif
