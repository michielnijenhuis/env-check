//=== Includes ===============================================================//
#include "output.h"

#include <array.h>
#include <assert.h>
#include <cli.h>
#include <colors.h>
#include <cstring.h>
#include <ctype.h>
#include <fs.h>
#include <ht.h>
#include <input.h>
#include <math-utils.h>
#include <output.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//=== Defines ================================================================//
#define ENVC_NAME "Env Check"

#ifndef ENVC_VERSION
# define ENVC_VERSION NULL
#endif // ENVC_VERSION

#define ENVC_ASCII_ART                                                                                                 \
    "\
 _____ _   ___     __   ____ _   _ _____ ____ _  __ \n\
| ____| \\ | \\ \\   / /  / ___| | | | ____/ ___| |/ / \n\
|  _| |  \\| |\\ \\ / /  | |   | |_| |  _|| |   | ' /  \n\
| |___| |\\  | \\ V /   | |___|  _  | |__| |___| . \\  \n\
|_____|_| \\_|  \\_/     \\____|_| |_|_____\\____|_|\\_\\ \n\
                                                   \
                          "

//=== Typedefs ===============================================================//
typedef enum EnvVarStatus {
    OK        = 0,
    MISSING   = 1,
    DIVERGENT = 2,
    UNDEFINED = 3,
} EnvVarStatus;

typedef struct EnvVar {
    char        *name;
    char        *val;
    char        *cmpval;
    EnvVarStatus status;
    size_t       namelen;
    size_t       vallen;
    size_t       cmpvallen;
} EnvVar;

DEFINE_HASH_MAP(HT, EnvVar *);

//=== Prototypes
//===================================================================//
HT      ht_create(size_t cap);
void    ht_free(HT *ht);
EnvVar *ht_get(HT *ht, const char *key);
bool    ht_put(HT *ht, const char *key, EnvVar *value);
void    ht_keys(HT *ht, const char **buf, size_t size);
void    free_env_var(EnvVar *var);
void    free_strings(char **strv, size_t strc);
void    create_env_var_from_line(HT          *ht,
                                 const char  *line,
                                 const char **ignorev,
                                 size_t       ignorec,
                                 const char **focusv,
                                 size_t       focusc,
                                 bool         comparing);
void    set_env_var_status(EnvVar *var);
int     read_env_file(HT          *ht,
                      const char  *path,
                      const char **ignorev,
                      size_t       ignorec,
                      const char **focusv,
                      size_t       focusc,
                      bool         comparing);
size_t  find_max_width_in_array(EnvVar **varv, size_t varc, bool name);
void    trim_string(char *str);
bool    is_numeric(const char *str);
void    prepare_value_for_printing(const char *value, size_t vallen, char *buf, bool is_empty, int colwidth);
void    print_env_var(const EnvVar *var, int first_colwidth, int second_colwidth, int third_colwidth, bool comparing);
void    sort_env_vars_array(const char **keyv, size_t keyc);
bool    match_pattern(const char *str, const char *pattern);
void    print_title(const char *filename);
int     handle_cmd(Command *self);
int     list(Command *self);
int     compare(Command *self);

