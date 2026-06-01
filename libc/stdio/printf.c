#include <limits.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static bool print(const char* data, size_t length) {
	const unsigned char* bytes = (const unsigned char*) data;
	for (size_t i = 0; i < length; i++)
		if (putchar(bytes[i]) == EOF)
			return false;
	return true;
}

int printf(const char* restrict format, ...) {
	va_list parameters;
	va_start(parameters, format);

	int written = 0;

	while (*format != '\0') {
		size_t maxrem = INT_MAX - written;

		if (format[0] != '%' || format[1] == '%') {
			if (format[0] == '%')
				format++;
			size_t amount = 1;
			while (format[amount] && format[amount] != '%')
				amount++;
			if (maxrem < amount) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
			if (!print(format, amount))
				return -1;
			format += amount;
			written += amount;
			continue;
		}

		const char* format_begun_at = format++;

		if (*format == 'c') {
			format++;
			char c = (char) va_arg(parameters, int /* char promotes to int */);
			if (!maxrem) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
			if (!print(&c, sizeof(c)))
				return -1;
			written++;
		} else if (*format == 's') {
			format++;
			const char* str = va_arg(parameters, const char*);
			size_t len = strlen(str);
			if (maxrem < len) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
			if (!print(str, len))
				return -1;
			written += len;
		} else if (*format == 'x' || *format == 'X') {
			char spec = *format++;
			unsigned long long val = va_arg(parameters, unsigned long long);
			char buf[17];
			int i = 0;
			if (val == 0) {
				buf[i++] = '0';
			} else {
				while (val != 0) {
					int digit = val % 16;
					buf[i++] = digit < 10 ? '0' + digit
					                      : (spec == 'X' ? 'A' : 'a') + (digit - 10);
					val /= 16;
				}
				for (int l = 0, r = i - 1; l < r; l++, r--) {
					char tmp = buf[l]; buf[l] = buf[r]; buf[r] = tmp;
				}
			}
			if (!print(buf, i))
				return -1;
			written += i;
		} else if (*format == 'u') {
			format++;
			unsigned long long val = va_arg(parameters, unsigned long long);
			char buf[21];
			int i = 0;
			if (val == 0) {
				buf[i++] = '0';
			} else {
				while (val != 0) {
					buf[i++] = '0' + (val % 10);
					val /= 10;
				}
				for (int l = 0, r = i - 1; l < r; l++, r--) {
					char tmp = buf[l]; buf[l] = buf[r]; buf[r] = tmp;
				}
			}
			if (!print(buf, i))
				return -1;
			written += i;
		} else if (*format == 'd' || *format == 'i') {
			format++;
			long long val = va_arg(parameters, long long);
			char buf[21];
			int i = 0;
			if (val == 0) {
				buf[i++] = '0';
			} else {
				int neg = val < 0;
				unsigned long long uval = neg ? (unsigned long long)(-val) : (unsigned long long)val;
				while (uval != 0) {
					buf[i++] = '0' + (uval % 10);
					uval /= 10;
				}
				if (neg) buf[i++] = '-';
				for (int l = 0, r = i - 1; l < r; l++, r--) {
					char tmp = buf[l]; buf[l] = buf[r]; buf[r] = tmp;
				}
			}
			if (!print(buf, i))
				return -1;
			written += i;
		} else {
			format = format_begun_at;
			size_t len = strlen(format);
			if (maxrem < len) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
			if (!print(format, len))
				return -1;
			written += len;
			format += len;
		}
	}

	va_end(parameters);
	return written;
}
