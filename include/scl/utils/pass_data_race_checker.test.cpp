//
// Created by David Rowland on 11/02/2025.
//

#define DATA_RACE_DETECTED { std::println ("ERROR: data race detected"); std::exit(1); }
#include "data_race_checker.test.h"

int main()
{
  test_no_data_race();

  // Single threaded-tests
  test_no_data_race_read_read();
  test_no_data_race_read_write();
  test_no_data_race_write_read();
  test_no_data_race_write_write();
}