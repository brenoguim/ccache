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

#include "catch2_tests.hpp"
#include "framework.hpp"

unsigned suite_args(unsigned);
unsigned suite_argument_processing(unsigned);
unsigned suite_compopt(unsigned);
unsigned suite_compr_type_none(unsigned);
unsigned suite_compr_type_zstd(unsigned);
unsigned suite_conf(unsigned);
unsigned suite_counters(unsigned);
unsigned suite_hash(unsigned);
unsigned suite_hashutil(unsigned);
unsigned suite_legacy_util(unsigned);
unsigned suite_lockfile(unsigned);
unsigned suite_stats(unsigned);

const suite_fn k_legacy_suites[] = {
  &suite_args,
  &suite_argument_processing,
  &suite_compopt,
  &suite_compr_type_none,
  &suite_compr_type_zstd,
  &suite_counters,
  &suite_hash,
  &suite_hashutil,
  &suite_legacy_util,
  &suite_lockfile,
  &suite_stats,
  NULL,
};

int
main(int argc, char** argv)
{
#ifdef _WIN32
  x_setenv("CCACHE_DETECT_SHEBANG", "1");
#endif

  char* testdir = format("testdir.%d", (int)getpid());
  cct_create_fresh_dir(testdir);
  char* dir_before = gnu_getcwd();
  cct_chdir(testdir);

  // Run Catch2 tests.
  int result = run_catch2_tests(argc, argv);

  // Run legacy tests.
  if (result == 0) {
    bool verbose = false;
    result = cct_run(k_legacy_suites, verbose);
  }

  if (result == 0) {
    cct_chdir(dir_before);
    cct_wipe(testdir);
  }
  free(testdir);
  free(dir_before);
  return result;
}
