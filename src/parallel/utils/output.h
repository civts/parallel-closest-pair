#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

FILE *setup_file(const int rank, const char *const output_path) {
  int num_digits = 0;
  int tmp_rank = rank;
  do {
    tmp_rank /= 10;
    num_digits++;
  } while (tmp_rank > 0);
  char *rank_str = malloc(sizeof(char) * (num_digits + 1));
  sprintf(rank_str, "%d.txt", rank);

  // Copy output_path to out, adding a / at the end if needed
  int out_length = strlen(output_path);
  int extra_for_slash = output_path[out_length - 1] == '/' ? 0 : 1;
  char *out = malloc(sizeof(char) *
                     (out_length + extra_for_slash + strlen(rank_str) + 1));
  out[0] = '\0';
  strcat(out, output_path);
  if (extra_for_slash == 1) {
    strcat(out, "/");
  }

  int _ = mkdir(out, 0666);

  // Copy rank_str at the end of the current output path
  strcat(out, rank_str);

  FILE *out_fp = fopen(out, "w+");
  fprintf(out_fp, "Process %d\n", rank);
  free(rank_str);
  return out_fp;
}

void close_file(FILE *fp) { fclose(fp); }