//=== Main ===================================================================//
int main(int argc, char **argv) {
    //=== Shared options =====================================================//
    Option ignore_opt =
        option_create_string_opt("ignore", "i", "Comma seperated list of variable name patterns to ignore", NULL, true);
    Option key_opt =
        option_create_string_opt("key", "k", "Comma seperated list of variable name patterns to focus on", NULL, true);
    Option truncate_opt =
        option_create_string_opt("truncate", "T", "The amount of chars to truncate keys or values to", "40", false);

    //=== Compare ============================================================//
    Command compare_cmd = command_create("cmp", "Compares two env files files.", compare);
    Option  cmp_target_opt =
        option_create_string_opt("target", "t", "Path to the .env file to compare with", "./.env", false);
    Option cmp_source_opt =
        option_create_string_opt("source", "s", "Path to the .env file to compare to", "./.env.example", false);
    Option cmp_missing_opt = option_create("missing", "m", "Show missing and empty variables");
    Option cmp_undefined_opt =
        option_create("undefined", "u", "Show variables in the target that aren't in the source file");
    Option  cmp_divergent_opt = option_create("divergent", "d", "Show variables with diverging values");
    Option *cmp_opts[]        = {&cmp_target_opt,
                                 &cmp_source_opt,
                                 &ignore_opt,
                                 &key_opt,
                                 &truncate_opt,
                                 &cmp_missing_opt,
                                 &cmp_undefined_opt,
                                 &cmp_divergent_opt};
    compare_cmd.optv          = cmp_opts;
    compare_cmd.optc          = ARRAY_LEN(cmp_opts);
    const char *aliasv[]      = {"compare"};
    command_set_aliases(&compare_cmd, aliasv, ARRAY_LEN(aliasv));

    //=== List ===============================================================//
    Command list_cmd =
        command_create("list", "Lists all variables in the target env file, sorted alphabetically.", list);
    Option  list_target_opt = option_create_string_opt("target", "t", "Path to the .env file", "./.env", false);
    Option *list_optv[]     = {&list_target_opt, &ignore_opt, &key_opt, &truncate_opt};
    list_cmd.optv           = list_optv;
    list_cmd.optc           = ARRAY_LEN(list_optv);

    //=== Env Check ==========================================================//
    Command *commands[] = {&compare_cmd, &list_cmd};
    Program  program    = program_create(ENVC_NAME, ENVC_VERSION);
    program_set_subcommands(&program, commands, ARRAY_LEN(commands));
    program_set_ascii_art(&program, ENVC_ASCII_ART);

    return run_application(&program, argc, argv);
}

//=== HT =====================================================================//
HT ht_create(size_t cap) {
    HT ht = HT_CREATE(HT, EnvVar *, cap, NULL, free_env_var);
    return ht;
}

void ht_free(HT *ht) {
    HT_FREE(HT, ht);
}

EnvVar *ht_get(HT *ht, const char *key) {
    HT_GET(HT, ht, key, NULL);
}

bool ht_put(HT *ht, const char *key, EnvVar *value) {
    HT_PUT(HT, ht, key, value);
}

void ht_keys(HT *ht, const char **buffer, size_t buffersize) {
    HT_KEYS(HT, ht, buffer, buffersize);
}

//=== Helpers ================================================================//
void free_env_var(EnvVar *var) {
    free(var->name);
    free(var->val);
    free(var->cmpval);
    free(var);
}

void free_strings(char **strv, size_t strc) {
    for (size_t i = 0; i < strc; ++i) {
        free(strv[i]);
    }
}

bool is_numeric(const char *str) {
    if (str == NULL || *str == '\0') {
        return false;
    }

    while (*str != '\0') {
        if (*str < '0' || *str > '9') {
            return false;
        }
        str++;
    }

    return true;
}

void trim_string(char *str) {
    if (str == NULL) {
        return;
    }

    // Trim line breaks at the end
    size_t length = strlen(str);
    while (length > 0 && (str[length - 1] == '\n' || str[length - 1] == '\r')) {
        str[--length] = '\0';
    }

    // Remove quotes if present
    if ((str[0] == '\'' && str[length - 1] == '\'') || (str[0] == '"' && str[length - 1] == '"')) {
        memmove(str, str + 1, length - 2);
        str[length - 2] = '\0';
    }
}

void sort_env_vars_array(const char **keyv, size_t keyc) {
    if (keyv == NULL || keyc == 0) {
        return;
    }

    const char *tmp     = NULL;
    bool        swapped = false;
    for (size_t i = 0; i < keyc; ++i) {
        swapped = false;
        for (size_t j = 0; j < keyc - 1 - i; ++j) {
            if (strcmp(keyv[j], keyv[j + 1]) > 0) {
                tmp         = keyv[j];
                keyv[j]     = keyv[j + 1];
                keyv[j + 1] = tmp;
                swapped     = true;
            }
        }
        if (!swapped) {
            break;
        }
    }
}

size_t find_max_width_in_array(EnvVar **varv, size_t varc, bool name) {
    size_t maxlen = 0;
    for (size_t i = 0; i < varc; ++i) {
        EnvVar *var = varv[i];
        size_t  len = 0;

        if (!name && var->val != NULL) {
            len = strlen(var->val);
        } else if (name && var->name != NULL) {
            len = strlen(var->name);
        }
        maxlen = max(maxlen, len);
    }
    return maxlen;
}

