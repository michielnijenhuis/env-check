//=== Includes ===============================================================//
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
} env_var_status_t;

typedef struct EnvVar {
    char            *name;
    char            *val;
    char            *cmpval;
    env_var_status_t status;
    size_t           namelen;
    size_t           vallen;
    size_t           cmpvallen;
    bool             interpolated;
} env_var_t;

DEFINE_HASH_MAP(hash_table_t, env_var_t *);

//=== Prototypes =============================================================//
hash_table_t ht_create(size_t cap);
void         ht_free(hash_table_t *ht);
env_var_t   *ht_get(hash_table_t *ht, const char *key);
bool         ht_put(hash_table_t *ht, const char *key, env_var_t *value);
void         ht_keys(hash_table_t *ht, const char **buf, size_t size);
void         free_env_var(env_var_t *var);
void         free_strings(char **strv, size_t strc);
void         create_env_var_from_line(hash_table_t *ht,
                                      const char   *line,
                                      const char  **ignorev,
                                      size_t        ignorec,
                                      const char  **focusv,
                                      size_t        focusc,
                                      bool          comparing,
                                      bool          interpolate);
void         set_env_var_status(env_var_t *var);
int          read_env_file(hash_table_t *ht,
                           const char   *path,
                           const char  **ignorev,
                           size_t        ignorec,
                           const char  **focusv,
                           size_t        focusc,
                           bool          comparing,
                           bool          interpolate);
size_t       find_max_width_in_array(env_var_t **varv, size_t varc, bool name);
size_t       trim_string(char *str);
bool         is_numeric(const char *str);
bool         is_interpolated(const char *str);
void         interpolate_env_vars(hash_table_t *ht, const char **keyv, size_t keyc);
void         prepare_value_for_printing(const char *value, size_t vallen, char *buf, bool is_empty, int colwidth);
void print_env_var(const env_var_t *var, int first_colwidth, int second_colwidth, int third_colwidth, bool comparing);
void sort_env_vars_array(const char **keyv, size_t keyc);
bool match_pattern(const char *str, const char *pattern);
void print_title(const char *filename);
int  handle_cmd(command_t *self);
int  list(command_t *self);
int  compare(command_t *self);

//=== Main ===================================================================//
int main(int argc, char **argv) {
    //=== Shared options =====================================================//
    option_t ignore_opt =
        option_create_string_opt("ignore", "i", "Comma seperated list of variable name patterns to ignore", NULL, true);
    option_t key_opt =
        option_create_string_opt("key", "k", "Comma seperated list of variable name patterns to focus on", NULL, true);
    option_t truncate_opt =
        option_create_string_opt("truncate", "T", "The amount of chars to truncate keys or values to", "40", false);
    option_t interpolate_opt = option_create("interpolate", "I", "Interpolate env var values that refer to other env vars");

    //=== Compare ============================================================//
    command_t compare_cmd = command_create("cmp", "Compares two env files files.", compare);
    option_t  cmp_target_opt =
        option_create_string_opt("target", "t", "Path to the .env file to compare with", "./.env", false);
    option_t cmp_source_opt =
        option_create_string_opt("source", "s", "Path to the .env file to compare to", "./.env.example", false);
    option_t cmp_missing_opt = option_create("missing", "m", "Show missing and empty variables");
    option_t cmp_undefined_opt =
        option_create("undefined", "u", "Show variables in the target that aren't in the source file");
    option_t  cmp_divergent_opt = option_create("divergent", "d", "Show variables with diverging values");
    option_t *cmp_opts[]        = {&cmp_target_opt,
                                   &cmp_source_opt,
                                   &ignore_opt,
                                   &key_opt,
                                   &truncate_opt,
                                   &cmp_missing_opt,
                                   &cmp_undefined_opt,
                                   &cmp_divergent_opt,
                                   &interpolate_opt};
    compare_cmd.optv            = cmp_opts;
    compare_cmd.optc            = ARRAY_LEN(cmp_opts);
    const char *aliasv[]        = {"compare"};
    command_set_aliases(&compare_cmd, aliasv, ARRAY_LEN(aliasv));

    //=== List ===============================================================//
    command_t list_cmd =
        command_create("list", "Lists all variables in the target env file, sorted alphabetically.", list);
    option_t  list_target_opt = option_create_string_opt("target", "t", "Path to the .env file", "./.env", false);
    option_t *list_optv[]     = {&list_target_opt, &ignore_opt, &key_opt, &truncate_opt, &interpolate_opt};
    list_cmd.optv             = list_optv;
    list_cmd.optc             = ARRAY_LEN(list_optv);

    //=== Env Check ==========================================================//
    command_t *commands[] = {&compare_cmd, &list_cmd};
    program_t  program    = program_create(ENVC_NAME, ENVC_VERSION);
    program_set_subcommands(&program, commands, ARRAY_LEN(commands));
    program_set_ascii_art(&program, ENVC_ASCII_ART);

    return run_application(&program, argc, argv);
}

