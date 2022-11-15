#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
	size_t len = 0;
  while(*s++ != '\0')
    len++;
  return len;
}

char *strcpy(char *dst, const char *src) {
	size_t i;
	for (i = 0; src[i] != '\0'; i++)
		dst[i] = src[i];
	dst[i] = '\0';
	return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
	size_t i;
	for (i = 0; i < n && (src[i] != '\0'); i++)
		dst[i] = src[i];
	for (; i < n; i++)
		dst[i] = '\0';
	return dst;
}

char *strcat(char *dst, const char *src) {
	size_t pos_dst = strlen(dst);
  	size_t pos_src = 0;
  	for(; src[pos_src] != '\0'; pos_src++, pos_dst++)
    	dst[pos_dst] = src[pos_src];
  	dst[pos_dst] = '\0';
  	return dst;
}

int strcmp(const char *s1, const char *s2) {
	int ret = 0;
	while ( *s2 && !(ret = *s1 - *s2) ) //相等且没有结束
		++s1, ++s2;
	return (ret);
  // size_t i = 0;
  // for(; s1[i] != '\0' && s2[i] != '\0'; ++i){
  //   if(s1[i] > s2[i]) return 1;
  //   else if(s1[i] == s2[i]) continue;
  //   else return -1;
  // }
  // if(s1[i] == '\0' && s2[i] == '\0') return 0;
  // else if(s1[i] == '\0') return -1;
  // else return 1;
}

int strncmp(const char *s1, const char *s2, size_t n) {
	int ret = 0;
	while ( *s2 && n-- && !(ret = *s1 - *s2)) //相等且没有结束
		++s1, ++s2;
	return (ret);
}

void *memset(void *s, int c, size_t n) {
	for(size_t i = 0; i < n; ++i){
    	*((char*)s + i) = (char)c;
	}
	return s;
}

void *memmove(void *dst, const void *src, size_t n) {
	char* p_out = (char*)dst;
	char* p_in = (char*)src;
	if(src >= dst)
	{
		while(n --) *p_out++ = *p_in++;
	}
	else//低地址向高地址拷贝 
	{
		p_out += n; p_in += n;
		while(n --) *p_out-- = *p_in--;
	}
	return dst;
}

int memcmp(const void *s1, const void *s2, size_t n) {
	int ret = 0;
	char *p_s1 = (char*)s1;
	char *p_s2 = (char*)s2;
	while ( n-- && !(ret = *p_s1 - *p_s2) ) //相等且没有结束
		++p_s1, ++p_s2;
	return (ret);
}

void *memcpy(void *out, const void *in, size_t n) {
	char* p_out = (char*)out;
	char* p_in = (char*)in;
	while(n --) *p_out++ = *p_in++;
	return out;
}



#endif
