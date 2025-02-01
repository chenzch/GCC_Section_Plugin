#include "gcc-plugin.h"
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "tree-pass.h"
#include "plugin-version.h"
#include "diagnostic-core.h"
#include "c-family/c-pragma.h"
#define GENERATOR_FILE
#include "options.h"

// 插件初始化函数
int plugin_is_GPL_compatible;

GTY(()) const char *text_section_string;
GTY(()) const char *data_section_string;
GTY(()) const char *bss_section_string;
GTY(()) const char *rodata_section_string;

#include <stdlib.h>
#include <string.h>
#define CONTEXT_SECTION_INITIAL_SIZE (256)
class GTY(()) context_section {
  public:
    const char *add(const char *name) {
        // Binary search to find if name is already there
        // if yes, return ptr to string found
        // if not, check space; strdup input; insert string
        //          keep sorted; return ptr to string
        unsigned int s1, s2, n;
        int          r1, r2;
        // Take care of corner cases fisrt
        if (cnt == 0) {
            names[0] = ggc_strdup(name);
            cnt++;
            return names[0];
        } else if (cnt == 1) {
            r1 = strcmp(name, names[0]);
            if (r1 == 0) {
                return names[0];
            } else if (r1 > 0) {
                names[1] = ggc_strdup(name);
                cnt++;
                return names[1];
            } else {
                names[1] = names[0];
                names[0] = ggc_strdup(name);
                cnt++;
                return names[0];
            }
        }
        // We have 2 or more names on the stack already
        s1 = 0;
        s2 = cnt - 1;
        n  = (s1 + s2) >> 1;
        while (n != s1 && n != s2) {
            r1 = strcmp(name, names[n]);
            if (r1 == 0)
                return names[n];
            else if (r1 > 0)
                s1 = n;
            else
                s2 = n;
            n = (s1 + s2) >> 1;
        }
        r1 = strcmp(name, names[s1]);
        if (r1 == 0)
            return names[s1];
        r2 = strcmp(name, names[s2]);
        if (r2 == 0)
            return names[s2];
        // We have a new string, and it should be inserted between s1 and s2
        check_space();
        if (r1 < 0 && s1 == 0) {
            memmove(&names[1], &names[0], cnt * sizeof(char *));
            names[0] = ggc_strdup(name);
            cnt++;
            return names[0];
        }
        if (r2 > 0 && s2 == cnt - 1) {
            names[cnt] = ggc_strdup(name);
            cnt++;
            return names[cnt - 1];
        }
        // Use s2 here, it works in case s1 == s2
        memmove(&names[s2 + 1], &names[s2], (cnt - s2) * sizeof(char *));
        names[s2] = ggc_strdup(name);
        cnt++;
        return names[s2];
    }

    const char **names;
    unsigned int size;
    unsigned int cnt;

    context_section() {
        names = (const char **)xmalloc(CONTEXT_SECTION_INITIAL_SIZE * sizeof(const char *));
        size  = CONTEXT_SECTION_INITIAL_SIZE;
        cnt   = 0;
    }

    inline void check_space(void) {
        if (cnt == size) {
            // Need more space
            names = (const char **)xrealloc(names, size * sizeof(const char *));
            size *= 2;
        }
    }
};

context_section GTY(()) cs;

static void handle_section_pragma(cpp_reader *ARG_UNUSED(dummy)) {
    tree           x;
    location_t     loc;
    enum cpp_ttype token = pragma_lex(&x, &loc);
    if (token != CPP_NAME) {
        warning_at(loc, 0,
                   "missing [text|data|bss|rodata]"
                   " after %<#pragma GCC section%>");
        return;
    }

    const char *kind_string = IDENTIFIER_POINTER(x);

    token = pragma_lex(&x, &loc);
    if (token != CPP_STRING) {
        if (strcmp(kind_string, "text") == 0) {
            text_section_string = NULL;
        } else if (strcmp(kind_string, "data") == 0) {
            data_section_string = NULL;
        } else if (strcmp(kind_string, "bss") == 0) {
            bss_section_string = NULL;
        } else if (strcmp(kind_string, "rodata") == 0) {
            rodata_section_string = NULL;
        } else {
            warning_at(loc, 0,
                       "Unsupported section name [text|data|bss|rodata]"
                       " after %<#pragma GCC section%>");
        }
        return;
    }

    const char *section_string = cs.add(TREE_STRING_POINTER(x));

    if (strcmp(kind_string, "text") == 0) {
        if (strcmp(section_string, "default") == 0) {
            text_section_string = NULL;
        } else {
            text_section_string = section_string;
        }
    } else if (strcmp(kind_string, "data") == 0) {
        if (strcmp(section_string, "default") == 0) {
            data_section_string = NULL;
        } else {
            data_section_string = section_string;
        }
    } else if (strcmp(kind_string, "bss") == 0) {
        if (strcmp(section_string, "default") == 0) {
            bss_section_string = NULL;
        } else {
            bss_section_string = section_string;
        }
    } else if (strcmp(kind_string, "rodata") == 0) {
        if (strcmp(section_string, "default") == 0) {
            rodata_section_string = NULL;
        } else {
            rodata_section_string = section_string;
        }
    } else {
        warning_at(loc, 0,
                   "Unsupported section name [text|data|bss|rodata]"
                   " after %<#pragma GCC section%>");
        return;
    }
}

