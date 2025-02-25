// Copyright (C) 2010-2019 Joel Rosdahl and other contributors
//
// See doc/AUTHORS.adoc for a complete list of contributors.
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 3 of the License, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 51
// Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

// This file contains tests for functions in hashutil.c.

#include "../src/ccache.hpp"
#include "../src/hashutil.hpp"
#include "framework.hpp"
#include "util.hpp"

TEST_SUITE(hashutil)

TEST(hash_command_output_simple)
{
  char d1[DIGEST_STRING_BUFFER_SIZE];
  char d2[DIGEST_STRING_BUFFER_SIZE];

  struct hash* h1 = hash_init();
  struct hash* h2 = hash_init();

  CHECK(hash_command_output(h1, "echo", "not used"));
  CHECK(hash_command_output(h2, "echo", "not used"));
  hash_result_as_string(h1, d1);
  hash_result_as_string(h2, d2);
  CHECK_STR_EQ(d1, d2);

  hash_free(h2);
  hash_free(h1);
}

TEST(hash_command_output_space_removal)
{
  char d1[DIGEST_STRING_BUFFER_SIZE];
  char d2[DIGEST_STRING_BUFFER_SIZE];

  struct hash* h1 = hash_init();
  struct hash* h2 = hash_init();

  CHECK(hash_command_output(h1, "echo", "not used"));
  CHECK(hash_command_output(h2, " echo ", "not used"));
  hash_result_as_string(h1, d1);
  hash_result_as_string(h2, d2);
  CHECK_STR_EQ(d1, d2);

  hash_free(h2);
  hash_free(h1);
}

TEST(hash_command_output_hash_inequality)
{
  char d1[DIGEST_STRING_BUFFER_SIZE];
  char d2[DIGEST_STRING_BUFFER_SIZE];

  struct hash* h1 = hash_init();
  struct hash* h2 = hash_init();

  CHECK(hash_command_output(h1, "echo foo", "not used"));
  CHECK(hash_command_output(h2, "echo bar", "not used"));
  hash_result_as_string(h1, d1);
  hash_result_as_string(h2, d2);
  CHECK(!str_eq(d1, d2));

  hash_free(h2);
  hash_free(h1);
}

TEST(hash_command_output_compiler_substitution)
{
  char d1[DIGEST_STRING_BUFFER_SIZE];
  char d2[DIGEST_STRING_BUFFER_SIZE];

  struct hash* h1 = hash_init();
  struct hash* h2 = hash_init();

  CHECK(hash_command_output(h1, "echo foo", "not used"));
  CHECK(hash_command_output(h2, "%compiler% foo", "echo"));
  hash_result_as_string(h1, d1);
  hash_result_as_string(h2, d2);
  CHECK_STR_EQ(d1, d2);

  hash_free(h2);
  hash_free(h1);
}

TEST(hash_command_output_stdout_versus_stderr)
{
  char d1[DIGEST_STRING_BUFFER_SIZE];
  char d2[DIGEST_STRING_BUFFER_SIZE];

  struct hash* h1 = hash_init();
  struct hash* h2 = hash_init();

#ifndef _WIN32
  create_file("stderr.sh", "#!/bin/sh\necho foo >&2\n");
  chmod("stderr.sh", 0555);
  CHECK(hash_command_output(h1, "echo foo", "not used"));
  CHECK(hash_command_output(h2, "./stderr.sh", "not used"));
#else
  create_file("stderr.bat", "@echo off\r\necho foo>&2\r\n");
  CHECK(hash_command_output(h1, "echo foo", "not used"));
  CHECK(hash_command_output(h2, "stderr.bat", "not used"));
#endif
  hash_result_as_string(h1, d1);
  hash_result_as_string(h2, d2);
  CHECK_STR_EQ(d1, d2);

  hash_free(h2);
  hash_free(h1);
}

TEST(hash_multicommand_output)
{
  char d1[DIGEST_STRING_BUFFER_SIZE];
  char d2[DIGEST_STRING_BUFFER_SIZE];

  struct hash* h1 = hash_init();
  struct hash* h2 = hash_init();

#ifndef _WIN32
  create_file("foo.sh", "#!/bin/sh\necho foo\necho bar\n");
  chmod("foo.sh", 0555);
  CHECK(hash_multicommand_output(h2, "echo foo; echo bar", "not used"));
  CHECK(hash_multicommand_output(h1, "./foo.sh", "not used"));
#else
  create_file("foo.bat", "@echo off\r\necho foo\r\necho bar\r\n");
  CHECK(hash_multicommand_output(h2, "echo foo; echo bar", "not used"));
  CHECK(hash_multicommand_output(h1, "foo.bat", "not used"));
#endif
  hash_result_as_string(h1, d1);
  hash_result_as_string(h2, d2);
  CHECK_STR_EQ(d1, d2);

  hash_free(h2);
  hash_free(h1);
}

