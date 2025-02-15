//
// Created by David Rowland on 11/02/2025.
//

// Use exit(1) as ctest seems to count a raised signal as a pass...
#define DATA_RACE_DETECTED { std::println ("ERROR: data race detected"); std::exit(1); }
#include "data_race_checker.test.h"

int main()
{
  test_data_race(); // won't return
}