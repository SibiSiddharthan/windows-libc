/*
   Copyright (c) 2024 - 2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <stdio.h>
#include <stdint.h>

int test_simple(void)
{
	int status = 0;
	int result = 0;

	uint8_t c1 = 0, c2 = 0;
	int n = 0;

	result = sscan("", 0, "");
	status += CHECK_RESULT(result, 0);

	result = sscan("", 0, "%c", &c1);
	status += CHECK_RESULT(result, 0);

	result = sscan("a", 1, "%c", &c1);
	status += CHECK_UVALUE(c1, 'a');
	status += CHECK_RESULT(result, 1);

	result = sscan("bc", 2, "%c%c", &c1, &c2);
	status += CHECK_UVALUE(c1, 'b');
	status += CHECK_UVALUE(c2, 'c');
	status += CHECK_RESULT(result, 2);

	result = sscan("de", 2, "%c  %c", &c1, &c2);
	status += CHECK_UVALUE(c1, 'd');
	status += CHECK_UVALUE(c2, 'e');
	status += CHECK_RESULT(result, 2);

	result = sscan("h  i", 4, "%c %c", &c1, &c2);
	status += CHECK_UVALUE(c1, 'h');
	status += CHECK_UVALUE(c2, 'i');
	status += CHECK_RESULT(result, 2);

	result = sscan("jh   gk", 7, "%ch g%c", &c1, &c2);
	status += CHECK_UVALUE(c1, 'j');
	status += CHECK_UVALUE(c2, 'k');
	status += CHECK_RESULT(result, 2);

	result = sscan("lh   gl", 7, "%ch l%c", &c1, &c2);
	status += CHECK_UVALUE(c1, 'l');
	status += CHECK_RESULT(result, 1);

	result = sscan("jh   gk", 7, "%ch g%c%n", &c1, &c2, &n);
	status += CHECK_UVALUE(c1, 'j');
	status += CHECK_UVALUE(c2, 'k');
	status += CHECK_UVALUE(n, 7);
	status += CHECK_RESULT(result, 2);

	result = sscan("jh   gk", 7, "%ch g%c  %n", &c1, &c2, &n);
	status += CHECK_UVALUE(c1, 'j');
	status += CHECK_UVALUE(c2, 'k');
	status += CHECK_UVALUE(n, 7);
	status += CHECK_RESULT(result, 2);

	result = sscan("jh   gk    ", 11, "%ch g%c%n", &c1, &c2, &n);
	status += CHECK_UVALUE(c1, 'j');
	status += CHECK_UVALUE(c2, 'k');
	status += CHECK_UVALUE(n, 7);
	status += CHECK_RESULT(result, 2);

	result = sscan("jh   gk    ", 11, "%ch g%c  %n", &c1, &c2, &n);
	status += CHECK_UVALUE(c1, 'j');
	status += CHECK_UVALUE(c2, 'k');
	status += CHECK_UVALUE(n, 11);
	status += CHECK_RESULT(result, 2);

	return status;
}

int test_int(void)
{
	int status = 0;
	int result = 0;

	int i = 0, j = 0;
	int n = 0;

	result = sscan("123", 3, "%d%n", &i, &n);
	status += CHECK_IVALUE(i, 123);
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscan("-123", 4, "%d%n", &i, &n);
	status += CHECK_IVALUE(i, -123);
	status += CHECK_UVALUE(n, 4);
	status += CHECK_RESULT(result, 1);

	result = sscan("-123", 4, "%1d%n", &i, &n);
	status += CHECK_IVALUE(i, 0);
	status += CHECK_UVALUE(n, 1);
	status += CHECK_RESULT(result, 1);

	result = sscan("000000000000123", 15, "%d%n", &i, &n);
	status += CHECK_IVALUE(i, 123);
	status += CHECK_UVALUE(n, 15);
	status += CHECK_RESULT(result, 1);

	result = sscan("000000000000123", 15, "%5d%n", &i, &n);
	status += CHECK_IVALUE(i, 0);
	status += CHECK_UVALUE(n, 5);
	status += CHECK_RESULT(result, 1);

	result = sscan("123", 3, "%2d %d%n", &i, &j, &n);
	status += CHECK_IVALUE(i, 12);
	status += CHECK_IVALUE(j, 3);
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 2);

	result = sscan("000000000000123", 15, "%5d%2d%n", &i, &j, &n);
	status += CHECK_IVALUE(i, 0);
	status += CHECK_IVALUE(j, 0);
	status += CHECK_UVALUE(n, 7);
	status += CHECK_RESULT(result, 2);

	result = sscan("123-56", 6, "%d %d%n", &i, &j, &n);
	status += CHECK_IVALUE(i, 123);
	status += CHECK_IVALUE(j, -56);
	status += CHECK_UVALUE(n, 6);
	status += CHECK_RESULT(result, 2);

	result = sscan("456  -089", 9, "%d %d%n", &i, &j, &n);
	status += CHECK_IVALUE(i, 456);
	status += CHECK_IVALUE(j, -89);
	status += CHECK_UVALUE(n, 9);
	status += CHECK_RESULT(result, 2);

	result = sscan("123,456", 7, "%d%n", &i, &n);
	status += CHECK_IVALUE(i, 123);
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscan("123,4,567", 9, "%'d%'d%n", &i, &j, &n);
	status += CHECK_IVALUE(i, 123);
	status += CHECK_IVALUE(j, 0);
	status += CHECK_RESULT(result, 1);

	result = sscan("123,4,567", 9, "%'4d%'d%n", &i, &j, &n);
	status += CHECK_IVALUE(i, 123);
	status += CHECK_IVALUE(j, 4567);
	status += CHECK_UVALUE(n, 9);
	status += CHECK_RESULT(result, 2);

	result = sscan("123,456", 7, "%'d%n", &i, &n);
	status += CHECK_IVALUE(i, 123456);
	status += CHECK_UVALUE(n, 7);
	status += CHECK_RESULT(result, 1);

	result = sscan("654321", 6, "%'d%n", &i, &n);
	status += CHECK_IVALUE(i, 654321);
	status += CHECK_UVALUE(n, 6);
	status += CHECK_RESULT(result, 1);

	result = sscan("-123,456", 8, "%'d%n", &i, &n);
	status += CHECK_IVALUE(i, -123456);
	status += CHECK_UVALUE(n, 8);
	status += CHECK_RESULT(result, 1);

	result = sscan("-654321", 7, "%'d%n", &i, &n);
	status += CHECK_IVALUE(i, -654321);
	status += CHECK_UVALUE(n, 7);
	status += CHECK_RESULT(result, 1);

	result = sscan("123,4567,890", 12, "%'d%'d%n", &i, &j, &n);
	status += CHECK_IVALUE(i, 123456);
	status += CHECK_IVALUE(j, 7890);
	status += CHECK_UVALUE(n, 12);
	status += CHECK_RESULT(result, 2);

	result = sscan("12,34567,890", 12, "%'d%'d%n", &i, &j, &n);
	status += CHECK_IVALUE(i, 12345);
	status += CHECK_IVALUE(j, 67890);
	status += CHECK_UVALUE(n, 12);
	status += CHECK_RESULT(result, 2);

	result = sscan("-123,4567,890", 13, "%'d%'d%n", &i, &j, &n);
	status += CHECK_IVALUE(i, -123456);
	status += CHECK_IVALUE(j, 7890);
	status += CHECK_UVALUE(n, 13);
	status += CHECK_RESULT(result, 2);

	result = sscan("-12,34567,890", 13, "%'d%'d%n", &i, &j, &n);
	status += CHECK_IVALUE(i, -12345);
	status += CHECK_IVALUE(j, 67890);
	status += CHECK_UVALUE(n, 13);
	status += CHECK_RESULT(result, 2);

	return status;
}

int test_uint(void)
{
	int status = 0;
	int result = 0;

	int u = 0, v = 0, o = 0, b = 0, x = 0;
	int n = 0;

	result = sscan("123", 3, "%u%n", &u, &n);
	status += CHECK_IVALUE(u, 123);
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscan("000000000000123", 15, "%u%n", &u, &n);
	status += CHECK_IVALUE(u, 123);
	status += CHECK_UVALUE(n, 15);
	status += CHECK_RESULT(result, 1);

	result = sscan("-123", 4, "%u%n", &u, &n);
	status += CHECK_RESULT(result, 0);

	result = sscan("123", 3, "%2u%n", &u, &n);
	status += CHECK_IVALUE(u, 12);
	status += CHECK_UVALUE(n, 2);
	status += CHECK_RESULT(result, 1);

	result = sscan("123,456", 7, "%'5u%n", &u, &n);
	status += CHECK_IVALUE(u, 123);
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscan("123,456", 7, "%'u%n", &u, &n);
	status += CHECK_IVALUE(u, 123456);
	status += CHECK_UVALUE(n, 7);
	status += CHECK_RESULT(result, 1);

	result = sscan("654321", 6, "%'u%n", &u, &n);
	status += CHECK_IVALUE(u, 654321);
	status += CHECK_UVALUE(n, 6);
	status += CHECK_RESULT(result, 1);

	result = sscan("123,4567,890", 12, "%'u%'u%n", &u, &v, &n);
	status += CHECK_IVALUE(u, 123456);
	status += CHECK_IVALUE(v, 7890);
	status += CHECK_UVALUE(n, 12);
	status += CHECK_RESULT(result, 2);

	result = sscan("12,34567,890", 12, "%'u%'u%n", &u, &v, &n);
	status += CHECK_IVALUE(u, 12345);
	status += CHECK_IVALUE(v, 67890);
	status += CHECK_UVALUE(n, 12);
	status += CHECK_RESULT(result, 2);

	result = sscan("00010", 5, "%b%n", &b, &n);
	status += CHECK_IVALUE(b, 2);
	status += CHECK_UVALUE(n, 5);
	status += CHECK_RESULT(result, 1);

	result = sscan("1000010", 7, "%b%n", &b, &n);
	status += CHECK_IVALUE(b, 66);
	status += CHECK_UVALUE(n, 7);
	status += CHECK_RESULT(result, 1);

	result = sscan("1000010", 7, "%3b%n", &b, &n);
	status += CHECK_IVALUE(b, 4);
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscan("0b1000010", 9, "%b%n", &b, &n);
	status += CHECK_IVALUE(b, 66);
	status += CHECK_UVALUE(n, 9);
	status += CHECK_RESULT(result, 1);

	result = sscan("0b1000010", 9, "%4b%n", &b, &n);
	status += CHECK_IVALUE(b, 2);
	status += CHECK_UVALUE(n, 4);
	status += CHECK_RESULT(result, 1);

	result = sscan("123", 3, "%o%n", &o, &n);
	status += CHECK_IVALUE(o, 83);
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscan("000000000000123", 15, "%o%n", &o, &n);
	status += CHECK_IVALUE(o, 83);
	status += CHECK_UVALUE(n, 15);
	status += CHECK_RESULT(result, 1);

	result = sscan("0456", 4, "%o%n", &o, &n);
	status += CHECK_IVALUE(o, 302);
	status += CHECK_UVALUE(n, 4);
	status += CHECK_RESULT(result, 1);

	result = sscan("0o457", 5, "%o%n", &o, &n);
	status += CHECK_IVALUE(o, 303);
	status += CHECK_UVALUE(n, 5);
	status += CHECK_RESULT(result, 1);

	result = sscan("0O455", 5, "%o%n", &o, &n);
	status += CHECK_IVALUE(o, 301);
	status += CHECK_UVALUE(n, 5);
	status += CHECK_RESULT(result, 1);

	result = sscan("0O458", 5, "%2o%n", &o, &n);
	status += CHECK_IVALUE(o, 0);
	status += CHECK_UVALUE(n, 2);
	status += CHECK_RESULT(result, 1);

	result = sscan("-123", 4, "%o%n", &o, &n);
	status += CHECK_RESULT(result, 0);

	result = sscan("789", 3, "%2o%n", &o, &n);
	status += CHECK_IVALUE(o, 7);
	status += CHECK_UVALUE(n, 1);
	status += CHECK_RESULT(result, 1);

	result = sscan("123", 3, "%x%n", &x, &n);
	status += CHECK_IVALUE(x, 291);
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscan("000000000000123", 15, "%x%n", &x, &n);
	status += CHECK_IVALUE(x, 291);
	status += CHECK_UVALUE(n, 15);
	status += CHECK_RESULT(result, 1);

	result = sscan("0x45Bf", 6, "%x%n", &x, &n);
	status += CHECK_IVALUE(x, 17855);
	status += CHECK_UVALUE(n, 6);
	status += CHECK_RESULT(result, 1);

	result = sscan("0x45Bf", 6, "%3x%n", &x, &n);
	status += CHECK_IVALUE(x, 4);
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscan("0X45cf", 6, "%8x%n", &x, &n);
	status += CHECK_IVALUE(x, 17871);
	status += CHECK_UVALUE(n, 6);
	status += CHECK_RESULT(result, 1);

	result = sscan("0X45df", 6, "%8X%n", &x, &n);
	status += CHECK_IVALUE(x, 17887);
	status += CHECK_UVALUE(n, 6);
	status += CHECK_RESULT(result, 1);

	return status;
}

int test_float(void)
{
	int status = 0;
	int result = 0;

	float f32 = 0;
	double f64 = 0;

	int n = 0;

	result = sscan("1.00", 3, "%f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, 1.00);
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscan("-1.00", 4, "%f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, -1.00);
	status += CHECK_UVALUE(n, 4);
	status += CHECK_RESULT(result, 1);

	result = sscan("1.5788987434987", 15, "%f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, 1.5788987434987);
	status += CHECK_UVALUE(n, 15);
	status += CHECK_RESULT(result, 1);

	result = sscan("13786492.5788987434987", 22, "%f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, 13786492.5788987434987);
	status += CHECK_UVALUE(n, 22);
	status += CHECK_RESULT(result, 1);

	result = sscan("-13786492.5788987434987", 23, "%f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, -13786492.5788987434987);
	status += CHECK_UVALUE(n, 23);
	status += CHECK_RESULT(result, 1);

	result = sscan("0.000000000000000000000000000000000001", 38, "%f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, 0.000000000000000000000000000000000001);
	status += CHECK_UVALUE(n, 38);
	status += CHECK_RESULT(result, 1);

	result = sscan("100000000000000000000000000000000000.0", 38, "%f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, 100000000000000000000000000000000000.0);
	status += CHECK_UVALUE(n, 38);
	status += CHECK_RESULT(result, 1);

	result = sscan("-0.000000000000000000000000000000000001", 39, "%f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, -0.000000000000000000000000000000000001);
	status += CHECK_UVALUE(n, 39);
	status += CHECK_RESULT(result, 1);

	result = sscan("-100000000000000000000000000000000000.0", 39, "%f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, -100000000000000000000000000000000000.0);
	status += CHECK_UVALUE(n, 39);
	status += CHECK_RESULT(result, 1);

	result = sscan("1.647e+28", 9, "%f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, 1.647e+28);
	status += CHECK_UVALUE(n, 9);
	status += CHECK_RESULT(result, 1);

	result = sscan("1.647e-28", 9, "%f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, 1.647e-28);
	status += CHECK_UVALUE(n, 9);
	status += CHECK_RESULT(result, 1);

	result = sscan("-1.647E+28", 10, "%f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, -1.647e+28);
	status += CHECK_UVALUE(n, 10);
	status += CHECK_RESULT(result, 1);

	result = sscan("-1.647E-28", 10, "%f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, -1.647e-28);
	status += CHECK_UVALUE(n, 10);
	status += CHECK_RESULT(result, 1);

	result = sscan("123.647e+20", 11, "%f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, 123.647e+20);
	status += CHECK_UVALUE(n, 11);
	status += CHECK_RESULT(result, 1);

	result = sscan("3321.647e+15", 12, "%'f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, 3321.647e+15);
	status += CHECK_UVALUE(n, 12);
	status += CHECK_RESULT(result, 1);

	result = sscan("1,234.647e20", 12, "%'f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, 1234.647e+20);
	status += CHECK_UVALUE(n, 12);
	status += CHECK_RESULT(result, 1);

	result = sscan("1111,234.647e20", 12, "%'f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, 1111.0);
	status += CHECK_UVALUE(n, 4);
	status += CHECK_RESULT(result, 1);

	result = sscan("1,234.647,898,7e+20", 19, "%'f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, 1234.6478987e+20);
	status += CHECK_UVALUE(n, 19);
	status += CHECK_RESULT(result, 1);

	result = sscan("1,234.647,8987e+20", 18, "%'f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, 1234.647898);
	status += CHECK_UVALUE(n, 13);
	status += CHECK_RESULT(result, 1);

	result = sscan("1,234.64,789,87e+20", 18, "%'f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, 1234.64);
	status += CHECK_UVALUE(n, 8);
	status += CHECK_RESULT(result, 1);

	result = sscan("1,234.64,789,87e+20", 18, "%'7f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, 1234.6);
	status += CHECK_UVALUE(n, 7);
	status += CHECK_RESULT(result, 1);

	result = sscan("1,234.6478987e+20", 17, "%'16f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, 123464.78987);
	status += CHECK_UVALUE(n, 16);
	status += CHECK_RESULT(result, 1);

	result = sscan("0x105.12p+2", 11, "%a%n", &f32, &n);
	status += CHECK_FLOAT32(f32, 0x105.12p+2);
	status += CHECK_UVALUE(n, 11);
	status += CHECK_RESULT(result, 1);

	result = sscan("-0x105.12p2", 11, "%a%n", &f32, &n);
	status += CHECK_FLOAT32(f32, -0x105.12p+2);
	status += CHECK_UVALUE(n, 11);
	status += CHECK_RESULT(result, 1);

	result = sscan("0xa05.1cp-12", 12, "%a%n", &f32, &n);
	status += CHECK_FLOAT32(f32, 0xa05.1cp-12);
	status += CHECK_UVALUE(n, 12);
	status += CHECK_RESULT(result, 1);

	result = sscan("0xfe5.dcp+12", 12, "%a%n", &f32, &n);
	status += CHECK_FLOAT32(f32, 0xfe5.dcp+12);
	status += CHECK_UVALUE(n, 12);
	status += CHECK_RESULT(result, 1);

	result = sscan("1.00", 3, "%lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, 1.00);
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscan("-1.00", 4, "%lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, -1.00);
	status += CHECK_UVALUE(n, 4);
	status += CHECK_RESULT(result, 1);

	result = sscan("1.5788987434987", 15, "%lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, 1.5788987434987);
	status += CHECK_UVALUE(n, 15);
	status += CHECK_RESULT(result, 1);

	result = sscan("13786492.5788987434987", 22, "%lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, 13786492.5788987434987);
	status += CHECK_UVALUE(n, 22);
	status += CHECK_RESULT(result, 1);

	result = sscan("-13786492.5788987434987", 23, "%lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, -13786492.5788987434987);
	status += CHECK_UVALUE(n, 23);
	status += CHECK_RESULT(result, 1);

	result = sscan("0.000000000000000000000000000000000001", 38, "%lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, 0.000000000000000000000000000000000001);
	status += CHECK_UVALUE(n, 38);
	status += CHECK_RESULT(result, 1);

	result = sscan("100000000000000000000000000000000000.0", 38, "%lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, 100000000000000000000000000000000000.0);
	status += CHECK_UVALUE(n, 38);
	status += CHECK_RESULT(result, 1);

	result = sscan("-0.000000000000000000000000000000000001", 39, "%lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, -0.000000000000000000000000000000000001);
	status += CHECK_UVALUE(n, 39);
	status += CHECK_RESULT(result, 1);

	result = sscan("-100000000000000000000000000000000000.0", 39, "%lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, -100000000000000000000000000000000000.0);
	status += CHECK_UVALUE(n, 39);
	status += CHECK_RESULT(result, 1);

	result = sscan("1.647e+28", 9, "%lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, 1.647e+28);
	status += CHECK_UVALUE(n, 9);
	status += CHECK_RESULT(result, 1);

	result = sscan("1.647e-28", 9, "%lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, 1.647e-28);
	status += CHECK_UVALUE(n, 9);
	status += CHECK_RESULT(result, 1);

	result = sscan("-1.647E+28", 10, "%lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, -1.647e+28);
	status += CHECK_UVALUE(n, 10);
	status += CHECK_RESULT(result, 1);

	result = sscan("-1.647E-28", 10, "%lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, -1.647e-28);
	status += CHECK_UVALUE(n, 10);
	status += CHECK_RESULT(result, 1);

	result = sscan("123.647e+20", 11, "%lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, 123.647e+20);
	status += CHECK_UVALUE(n, 11);
	status += CHECK_RESULT(result, 1);

	result = sscan("1,234.647e20", 12, "%'lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, 1234.647e+20);
	status += CHECK_UVALUE(n, 12);
	status += CHECK_RESULT(result, 1);

	result = sscan("1111,234.647e20", 12, "%'lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, 1111.0);
	status += CHECK_UVALUE(n, 4);
	status += CHECK_RESULT(result, 1);

	result = sscan("1,234.647,898,7e+20", 19, "%'lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, 1234.6478987e+20);
	status += CHECK_UVALUE(n, 19);
	status += CHECK_RESULT(result, 1);

	result = sscan("1,234.647,8987e+20", 18, "%'lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, 1234.647898);
	status += CHECK_UVALUE(n, 13);
	status += CHECK_RESULT(result, 1);

	result = sscan("1,234.64,789,87e+20", 18, "%'lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, 1234.64);
	status += CHECK_UVALUE(n, 8);
	status += CHECK_RESULT(result, 1);

	result = sscan("1,234.64,789,87e+20", 18, "%'7lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, 1234.6);
	status += CHECK_UVALUE(n, 7);
	status += CHECK_RESULT(result, 1);

	result = sscan("1,234.6478987e+20", 17, "%'16lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, 123464.78987);
	status += CHECK_UVALUE(n, 16);
	status += CHECK_RESULT(result, 1);

	result = sscan("3321.647e+15", 12, "%'lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, 3321.647e+15);
	status += CHECK_UVALUE(n, 12);
	status += CHECK_RESULT(result, 1);

	result = sscan("0x105.12p+2", 11, "%la%n", &f64, &n);
	status += CHECK_FLOAT64(f64, 0x105.12p+2);
	status += CHECK_UVALUE(n, 11);
	status += CHECK_RESULT(result, 1);

	result = sscan("-0x105.12p2", 11, "%la%n", &f64, &n);
	status += CHECK_FLOAT64(f64, -0x105.12p+2);
	status += CHECK_UVALUE(n, 11);
	status += CHECK_RESULT(result, 1);

	result = sscan("0xa05.1cp-12", 12, "%la%n", &f64, &n);
	status += CHECK_FLOAT64(f64, 0xa05.1cp-12);
	status += CHECK_UVALUE(n, 12);
	status += CHECK_RESULT(result, 1);

	result = sscan("0xfe5.dcp+12", 12, "%la%n", &f64, &n);
	status += CHECK_FLOAT64(f64, 0xfe5.dcp+12);
	status += CHECK_UVALUE(n, 12);
	status += CHECK_RESULT(result, 1);

	return status;
}

int test_float_special(void)
{
	int status = 0;
	int result = 0;

	float f32 = 0;
	double f64 = 0;

	uint32_t a32 = 0;
	uint64_t a64 = 0;

	uint32_t pinf32 = ((int)0xFF << 23);
	uint32_t pnan32 = 0x7FFFFFFF;
	uint32_t ninf32 = pinf32 | ((int)1 << 31);
	uint32_t nnan32 = pnan32 | ((int)1 << 31);

	uint64_t pinf64 = ((uint64_t)0x7FF << 52);
	uint64_t pnan64 = 0x7FFFFFFFFFFFFFFF;
	uint64_t ninf64 = pinf64 | ((uint64_t)1 << 63);
	uint64_t nnan64 = pnan64 | ((uint64_t)1 << 63);

	int n = 0;

	result = sscan("0", 1, "%f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, 0);
	status += CHECK_UVALUE(n, 1);
	status += CHECK_RESULT(result, 1);

	result = sscan("-0", 2, "%f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, 0);
	status += CHECK_UVALUE(n, 2);
	status += CHECK_RESULT(result, 1);

	result = sscan("inf", 3, "%f%n", &a32, &n);
	status += CHECK_UVALUE(a32, pinf32);
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscan("-inf", 4, "%f%n", &a32, &n);
	status += CHECK_UVALUE(a32, ninf32);
	status += CHECK_UVALUE(n, 4);
	status += CHECK_RESULT(result, 1);

	result = sscan("nan", 3, "%f%n", &a32, &n);
	status += CHECK_UVALUE(a32, pnan32);
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscan("-nan", 4, "%f%n", &a32, &n);
	status += CHECK_UVALUE(a32, nnan32);
	status += CHECK_UVALUE(n, 4);
	status += CHECK_RESULT(result, 1);

	result = sscan("infinity", 8, "%f%n", &a32, &n);
	status += CHECK_UVALUE(a32, pinf32);
	status += CHECK_UVALUE(n, 8);
	status += CHECK_RESULT(result, 1);

	result = sscan("0", 1, "%lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, 0);
	status += CHECK_UVALUE(n, 1);
	status += CHECK_RESULT(result, 1);

	result = sscan("-0", 2, "%lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, 0);
	status += CHECK_UVALUE(n, 2);
	status += CHECK_RESULT(result, 1);

	result = sscan("INF", 3, "%lf%n", &a64, &n);
	status += CHECK_UVALUE(a64, pinf64);
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscan("-inf", 4, "%lf%n", &a64, &n);
	status += CHECK_UVALUE(a64, ninf64);
	status += CHECK_UVALUE(n, 4);
	status += CHECK_RESULT(result, 1);

	result = sscan("NaN", 3, "%lf%n", &a64, &n);
	status += CHECK_UVALUE(a64, pnan64);
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscan("-nan", 4, "%lf%n", &a64, &n);
	status += CHECK_UVALUE(a64, nnan64);
	status += CHECK_UVALUE(n, 4);
	status += CHECK_RESULT(result, 1);

	result = sscan("-INFINITY", 9, "%lf%n", &a64, &n);
	status += CHECK_UVALUE(a64, ninf64);
	status += CHECK_UVALUE(n, 9);
	status += CHECK_RESULT(result, 1);

	return status;
}

int test_char(void)
{
	int status = 0;
	int result = 0;

	uint8_t utf8_ch = 0;
	uint16_t utf16_ch = 0;
	uint32_t utf32_ch = 0;

	int n = 0;

	result = sscan("a", 1, "%c%n", &utf8_ch, &n);
	status += CHECK_UVALUE(utf8_ch, 'a');
	status += CHECK_UVALUE(n, 1);
	status += CHECK_RESULT(result, 1);

	result = sscan("€", 3, "%lc%n", &utf16_ch, &n);
	status += CHECK_UVALUE(utf16_ch, u'€');
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscan("€", 3, "%llc%n", &utf32_ch, &n);
	status += CHECK_UVALUE(utf32_ch, U'€');
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscan("😊", 4, "%llc%n", &utf32_ch, &n);
	status += CHECK_UVALUE(utf32_ch, U'😊');
	status += CHECK_UVALUE(n, 4);
	status += CHECK_RESULT(result, 1);

	return status;
}

int test_string(void)
{
	int status = 0;
	int result = 0;

	char *str = NULL;
	char u8_str1[256] = {0}, u8_str2[256] = {0};
	uint16_t u16_str1[256] = {0}, u16_str2[256] = {0};
	uint32_t u32_str1[256] = {0}, u32_str2[256] = {0};

	int n = 0;

	// ------------------------------------------------------------------------------------

	result = sscan("abcd efgh", 9, "%s%s%n", u8_str1, u8_str2, &n);
	status += CHECK_STRING(u8_str1, "abcd");
	status += CHECK_STRING(u8_str2, "efgh");
	status += CHECK_UVALUE(n, 9);
	status += CHECK_RESULT(result, 2);

	result = sscan("€", 3, "%s%n", &u8_str1, &n);
	status += CHECK_STRING(u8_str1, "€");
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscan("😊", 4, "%s%n", &u8_str2, &n);
	status += CHECK_STRING(u8_str2, "😊");
	status += CHECK_UVALUE(n, 4);
	status += CHECK_RESULT(result, 1);

	// ------------------------------------------------------------------------------------

	result = sscan("abcd efgh", 9, "%ls%ls%n", u16_str1, u16_str2, &n);
	status += CHECK_WSTRING(u16_str1, u"abcd");
	status += CHECK_WSTRING(u16_str2, u"efgh");
	status += CHECK_UVALUE(n, 9);
	status += CHECK_RESULT(result, 2);

	result = sscan("€", 3, "%ls%n", &u16_str1, &n);
	status += CHECK_WSTRING(u16_str1, u"€");
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscan("😊", 4, "%ls%n", &u16_str2, &n);
	status += CHECK_WSTRING(u16_str2, u"😊");
	status += CHECK_UVALUE(n, 4);
	status += CHECK_RESULT(result, 1);

	// ------------------------------------------------------------------------------------

	result = sscan("abcd efgh", 9, "%lls%lls%n", u32_str1, u32_str2, &n);
	status += CHECK_WSTRING(u32_str1, U"abcd");
	status += CHECK_WSTRING(u32_str2, U"efgh");
	status += CHECK_UVALUE(n, 9);
	status += CHECK_RESULT(result, 2);

	result = sscan("€", 3, "%lls%n", &u32_str1, &n);
	status += CHECK_WSTRING(u32_str1, U"€");
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscan("😊", 4, "%lls%n", &u32_str2, &n);
	status += CHECK_WSTRING(u32_str2, U"😊");
	status += CHECK_UVALUE(n, 4);
	status += CHECK_RESULT(result, 1);

	// ------------------------------------------------------------------------------------

	result = sscan("abcdef", 6, "%ms%n", &str, &n);
	status += CHECK_STRING(str, "abcdef");
	status += CHECK_UVALUE(n, 6);
	status += CHECK_RESULT(result, 1);

	free(str);

	// ------------------------------------------------------------------------------------

	return status;
}

int test_set(void)
{
	int status = 0;
	int result = 0;

	char *str = NULL;
	char u8_str[256] = {0};
	char u16_str[256] = {0};
	char u32_str[256] = {0};

	int n = 0;

	// ------------------------------------------------------------------------------------

	result = sscan("abcd", 4, "%[abc]%n", u8_str, &n);
	status += CHECK_STRING(u8_str, "abc");
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscan("abcdefg", 7, "%[a-f]%n", u8_str, &n);
	status += CHECK_STRING(u8_str, "abcdef");
	status += CHECK_UVALUE(n, 6);
	status += CHECK_RESULT(result, 1);

	result = sscan("abcd abcd", 9, "%[a-f ]%n", u8_str, &n);
	status += CHECK_STRING(u8_str, "abcd abcd");
	status += CHECK_UVALUE(n, 9);
	status += CHECK_RESULT(result, 1);

	result = sscan("abcd-abcd", 9, "%[a-f -]%n", u8_str, &n);
	status += CHECK_STRING(u8_str, "abcd-abcd");
	status += CHECK_UVALUE(n, 9);
	status += CHECK_RESULT(result, 1);

	result = sscan("abcd-abcd", 9, "%6[a-f -]%n", u8_str, &n);
	status += CHECK_STRING(u8_str, "abcd-a");
	status += CHECK_UVALUE(n, 6);
	status += CHECK_RESULT(result, 1);

	result = sscan("][]]", 4, "%[][]%n", u8_str, &n);
	status += CHECK_STRING(u8_str, "][]]");
	status += CHECK_UVALUE(n, 4);
	status += CHECK_RESULT(result, 1);

	result = sscan("[[]][]]", 7, "%[[]%n", u8_str, &n);
	status += CHECK_STRING(u8_str, "[[");
	status += CHECK_UVALUE(n, 2);
	status += CHECK_RESULT(result, 1);

	result = sscan("abcd abcd", 9, "%[^d]%n", u8_str, &n);
	status += CHECK_STRING(u8_str, "abc");
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscan("1234978", 7, "%[^5-8]%n", u8_str, &n);
	status += CHECK_STRING(u8_str, "12349");
	status += CHECK_UVALUE(n, 5);
	status += CHECK_RESULT(result, 1);

	result = sscan("1234978", 7, "%2[^5-8]%n", u8_str, &n);
	status += CHECK_STRING(u8_str, "12");
	status += CHECK_UVALUE(n, 2);
	status += CHECK_RESULT(result, 1);

	result = sscan("[[]]", 4, "%[^]]%n", u8_str, &n);
	status += CHECK_STRING(u8_str, "[[");
	status += CHECK_UVALUE(n, 2);
	status += CHECK_RESULT(result, 1);

	result = sscan("[[]]", 4, "%[^][]%n", u8_str, &n);
	status += CHECK_STRING(u8_str, "");
	status += CHECK_UVALUE(n, 0);
	status += CHECK_RESULT(result, 1);

	result = sscan("ab-", 3, "%[^-]%n", u8_str, &n);
	status += CHECK_STRING(u8_str, "ab");
	status += CHECK_UVALUE(n, 2);
	status += CHECK_RESULT(result, 1);

	// ------------------------------------------------------------------------------------

	result = sscan("abcd", 4, "%l[abc]%n", u16_str, &n);
	status += CHECK_WSTRING(u16_str, u"abc");
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscan("abcdefg", 7, "%l[a-f]%n", u16_str, &n);
	status += CHECK_WSTRING(u16_str, u"abcdef");
	status += CHECK_UVALUE(n, 6);
	status += CHECK_RESULT(result, 1);

	result = sscan("abcd abcd", 9, "%l[a-f ]%n", u16_str, &n);
	status += CHECK_WSTRING(u16_str, u"abcd abcd");
	status += CHECK_UVALUE(n, 9);
	status += CHECK_RESULT(result, 1);

	result = sscan("abcd-abcd", 9, "%l[a-f -]%n", u16_str, &n);
	status += CHECK_WSTRING(u16_str, u"abcd-abcd");
	status += CHECK_UVALUE(n, 9);
	status += CHECK_RESULT(result, 1);

	result = sscan("abcd-abcd", 9, "%6l[a-f -]%n", u16_str, &n);
	status += CHECK_WSTRING(u16_str, u"abcd-a");
	status += CHECK_UVALUE(n, 6);
	status += CHECK_RESULT(result, 1);

	result = sscan("][]]", 4, "%l[][]%n", u16_str, &n);
	status += CHECK_WSTRING(u16_str, u"][]]");
	status += CHECK_UVALUE(n, 4);
	status += CHECK_RESULT(result, 1);

	result = sscan("[[]][]]", 7, "%l[[]%n", u16_str, &n);
	status += CHECK_WSTRING(u16_str, u"[[");
	status += CHECK_UVALUE(n, 2);
	status += CHECK_RESULT(result, 1);

	result = sscan("abcd abcd", 9, "%l[^d]%n", u16_str, &n);
	status += CHECK_WSTRING(u16_str, u"abc");
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscan("1234978", 7, "%l[^5-8]%n", u16_str, &n);
	status += CHECK_WSTRING(u16_str, u"12349");
	status += CHECK_UVALUE(n, 5);
	status += CHECK_RESULT(result, 1);

	result = sscan("1234978", 7, "%2l[^5-8]%n", u16_str, &n);
	status += CHECK_WSTRING(u16_str, u"12");
	status += CHECK_UVALUE(n, 2);
	status += CHECK_RESULT(result, 1);

	result = sscan("[[]]", 4, "%l[^]]%n", u16_str, &n);
	status += CHECK_WSTRING(u16_str, u"[[");
	status += CHECK_UVALUE(n, 2);
	status += CHECK_RESULT(result, 1);

	result = sscan("[[]]", 4, "%l[^][]%n", u16_str, &n);
	status += CHECK_WSTRING(u16_str, u"");
	status += CHECK_UVALUE(n, 0);
	status += CHECK_RESULT(result, 1);

	result = sscan("ab-", 3, "%l[^-]%n", u16_str, &n);
	status += CHECK_WSTRING(u16_str, u"ab");
	status += CHECK_UVALUE(n, 2);
	status += CHECK_RESULT(result, 1);

	// ------------------------------------------------------------------------------------

	result = sscan("abcd", 4, "%ll[abc]%n", u32_str, &n);
	status += CHECK_WSTRING(u32_str, U"abc");
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscan("abcdefg", 7, "%ll[a-f]%n", u32_str, &n);
	status += CHECK_WSTRING(u32_str, U"abcdef");
	status += CHECK_UVALUE(n, 6);
	status += CHECK_RESULT(result, 1);

	result = sscan("abcd abcd", 9, "%ll[a-f ]%n", u32_str, &n);
	status += CHECK_WSTRING(u32_str, U"abcd abcd");
	status += CHECK_UVALUE(n, 9);
	status += CHECK_RESULT(result, 1);

	result = sscan("abcd-abcd", 9, "%ll[a-f -]%n", u32_str, &n);
	status += CHECK_WSTRING(u32_str, U"abcd-abcd");
	status += CHECK_UVALUE(n, 9);
	status += CHECK_RESULT(result, 1);

	result = sscan("abcd-abcd", 9, "%6ll[a-f -]%n", u32_str, &n);
	status += CHECK_WSTRING(u32_str, U"abcd-a");
	status += CHECK_UVALUE(n, 6);
	status += CHECK_RESULT(result, 1);

	result = sscan("][]]", 4, "%ll[][]%n", u32_str, &n);
	status += CHECK_WSTRING(u32_str, U"][]]");
	status += CHECK_UVALUE(n, 4);
	status += CHECK_RESULT(result, 1);

	result = sscan("[[]][]]", 7, "%ll[[]%n", u32_str, &n);
	status += CHECK_WSTRING(u32_str, U"[[");
	status += CHECK_UVALUE(n, 2);
	status += CHECK_RESULT(result, 1);

	result = sscan("abcd abcd", 9, "%ll[^d]%n", u32_str, &n);
	status += CHECK_WSTRING(u32_str, U"abc");
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscan("1234978", 7, "%ll[^5-8]%n", u32_str, &n);
	status += CHECK_WSTRING(u32_str, U"12349");
	status += CHECK_UVALUE(n, 5);
	status += CHECK_RESULT(result, 1);

	result = sscan("1234978", 7, "%2ll[^5-8]%n", u32_str, &n);
	status += CHECK_WSTRING(u32_str, U"12");
	status += CHECK_UVALUE(n, 2);
	status += CHECK_RESULT(result, 1);

	result = sscan("[[]]", 4, "%ll[^]]%n", u32_str, &n);
	status += CHECK_WSTRING(u32_str, U"[[");
	status += CHECK_UVALUE(n, 2);
	status += CHECK_RESULT(result, 1);

	result = sscan("[[]]", 4, "%ll[^][]%n", u32_str, &n);
	status += CHECK_WSTRING(u32_str, U"");
	status += CHECK_UVALUE(n, 0);
	status += CHECK_RESULT(result, 1);

	result = sscan("ab-", 3, "%ll[^-]%n", u32_str, &n);
	status += CHECK_WSTRING(u32_str, U"ab");
	status += CHECK_UVALUE(n, 2);
	status += CHECK_RESULT(result, 1);

	// ------------------------------------------------------------------------------------

	result = sscan("abcd", 4, "%m[abc]%n", &str, &n);
	status += CHECK_STRING(str, "abc");
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	free(str);

	// ------------------------------------------------------------------------------------

	return status;
}

int test_pointer(void)
{
	int status = 0;
	int result = 0;

	uintptr_t p = 0;
	int n = 0;

	result = sscan("0x22", 4, "%p%n", &p, &n);
	status += CHECK_UVALUE(p, 0x22);
	status += CHECK_UVALUE(n, 4);
	status += CHECK_RESULT(result, 1);

	result = sscan("0x22", 4, "%2p%n", &p, &n);
	status += CHECK_UVALUE(p, 0x0);
	status += CHECK_UVALUE(n, 2);
	status += CHECK_RESULT(result, 1);

	result = sscan("0x22", 4, "%3p%n", &p, &n);
	status += CHECK_UVALUE(p, 0x2);
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscan("0x22", 4, "%4p%n", &p, &n);
	status += CHECK_UVALUE(p, 0x22);
	status += CHECK_UVALUE(n, 4);
	status += CHECK_RESULT(result, 1);

	result = sscan("0x22", 4, "%5p%n", &p, &n);
	status += CHECK_UVALUE(p, 0x22);
	status += CHECK_UVALUE(n, 4);
	status += CHECK_RESULT(result, 1);

	result = sscan("0x22  ", 6, "%5p%n", &p, &n);
	status += CHECK_UVALUE(p, 0x22);
	status += CHECK_UVALUE(n, 4);
	status += CHECK_RESULT(result, 1);

	return status;
}

int test_position(void)
{
	int status = 0;
	int result = 0;

	int i = 0, j = 0;
	double x = 0, y = 0;
	int n = 0, m = 0;

	result = sscan("123", 3, "%2$2d %1$d%3$n", &i, &j, &n);
	status += CHECK_IVALUE(i, 3);
	status += CHECK_IVALUE(j, 12);
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 2);

	result = sscan("123e457e-2", 10, "%2$5lf%4$n%1$lf%3$n", &x, &y, &n, &m);
	status += CHECK_FLOAT64(x, 57e-2);
	status += CHECK_FLOAT64(y, 123e+4);
	status += CHECK_UVALUE(n, 10);
	status += CHECK_UVALUE(m, 5);
	status += CHECK_RESULT(result, 2);

	result = sscan("123 567 4e2", 11, "%1$d %5$n %3$d %2$lf %4$n", &i, &x, &j, &n, &m);
	status += CHECK_IVALUE(i, 123);
	status += CHECK_IVALUE(j, 567);
	status += CHECK_FLOAT64(x, 4e2);
	status += CHECK_UVALUE(n, 11);
	status += CHECK_UVALUE(m, 4);
	status += CHECK_RESULT(result, 3);

	return status;
}

int test_overflow(void)
{
	int status = 0;
	int result = 0;

	int8_t i8 = 0;
	int16_t i16 = 0;
	int32_t i32 = 0;
	int64_t i64 = 0;
	intmax_t imax = 0;
	intptr_t iptr = 0;

	uint8_t u8 = 0;
	uint16_t u16 = 0;
	uint32_t u32 = 0;
	uint64_t u64 = 0;
	uintmax_t umax = 0;
	uintptr_t uptr = 0;
	size_t usize = 0;

	result = sscan("-128 -32768 -2147483648 -9223372036854775808 -9223372036854775808 -9223372036854775808", 86, "%hhi %hi %i %li %ji %ti",
				   &i8, &i16, &i32, &i64, &imax, &iptr);
	status += CHECK_RESULT(result, 6);
	status += CHECK_IVALUE(i8, INT8_MIN);
	status += CHECK_IVALUE(i16, INT16_MIN);
	status += CHECK_IVALUE(i32, INT32_MIN);
	status += CHECK_IVALUE(i64, INT64_MIN);
	status += CHECK_IVALUE(imax, INTMAX_MIN);
	status += CHECK_IVALUE(iptr, INTPTR_MIN);

	result = sscan("127 32767 2147483647 9223372036854775807 9223372036854775807 9223372036854775807", 80, "%hhi %hi %i %li %ji %ti", &i8,
				   &i16, &i32, &i64, &imax, &iptr);
	status += CHECK_RESULT(result, 6);
	status += CHECK_IVALUE(i8, INT8_MAX);
	status += CHECK_IVALUE(i16, INT16_MAX);
	status += CHECK_IVALUE(i32, INT32_MAX);
	status += CHECK_IVALUE(i64, INT64_MAX);
	status += CHECK_IVALUE(imax, INTMAX_MAX);
	status += CHECK_IVALUE(iptr, INTPTR_MAX);

	result = sscan("255 65535 4294967295 18446744073709551615 18446744073709551615 18446744073709551615 18446744073709551615", 104,
				   "%hhu %hu %u %lu %ju %tu %zu", &u8, &u16, &u32, &u64, &umax, &uptr, &usize);
	status += CHECK_RESULT(result, 7);
	status += CHECK_IVALUE(u8, UINT8_MAX);
	status += CHECK_IVALUE(u16, UINT16_MAX);
	status += CHECK_IVALUE(u32, UINT32_MAX);
	status += CHECK_IVALUE(u64, UINT64_MAX);
	status += CHECK_IVALUE(umax, UINTMAX_MAX);
	status += CHECK_IVALUE(uptr, UINTPTR_MAX);
	status += CHECK_IVALUE(usize, UINT64_MAX);

	return status;
}

int test_suppress(void)
{
	int status = 0;
	int result = 0;

	uint32_t a = 0;
	uint8_t c = 0;
	int n = 0;

	result = sscan("123 567", 7, "%*u%n%u", &n, &a);
	status += CHECK_UVALUE(a, 567);
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscan("123567", 6, "%*4u%u%n", &a, &n);
	status += CHECK_UVALUE(a, 67);
	status += CHECK_UVALUE(n, 6);
	status += CHECK_RESULT(result, 1);

	result = sscan("1234567", 7, "%4$*4u%2$u%1$n", &n, &a);
	status += CHECK_UVALUE(a, 567);
	status += CHECK_UVALUE(n, 7);
	status += CHECK_RESULT(result, 1);

	result = sscan("abcdef 123", 10, "%*s%u%n", &a, &n);
	status += CHECK_UVALUE(a, 123);
	status += CHECK_UVALUE(n, 10);
	status += CHECK_RESULT(result, 1);

	result = sscan("abcdef 123", 10, "%*[a-e]%c%n", &c, &n);
	status += CHECK_UVALUE(c, 'f');
	status += CHECK_UVALUE(n, 6);
	status += CHECK_RESULT(result, 1);

	return status;
}

int main()
{
	return test_simple() + test_int() + test_uint() + test_float() + test_float_special() + test_char() + test_string() + test_set() +
		   test_pointer() + test_position() + test_overflow() + test_suppress();
}
