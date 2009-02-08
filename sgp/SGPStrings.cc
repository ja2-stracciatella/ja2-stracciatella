#include "SGPStrings.h"

#if defined(__linux__) || defined(_WIN32)

#include <sys/types.h>
/*	$OpenBSD: wcslcpy.c,v 1.4 2006/05/05 15:27:38 millert Exp $	*/
/*	$NetBSD: wcslcpy.c,v 1.2 2001/01/03 14:33:02 lukem Exp $	*/
/*	from OpenBSD: strlcpy.c,v 1.4 1999/05/01 18:56:41 millert Exp 	*/

/*
 * Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns wcslen(src); if retval >= siz, truncation occurred.
 */
size_t
wcslcpy(wchar_t *dst, const wchar_t *src, size_t siz)
{
	wchar_t *d = dst;
	const wchar_t *s = src;
	size_t n = siz;

	/* Copy as many bytes as will fit */
	if (n != 0) {
		while (--n != 0) {
			if ((*d++ = *s++) == '\0')
				break;
		}
	}

	/* Not enough room in dst, add NUL and traverse rest of src */
	if (n == 0) {
		if (siz != 0)
			*d = '\0';		/* NUL-terminate dst */
		while (*s++)
			;
	}

	return(s - src - 1);	/* count does not include NUL */
}

/*	$OpenBSD: strlcpy.c,v 1.11 2006/05/05 15:27:38 millert Exp $	*/

/*
 * Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
size_t
strlcpy(char *dst, const char *src, size_t siz)
{
	char *d = dst;
	const char *s = src;
	size_t n = siz;

	/* Copy as many bytes as will fit */
	if (n != 0) {
		while (--n != 0) {
			if ((*d++ = *s++) == '\0')
				break;
		}
	}

	/* Not enough room in dst, add NUL and traverse rest of src */
	if (n == 0) {
		if (siz != 0)
			*d = '\0';		/* NUL-terminate dst */
		while (*s++)
			;
	}

	return(s - src - 1);	/* count does not include NUL */
}

#endif


#ifdef _WIN32

void WINsnprintf(char* s, size_t n, const char* format, ...)
{
	va_list arg;
	va_start(arg, format);
	_vsnprintf(s, n, format, arg);
	va_end(arg);
	if (n != 0) s[n - 1] = '\0'; // _snprintf() does not guarantee NUL termination
}


void WINswprintf(wchar_t* s, size_t n, const wchar_t* format, ...)
{
	va_list arg;
	va_start(arg, format);
	WINvswprintf(s, n, format, arg);
	va_end(arg);
}


void WINvswprintf(wchar_t* s, size_t n, const wchar_t* format, va_list arg)
{
	_vsnwprintf(s, n, format, arg);
	if (n != 0) s[n - 1] = L'\0'; // _vsnwprintf() does not guarantee NUL termination
}

#endif


void ReplacePath(char* const buf, size_t const size, char const* path, char const* const filename, char const* const ext)
{
	char const* base    = filename;
	char const* old_ext = 0;
	for (char const* i = filename;; ++i)
	{
		switch (*i)
		{
			case '.':
				old_ext = i;
				break;

			case '/':
				base    = i + 1;
				old_ext = 0;
				break;

			case '\0':
				if (!path)
				{
					base = filename;
					path = "";
				}
				int const n = (old_ext ? old_ext : i) - base;
				snprintf(buf, size, "%s%.*s%s", path, n, base, ext);
				return;
		}
	}
}