#define CURR_NAME  IDENTIFIER_POINTER(DECL_NAME(decl))
#define NO_SECTION (NULL == DECL_SECTION_NAME(decl))

static void decl_callback(void *event_data, void *data) {
    tree decl = (tree)event_data;
    if ((TREE_CODE(decl) == VAR_DECL)) {
        if (TREE_STATIC(decl)) {

            const char *section_string = NULL;

            if (TREE_READONLY(decl)) {
                // rodata
                if (rodata_section_string && NO_SECTION) {
                    section_string = rodata_section_string;
                }
            } else if (DECL_INITIAL(decl) != NULL_TREE) {
                // data
                if (data_section_string && NO_SECTION) {
                    section_string = data_section_string;
                }
            } else {
                // bss
                if (bss_section_string && NO_SECTION) {
                    section_string = bss_section_string;
                }
            }
            if (section_string != NULL) {
                if (flag_data_sections) {
                    char secname[256];
                    sprintf(&secname[0], "%s.%s", section_string, CURR_NAME);
                    set_decl_section_name(decl, cs.add(&secname[0]));
                    if (verbose_flag) {
                        fprintf(stderr, "Put %s into %s\n", CURR_NAME, &secname[0]);
                    }
                } else {
                    set_decl_section_name(decl, section_string);
                    if (verbose_flag) {
                        fprintf(stderr, "Put %s into %s\n", CURR_NAME, section_string);
                    }
                }
            }
        } else {
            // fprintf(stderr, "Non-static Decl node found %s, Readonly %d, Initial %d\n", CURR_NAME,
            //         TREE_READONLY(decl), DECL_INITIAL(decl) != NULL_TREE);
        }
    } else if ((TREE_CODE(decl) == CONST_DECL)) {
        fprintf(stderr, "Const Decl node found %s\n", CURR_NAME);
    }
}

static void function_callback(void *event_data, void *data) {
    tree decl = (tree)event_data;
    if (text_section_string && NO_SECTION) {
        if (flag_function_sections) {
            char secname[256];
            sprintf(&secname[0], "%s.%s", text_section_string, CURR_NAME);
            set_decl_section_name(decl, cs.add(&secname[0]));
            if (verbose_flag) {
                fprintf(stderr, "Put %s into %s\n", CURR_NAME, &secname[0]);
            }
        } else {
            set_decl_section_name(decl, text_section_string);
            if (verbose_flag) {
                fprintf(stderr, "Put %s into %s\n", CURR_NAME, text_section_string);
            }
        }
    }
}

static struct plugin_info my_plugin_info = {"1.0", "plugin to handle GCC section pragmas"};

int plugin_init(struct plugin_name_args *info, struct plugin_gcc_version *ver) {
    if (!plugin_default_version_check(ver, &gcc_version)) {
        return 1;
    }

    // fprintf(stderr, "Verbose %d, FuncName %d, DataName %d\n", verbose_flag, flag_function_sections,
    //         flag_data_sections);

    c_register_pragma("GCC", "section", handle_section_pragma);

    register_callback(info->base_name, PLUGIN_INFO, NULL, &my_plugin_info);
    register_callback(info->base_name, PLUGIN_FINISH_PARSE_FUNCTION, function_callback, NULL);
    register_callback(info->base_name, PLUGIN_FINISH_DECL, decl_callback, NULL);

    return 0;
}
