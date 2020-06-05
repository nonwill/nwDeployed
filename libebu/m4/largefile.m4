dnl *
dnl * Copyright (c) 2009  Motoyuki Kasahara
dnl *
dnl * Redistribution and use in source and binary forms, with or without
dnl * modification, are permitted provided that the following conditions
dnl * are met:
dnl * 1. Redistributions of source code must retain the above copyright
dnl *    notice, this list of conditions and the following disclaimer.
dnl * 2. Redistributions in binary form must reproduce the above copyright
dnl *    notice, this list of conditions and the following disclaimer in the
dnl *    documentation and/or other materials provided with the distribution.
dnl * 3. Neither the name of the project nor the names of its contributors
dnl *    may be used to endorse or promote products derived from this software
dnl *    without specific prior written permission.
dnl * 
dnl * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
dnl * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
dnl * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
dnl * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
dnl * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
dnl * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
dnl * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
dnl * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
dnl * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
dnl * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
dnl * SUCH DAMAGE.
dnl *

dnl * 
dnl * Check for large file support.
dnl * 
AC_DEFUN([eb_SYS_LARGEFILE], [dnl
AC_REQUIRE([AC_SYS_LARGEFILE])
AC_CACHE_CHECK([for ll modifier of printf], ac_cv_func_printf_ll,
[AC_RUN_IFELSE([AC_LANG_SOURCE([
    #include <stdio.h>
    #include <string.h>
    #include <limits.h>
    int main() {
	char buffer[[128]];
	sprintf(buffer, "%llx", (unsigned long long) 1 << 32);
	return (strcmp(buffer, "100000000") == 0) ? 0 : 1;
    }
])], [ac_cv_func_printf_ll=yes], [ac_cv_func_printf_ll=no])])
AC_CACHE_CHECK([for I64 modifier of printf], ac_cv_func_printf_i64,
[AC_RUN_IFELSE([AC_LANG_SOURCE([
    #include <stdio.h>
    #include <string.h>
    #include <limits.h>
    int main() {
	char buffer[[128]];
	sprintf(buffer, "%I64x", (unsigned __int64) 1 << 32);
	return (strcmp(buffer, "100000000") == 0) ? 0 : 1;
    }
])], [ac_cv_func_printf_i64=yes], [ac_cv_func_printf_i64=no])])
if test "$ac_cv_func_printf_ll" = yes; then
    AC_DEFINE(PRINTF_LL_MODIFIER, 1,
[Define to `1' if printf() recognizes "ll" modifier for long long])
fi
if test "$ac_cv_func_printf_i64" = yes; then
    AC_DEFINE(PRINTF_I64_MODIFIER, 1,
[Define to `1' if printf() recognizes "I64" modifier for __int64])
fi])
