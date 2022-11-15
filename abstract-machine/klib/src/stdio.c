#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
#define OSTRING_LEN 512

void get_parse_out_d(char *p_out, int d){
	int len = 0, tmp[20];
	if (d == 0)
		*p_out++ = '0';
	
	else if (d > 0){
		len = 0;
		while (d){
			tmp[len++] = d % 10 + '0';
			d /= 10;
		}
		while (len--)
			*p_out++ = tmp[len];
	}
	else{
		*p_out++ = '-';
		d = -d;
		len = 0;
		while (d){
			tmp[len++] = d % 10 + '0';
			d /= 10;
		}
		while (len--)
			*p_out++ = tmp[len];
	}
}

void get_parse_out_x(char *p_out, unsigned int d){
	unsigned int len = 0, tmp[20];//栈-高低位
	if (d == 0)
		*p_out++ = '0';
	else{
		len = 0;
		while (d){
			tmp[len++] = d % 16;
			d /= 16;
		}
		while (len--){
			if (tmp[len] < 10)
				*p_out++ = tmp[len] + '0';
			else
				*p_out++ = tmp[len] + 'a' - 10;
    	}
	}
}

void get_parse_out_p(char *p_out, unsigned int *pd){
	int len = 0, tmp[20];
	unsigned int d = (unsigned int)pd;
	*p_out++ = '0';
	*p_out++ = 'x';
	if (d == 0)
		*p_out++ = '0';
	else{
		len = 0;
		while (d){
			tmp[len++] = d % 16;
			d /= 16;
		}
		while (len--){
			if (tmp[len] < 10)
				*p_out++ = tmp[len] + '0';
			else
				*p_out++ = tmp[len] + 'a' - 10;
    	}
	}
}

int printf(const char *fmt, ...) {
  char out[OSTRING_LEN];
  va_list ap;
  va_start(ap, fmt);
  int num = vsprintf(out, fmt, ap);
  va_end(ap);

  for (int i = 0; i < num; ++i)
    putch(out[i]);

  return num;
}

int vsprintf(char *out, const char *fmt, va_list ap) {//传参中参数类型和个数不确定->vsprintf
	char *pout = out;
	size_t i = 0;
	while(fmt[i] != '\0'){
		if(fmt[i] != '%'){
			*(pout++) = fmt[i];//优先级
		}
		else{
			bool is_end = false;
			char num_leftpad_sym = ' ';//填充
			bool left_align = false;
			int width = 0;
			while(!is_end){
				switch(fmt[++i]){
					case '0':{
						if(fmt[i-1]=='%'){
							num_leftpad_sym = '0';
							break;
						}
					}
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':{
						width = width * 10 + fmt[i] - '0';
						break;
					}
					case '-':{
						if(fmt[i-1] == '%'){
							left_align = true;
						}
						break;
					}
					case 's':{
						const char *str = va_arg(ap, char *);
						if (!left_align){
							for (int j = 0; j < width - (int)strlen(str); ++j)
								*(pout++) = ' ';
						
							strcpy(pout, str);
							pout += strlen(str);
						}
						else{
							strcpy(pout, str);
							pout += strlen(str);
							for (int j = 0; j < width - (int)strlen(str); ++j)
								*(pout++) = ' ';
						}
						is_end = true;
						break;
					}
					case 'd':
					case 'i':{
						int d = va_arg(ap, int);
						char buff[20];
						get_parse_out_d(buff, d);
						if (!left_align){
							for (int j = 0; j < width - (int)strlen(buff); ++j)
							*(pout++) = num_leftpad_sym;
							strcpy(pout, buff);
							pout += strlen(buff);
						}
						else{
							strcpy(pout, buff);
							pout += strlen(buff);
							for (int j = 0; j < width - (int)strlen(buff); ++j)
							*(pout++) = num_leftpad_sym;
						}
						is_end = true;
						break;
					}
					case 'x':{
						unsigned int d = va_arg(ap, unsigned int);
						char buff[20];
						get_parse_out_x(buff, d);
						if (!left_align){
							for (int j = 0; j < width - (int)strlen(buff); ++j)
							*(pout++) = num_leftpad_sym;
							strcpy(pout, buff);
							pout += strlen(buff);
						}
						else{
							strcpy(pout, buff);
							pout += strlen(buff);
							for (int j = 0; j < width - (int)strlen(buff); ++j)
							*(pout++) = num_leftpad_sym;
						}
						is_end = true;
						break;
					}
					case 'p':{
						unsigned int *pd = va_arg(ap, unsigned int*);
						char buff[20];
						get_parse_out_p(buff, pd);
						if (!left_align){
							for (int j = 0; j < width - (int)strlen(buff); ++j)
							*(pout++) = num_leftpad_sym;
							strcpy(pout, buff);
							pout += strlen(buff);
						}
						else{
							strcpy(pout, buff);
							pout += strlen(buff);
							for (int j = 0; j < width - (int)strlen(buff); ++j)
							*(pout++) = num_leftpad_sym;
						}
						is_end = true;
						break;
					}
					case 'c':{
						char c = (char)va_arg(ap, int);
						*pout++ = c;
						break;
					}
					default:{
          				printf("print format is wrong!\n");
          				assert(0);
        			}
				}
			}
		}
		i++;
	}
	*pout = '\0';
	va_end(ap);
	return strlen(out);
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int num = vsprintf(out, fmt, ap);
  va_end(ap);
  return num;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}


#endif