void prepare_value_for_printing(const char *val, size_t vallen, char *buf, bool is_empty, int colwidth) {
    if (is_empty) {
        sprintf(buf, "(NULL)");
        return;
    }

    if ((int) vallen > colwidth) {
        strncpy(buf, val, colwidth - 3); // TODO: fix magic number 3
        strcat(buf, "...");
        buf[colwidth] = '\0';
    } else {
        sprintf(buf, "%s", val);
    }
}

void print_env_var(const EnvVar *var, int first_colwidth, int second_colwidth, int third_colwidth, bool comparing) {
    char *keyclr         = WHITE_BOLD;
    char *boolclr        = CYAN;
    char *numclr         = EMERALD;
    char *strclr         = NO_COLOUR;

    bool  val_a_is_empty = str_is_empty(var->val);
    bool  val_a_is_bool =
        str_equals_case_insensitive(var->val, "true") || str_equals_case_insensitive(var->val, "false");
    bool   val_a_is_num   = !val_a_is_bool && is_numeric(var->val);
    char  *val_a_clr      = val_a_is_bool ? boolclr : val_a_is_num ? numclr : strclr;
    size_t val_a_size     = max(second_colwidth + 1, 8);

    bool   val_b_is_empty = comparing ? str_is_empty(var->cmpval) : true;
    bool   val_b_is_bool  = comparing ? str_equals_case_insensitive(var->cmpval, "true") ||
                                         str_equals_case_insensitive(var->cmpval, "false")
                                      : false;
    bool   val_b_is_num   = !val_b_is_bool && is_numeric(var->cmpval);
    char  *val_b_clr      = val_b_is_bool ? boolclr : val_b_is_num ? numclr : strclr;
    size_t val_b_size     = comparing ? max(third_colwidth + 1, 8) : 0;

    char   __val_a[val_a_size];
    char   __val_b[val_b_size];

    prepare_value_for_printing(var->val, var->vallen, __val_a, val_a_is_empty, second_colwidth);

    if (val_a_is_empty) {
        val_a_clr = DARK_GRAY;
    }

    if (comparing) {
        prepare_value_for_printing(var->cmpval, var->cmpvallen, __val_b, val_b_is_empty, third_colwidth);

        if (val_b_is_empty) {
            val_b_clr = RED;
        }
    }

    char *status;
    char *statusclr = NO_COLOUR;

    if (comparing) {
        switch (var->status) {
            case MISSING:
                status    = "x";
                statusclr = RED_BOLD;
                // keyclr    = RED_BOLD;
                break;
            case UNDEFINED:
                status    = "?";
                statusclr = MAGENTA_LIGHT;
                // keyclr    = MAGENTA_LIGHT;
                break;
            case DIVERGENT:
                status    = "!";
                statusclr = YELLOW_BOLD;
                // keyclr    = YELLOW_BOLD;
                break;
            case OK:
                status = " ";
        }
    }

    printf("  "); // leading spaces
    if (comparing) {
        printf("%s%s%s ", statusclr, status, NO_COLOUR);
    }
    printf("%s%-*.*s%s", keyclr, first_colwidth + 4, first_colwidth, var->name,
           NO_COLOUR); // column 1
    printf("%s%-*.*s%s", val_a_clr, second_colwidth + 3, second_colwidth, __val_a,
           NO_COLOUR); // column 2
    if (comparing) {
        printf("%s%-*.*s%s", val_b_clr, third_colwidth, third_colwidth, __val_b,
               NO_COLOUR); // column 3
    }
    printf("\n"); // line break
}

