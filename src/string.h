//
// string.h
//
// String routines
//
// Copyright (C) 2002 Michael Ringgaard. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 
// 1. Redistributions of source code must retain the above copyright 
//    notice, this list of conditions and the following disclaimer.  
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.  
// 3. Neither the name of the project nor the names of its contributors
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission. 
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
// SUCH DAMAGE.
// 

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef STRING_H
#define STRING_H

#include <sys/types.h>

#ifdef  __cplusplus
extern "C" {
#endif

// Split a string by a delimiter.
//
// This function writes the beginning of each item to @pointers_out
// (forming an array of C strings).Both buffers are assumed to be big enough for all of the
// strings.
//
// Returns the number of strings written to @pointers_out.
//

int explode(char **pointers_out, const char *delim, const char *str, char *tmp);
char *strncpy(char *dest, const char *source, size_t n);
int strncmp(const char *s1, const char *s2, size_t n);
int stricmp(const char *s1, const char *s2);
int strnicmp(const char *s1, const char *s2, size_t n);
char *strchr(const char *s, int ch);
char *strrchr(const char *s, int ch);
char *strstr(const char *s1, const char *s2);
size_t strspn(const char *string, const char *control);
size_t strcspn(const char *string, const char *control);
char *strpbrk(const char *string, const char *control);
char *stpcpy(char *dst, const char *src);

int strcasecmp(const char *s1, const char *s2);
int strncasecmp(const char *s1, const char *s2, size_t n);

int strcoll(const char *s1, const char *s2);



unsigned char *strlwr(unsigned char *s);
unsigned char *strupr(unsigned char *s);

char *strncat(char *s1, const char *s2, size_t n);
char *strnset(char *s, int c, size_t n);
char *strset(char *s, int c);
char *strrev(char *s);
char *strtok_r(char *string, const char *control, char **lasts);
char *strsep(char **stringp, const char *delim);

void *memmove(void *dst, const void *src, size_t n);
void *memchr(const void *buf, int ch, size_t n);

void *memccpy(void *dst, const void *src, int c, size_t n);
int memicmp(const void *buf1, const void *buf2, size_t n);

// Intrinsic functions

void *memset(void *, int, size_t);

char *strcpy(char *, const char *);
char *strcat(char *, const char *);
int strcmp(const char *, const char *);
size_t strlen(const char *);

#ifdef  __cplusplus
}
#endif

#endif

