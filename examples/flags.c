#include <stdio.h>

#include "../lib/cli.h"

DEFINE_REQUIRED_ARG(file_arg, "file", NULL, "Path to output file", ARG_PATH,
                    ARG_PATH_EXIST | ARG_PATH_IS_FILE);
DEFINE_REQUIRED_ARG(rep_flag, "rep", NULL, "Number of reps", ARG_NUMBER, 0);
DEFINE_OPTIONAL_ARG(name_arg, "--name", "-n", "Name of the user", ARG_STRING,
                    NULL, 0);
DEFINE_OPTIONAL_ARG(age_arg, "--age", "-a", "Age of the user", ARG_NUMBER, NULL,
                    0);
DEFINE_OPTIONAL_ARG(score_arg, "--score", "-s", "User scores", ARG_NUMBER_ARRAY,
                    NULL, 0);
DEFINE_OPTIONAL_ARG(interests_arg, "--interest", "-i", "List of user interests",
                    ARG_STRING_ARRAY, NULL, 0);
DEFINE_OPTIONAL_ARG(is_grad, "--grad", NULL, "Boolean flag", ARG_FLAG, NULL, 0);
DEFINE_OPTIONAL_ARG(verbosity_flag, "-v", NULL, "Define verbosity level",
                    ARG_COUNTER, NULL, 0);
DEFINE_OPTIONAL_ARG(show_help, "--help", "-h", "Show help and exit", ARG_FLAG,
                    NULL, 0);

int main(int argc, char *argv[]) {
  argument_t *arg_configs[] = {&verbosity_flag, &name_arg,      &age_arg,
                               &score_arg,      &interests_arg, &is_grad,
                               &file_arg,       &rep_flag,      NULL};
  argparse_context_t *argparse_ctx = make_argparse_ctx(argc, argv);
  int retval = parse_args(argparse_ctx, arg_configs, &show_help);
  if (retval != ARG_PARSE_SUCCESS) {
    pdbg("retval %d", retval);
    print_cmd_usage(argv[0], arg_configs);
    return 1;
  }

  if (file_arg.value.string)
    printf("file: %s\n", file_arg.value.string);

  if (rep_flag.value.string)
    printf("reps: %d\n", rep_flag.value.number);

  if (name_arg.value.string)
    printf("name: %s\n", name_arg.value.string);

  if (age_arg.value.number)
    printf("age: %d\n", age_arg.value.number);

  if (score_arg.value.number_array)
    for (int i = 0; i < score_arg.size; i++)
      printf("score(%d): %d\n", i, score_arg.value.number_array[i]);

  if (interests_arg.value.string_array)
    for (int i = 0; i < interests_arg.size; i++)
      printf("interest(%d): %s\n", i, interests_arg.value.string_array[i]);

  if (is_grad.value.boolean)
    printf("is_grad: %d\n", is_grad.value.boolean);

  if (verbosity_flag.value.number)
    printf("verbosity: %d\n", verbosity_flag.value.number);

  return 0;
}
