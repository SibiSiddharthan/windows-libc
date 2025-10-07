/*
   Copyright (c) 2024 - 2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#pragma warning(push)
#pragma warning(disable : 4305) // truncation from 'double' to 'float'
#pragma warning(disable : 4474) // format
#pragma warning(disable : 4477) // format
#pragma warning(disable : 4475) // format
#pragma warning(disable : 4476) // format

int test_simple(void)
{
	int status = 0;
	int result = 0;

	uint8_t c1 = 0, c2 = 0;
	int n = 0;

	result = sscanf("", "");
	status += CHECK_RESULT(result, 0);

	result = sscanf("", "%c", &c1);
	status += CHECK_RESULT(result, 0);

	result = sscanf("a", "%c", &c1);
	status += CHECK_UVALUE(c1, 'a');
	status += CHECK_RESULT(result, 1);

	result = sscanf("bc", "%c%c", &c1, &c2);
	status += CHECK_UVALUE(c1, 'b');
	status += CHECK_UVALUE(c2, 'c');
	status += CHECK_RESULT(result, 2);

	result = sscanf("de", "%c  %c", &c1, &c2);
	status += CHECK_UVALUE(c1, 'd');
	status += CHECK_UVALUE(c2, 'e');
	status += CHECK_RESULT(result, 2);

	result = sscanf("h  i", "%c %c", &c1, &c2);
	status += CHECK_UVALUE(c1, 'h');
	status += CHECK_UVALUE(c2, 'i');
	status += CHECK_RESULT(result, 2);

	result = sscanf("jh   gk", "%ch g%c", &c1, &c2);
	status += CHECK_UVALUE(c1, 'j');
	status += CHECK_UVALUE(c2, 'k');
	status += CHECK_RESULT(result, 2);

	result = sscanf("lh   gl", "%ch l%c", &c1, &c2);
	status += CHECK_UVALUE(c1, 'l');
	status += CHECK_RESULT(result, 1);

	result = sscanf("jh   gk", "%ch g%c%n", &c1, &c2, &n);
	status += CHECK_UVALUE(c1, 'j');
	status += CHECK_UVALUE(c2, 'k');
	status += CHECK_UVALUE(n, 7);
	status += CHECK_RESULT(result, 2);

	result = sscanf("jh   gk", "%ch g%c  %n", &c1, &c2, &n);
	status += CHECK_UVALUE(c1, 'j');
	status += CHECK_UVALUE(c2, 'k');
	status += CHECK_UVALUE(n, 7);
	status += CHECK_RESULT(result, 2);

	result = sscanf("jh   gk    ", "%ch g%c%n", &c1, &c2, &n);
	status += CHECK_UVALUE(c1, 'j');
	status += CHECK_UVALUE(c2, 'k');
	status += CHECK_UVALUE(n, 7);
	status += CHECK_RESULT(result, 2);

	result = sscanf("jh   gk    ", "%ch g%c  %n", &c1, &c2, &n);
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

	result = sscanf("123", "%d%n", &i, &n);
	status += CHECK_IVALUE(i, 123);
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscanf("-123", "%d%n", &i, &n);
	status += CHECK_IVALUE(i, -123);
	status += CHECK_UVALUE(n, 4);
	status += CHECK_RESULT(result, 1);

	result = sscanf("-123", "%1d%n", &i, &n);
	status += CHECK_IVALUE(i, 0);
	status += CHECK_UVALUE(n, 1);
	status += CHECK_RESULT(result, 1);

	result = sscanf("000000000000123", "%d%n", &i, &n);
	status += CHECK_IVALUE(i, 123);
	status += CHECK_UVALUE(n, 15);
	status += CHECK_RESULT(result, 1);

	result = sscanf("000000000000123", "%5d%n", &i, &n);
	status += CHECK_IVALUE(i, 0);
	status += CHECK_UVALUE(n, 5);
	status += CHECK_RESULT(result, 1);

	result = sscanf("123", "%2d %d%n", &i, &j, &n);
	status += CHECK_IVALUE(i, 12);
	status += CHECK_IVALUE(j, 3);
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 2);

	result = sscanf("000000000000123", "%5d%2d%n", &i, &j, &n);
	status += CHECK_IVALUE(i, 0);
	status += CHECK_IVALUE(j, 0);
	status += CHECK_UVALUE(n, 7);
	status += CHECK_RESULT(result, 2);

	result = sscanf("123-56", "%d %d%n", &i, &j, &n);
	status += CHECK_IVALUE(i, 123);
	status += CHECK_IVALUE(j, -56);
	status += CHECK_UVALUE(n, 6);
	status += CHECK_RESULT(result, 2);

	result = sscanf("456  -089", "%d %d%n", &i, &j, &n);
	status += CHECK_IVALUE(i, 456);
	status += CHECK_IVALUE(j, -89);
	status += CHECK_UVALUE(n, 9);
	status += CHECK_RESULT(result, 2);

	result = sscanf("123,456", "%d%n", &i, &n);
	status += CHECK_IVALUE(i, 123);
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscanf("123,4,567", "%'d%'d%n", &i, &j, &n);
	status += CHECK_IVALUE(i, 123);
	status += CHECK_IVALUE(j, 0);
	status += CHECK_RESULT(result, 1);

	result = sscanf("123,4,567", "%'4d%'d%n", &i, &j, &n);
	status += CHECK_IVALUE(i, 123);
	status += CHECK_IVALUE(j, 4567);
	status += CHECK_UVALUE(n, 9);
	status += CHECK_RESULT(result, 2);

	result = sscanf("123,456", "%'d%n", &i, &n);
	status += CHECK_IVALUE(i, 123456);
	status += CHECK_UVALUE(n, 7);
	status += CHECK_RESULT(result, 1);

	result = sscanf("654321", "%'d%n", &i, &n);
	status += CHECK_IVALUE(i, 654321);
	status += CHECK_UVALUE(n, 6);
	status += CHECK_RESULT(result, 1);

	result = sscanf("-123,456", "%'d%n", &i, &n);
	status += CHECK_IVALUE(i, -123456);
	status += CHECK_UVALUE(n, 8);
	status += CHECK_RESULT(result, 1);

	result = sscanf("-654321", "%'d%n", &i, &n);
	status += CHECK_IVALUE(i, -654321);
	status += CHECK_UVALUE(n, 7);
	status += CHECK_RESULT(result, 1);

	result = sscanf("123,4567,890", "%'d%'d%n", &i, &j, &n);
	status += CHECK_IVALUE(i, 123456);
	status += CHECK_IVALUE(j, 7890);
	status += CHECK_UVALUE(n, 12);
	status += CHECK_RESULT(result, 2);

	result = sscanf("12,34567,890", "%'d%'d%n", &i, &j, &n);
	status += CHECK_IVALUE(i, 12345);
	status += CHECK_IVALUE(j, 67890);
	status += CHECK_UVALUE(n, 12);
	status += CHECK_RESULT(result, 2);

	result = sscanf("-123,4567,890", "%'d%'d%n", &i, &j, &n);
	status += CHECK_IVALUE(i, -123456);
	status += CHECK_IVALUE(j, 7890);
	status += CHECK_UVALUE(n, 13);
	status += CHECK_RESULT(result, 2);

	result = sscanf("-12,34567,890", "%'d%'d%n", &i, &j, &n);
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

	result = sscanf("123", "%u%n", &u, &n);
	status += CHECK_IVALUE(u, 123);
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscanf("000000000000123", "%u%n", &u, &n);
	status += CHECK_IVALUE(u, 123);
	status += CHECK_UVALUE(n, 15);
	status += CHECK_RESULT(result, 1);

	result = sscanf("-123", "%u%n", &u, &n);
	status += CHECK_RESULT(result, 0);

	result = sscanf("123", "%2u%n", &u, &n);
	status += CHECK_IVALUE(u, 12);
	status += CHECK_UVALUE(n, 2);
	status += CHECK_RESULT(result, 1);

	result = sscanf("123,456", "%'5u%n", &u, &n);
	status += CHECK_IVALUE(u, 123);
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscanf("123,456", "%'u%n", &u, &n);
	status += CHECK_IVALUE(u, 123456);
	status += CHECK_UVALUE(n, 7);
	status += CHECK_RESULT(result, 1);

	result = sscanf("654321", "%'u%n", &u, &n);
	status += CHECK_IVALUE(u, 654321);
	status += CHECK_UVALUE(n, 6);
	status += CHECK_RESULT(result, 1);

	result = sscanf("123,4567,890", "%'u%'u%n", &u, &v, &n);
	status += CHECK_IVALUE(u, 123456);
	status += CHECK_IVALUE(v, 7890);
	status += CHECK_UVALUE(n, 12);
	status += CHECK_RESULT(result, 2);

	result = sscanf("12,34567,890", "%'u%'u%n", &u, &v, &n);
	status += CHECK_IVALUE(u, 12345);
	status += CHECK_IVALUE(v, 67890);
	status += CHECK_UVALUE(n, 12);
	status += CHECK_RESULT(result, 2);

	result = sscanf("00010", "%b%n", &b, &n);
	status += CHECK_IVALUE(b, 2);
	status += CHECK_UVALUE(n, 5);
	status += CHECK_RESULT(result, 1);

	result = sscanf("1000010", "%b%n", &b, &n);
	status += CHECK_IVALUE(b, 66);
	status += CHECK_UVALUE(n, 7);
	status += CHECK_RESULT(result, 1);

	result = sscanf("1000010", "%3b%n", &b, &n);
	status += CHECK_IVALUE(b, 4);
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscanf("0b1000010", "%b%n", &b, &n);
	status += CHECK_IVALUE(b, 66);
	status += CHECK_UVALUE(n, 9);
	status += CHECK_RESULT(result, 1);

	result = sscanf("0b1000010", "%4b%n", &b, &n);
	status += CHECK_IVALUE(b, 2);
	status += CHECK_UVALUE(n, 4);
	status += CHECK_RESULT(result, 1);

	result = sscanf("123", "%o%n", &o, &n);
	status += CHECK_IVALUE(o, 83);
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscanf("000000000000123", "%o%n", &o, &n);
	status += CHECK_IVALUE(o, 83);
	status += CHECK_UVALUE(n, 15);
	status += CHECK_RESULT(result, 1);

	result = sscanf("0456", "%o%n", &o, &n);
	status += CHECK_IVALUE(o, 302);
	status += CHECK_UVALUE(n, 4);
	status += CHECK_RESULT(result, 1);

	result = sscanf("0o457", "%o%n", &o, &n);
	status += CHECK_IVALUE(o, 303);
	status += CHECK_UVALUE(n, 5);
	status += CHECK_RESULT(result, 1);

	result = sscanf("0O455", "%o%n", &o, &n);
	status += CHECK_IVALUE(o, 301);
	status += CHECK_UVALUE(n, 5);
	status += CHECK_RESULT(result, 1);

	result = sscanf("0O458", "%2o%n", &o, &n);
	status += CHECK_IVALUE(o, 0);
	status += CHECK_UVALUE(n, 2);
	status += CHECK_RESULT(result, 1);

	result = sscanf("-123", "%o%n", &o, &n);
	status += CHECK_RESULT(result, 0);

	result = sscanf("789", "%2o%n", &o, &n);
	status += CHECK_IVALUE(o, 7);
	status += CHECK_UVALUE(n, 1);
	status += CHECK_RESULT(result, 1);

	result = sscanf("123", "%x%n", &x, &n);
	status += CHECK_IVALUE(x, 291);
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscanf("000000000000123", "%x%n", &x, &n);
	status += CHECK_IVALUE(x, 291);
	status += CHECK_UVALUE(n, 15);
	status += CHECK_RESULT(result, 1);

	result = sscanf("0x45Bf", "%x%n", &x, &n);
	status += CHECK_IVALUE(x, 17855);
	status += CHECK_UVALUE(n, 6);
	status += CHECK_RESULT(result, 1);

	result = sscanf("0x45Bf", "%3x%n", &x, &n);
	status += CHECK_IVALUE(x, 4);
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscanf("0X45cf", "%8x%n", &x, &n);
	status += CHECK_IVALUE(x, 17871);
	status += CHECK_UVALUE(n, 6);
	status += CHECK_RESULT(result, 1);

	result = sscanf("0X45df", "%8X%n", &x, &n);
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

	result = sscanf("1.00", "%f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, 1.00);
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscanf("-1.00", "%f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, -1.00);
	status += CHECK_UVALUE(n, 4);
	status += CHECK_RESULT(result, 1);

	result = sscanf("1.5788987434987", "%f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, 1.5788987434987);
	status += CHECK_UVALUE(n, 15);
	status += CHECK_RESULT(result, 1);

	result = sscanf("13786492.5788987434987", "%f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, 13786492.5788987434987);
	status += CHECK_UVALUE(n, 22);
	status += CHECK_RESULT(result, 1);

	result = sscanf("-13786492.5788987434987", "%f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, -13786492.5788987434987);
	status += CHECK_UVALUE(n, 23);
	status += CHECK_RESULT(result, 1);

	result = sscanf("0.000000000000000000000000000000000001", "%f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, 0.000000000000000000000000000000000001);
	status += CHECK_UVALUE(n, 38);
	status += CHECK_RESULT(result, 1);

	result = sscanf("100000000000000000000000000000000000.0", "%f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, 100000000000000000000000000000000000.0);
	status += CHECK_UVALUE(n, 38);
	status += CHECK_RESULT(result, 1);

	result = sscanf("-0.000000000000000000000000000000000001", "%f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, -0.000000000000000000000000000000000001);
	status += CHECK_UVALUE(n, 39);
	status += CHECK_RESULT(result, 1);

	result = sscanf("-100000000000000000000000000000000000.0", "%f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, -100000000000000000000000000000000000.0);
	status += CHECK_UVALUE(n, 39);
	status += CHECK_RESULT(result, 1);

	result = sscanf("1.647e+28", "%f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, 1.647e+28);
	status += CHECK_UVALUE(n, 9);
	status += CHECK_RESULT(result, 1);

	result = sscanf("1.647e-28", "%f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, 1.647e-28);
	status += CHECK_UVALUE(n, 9);
	status += CHECK_RESULT(result, 1);

	result = sscanf("-1.647E+28", "%f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, -1.647e+28);
	status += CHECK_UVALUE(n, 10);
	status += CHECK_RESULT(result, 1);

	result = sscanf("-1.647E-28", "%f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, -1.647e-28);
	status += CHECK_UVALUE(n, 10);
	status += CHECK_RESULT(result, 1);

	result = sscanf("123.647e+20", "%f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, 123.647e+20);
	status += CHECK_UVALUE(n, 11);
	status += CHECK_RESULT(result, 1);

	result = sscanf("3321.647e+15", "%'f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, 3321.647e+15);
	status += CHECK_UVALUE(n, 12);
	status += CHECK_RESULT(result, 1);

	result = sscanf("1,234.647e20", "%'f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, 1234.647e+20);
	status += CHECK_UVALUE(n, 12);
	status += CHECK_RESULT(result, 1);

	result = sscanf("1111,234.647e20", "%'f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, 1111.0);
	status += CHECK_UVALUE(n, 4);
	status += CHECK_RESULT(result, 1);

	result = sscanf("1,234.647,898,7e+20", "%'f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, 1234.6478987e+20);
	status += CHECK_UVALUE(n, 19);
	status += CHECK_RESULT(result, 1);

	result = sscanf("1,234.647,8987e+20", "%'f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, 1234.647898);
	status += CHECK_UVALUE(n, 13);
	status += CHECK_RESULT(result, 1);

	result = sscanf("1,234.64,789,87e+20", "%'f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, 1234.64);
	status += CHECK_UVALUE(n, 8);
	status += CHECK_RESULT(result, 1);

	result = sscanf("1,234.64,789,87e+20", "%'7f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, 1234.6);
	status += CHECK_UVALUE(n, 7);
	status += CHECK_RESULT(result, 1);

	result = sscanf("1,234.6478987e+20", "%'16f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, 123464.78987);
	status += CHECK_UVALUE(n, 16);
	status += CHECK_RESULT(result, 1);

	result = sscanf("0x105.12p+2", "%a%n", &f32, &n);
	status += CHECK_FLOAT32(f32, 0x105.12p+2);
	status += CHECK_UVALUE(n, 11);
	status += CHECK_RESULT(result, 1);

	result = sscanf("-0x105.12p2", "%a%n", &f32, &n);
	status += CHECK_FLOAT32(f32, -0x105.12p+2);
	status += CHECK_UVALUE(n, 11);
	status += CHECK_RESULT(result, 1);

	result = sscanf("0xa05.1cp-12", "%a%n", &f32, &n);
	status += CHECK_FLOAT32(f32, 0xa05.1cp-12);
	status += CHECK_UVALUE(n, 12);
	status += CHECK_RESULT(result, 1);

	result = sscanf("0xfe5.dcp+12", "%a%n", &f32, &n);
	status += CHECK_FLOAT32(f32, 0xfe5.dcp+12);
	status += CHECK_UVALUE(n, 12);
	status += CHECK_RESULT(result, 1);

	result = sscanf("1.00", "%lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, 1.00);
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscanf("-1.00", "%lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, -1.00);
	status += CHECK_UVALUE(n, 4);
	status += CHECK_RESULT(result, 1);

	result = sscanf("1.5788987434987", "%lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, 1.5788987434987);
	status += CHECK_UVALUE(n, 15);
	status += CHECK_RESULT(result, 1);

	result = sscanf("13786492.5788987434987", "%lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, 13786492.5788987434987);
	status += CHECK_UVALUE(n, 22);
	status += CHECK_RESULT(result, 1);

	result = sscanf("-13786492.5788987434987", "%lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, -13786492.5788987434987);
	status += CHECK_UVALUE(n, 23);
	status += CHECK_RESULT(result, 1);

	result = sscanf("0.000000000000000000000000000000000001", "%lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, 0.000000000000000000000000000000000001);
	status += CHECK_UVALUE(n, 38);
	status += CHECK_RESULT(result, 1);

	result = sscanf("100000000000000000000000000000000000.0", "%lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, 100000000000000000000000000000000000.0);
	status += CHECK_UVALUE(n, 38);
	status += CHECK_RESULT(result, 1);

	result = sscanf("-0.000000000000000000000000000000000001", "%lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, -0.000000000000000000000000000000000001);
	status += CHECK_UVALUE(n, 39);
	status += CHECK_RESULT(result, 1);

	result = sscanf("-100000000000000000000000000000000000.0", "%lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, -100000000000000000000000000000000000.0);
	status += CHECK_UVALUE(n, 39);
	status += CHECK_RESULT(result, 1);

	result = sscanf("1.647e+28", "%lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, 1.647e+28);
	status += CHECK_UVALUE(n, 9);
	status += CHECK_RESULT(result, 1);

	result = sscanf("1.647e-28", "%lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, 1.647e-28);
	status += CHECK_UVALUE(n, 9);
	status += CHECK_RESULT(result, 1);

	result = sscanf("-1.647E+28", "%lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, -1.647e+28);
	status += CHECK_UVALUE(n, 10);
	status += CHECK_RESULT(result, 1);

	result = sscanf("-1.647E-28", "%lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, -1.647e-28);
	status += CHECK_UVALUE(n, 10);
	status += CHECK_RESULT(result, 1);

	result = sscanf("123.647e+20", "%lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, 123.647e+20);
	status += CHECK_UVALUE(n, 11);
	status += CHECK_RESULT(result, 1);

	result = sscanf("1,234.647e20", "%'lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, 1234.647e+20);
	status += CHECK_UVALUE(n, 12);
	status += CHECK_RESULT(result, 1);

	result = sscanf("1111,234.647e20", "%'lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, 1111.0);
	status += CHECK_UVALUE(n, 4);
	status += CHECK_RESULT(result, 1);

	result = sscanf("1,234.647,898,7e+20", "%'lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, 1234.6478987e+20);
	status += CHECK_UVALUE(n, 19);
	status += CHECK_RESULT(result, 1);

	result = sscanf("1,234.647,8987e+20", "%'lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, 1234.647898);
	status += CHECK_UVALUE(n, 13);
	status += CHECK_RESULT(result, 1);

	result = sscanf("1,234.64,789,87e+20", "%'lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, 1234.64);
	status += CHECK_UVALUE(n, 8);
	status += CHECK_RESULT(result, 1);

	result = sscanf("1,234.64,789,87e+20", "%'7lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, 1234.6);
	status += CHECK_UVALUE(n, 7);
	status += CHECK_RESULT(result, 1);

	result = sscanf("1,234.6478987e+20", "%'16lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, 123464.78987);
	status += CHECK_UVALUE(n, 16);
	status += CHECK_RESULT(result, 1);

	result = sscanf("3321.647e+15", "%'lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, 3321.647e+15);
	status += CHECK_UVALUE(n, 12);
	status += CHECK_RESULT(result, 1);

	result = sscanf("0x105.12p+2", "%la%n", &f64, &n);
	status += CHECK_FLOAT64(f64, 0x105.12p+2);
	status += CHECK_UVALUE(n, 11);
	status += CHECK_RESULT(result, 1);

	result = sscanf("-0x105.12p2", "%la%n", &f64, &n);
	status += CHECK_FLOAT64(f64, -0x105.12p+2);
	status += CHECK_UVALUE(n, 11);
	status += CHECK_RESULT(result, 1);

	result = sscanf("0xa05.1cp-12", "%la%n", &f64, &n);
	status += CHECK_FLOAT64(f64, 0xa05.1cp-12);
	status += CHECK_UVALUE(n, 12);
	status += CHECK_RESULT(result, 1);

	result = sscanf("0xfe5.dcp+12", "%la%n", &f64, &n);
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

	result = sscanf("0", "%f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, 0);
	status += CHECK_UVALUE(n, 1);
	status += CHECK_RESULT(result, 1);

	result = sscanf("-0", "%f%n", &f32, &n);
	status += CHECK_FLOAT32(f32, 0);
	status += CHECK_UVALUE(n, 2);
	status += CHECK_RESULT(result, 1);

	result = sscanf("inf", "%f%n", &a32, &n);
	status += CHECK_UVALUE(a32, pinf32);
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscanf("-inf", "%f%n", &a32, &n);
	status += CHECK_UVALUE(a32, ninf32);
	status += CHECK_UVALUE(n, 4);
	status += CHECK_RESULT(result, 1);

	result = sscanf("nan", "%f%n", &a32, &n);
	status += CHECK_UVALUE(a32, pnan32);
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscanf("-nan", "%f%n", &a32, &n);
	status += CHECK_UVALUE(a32, nnan32);
	status += CHECK_UVALUE(n, 4);
	status += CHECK_RESULT(result, 1);

	result = sscanf("infinity", "%f%n", &a32, &n);
	status += CHECK_UVALUE(a32, pinf32);
	status += CHECK_UVALUE(n, 8);
	status += CHECK_RESULT(result, 1);

	result = sscanf("0", "%lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, 0);
	status += CHECK_UVALUE(n, 1);
	status += CHECK_RESULT(result, 1);

	result = sscanf("-0", "%lf%n", &f64, &n);
	status += CHECK_FLOAT64(f64, 0);
	status += CHECK_UVALUE(n, 2);
	status += CHECK_RESULT(result, 1);

	result = sscanf("INF", "%lf%n", &a64, &n);
	status += CHECK_UVALUE(a64, pinf64);
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscanf("-inf", "%lf%n", &a64, &n);
	status += CHECK_UVALUE(a64, ninf64);
	status += CHECK_UVALUE(n, 4);
	status += CHECK_RESULT(result, 1);

	result = sscanf("NaN", "%lf%n", &a64, &n);
	status += CHECK_UVALUE(a64, pnan64);
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscanf("-nan", "%lf%n", &a64, &n);
	status += CHECK_UVALUE(a64, nnan64);
	status += CHECK_UVALUE(n, 4);
	status += CHECK_RESULT(result, 1);

	result = sscanf("-INFINITY", "%lf%n", &a64, &n);
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

	result = sscanf("a", "%c%n", &utf8_ch, &n);
	status += CHECK_UVALUE(utf8_ch, 'a');
	status += CHECK_UVALUE(n, 1);
	status += CHECK_RESULT(result, 1);

	result = sscanf("€", "%lc%n", &utf16_ch, &n);
	status += CHECK_UVALUE(utf16_ch, u'€');
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscanf("€", "%llc%n", &utf32_ch, &n);
	status += CHECK_UVALUE(utf32_ch, U'€');
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscanf("😊", "%llc%n", &utf32_ch, &n);
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

	result = sscanf("abcd efgh", "%s%s%n", u8_str1, u8_str2, &n);
	status += CHECK_STRING(u8_str1, "abcd");
	status += CHECK_STRING(u8_str2, "efgh");
	status += CHECK_UVALUE(n, 9);
	status += CHECK_RESULT(result, 2);

	result = sscanf("€", "%s%n", &u8_str1, &n);
	status += CHECK_STRING(u8_str1, "€");
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscanf("😊", "%s%n", &u8_str2, &n);
	status += CHECK_STRING(u8_str2, "😊");
	status += CHECK_UVALUE(n, 4);
	status += CHECK_RESULT(result, 1);

	// ------------------------------------------------------------------------------------

	result = sscanf("abcd efgh", "%ls%ls%n", u16_str1, u16_str2, &n);
	status += CHECK_WSTRING(u16_str1, u"abcd");
	status += CHECK_WSTRING(u16_str2, u"efgh");
	status += CHECK_UVALUE(n, 9);
	status += CHECK_RESULT(result, 2);

	result = sscanf("€", "%ls%n", &u16_str1, &n);
	status += CHECK_WSTRING(u16_str1, u"€");
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscanf("😊", "%ls%n", &u16_str2, &n);
	status += CHECK_WSTRING(u16_str2, u"😊");
	status += CHECK_UVALUE(n, 4);
	status += CHECK_RESULT(result, 1);

	// ------------------------------------------------------------------------------------

	result = sscanf("abcd efgh", "%lls%lls%n", u32_str1, u32_str2, &n);
	status += CHECK_WSTRING(u32_str1, U"abcd");
	status += CHECK_WSTRING(u32_str2, U"efgh");
	status += CHECK_UVALUE(n, 9);
	status += CHECK_RESULT(result, 2);

	result = sscanf("€", "%lls%n", &u32_str1, &n);
	status += CHECK_WSTRING(u32_str1, U"€");
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscanf("😊", "%lls%n", &u32_str2, &n);
	status += CHECK_WSTRING(u32_str2, U"😊");
	status += CHECK_UVALUE(n, 4);
	status += CHECK_RESULT(result, 1);

	// ------------------------------------------------------------------------------------

	result = sscanf("abcdef", "%ms%n", &str, &n);
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

	result = sscanf("abcd", "%[abc]%n", u8_str, &n);
	status += CHECK_STRING(u8_str, "abc");
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscanf("abcdefg", "%[a-f]%n", u8_str, &n);
	status += CHECK_STRING(u8_str, "abcdef");
	status += CHECK_UVALUE(n, 6);
	status += CHECK_RESULT(result, 1);

	result = sscanf("abcd abcd", "%[a-f ]%n", u8_str, &n);
	status += CHECK_STRING(u8_str, "abcd abcd");
	status += CHECK_UVALUE(n, 9);
	status += CHECK_RESULT(result, 1);

	result = sscanf("abcd-abcd", "%[a-f -]%n", u8_str, &n);
	status += CHECK_STRING(u8_str, "abcd-abcd");
	status += CHECK_UVALUE(n, 9);
	status += CHECK_RESULT(result, 1);

	result = sscanf("abcd-abcd", "%6[a-f -]%n", u8_str, &n);
	status += CHECK_STRING(u8_str, "abcd-a");
	status += CHECK_UVALUE(n, 6);
	status += CHECK_RESULT(result, 1);

	result = sscanf("][]]", "%[][]%n", u8_str, &n);
	status += CHECK_STRING(u8_str, "][]]");
	status += CHECK_UVALUE(n, 4);
	status += CHECK_RESULT(result, 1);

	result = sscanf("[[]][]]", "%[[]%n", u8_str, &n);
	status += CHECK_STRING(u8_str, "[[");
	status += CHECK_UVALUE(n, 2);
	status += CHECK_RESULT(result, 1);

	result = sscanf("abcd abcd", "%[^d]%n", u8_str, &n);
	status += CHECK_STRING(u8_str, "abc");
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscanf("1234978", "%[^5-8]%n", u8_str, &n);
	status += CHECK_STRING(u8_str, "12349");
	status += CHECK_UVALUE(n, 5);
	status += CHECK_RESULT(result, 1);

	result = sscanf("1234978", "%2[^5-8]%n", u8_str, &n);
	status += CHECK_STRING(u8_str, "12");
	status += CHECK_UVALUE(n, 2);
	status += CHECK_RESULT(result, 1);

	result = sscanf("[[]]", "%[^]]%n", u8_str, &n);
	status += CHECK_STRING(u8_str, "[[");
	status += CHECK_UVALUE(n, 2);
	status += CHECK_RESULT(result, 1);

	result = sscanf("[[]]", "%[^][]%n", u8_str, &n);
	status += CHECK_STRING(u8_str, "");
	status += CHECK_UVALUE(n, 0);
	status += CHECK_RESULT(result, 1);

	result = sscanf("ab-", "%[^-]%n", u8_str, &n);
	status += CHECK_STRING(u8_str, "ab");
	status += CHECK_UVALUE(n, 2);
	status += CHECK_RESULT(result, 1);

	// ------------------------------------------------------------------------------------

	result = sscanf("abcd", "%l[abc]%n", u16_str, &n);
	status += CHECK_WSTRING(u16_str, u"abc");
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscanf("abcdefg", "%l[a-f]%n", u16_str, &n);
	status += CHECK_WSTRING(u16_str, u"abcdef");
	status += CHECK_UVALUE(n, 6);
	status += CHECK_RESULT(result, 1);

	result = sscanf("abcd abcd", "%l[a-f ]%n", u16_str, &n);
	status += CHECK_WSTRING(u16_str, u"abcd abcd");
	status += CHECK_UVALUE(n, 9);
	status += CHECK_RESULT(result, 1);

	result = sscanf("abcd-abcd", "%l[a-f -]%n", u16_str, &n);
	status += CHECK_WSTRING(u16_str, u"abcd-abcd");
	status += CHECK_UVALUE(n, 9);
	status += CHECK_RESULT(result, 1);

	result = sscanf("abcd-abcd", "%6l[a-f -]%n", u16_str, &n);
	status += CHECK_WSTRING(u16_str, u"abcd-a");
	status += CHECK_UVALUE(n, 6);
	status += CHECK_RESULT(result, 1);

	result = sscanf("][]]", "%l[][]%n", u16_str, &n);
	status += CHECK_WSTRING(u16_str, u"][]]");
	status += CHECK_UVALUE(n, 4);
	status += CHECK_RESULT(result, 1);

	result = sscanf("[[]][]]", "%l[[]%n", u16_str, &n);
	status += CHECK_WSTRING(u16_str, u"[[");
	status += CHECK_UVALUE(n, 2);
	status += CHECK_RESULT(result, 1);

	result = sscanf("abcd abcd", "%l[^d]%n", u16_str, &n);
	status += CHECK_WSTRING(u16_str, u"abc");
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscanf("1234978", "%l[^5-8]%n", u16_str, &n);
	status += CHECK_WSTRING(u16_str, u"12349");
	status += CHECK_UVALUE(n, 5);
	status += CHECK_RESULT(result, 1);

	result = sscanf("1234978", "%2l[^5-8]%n", u16_str, &n);
	status += CHECK_WSTRING(u16_str, u"12");
	status += CHECK_UVALUE(n, 2);
	status += CHECK_RESULT(result, 1);

	result = sscanf("[[]]", "%l[^]]%n", u16_str, &n);
	status += CHECK_WSTRING(u16_str, u"[[");
	status += CHECK_UVALUE(n, 2);
	status += CHECK_RESULT(result, 1);

	result = sscanf("[[]]", "%l[^][]%n", u16_str, &n);
	status += CHECK_WSTRING(u16_str, u"");
	status += CHECK_UVALUE(n, 0);
	status += CHECK_RESULT(result, 1);

	result = sscanf("ab-", "%l[^-]%n", u16_str, &n);
	status += CHECK_WSTRING(u16_str, u"ab");
	status += CHECK_UVALUE(n, 2);
	status += CHECK_RESULT(result, 1);

	// ------------------------------------------------------------------------------------

	result = sscanf("abcd", "%ll[abc]%n", u32_str, &n);
	status += CHECK_WSTRING(u32_str, U"abc");
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscanf("abcdefg", "%ll[a-f]%n", u32_str, &n);
	status += CHECK_WSTRING(u32_str, U"abcdef");
	status += CHECK_UVALUE(n, 6);
	status += CHECK_RESULT(result, 1);

	result = sscanf("abcd abcd", "%ll[a-f ]%n", u32_str, &n);
	status += CHECK_WSTRING(u32_str, U"abcd abcd");
	status += CHECK_UVALUE(n, 9);
	status += CHECK_RESULT(result, 1);

	result = sscanf("abcd-abcd", "%ll[a-f -]%n", u32_str, &n);
	status += CHECK_WSTRING(u32_str, U"abcd-abcd");
	status += CHECK_UVALUE(n, 9);
	status += CHECK_RESULT(result, 1);

	result = sscanf("abcd-abcd", "%6ll[a-f -]%n", u32_str, &n);
	status += CHECK_WSTRING(u32_str, U"abcd-a");
	status += CHECK_UVALUE(n, 6);
	status += CHECK_RESULT(result, 1);

	result = sscanf("][]]", "%ll[][]%n", u32_str, &n);
	status += CHECK_WSTRING(u32_str, U"][]]");
	status += CHECK_UVALUE(n, 4);
	status += CHECK_RESULT(result, 1);

	result = sscanf("[[]][]]", "%ll[[]%n", u32_str, &n);
	status += CHECK_WSTRING(u32_str, U"[[");
	status += CHECK_UVALUE(n, 2);
	status += CHECK_RESULT(result, 1);

	result = sscanf("abcd abcd", "%ll[^d]%n", u32_str, &n);
	status += CHECK_WSTRING(u32_str, U"abc");
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscanf("1234978", "%ll[^5-8]%n", u32_str, &n);
	status += CHECK_WSTRING(u32_str, U"12349");
	status += CHECK_UVALUE(n, 5);
	status += CHECK_RESULT(result, 1);

	result = sscanf("1234978", "%2ll[^5-8]%n", u32_str, &n);
	status += CHECK_WSTRING(u32_str, U"12");
	status += CHECK_UVALUE(n, 2);
	status += CHECK_RESULT(result, 1);

	result = sscanf("[[]]", "%ll[^]]%n", u32_str, &n);
	status += CHECK_WSTRING(u32_str, U"[[");
	status += CHECK_UVALUE(n, 2);
	status += CHECK_RESULT(result, 1);

	result = sscanf("[[]]", "%ll[^][]%n", u32_str, &n);
	status += CHECK_WSTRING(u32_str, U"");
	status += CHECK_UVALUE(n, 0);
	status += CHECK_RESULT(result, 1);

	result = sscanf("ab-", "%ll[^-]%n", u32_str, &n);
	status += CHECK_WSTRING(u32_str, U"ab");
	status += CHECK_UVALUE(n, 2);
	status += CHECK_RESULT(result, 1);

	// ------------------------------------------------------------------------------------

	result = sscanf("abcd", "%m[abc]%n", &str, &n);
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

	result = sscanf("0x22", "%p%n", &p, &n);
	status += CHECK_UVALUE(p, 0x22);
	status += CHECK_UVALUE(n, 4);
	status += CHECK_RESULT(result, 1);

	result = sscanf("0x22", "%2p%n", &p, &n);
	status += CHECK_UVALUE(p, 0x0);
	status += CHECK_UVALUE(n, 2);
	status += CHECK_RESULT(result, 1);

	result = sscanf("0x22", "%3p%n", &p, &n);
	status += CHECK_UVALUE(p, 0x2);
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscanf("0x22", "%4p%n", &p, &n);
	status += CHECK_UVALUE(p, 0x22);
	status += CHECK_UVALUE(n, 4);
	status += CHECK_RESULT(result, 1);

	result = sscanf("0x22", "%5p%n", &p, &n);
	status += CHECK_UVALUE(p, 0x22);
	status += CHECK_UVALUE(n, 4);
	status += CHECK_RESULT(result, 1);

	result = sscanf("0x22  ", "%5p%n", &p, &n);
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

	result = sscanf("123", "%2$2d %1$d%3$n", &i, &j, &n);
	status += CHECK_IVALUE(i, 3);
	status += CHECK_IVALUE(j, 12);
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 2);

	result = sscanf("123e457e-2", "%2$5lf%4$n%1$lf%3$n", &x, &y, &n, &m);
	status += CHECK_FLOAT64(x, 57e-2);
	status += CHECK_FLOAT64(y, 123e+4);
	status += CHECK_UVALUE(n, 10);
	status += CHECK_UVALUE(m, 5);
	status += CHECK_RESULT(result, 2);

	result = sscanf("123 567 4e2", "%1$d %5$n %3$d %2$lf %4$n", &i, &x, &j, &n, &m);
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

	result = sscanf("-128 -32768 -2147483648 -9223372036854775808 -9223372036854775808 -9223372036854775808", "%hhi %hi %i %li %ji %ti",
					&i8, &i16, &i32, &i64, &imax, &iptr);
	status += CHECK_RESULT(result, 6);
	status += CHECK_IVALUE(i8, INT8_MIN);
	status += CHECK_IVALUE(i16, INT16_MIN);
	status += CHECK_IVALUE(i32, INT32_MIN);
	status += CHECK_IVALUE(i64, INT64_MIN);
	status += CHECK_IVALUE(imax, INTMAX_MIN);
	status += CHECK_IVALUE(iptr, INTPTR_MIN);

	result = sscanf("127 32767 2147483647 9223372036854775807 9223372036854775807 9223372036854775807", "%hhi %hi %i %li %ji %ti", &i8,
					&i16, &i32, &i64, &imax, &iptr);
	status += CHECK_RESULT(result, 6);
	status += CHECK_IVALUE(i8, INT8_MAX);
	status += CHECK_IVALUE(i16, INT16_MAX);
	status += CHECK_IVALUE(i32, INT32_MAX);
	status += CHECK_IVALUE(i64, INT64_MAX);
	status += CHECK_IVALUE(imax, INTMAX_MAX);
	status += CHECK_IVALUE(iptr, INTPTR_MAX);

	result = sscanf("255 65535 4294967295 18446744073709551615 18446744073709551615 18446744073709551615 18446744073709551615",
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

	result = sscanf("123 567", "%*u%n%u", &n, &a);
	status += CHECK_UVALUE(a, 567);
	status += CHECK_UVALUE(n, 3);
	status += CHECK_RESULT(result, 1);

	result = sscanf("123567", "%*4u%u%n", &a, &n);
	status += CHECK_UVALUE(a, 67);
	status += CHECK_UVALUE(n, 6);
	status += CHECK_RESULT(result, 1);

	result = sscanf("1234567", "%4$*4u%2$u%1$n", &n, &a);
	status += CHECK_UVALUE(a, 567);
	status += CHECK_UVALUE(n, 7);
	status += CHECK_RESULT(result, 1);

	result = sscanf("abcdef 123", "%*s%u%n", &a, &n);
	status += CHECK_UVALUE(a, 123);
	status += CHECK_UVALUE(n, 10);
	status += CHECK_RESULT(result, 1);

	result = sscanf("abcdef 123", "%*[a-e]%c%n", &c, &n);
	status += CHECK_UVALUE(c, 'f');
	status += CHECK_UVALUE(n, 6);
	status += CHECK_RESULT(result, 1);

	return status;
}

#pragma warning(pop)

int main()
{
	INITIAILIZE_TESTS();

	TEST(test_simple());
	TEST(test_int());
	TEST(test_uint());
	TEST(test_float());
	TEST(test_float_special());
	TEST(test_char());
	TEST(test_string());
	TEST(test_set());
	TEST(test_pointer());
	TEST(test_position());
	TEST(test_overflow());
	TEST(test_suppress());

	VERIFY_RESULT_AND_EXIT();
}
