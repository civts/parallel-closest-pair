#ifndef __ARG_PARGING_H__
#define __ARG_PARGING_H__

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define VERSION "0.0.1"

// Prints the help message, with the instructions to use this program
void print_help() {
  printf("parallel_closest_points, v%s", VERSION);
  printf("This program solves the closest pair of points problem.");
  printf("In doing so, it uses the MPI library in order to divide the "
         "computation between");
  printf("multiple processes to speed it up.\n");
  printf("Usage: parallel_closest_points [input_path] [output_path] "
         "[[finalize_script_path]]");
  printf("The input_path is the path to the input file. See the README of this "
         "project for");
  printf("the format.");
  printf("The output_path is the path to the directory where the output files "
         "will be\nwritten.");
  printf("The finalize_script_path is the path to a program to run once the "
         "computation");
  printf("has finished -for example, to notify of its completion-. This last "
         "parameter is");
  printf("OPTIONAL.");
}

// Checks if the user included the help flag in the CLI arguments given to this
// program. If so, prints the help menu and exit normally
void print_help_if_needed(const int argc, const char *const *const argv) {
  int i;
  for (i = 0; i < argc; i++) {
    const char *const arg = argv[i];
    bool is_long_help = arg == "--help";
    bool is_brief_help = arg == "-h";
    if (is_long_help || is_brief_help) {
      print_help();
      exit(0);
    }
  }
}

// Extracts the path to the input file from the arguments
const char const *parse_dataset_path(const int argc,
                                     const char *const *const argv) {
  const char *const dataset_path = argv[1];
  if (dataset_path == NULL || *dataset_path == '\0' || argc < 2) {
    printf("Missing positional argument 1: the path to the dataset file. "
           "Terminating\n");
    exit(1);
  }
  return dataset_path;
}

// Extracts the path to the output directory from the arguments
const char const *parse_output_path(const int argc,
                                    const char *const *const argv) {
  const char *const output_path = argv[2];
  if (output_path == NULL || *output_path == '\0' || argc < 3) {
    printf("Missing positional argument 2: the path to the output file. "
           "Terminating\n");
    exit(1);
  }
  return output_path;
}

#endif
