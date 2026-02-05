#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "./cli.h"

argparse_context_t *make_argparse_ctx(int argc, char **argv) {
  argparse_context_t *ctx = malloc(sizeof(argparse_context_t));
  ctx->__argc = argc;
  ctx->__argv = argv;

  arg_nodes_t *args = malloc(sizeof(arg_nodes_t));
  arg_node_t *head = malloc(sizeof(arg_node_t));
  arg_node_t *tail = head;

  ctx->args = args;
  ctx->args->head = NULL;
  ctx->args->tail = NULL;

  char **iter = ++argv; // skip the first argument
  char *arg = *iter++;
  if (!arg)
    return ctx;

  head->prev = NULL;
  head->next = NULL;
  head->value = arg;

  while ((arg = *iter++)) {
    arg_node_t *node = malloc(sizeof(arg_node_t));
    tail->next = node;
    node->prev = tail;
    node->value = arg;
    tail = node;
  }

  ctx->args->head = head;
  ctx->args->tail = tail;
  return ctx;
}

static int __get_printable_flag_len__(argument_t *arg) {
  if (arg->__printable_flag_len)
    return arg->__printable_flag_len;

  arg->__primary_flag_len = strlen(arg->primary_flag);
  arg->__secondary_flag_len =
      arg->secondary_flag ? strlen(arg->secondary_flag) : 0;
  arg->__printable_flag_len =
      arg->__primary_flag_len + arg->__secondary_flag_len;
  return arg->__printable_flag_len;
}

static void __print_flag__(argument_t *flag, int cols, int max_opt_len) {
  printf("  %s", flag->primary_flag);
  if (flag->secondary_flag != NULL) {
    printf(", %s", flag->secondary_flag);
    printf("%*s", max_opt_len - flag->__primary_flag_len, " ");
  } else {
    // pad the whitespace if secondary flag is not available;
    printf("%*s", max_opt_len - flag->__primary_flag_len + 4, " ");
  }

  printf("%s", flag->help);
  if (flag->required)
    printf(" (Required)");
  printf("\n");
}

void print_cmd_usage(char *cmd, argument_t **args) {
  argument_t *flag;
  argument_t **iter = args;

  int opt_size = 0;
  int non_kwdc = 0;
  char **non_kwdv = malloc(sizeof(char *));
  while ((flag = *iter++)) {
    opt_size = max(__get_printable_flag_len__(flag), opt_size);
    flag->__kwd = *flag->primary_flag == '-';
    if (flag->__kwd)
      continue;

    int idx = non_kwdc++;
    non_kwdv = realloc(non_kwdv, sizeof(char *) * non_kwdc);
    non_kwdv[idx] = flag->primary_flag;
  }

  int col_size;

#ifdef _SYS_IOCTL_H
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  col_size = w.ws_col;
#else
  col_size = 64;
#endif

  printf("Usage: %s [options]", cmd);
  for (int i = 0; i < non_kwdc; i++)
    printf(" <%s>", non_kwdv[i]);

  printf("\n");
  printf("Options:\n");

  iter = args;
  while ((flag = *iter++))
    if (flag->__kwd)
      __print_flag__(flag, col_size, opt_size);

  free(flag);
  free(non_kwdv);
}

static void __remove_arg__(arg_nodes_t *args, arg_node_t *arg) {
  arg_node_t *prev = arg->prev;
  if (arg->prev)
    arg->prev->next = arg->next;
  else
    args->head = arg->next;

  if (arg->next)
    arg->next->prev = prev;
  else
    args->tail = prev;
}

#define remove_arg(args, arg)                                                  \
  __remove_arg__(args, arg);                                                   \
  free(arg)

#define remove_arg_with_adj(args, arg)                                         \
  __remove_arg__(args, arg);                                                   \
  __remove_arg__(args, arg->next);                                             \
  free(arg->next);                                                             \
  free(arg)

int read_boolean(arg_nodes_t *args, argument_t *arg) {
  arg->value.boolean = 0;
  arg_node_t *cursor = args->head;
  while (cursor) {
    if (!match_flags(arg->primary_flag, arg->secondary_flag, cursor->value)) {
      cursor = cursor->next;
      continue;
    }
    arg->value.boolean = 1;
    remove_arg(args, cursor);
    break;
  }
  return ARG_PARSE_SUCCESS;
}

int read_number(arg_nodes_t *args, argument_t *arg) {
  arg_node_t *cursor = args->head;
  if (arg->required) {
    if (!cursor || !cursor->value || *cursor->value == '-')
      return ARG_PARSE_ERROR_NOT_FOUND;

    arg->value.number = atoi(cursor->value);
    remove_arg(args, cursor);
    return ARG_PARSE_SUCCESS;
  }

  while (cursor) {
    if (!match_flags(arg->primary_flag, arg->secondary_flag, cursor->value)) {
      cursor = cursor->next;
      continue;
    }

    arg->value.number = atoi(cursor->next->value);
    remove_arg_with_adj(args, cursor);
    return ARG_PARSE_SUCCESS;
  }

  if (arg->default_)
    arg->value.number = *(int *)arg->default_;
  return ARG_PARSE_SUCCESS;
}