TEST(hash_multicommand_output_error_handling)
{
  struct hash* h1 = hash_init();
  struct hash* h2 = hash_init();

  CHECK(!hash_multicommand_output(h2, "false; true", "not used"));

  hash_free(h2);
  hash_free(h1);
}

TEST(check_for_temporal_macros)
{
  const char time_start[] =
    "__TIME__\n"
    "int a;\n";
  const char time_middle[] =
    "#define a __TIME__\n"
    "int a;\n";
  const char time_end[] = "#define a __TIME__";

  const char date_start[] =
    "__DATE__\n"
    "int ab;\n";
  const char date_middle[] =
    "#define ab __DATE__\n"
    "int ab;\n";
  const char date_end[] = "#define ab __DATE__";

  const char no_temporal[] =
    "#define ab a__DATE__\n"
    "#define ab  __DATE__a\n"
    "#define ab A__DATE__\n"
    "#define ab  __DATE__A\n"
    "#define ab 0__DATE__\n"
    "#define ab  __DATE__0\n"
    "#define ab _ _DATE__\n"
    "#define ab _ _DATE__\n"
    "#define ab __ DATE__\n"
    "#define ab __D ATE__\n"
    "#define ab __DA TE__\n"
    "#define ab __DAT E__\n"
    "#define ab __DATE __\n"
    "#define ab __DATE_ _\n"
    "#define ab _ _TIME__\n"
    "#define ab __ TIME__\n"
    "#define ab __T IME__\n"
    "#define ab __TI ME__\n"
    "#define ab __TIM E__\n"
    "#define ab __TIME __\n"
    "#define ab __TIME_ _\n";

  CHECK(check_for_temporal_macros(time_start + 0, sizeof(time_start) - 0));
  CHECK(!check_for_temporal_macros(time_start + 1, sizeof(time_start) - 1));

  CHECK(check_for_temporal_macros(time_middle + 0, sizeof(time_middle) - 0));
  CHECK(check_for_temporal_macros(time_middle + 1, sizeof(time_middle) - 1));
  CHECK(check_for_temporal_macros(time_middle + 2, sizeof(time_middle) - 2));
  CHECK(check_for_temporal_macros(time_middle + 3, sizeof(time_middle) - 3));
  CHECK(check_for_temporal_macros(time_middle + 4, sizeof(time_middle) - 4));
  CHECK(check_for_temporal_macros(time_middle + 5, sizeof(time_middle) - 5));
  CHECK(check_for_temporal_macros(time_middle + 6, sizeof(time_middle) - 6));
  CHECK(check_for_temporal_macros(time_middle + 7, sizeof(time_middle) - 7));

  CHECK(check_for_temporal_macros(time_end + 0, sizeof(time_end) - 0));
  CHECK(check_for_temporal_macros(time_end + sizeof(time_end) - 9, 9));
  CHECK(!check_for_temporal_macros(time_end + sizeof(time_end) - 8, 8));

  CHECK(check_for_temporal_macros(date_start + 0, sizeof(date_start) - 0));
  CHECK(!check_for_temporal_macros(date_start + 1, sizeof(date_start) - 1));

  CHECK(check_for_temporal_macros(date_middle + 0, sizeof(date_middle) - 0));
  CHECK(check_for_temporal_macros(date_middle + 1, sizeof(date_middle) - 1));
  CHECK(check_for_temporal_macros(date_middle + 2, sizeof(date_middle) - 2));
  CHECK(check_for_temporal_macros(date_middle + 3, sizeof(date_middle) - 3));
  CHECK(check_for_temporal_macros(date_middle + 4, sizeof(date_middle) - 4));
  CHECK(check_for_temporal_macros(date_middle + 5, sizeof(date_middle) - 5));
  CHECK(check_for_temporal_macros(date_middle + 6, sizeof(date_middle) - 6));
  CHECK(check_for_temporal_macros(date_middle + 7, sizeof(date_middle) - 7));

  CHECK(check_for_temporal_macros(date_end + 0, sizeof(date_end) - 0));
  CHECK(check_for_temporal_macros(date_end + sizeof(date_end) - 9, 9));
  CHECK(!check_for_temporal_macros(date_end + sizeof(date_end) - 8, 8));

  CHECK(!check_for_temporal_macros(no_temporal + 0, sizeof(no_temporal) - 0));
  CHECK(!check_for_temporal_macros(no_temporal + 1, sizeof(no_temporal) - 1));
  CHECK(!check_for_temporal_macros(no_temporal + 2, sizeof(no_temporal) - 2));
  CHECK(!check_for_temporal_macros(no_temporal + 3, sizeof(no_temporal) - 3));
  CHECK(!check_for_temporal_macros(no_temporal + 4, sizeof(no_temporal) - 4));
  CHECK(!check_for_temporal_macros(no_temporal + 5, sizeof(no_temporal) - 5));
  CHECK(!check_for_temporal_macros(no_temporal + 6, sizeof(no_temporal) - 6));
  CHECK(!check_for_temporal_macros(no_temporal + 7, sizeof(no_temporal) - 7));
}

TEST_SUITE_END
