# LibCLI

## Quick Start
```c
#include <stdio.h>
#include "cli.h"

// Required arguments
DEFINE_REQUIRED_ARG(file_arg, "file", NULL, "Path to output file", ARG_PATH, ARG_PATH_EXIST | ARG_PATH_IS_FILE);

// Optional arguments
DEFINE_REQUIRED_ARG(verbosity_arg, "-v", NULL, "Verbosity level", ARG_COUNTER, NULL);
DEFINE_REQUIRED_ARG(user_group_arg, "--group", "-g", "User group", ARG_STRING, NULL);
DEFINE_REQUIRED_ARG(user_id_arg, "--id", "-i", "User ID", ARG_NUMBER, NULL);
DEFINE_OPTIONAL_ARG(show_help, "--help", "-h", "Show help and exit", ARG_FLAG, NULL, 0);

int main(int argc, char *argv[]) {
  // Create argparse context
  argument_t *arg_configs[] = {&file_arg, &user_group_arg, &user_id_arg, &show_help, NULL};
  argparse_context_t *argparse_ctx = make_argparse_ctx(argc, argv);

  // Parse arguments
  int retval = parse_args(argparse_ctx, arg_configs, &show_help);

  // Check for errors (check `Error Codes` section for more details)
  if (retval != ARG_PARSE_SUCCESS) {
    // Print usage and exit
    print_cmd_usage(argv[0], arg_configs);
    return 1;
  }

  // Use parsed arguments
  if (file_arg.value.string)
    printf("file: %s\n", file_arg.value.string);
  
  if (user_group_arg.value.string)
    printf("user group: %s\n", user_group_arg.value.string);

  if (user_id_arg.value.number)
    printf("user id: %d\n", user_id_arg.value.number);

  if (verbosity_arg.value.number)
    printf("verbosity: %d\n", verbosity_arg.value.number);

  return 0;
}
```


## Available Flag Types

- `ARG_FLAG`: Boolean flag
- `ARG_NUMBER`: Integer number
- `ARG_NUMBER_ARRAY`: Integer array
- `ARG_STRING`: String
- `ARG_STRING_ARRAY`: String array
- `ARG_COUNTER`: Counter
- `ARG_PATH`: Path to a file or directory

## Error Codes

- `ARG_PARSE_SUCCESS`: Success
- `ARG_PARSE_ERROR_BAD_CONFIG`: Bad configuration
- `ARG_PARSE_ERROR_EXTRA_ARGS`: Extra arguments found
- `ARG_PARSE_ERROR_INVALID_VALUE`: Invalid value
- `ARG_PARSE_ERROR_NOT_FOUND`: Argument not found
