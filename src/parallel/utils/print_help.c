#include <stdio.h>

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
  printf("The output_path is the path where the output files will be written.");
  printf("The finalize_script_path is the path to a program to run once the "
         "computation");
  printf("has finished -for example, to notify of its completion-. This last "
         "parameter is");
  printf("OPTIONAL.");
}