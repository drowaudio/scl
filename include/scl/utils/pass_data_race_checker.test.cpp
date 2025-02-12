//
// Created by David Rowland on 11/02/2025.
//

#define DATA_RACE_DETECTED std::exit(1);
#include "data_race_checker.test.h"

int main()
{
  test_no_data_race();
}