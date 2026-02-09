#ifndef _LIBARGS_H
#define _LIBARGS_H

#include <stdlib.h>

#define ARG_ERROR_UNKNOWN -1
#define ARG_PARSE_SUCCESS 0
#define ARG_PARSE_ERROR_BAD_CONFIG 1
#define ARG_PARSE_ERROR_EXTRA_ARGS 2
#define ARG_PARSE_ERROR_INVALID_VALUE 2
#define ARG_PARSE_ERROR_NOT_FOUND 3

#define ARG_PATH_EXIST (1 << 0)
#define ARG_PATH_IS_DIR (1 << 1)
#define ARG_PATH_IS_FILE (1 << 2)
#define ARG_PATH_NOT_EXIST (1 << 3)

#define max(a, b) (((a) > (b)) ? (a) : (b))
#define streq(s1, s2) (strcmp(s1, s2) == 0)
#define match_flags(p, s, v)                                                   \
  (strcmp(p, v) == 0 || (s != NULL && strcmp(s, v) == 0))

typedef int(validate_t)(char *);

typedef union {
  // int
  int number;
  int *number_array;

  // str
  char *string;
  char **string_array;

  // path
  char *path;

  // flag
  int boolean;

  // null
  void *null;
} val_t;

typedef enum {
  ARG_NUMBER,
  ARG_STRING,
  ARG_FLAG,
  ARG_PATH,
  ARG_NUMBER_ARRAY,
  ARG_STRING_ARRAY,
  ARG_COUNTER,
} ArgumentType;

typedef struct {
  // descriptors
  char *primary_flag;   // primary
  char *secondary_flag; // secondary
  char *help;           // help string

  // config
  ArgumentType type;
  int required;   // is required
  void *default_; // default value if not provided by argument
  unsigned int config;

  // value
  int size; // for arrays
  val_t value;

  // helper functions
  validate_t *validate_f;

  // config
  int __kwd;

  // reusable cache
  int __primary_flag_len;
  int __secondary_flag_len;
  int __printable_flag_len;
} argument_t;

#define DEFINE_REQUIRED_ARG(name, flag_p, flag_s, help, type, config)          \
  argument_t name = {flag_p, flag_s, help, type, 1, NULL, config}

#define DEFINE_OPTIONAL_ARG(name, flag_p, flag_s, help, type, default_,        \
                            config)                                            \
  argument_t name = {flag_p, flag_s, help, type, 0, default_, config}

typedef struct __ARG_NODE__ {
  char *value;
  struct __ARG_NODE__ *next;
  struct __ARG_NODE__ *prev;
} arg_node_t;

typedef struct {
  int size;
  arg_node_t *head;
  arg_node_t *tail;
} arg_nodes_t;

typedef struct {
  int __argc;
  char **__argv;
  arg_nodes_t *args;
} argparse_context_t;

typedef int(parser_t)(int, char **, argument_t *);

argparse_context_t *make_argparse_ctx(int argc, char **argv);

void print_cmd_usage(char *cmd, argument_t **args);

int read_boolean(arg_nodes_t *args, argument_t *arg);

int read_number(arg_nodes_t *args, argument_t *arg);

int read_number_array(arg_nodes_t *args, argument_t *arg);

int read_string(arg_nodes_t *args, argument_t *arg);

int read_path(arg_nodes_t *args, argument_t *arg);

int read_string_array(arg_nodes_t *args, argument_t *arg);

int read_counter(arg_nodes_t *args, argument_t *arg);

int parse_args(argparse_context_t *ctx, argument_t *slots[],
               argument_t *__help_flag);

#ifdef DEBUG
#define pdbg(...)                                                              \
  printf("[%s:%d %s] ", __FILE__, __LINE__, __func__);                                   \
  printf(__VA_ARGS__);                                                         \
  printf("\n")

#define perr(...)                                                              \
  sprintf(stderr, "[%s:%d %s] ", __FILE__, __LINE__, __func__);                  \
  sprintf(stderr, __VA_ARGS__);                                                \
  sprintf(stderr, "\n")
#else
// do nothing
#define pdbg(...);
#define perr(...);

#endif // DEBUG
#endif // LIBFLAG_H
