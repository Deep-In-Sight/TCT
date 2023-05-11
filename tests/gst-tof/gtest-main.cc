#include <gst/gst.h>
#include <gtest/gtest.h>
#include <unistd.h>

#include <iostream>

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  gst_init(&argc, &argv);

  // cwd is set in meson or cmake when running tests.
  // cwd must be set correctly in order for the tests to find test data.
  char* cwd = get_current_dir_name();
  std::cout << "Current working dir: " << cwd << std::endl;
  free(cwd);

  return RUN_ALL_TESTS();
}