//=== hash_table_t ===========================================================//
hash_table_t ht_create(size_t cap) {
    hash_table_t ht = HT_CREATE(hash_table_t, env_var_t *, cap, NULL, free_env_var);
    return ht;
}

void ht_free(hash_table_t *ht) {
    HT_FREE(hash_table_t, ht);
}

env_var_t *ht_get(hash_table_t *ht, const char *key) {
    HT_GET(hash_table_t, ht, key, NULL);
}

bool ht_put(hash_table_t *ht, const char *key, env_var_t *value) {
    HT_PUT(hash_table_t, ht, key, value);
}

void ht_keys(hash_table_t *ht, const char **buffer, size_t buffersize) {
    HT_KEYS(hash_table_t, ht, buffer, buffersize);
}

//=== Helpers ================================================================//
void free_env_var(env_var_t *var) {
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

size_t trim_string(char *str) {
    if (str == NULL) {
        return 0;
    }

    // Trim line breaks at the end
    size_t length = strlen(str);
    while (length > 0 && (str[length - 1] == '\n' || str[length - 1] == '\r')) {
        str[--length] = '\0';
    }

    // Remove quotes if present
    if ((str[0] == '\'' && str[length - 1] == '\'') || (str[0] == '"' && str[length - 1] == '"')) {
        length -= 2;
        memmove(str, str + 1, length);
        str[length] = '\0';
    }

    return length;
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

size_t find_max_width_in_array(env_var_t **varv, size_t varc, bool name) {
    size_t maxlen = 0;
    for (size_t i = 0; i < varc; ++i) {
        env_var_t *var = varv[i];
        size_t     len = 0;

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

void print_env_var(const env_var_t *var, int first_colwidth, int second_colwidth, int third_colwidth, bool comparing) {
    char *keyclr         = WHITE_BOLD;
    char *boolclr        = CYAN;
    char *numclr         = EMERALD;
    char *strclr         = NO_COLOR;

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
    char *statusclr = NO_COLOR;

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

    writef("  "); // leading spaces
    if (comparing) {
        writef("%s%s%s ", statusclr, status, NO_COLOR);
    }
    writef("%s%-*.*s%s", keyclr, first_colwidth + 4, first_colwidth, var->name,
           NO_COLOR); // column 1
    writef("%s%-*.*s%s", val_a_clr, second_colwidth + 3, second_colwidth, __val_a,
           NO_COLOR); // column 2
    if (comparing) {
        writef("%s%-*.*s%s", val_b_clr, third_colwidth, third_colwidth, __val_b,
               NO_COLOR); // column 3
    }
    writef("\n"); // line break
}

void create_env_var_from_line(hash_table_t *ht,
                              const char   *line,
                              const char  **ignorev,
                              size_t        ignorec,
                              const char  **focusv,
                              size_t        focusc,
                              bool          comparing,
                              bool          interpolate) {
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
    vallen = trim_string(trimmed);
    // printf("%s: %zu\n", name, vallen);
    trimmed[vallen] = '\0';
    if (!str_equals_case_insensitive(trimmed, "null") && !str_equals_case_insensitive(trimmed, "(null)")) {
        strcpy(value, trimmed);
    } else {
        value[0] = '\0';
        vallen   = 0;
    }

    bool interpolates = interpolate && !str_is_empty(value) && str_starts_with(value, "${") && str_ends_with(value, "}");
    // TODO: loop over all vars after creating the HT, so we can reference values
    // of vars that at this point might not be present yet in the HT
    // TODO: add coloring to interpolated values when printing

    env_var_t *var = ht_get(ht, name);
    if (var != NULL) {
        if (comparing && var->cmpval == NULL) {
            var->cmpval    = value;
            var->cmpvallen = vallen;

            set_env_var_status(var);
        }

        if (interpolates) {
            var->interpolated = true;
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

        var->cmpval     = comparing ? value : NULL;
        var->name       = p;
        var->namelen    = namelen;
        var->status     = OK;
        var->val        = comparing ? NULL : value;
        var->vallen     = comparing ? 0 : vallen;
        var->cmpvallen  = comparing ? vallen : 0;
        var->interpolated = interpolates;

        ht_put(ht, name, var);
        set_env_var_status(var);
    }
}

bool is_interpolated(const char *str) {
    if (str == NULL) {
        return false;
    }
    return str_starts_with(str, "${") && str_ends_with(str, "}");
}

void interpolate_env_vars(hash_table_t *ht, const char **keyv, size_t keyc) {
    for (size_t i = 0; i < keyc; ++i) {
        const char *key = keyv[i];
        env_var_t  *var = ht_get(ht, key);
        if (!var || !var->interpolated) {
            continue;
        }
        bool val_is_interpolated = is_interpolated(var->val);
        if (val_is_interpolated) {
            char buf[var->vallen];
            str_slice(var->val, 2, var->vallen - 1, buf);
            env_var_t *ref = ht_get(ht, buf);
            if (ref) {
                var->val = ref->val ? strdup(ref->val) : NULL;
            }
        }
        bool cmpval_is_interpolated = is_interpolated(var->cmpval);
        if (cmpval_is_interpolated) {
            char buf[var->cmpvallen];
            str_slice(var->cmpval, 2, var->cmpvallen - 1, buf);
            env_var_t *ref = ht_get(ht, buf);
            if (ref) {
                var->cmpval = ref->cmpval ? strdup(ref->cmpval) : NULL;
            }
        }
    }
}

void set_env_var_status(env_var_t *var) {
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

int read_env_file(hash_table_t *ht,
                  const char   *path,
                  const char  **ignorev,
                  size_t        ignorec,
                  const char  **focusv,
                  size_t        focusc,
                  bool          comparing,
                  bool          interpolate) {
    assert(ht != NULL);
    assert(path != NULL);

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
        create_env_var_from_line(ht, buffer, ignorev, ignorec, focusv, focusc, comparing, interpolate);
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
    char *titleclr = NO_COLOR;
    pcolorlnf(titleclr, "%s", filename);
    pcolorlnf(titleclr, "%s", underline);
}

//=== command impl ===========================================================//
int handle_cmd(command_t *self) {
    char *target       = get_string_opt(self, "target");
    char *source       = get_string_opt(self, "source");
    char *ignore       = get_string_opt(self, "ignore");
    char *key          = get_string_opt(self, "key");
    char *truncate     = get_string_opt(self, "truncate");
    bool  missing      = get_bool_opt(self, "missing");
    bool  undefined    = get_bool_opt(self, "undefined");
    bool  divergent    = get_bool_opt(self, "divergent");
    bool  interpolate    = get_bool_opt(self, "interpolate");

    int   truncate_val = truncate ? atoi(truncate) : 0;
    if (truncate_val > 0) {
        truncate_val = max(truncate_val, 7);
    }
    bool   selective = missing || undefined || divergent;
    bool   comparing = source != NULL;

    size_t ignorec   = ignore ? str_count_char(ignore, ',') + 1 : 0;
    char  *ignorev[ignorec];
    str_split_by_delim(ignore, ',', ignorev, ignorec);

    size_t focusc = key ? str_count_char(key, ',') + 1 : 0;
    char  *focusv[focusc];
    str_split_by_delim(key, ',', focusv, focusc);

    hash_table_t ht = ht_create(50);

    if (comparing &&
        read_env_file(&ht, source, (const char **) ignorev, ignorec, (const char **) focusv, focusc, false, interpolate) >
            0) {
        free_strings(ignorev, ignorec);
        free_strings(focusv, focusc);
        ht_free(&ht);
        return EXIT_FAILURE;
    }

    if (read_env_file(&ht,
                      target,
                      (const char **) ignorev,
                      ignorec,
                      (const char **) focusv,
                      focusc,
                      comparing,
                      interpolate) > 0) {
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

    if (interpolate) {
        interpolate_env_vars(&ht, keys, vars);
    }

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
        env_var_t *var          = ht_get(&ht, keys[i]);
        bool       should_print = !selective || (var->status == MISSING && missing) ||
                            (var->status == UNDEFINED && undefined) || (var->status == DIVERGENT && divergent);

        if (!should_print) {
            continue;
        }

        first_colwidth  = max(first_colwidth, var->namelen);
        second_colwidth = max(second_colwidth, var->vallen);
        third_colwidth  = max(third_colwidth, var->cmpvallen);
    }

    if (truncate_val > 0) {
        second_colwidth = min((int) second_colwidth, truncate_val);
        third_colwidth  = min((int) third_colwidth, truncate_val);
    }

    for (size_t i = 0; i < vars; ++i) {
        env_var_t *var          = ht_get(&ht, keys[i]);
        bool       should_print = !selective || (var->status == MISSING && missing) ||
                            (var->status == UNDEFINED && undefined) || (var->status == DIVERGENT && divergent);

        if (should_print) {
            print_env_var(var, (int) first_colwidth, (int) second_colwidth, (int) third_colwidth, comparing);
        }
    }

    ht_free(&ht);

    return EXIT_SUCCESS;
}

//=== List ===================================================================//
int list(command_t *self) {
    return handle_cmd(self);
}

//=== Compare ================================================================//
int compare(command_t *self) {
    return handle_cmd(self);
}
