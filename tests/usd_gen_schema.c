#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_CLASSES 16
#define MAX_FIELDS 16

typedef struct {
    char type[32];
    char name[64];
    char default_val[64];
} schema_field_t;

typedef struct {
    char name[128];
    char inherit_type[64]; // "Typed" or "API"
    schema_field_t fields[MAX_FIELDS];
    int field_count;
} schema_class_t;

static void trim_string(char *str) {
    // Trim leading
    char *start = str;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
    // Trim trailing
    int len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        str[len - 1] = '\0';
        len--;
    }
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <input_schema.usda> <output_header.h>\n", argv[0]);
        return 1;
    }

    FILE *in = fopen(argv[1], "r");
    if (!in) {
        perror("Failed to open input schema");
        return 1;
    }

    schema_class_t classes[MAX_CLASSES];
    int class_count = 0;
    memset(classes, 0, sizeof(classes));

    char line[256];
    schema_class_t *current_class = NULL;
    bool in_class = false;

    while (fgets(line, sizeof(line), in)) {
        trim_string(line);
        if (strlen(line) == 0 || line[0] == '#') {
            continue;
        }

        if (strncmp(line, "class", 5) == 0) {
            if (class_count >= MAX_CLASSES) continue;
            current_class = &classes[class_count++];
            
            // Extract class name
            char *p1 = strchr(line, '"');
            if (p1) {
                char *p2 = strchr(p1 + 1, '"');
                if (p2) {
                    *p2 = '\0';
                    strcpy(current_class->name, p1 + 1);
                }
            }
            in_class = true;
            continue;
        }

        if (in_class && strstr(line, "inherits = </")) {
            char *p = strstr(line, "inherits = </");
            p += 13;
            char *p2 = strchr(p, '>');
            if (p2) {
                *p2 = '\0';
                strcpy(current_class->inherit_type, p);
            }
            continue;
        }

        if (in_class && line[0] == '}') {
            in_class = false;
            current_class = NULL;
            continue;
        }

        if (in_class && current_class && strchr(line, '=')) {
            if (current_class->field_count >= MAX_FIELDS) continue;
            schema_field_t *field = &current_class->fields[current_class->field_count++];
            
            char *eq = strchr(line, '=');
            *eq = '\0';
            char *val = eq + 1;
            trim_string(val);
            
            // Parse field type and name
            char *space = strchr(line, ' ');
            if (space) {
                *space = '\0';
                char *fname = space + 1;
                trim_string(line);
                trim_string(fname);
                strcpy(field->type, line);
                strcpy(field->name, fname);
            }
            
            // Parse default value
            strcpy(field->default_val, val);
        }
    }
    fclose(in);

    FILE *out = fopen(argv[2], "w");
    if (!out) {
        perror("Failed to open output header");
        return 1;
    }

    fprintf(out, "/* Generated dynamically by Auncient usd_gen_schema tool. Do not edit. */\n");
    fprintf(out, "#ifndef AUNCIENT_CACTUS_SCHEMA_H\n");
    fprintf(out, "#define AUNCIENT_CACTUS_SCHEMA_H\n\n");
    fprintf(out, "#include <string.h>\n\n");

    // Output dynamic structures
    for (int i = 0; i < class_count; i++) {
        schema_class_t *c = &classes[i];
        
        // Define struct name mapping camelCase/PascalCase to snake_case
        char struct_name[256];
        if (strcmp(c->name, "AuncientCactusSchema") == 0) {
            strcpy(struct_name, "usd_auncient_cactus_schema_t");
        } else if (strcmp(c->name, "AuncientTextureAPI") == 0) {
            strcpy(struct_name, "usd_auncient_texture_api_t");
        } else if (strcmp(c->name, "AuncientPhysicsAPI") == 0) {
            strcpy(struct_name, "usd_auncient_physics_api_t");
        } else if (strcmp(c->name, "AuncientMANNAPI") == 0) {
            strcpy(struct_name, "usd_auncient_mann_api_t");
        } else {
            snprintf(struct_name, sizeof(struct_name), "usd_%s_t", c->name);
        }

        fprintf(out, "typedef struct {\n");
        for (int j = 0; j < c->field_count; j++) {
            schema_field_t *f = &c->fields[j];
            if (strcmp(f->type, "string") == 0) {
                fprintf(out, "    char %s[32];\n", f->name);
            } else if (strcmp(f->type, "int") == 0) {
                fprintf(out, "    int %s;\n", f->name);
            } else {
                fprintf(out, "    float %s;\n", f->name);
            }
        }
        fprintf(out, "} %s;\n\n", struct_name);

        // Define initializer
        char init_name[256];
        if (strcmp(c->name, "AuncientCactusSchema") == 0) {
            strcpy(init_name, "usd_init_auncient_cactus_schema");
        } else if (strcmp(c->name, "AuncientTextureAPI") == 0) {
            strcpy(init_name, "usd_init_auncient_texture_api");
        } else if (strcmp(c->name, "AuncientPhysicsAPI") == 0) {
            strcpy(init_name, "usd_init_auncient_physics_api");
        } else if (strcmp(c->name, "AuncientMANNAPI") == 0) {
            strcpy(init_name, "usd_init_auncient_mann_api");
        } else {
            snprintf(init_name, sizeof(init_name), "usd_init_%s", c->name);
        }

        fprintf(out, "static inline void %s(%s *obj) {\n", init_name, struct_name);
        for (int j = 0; j < c->field_count; j++) {
            schema_field_t *f = &c->fields[j];
            if (strcmp(f->type, "string") == 0) {
                fprintf(out, "    strcpy(obj->%s, %s);\n", f->name, f->default_val);
            } else if (strcmp(f->type, "int") == 0) {
                fprintf(out, "    obj->%s = %s;\n", f->name, f->default_val);
            } else {
                fprintf(out, "    obj->%s = %sf;\n", f->name, f->default_val);
            }
        }
        fprintf(out, "}\n\n");

        // Getters and setters for compatibility
        for (int j = 0; j < c->field_count; j++) {
            schema_field_t *f = &c->fields[j];
            if (strcmp(f->type, "float") == 0 || strcmp(f->type, "int") == 0) {
                char prefix[128];
                if (strcmp(c->name, "AuncientCactusSchema") == 0) {
                    strcpy(prefix, "usd_cactus_schema");
                } else if (strcmp(c->name, "AuncientTextureAPI") == 0) {
                    strcpy(prefix, "usd_texture_api");
                } else if (strcmp(c->name, "AuncientMANNAPI") == 0) {
                    strcpy(prefix, "usd_mann_api");
                } else {
                    strcpy(prefix, "usd_physics_api");
                }
                
                const char *type_str = strcmp(f->type, "int") == 0 ? "int" : "float";
                
                fprintf(out, "static inline %s %s_get_%s(const %s *obj) {\n", type_str, prefix, f->name, struct_name);
                fprintf(out, "    return obj->%s;\n", f->name);
                fprintf(out, "}\n\n");
                
                fprintf(out, "static inline void %s_set_%s(%s *obj, %s val) {\n", prefix, f->name, struct_name, type_str);
                fprintf(out, "    obj->%s = val;\n", f->name);
                fprintf(out, "}\n\n");
            }
        }
    }

    fprintf(out, "#endif /* AUNCIENT_CACTUS_SCHEMA_H */\n");
    fclose(out);

    return 0;
}
