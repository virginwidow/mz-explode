#include <stddef.h>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "explode/io.hh"
#include "explode/exe_file.hh"
#include "explode/unpklite.hh"

#include "unittest/md5.h"

#include "unittest/pklite_112.cc"  
#include "unittest/pklite_120.cc"  
#include "unittest/pklite_201.cc"    
#include "unittest/pklite_E_115.cc"
#include "unittest/pklite_100.cc"  
#include "unittest/pklite_115.cc"  
#include "unittest/pklite_150.cc"  
#include "unittest/pklite_E_112.cc"  
#include "unittest/pklite_E_120.cc"

static int total_tests = 0;
static int failed_tests = 0;

static const char* digest_pklite_112   = "e1f98f301ef8bb8710ae14469bcb2cd0";
static const char* digest_pklite_115   = "13482d37794b1106a85712b5e7a1227a";
static const char* digest_pklite_120   = "e1f98f301ef8bb8710ae14469bcb2cd0";
static const char* digest_pklite_150   = "36ce063f2a979acc3ba887f4f3b9f735";
static const char* digest_pklite_201   = "e6cf27d7818c320ce64bcb4caba7f5a4";
static const char* digest_pklite_E_112 = "8a4b841106bae1f32c7ca45e9d41c016";
static const char* digest_pklite_E_115 = "56dccb4b55bdd7c57f09dbb584050a51";
static const char* digest_pklite_E_120 = "8a4b841106bae1f32c7ca45e9d41c016";

typedef unsigned char md5_digest[MD5_DIGEST_LENGTH];

static void pklite_digest(const unsigned char* data, std::size_t length, md5_digest& digest)
{
	explode::inmem_input input(data, length);
	explode::input_exe_file iexe(input);
	if (!iexe.is_pklite())
	{
		throw std::runtime_error("not a PKLITE");
	}
	explode::unpklite decoder(iexe);
	explode::full_exe_file fo(decoder.decomp_size());
	decoder.unpak(fo);
	std::vector <char> out_buff;
	explode::inmem_output out(out_buff);
	fo.write(out);

	MD5_CTX c;
	MD5_Init(&c);
	MD5_Update(&c, out_buff.data (), out_buff.size ());
	MD5_Final(digest, &c);
}

static void pklite_test(const char* test_name, const unsigned char* data, std::size_t length, const char* expected)
{
	
	md5_digest dgst;
	bool ok = true;
	try
	{
		pklite_digest(data, length, dgst);
		for (int n = 0; n < MD5_DIGEST_LENGTH; n++)
		{
			std::string h(expected + 2 * n, expected + 2 * (n + 1));
			std::istringstream is(h);
			int x;
			is >> std::hex >> x;
			if (x != dgst[n])
			{
				ok = false;
				break;
			}
		}
	}
	catch (...)
	{
		ok = false;
	}
	total_tests++;
	const char* s_ok = "PASSED";
	if (!ok)
	{
		failed_tests++;
		s_ok = "FAILED";
	}
	std::cout << "TEST #" << total_tests << ": PKLITE-" << test_name << " " << s_ok << std::endl;
}

#define CONCATENATE_DIRECT(s1,s2) s1##s2
#define CONCATENATE(s1,s2) CONCATENATE_DIRECT(s1,s2)

#define STRINGIZE_HELPER(exp) #exp
#define STRINGIZE(exp) STRINGIZE_HELPER(exp)

#define PKLITE_TEST(NAME) pklite_test(STRINGIZE(NAME), CONCATENATE (data::pklite_, NAME), CONCATENATE(CONCATENATE (data::pklite_, NAME), _len), CONCATENATE(digest_, CONCATENATE (pklite_, NAME)))

int main(int argc, char* argv[])
{
	PKLITE_TEST(112);
	PKLITE_TEST(E_112);

	PKLITE_TEST(115);
	PKLITE_TEST(E_115);

	PKLITE_TEST(150);
	PKLITE_TEST(201);

	std::cout << "Total Tests: " << total_tests << " Failed tests: " << failed_tests << std::endl;

	return failed_tests;
}