void create_env_var_from_line(HT          *ht,
                              const char  *line,
                              const char **ignorev,
                              size_t       ignorec,
                              const char **focusv,
                              size_t       focusc,
                              bool         comparing) {
    assert(ht != NULL);

    if (!line) {
        return;
    }

    if (str_starts_with(line, "#") || str_is_empty(line)) {
        return;
    }

    char *delimpos = strchr(line, '=');

    if (delimpos == NULL) {
        return;
    }

    size_t namelen = delimpos - line;
    char   name[namelen + 1];
    strncpy(name, line, namelen);
    name[namelen] = '\0';

    if (ignorec > 0) {
        for (size_t i = 0; i < ignorec; i++) {
            if (match_pattern(name, ignorev[i])) {
                return;
            }
        }
    }

    if (focusc > 0) {
        bool ignore = true;
        for (size_t i = 0; i < focusc; i++) {
            if (match_pattern(name, focusv[i])) {
                ignore = false;
                break;
            }
        }

        if (ignore) {
            return;
        }
    }

    size_t vallen = strlen(delimpos + 1);
    char  *value  = (char *) malloc((vallen + 1) * sizeof(char));

    if (value == NULL) {
        panic("Failed to allocate memory");
        /* NOT REACHED */
    }

    char trimmed[vallen + 1];
    strcpy(trimmed, delimpos + 1);
    trim_string(trimmed);
    trimmed[vallen] = '\0';
    if (!str_equals_case_insensitive(trimmed, "null") && !str_equals_case_insensitive(trimmed, "(null)")) {
        strcpy(value, trimmed);
    } else {
        value[0] = '\0';
        vallen   = 0;
    }

    EnvVar *var = ht_get(ht, name);
    if (var != NULL) {
        if (comparing && var->cmpval == NULL) {
            var->cmpval    = value;
            var->cmpvallen = vallen;
            set_env_var_status(var);
        }
    } else {
        var = malloc(sizeof(*var));

        if (var == NULL) {
            free(value);
            panic("Failed to allocate memory");
            /* NOT REACHED */
        }

        char *p = (char *) malloc((namelen + 1) * sizeof(char));

        if (p == NULL) {
            free(value);
            free(var);
            panic("Failed to allocate memory");
            /* NOT REACHED */
        }

        strcpy(p, name);

        var->cmpval    = comparing ? value : NULL;
        var->name      = p;
        var->namelen   = namelen;
        var->status    = OK;
        var->val       = comparing ? NULL : value;
        var->vallen    = comparing ? 0 : vallen;
        var->cmpvallen = comparing ? vallen : 0;
        ht_put(ht, name, var);

        set_env_var_status(var);
    }
}

void set_env_var_status(EnvVar *var) {
    bool val_is_empty    = str_is_empty(var->val);
    bool cmpval_is_empty = str_is_empty(var->cmpval);

    if (var->val != NULL && cmpval_is_empty) {
        var->status = MISSING;
        return;
    }

    if (var->val == NULL) {
        if (var->cmpval != NULL) {
            var->status = UNDEFINED;
        } else {
            var->status = OK;
        }
        return;
    }

    if (!val_is_empty && !str_equals(var->val, var->cmpval)) {
        var->status = DIVERGENT;
        return;
    }

    var->status = OK;
}

int read_env_file(HT          *ht,
                  const char  *path,
                  const char **ignorev,
                  size_t       ignorec,
                  const char **focusv,
                  size_t       focusc,
                  bool         comparing) {
    assert(ht != NULL);
    // assert(path != NULL);

    if (!file_exists(path)) {
        panicf("File '%s' does not exist", path);
        /* NOT REACHED */
    }

    if (ignorec > 0 && focusc > 0) {
        panic("Cannot read env file while taking both ignore and focus arguments");
        /* NOT REACHED */
    }

    FILE   *file;
    char   *buffer = NULL;
    size_t  len    = 0;
    ssize_t read;
    file = fopen(path, "r");

    if (file == NULL) {
        panicf("Failed to read file '%s'", path);
        /* NOT REACHED */
    }

    while ((read = getline(&buffer, &len, file)) != -1) {
        create_env_var_from_line(ht, buffer, ignorev, ignorec, focusv, focusc, comparing);
    }

    fclose(file);
    return EXIT_SUCCESS;
}

bool match_pattern(const char *str, const char *pattern) {
    // Base case: both strings are empty, pattern matches
    if (str_is_empty(str) && str_is_empty(pattern)) {
        return true;
    }

    // If pattern has '*', try matching with and without consuming a character
    // in the string
    if (*pattern == '*') {
        return match_pattern(str, pattern + 1) || (*str != '\0' && match_pattern(str + 1, pattern));
    }

    // If pattern has '?' or characters match, move to the next character in
    // both strings
    if (*pattern == '?' || (*str != '\0' && *str == *pattern)) {
        return match_pattern(str + 1, pattern + 1);
    }

    // None of the conditions match
    return false;
}