int read_number_array(arg_nodes_t *args, argument_t *arg) {
  arg->value.number_array = malloc(sizeof(int *));
  arg_node_t *temp, *cursor = args->head;
  while (cursor) {
    if (!match_flags(arg->primary_flag, arg->secondary_flag, cursor->value)) {
      cursor = cursor->next;
      continue;
    }

    int idx = arg->size++;
    arg->value.number_array =
        realloc(arg->value.number_array, sizeof(int *) * arg->size);
    arg->value.number_array[idx] = atoi(cursor->next->value);

    temp = cursor;
    cursor = cursor->next->next;
    remove_arg_with_adj(args, temp);
  }
  return arg->required && arg->size == 0 ? ARG_PARSE_ERROR_NOT_FOUND
                                         : ARG_PARSE_SUCCESS;
}

int read_string(arg_nodes_t *args, argument_t *arg) {
  arg_node_t *cursor = args->head;
  if (arg->required) {
    if (!cursor || !cursor->value || *cursor->value == '-')
      return ARG_PARSE_ERROR_NOT_FOUND;

    arg->value.string = cursor->value;
    __remove_arg__(args, cursor);
    return ARG_PARSE_SUCCESS;
  }

  while (cursor) {
    if (!match_flags(arg->primary_flag, arg->secondary_flag, cursor->value)) {
      cursor = cursor->next;
      continue;
    }

    arg->value.string = cursor->next->value;
    remove_arg_with_adj(args, cursor);
    return ARG_PARSE_SUCCESS;
  }

  if (arg->default_)
    arg->value.string = (char *)arg->default_;
  return ARG_PARSE_SUCCESS;
}

int read_path(arg_nodes_t *args, argument_t *arg) {
  return read_string(args, arg);
}

int read_string_array(arg_nodes_t *args, argument_t *arg) {
  arg->value.string_array = malloc(sizeof(char *));
  arg_node_t *temp, *cursor = args->head;
  while (cursor) {
    if (!match_flags(arg->primary_flag, arg->secondary_flag, cursor->value)) {
      cursor = cursor->next;
      continue;
    }

    int idx = arg->size++;
    arg->value.string_array =
        realloc(arg->value.string_array, sizeof(char *) * arg->size);
    arg->value.string_array[idx] = cursor->next->value;

    temp = cursor;
    cursor = cursor->next->next;
    remove_arg_with_adj(args, temp);
  }
  return ARG_PARSE_SUCCESS;
}

int read_counter(arg_nodes_t *args, argument_t *arg) {
  arg_node_t *temp, *cursor = args->head;
  arg->value.number = 0;

  char c, *iter;
  char denom = *(arg->primary_flag + 1);
  while (cursor) {
    if (*(cursor->value + 1) != denom)
      goto cont;

    iter = cursor->value + 1;
    while ((c = *iter++)) {
      if (c != denom)
        goto cont;
      arg->value.number++;
    }
    remove_arg(args, cursor);
    return ARG_PARSE_SUCCESS;

  cont:
    cursor = cursor->next;
    continue;
  }
  return ARG_PARSE_SUCCESS;
}

int parse_arg(argparse_context_t *ctx, argument_t *arg) {
  if (arg->__parsed)
    return ARG_PARSE_ERROR_ALREADY_PARSED;

  arg->__parsed = 1;
  switch (arg->type) {
  case ARG_STRING:
    return read_string(ctx->args, arg);
  case ARG_STRING_ARRAY:
    return read_string_array(ctx->args, arg);
  case ARG_NUMBER:
    return read_number(ctx->args, arg);
  case ARG_NUMBER_ARRAY:
    return read_number_array(ctx->args, arg);
  case ARG_FLAG:
    return read_boolean(ctx->args, arg);
  case ARG_PATH:
    return read_path(ctx->args, arg);
  case ARG_COUNTER:
    return read_counter(ctx->args, arg);
  }
};

void print_extra_args(argparse_context_t *ctx) {
  arg_node_t *cursor = ctx->args->head;
  fprintf(stderr, "error: found extra arguments {");
  while (1) {
    fprintf(stderr, "%s", cursor->value);
    cursor = cursor->next;
    if (!cursor)
      break;
    fprintf(stderr, ", ");
  }
  fprintf(stderr, "}\n");
}

int parse_args(argparse_context_t *ctx, argument_t *slots[],
               argument_t *__help_flag) {

  int ret = 0, __ret;
  if (__help_flag &&
      (parse_arg(ctx, __help_flag) || __help_flag->value.boolean)) {
    print_cmd_usage(ctx->__argv[0], slots);
    return ret;
  }

  // iterator
  argument_t *slot, **iter;

  // parse optional arguments first
  iter = slots;
  while ((slot = *iter++))
    if (!slot->required) {
      __ret = parse_arg(ctx, slot);
      ret = max(__ret, ret);
    }

  // parse required/positional arguments
  iter = slots;
  while ((slot = *iter++))
    if (slot->required) {
      __ret = parse_arg(ctx, slot);
      ret = max(__ret, ret);
    }

  if (!ctx->args->head)
    return ret;
  return max(ret, ARG_PARSE_ERROR_EXTRA_ARGS);
}