void print_title(const char *filename) {
    if (filename == NULL) {
        return;
    }

    // create title underline
    size_t pathlen = strlen(filename);
    char   underline[pathlen + 1];
    for (size_t i = 0; i < pathlen; ++i) {
        underline[i] = '=';
    }
    underline[pathlen] = '\0';

    // print title and underline
    char *titleclr = NO_COLOUR;
    pcolorf(titleclr, "%s\n", filename);
    pcolorf(titleclr, "%s\n", underline);
}

//=== Command impl ===========================================================//
int handle_cmd(Command *self) {
    char *target       = get_string_opt(self, "target");
    char *source       = get_string_opt(self, "source");
    char *ignore       = get_string_opt(self, "ignore");
    char *key          = get_string_opt(self, "key");
    char *truncate     = get_string_opt(self, "truncate");
    bool  missing      = get_bool_opt(self, "missing");
    bool  undefined    = get_bool_opt(self, "undefined");
    bool  divergent    = get_bool_opt(self, "divergent");

    int   truncate_val = truncate ? atoi(truncate) : 0;
    if (truncate_val > 0) {
        truncate_val = max(truncate_val, 4);
    }
    bool   selective = missing || undefined || divergent;
    bool   comparing = source != NULL;

    size_t ignorec   = ignore ? str_count_char(ignore, ',') + 1 : 0;
    char  *ignorev[ignorec];
    str_split_by_delim(ignore, ',', ignorev, ignorec);

    size_t focusc = key ? str_count_char(key, ',') + 1 : 0;
    char  *focusv[focusc];
    str_split_by_delim(key, ',', focusv, focusc);

    HT ht = ht_create(50);

    if (comparing &&
        read_env_file(&ht, source, (const char **) ignorev, ignorec, (const char **) focusv, focusc, false) > 0) {
        free_strings(ignorev, ignorec);
        free_strings(focusv, focusc);
        ht_free(&ht);
        return EXIT_FAILURE;
    }

    if (read_env_file(&ht, target, (const char **) ignorev, ignorec, (const char **) focusv, focusc, comparing) > 0) {
        free_strings(ignorev, ignorec);
        free_strings(focusv, focusc);
        ht_free(&ht);
        return EXIT_FAILURE;
    }

    free_strings(ignorev, ignorec);
    free_strings(focusv, focusc);

    size_t      vars = ht.size;
    const char *keys[vars];
    ht_keys(&ht, keys, vars);
    sort_env_vars_array(keys, vars);

    if (comparing) {
        char title[FILENAME_MAX];
        sprintf(title, "Comparing '%s' to '%s'", source, target);
        print_title(title);
    } else {
        print_title(target);
    }

    size_t first_colwidth  = 7;
    size_t second_colwidth = 7;
    size_t third_colwidth  = 7;

    for (size_t i = 0; i < vars; ++i) {
        EnvVar *var          = ht_get(&ht, keys[i]);
        bool    should_print = !selective || (var->status == MISSING && missing) ||
                            (var->status == UNDEFINED && undefined) || (var->status == DIVERGENT && divergent);

        if (!should_print) {
            continue;
        }

        first_colwidth  = max(first_colwidth, var->namelen);
        second_colwidth = max(second_colwidth, var->vallen);
        third_colwidth  = max(third_colwidth, var->cmpvallen);
    }

    if (truncate_val > 0) {
        first_colwidth  = min((int) first_colwidth, truncate_val);
        second_colwidth = min((int) second_colwidth, truncate_val);
        third_colwidth  = min((int) third_colwidth, truncate_val);
    }

    for (size_t i = 0; i < vars; ++i) {
        EnvVar *var          = ht_get(&ht, keys[i]);
        bool    should_print = !selective || (var->status == MISSING && missing) ||
                            (var->status == UNDEFINED && undefined) || (var->status == DIVERGENT && divergent);

        if (should_print) {
            print_env_var(var, (int) first_colwidth, (int) second_colwidth, (int) third_colwidth, comparing);
        }
    }

    ht_free(&ht);

    return EXIT_SUCCESS;
}

//=== List ===================================================================//
int list(Command *self) {
    return handle_cmd(self);
}

//=== Compare ================================================================//
int compare(Command *self) {
    return handle_cmd(self);
}
