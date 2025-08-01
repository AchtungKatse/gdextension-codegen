#include "code_gen/class_parser.h"
#include "code_gen/string_utils.h"
#include "code_gen/logging.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// Private data
#define EXPORT_TAG "GD_EXPORT"
#define EXPORT_TAG_SIZE sizeof(EXPORT_TAG)

#define GROUP_TAG "GD_GROUP"
#define GROUP_TAG_SIZE sizeof(GROUP_TAG)

#define SUBGROUP_TAG "GD_SUBGROUP"
#define SUBGROUP_TAG_SIZE sizeof(SUBGROUP_TAG)

// Private structs

// Private functions
void add_property(export_t* export, export_type_t type, const char* type_str, FILE* file);
void write_impl(class_t* class, const char* source_file, FILE* file);
void parse_group(const char* line, char** out_name, char** out_prefix);

// Function Impls
void class_parse_line(class_t* class, const char* line) {
    // Ignore comments
    if (strcmp(line, "//") == 0) {
        return;
    }
        log_debug("Parsing line: '%s'", line);

    // Get groups
    if (strncmp(line, GROUP_TAG, GROUP_TAG_SIZE - 1) == 0) {
        group_t* group = &class->groups[class->group_count++];
        group->start_export_index = class->export_count;
        parse_group(line, (char**)&group->name, (char**)&group->prefix);

        log_debug("Found group. Name: '%s'; Prefix, '%s'", group->name, group->prefix);
    }
    if (strncmp(line, SUBGROUP_TAG, SUBGROUP_TAG_SIZE - 1) == 0) {
        group_t* group = &class->subgroups[class->subgroup_count++];
        group->start_export_index = class->export_count;
        parse_group(line, (char**)&group->name, (char**)&group->prefix);

        log_debug("Found sub group. Name: '%s'; Prefix, '%s'", group->name, group->prefix);
    }

    // Get line length
    int line_length = strlen(line);
    const int line_start = EXPORT_TAG_SIZE; // Data starts after the tag

    // Check for bind methods
    for (int i = line_start; i < line_length; i++) {
        const char c = line[i];
        if (c != '_') {
            continue;
        }

        if (strncmp(line + i, "_bind_methods", sizeof("_bind_methods") - 1) == 0) {
            class->generate = 1;
            break;
        }
    }

    // Check if the line is an export
    if (strncmp(line, EXPORT_TAG, EXPORT_TAG_SIZE - 1) != 0) {
        return;
    }

    // Parse out the type
    char type[2048] = {}; // Temporary buffer that can be modified
    for (int i = line_start; i < line_length; i++) {
        const char c = line[i];

        // Searches until the comment in the export tag
        // GD_EXPORT(float, time) = 0;
        //                ^ looks for this since 'GD_EXPORT(' is trimmed
        if (c == ',') {
            memcpy(type, line + line_start, i - line_start);
            break;
        }
    }

    // Parese out the name
    b8 found_name_start = false;
    char name[2048] = {};
    for (int i = line_start, name_start = 0; i < line_length; i++) {
        char c = line[i];

        // Finds the start if the name
        if (c == ',') {
            found_name_start = true;
            // Read forward until a character that cannot be in a variable name is found
            while (!char_valid_in_name(c)) {
                c = line[i++];
            }

            // Decrement since c will be an invalid character
            i--;
            name_start = i;
            continue;
        }

        // Search until the end of name is found
        if (!char_valid_in_name(c) && found_name_start) {
            int name_length = i - name_start;
            memcpy(name, line + name_start, name_length);
            name[name_length + 1] = 0;
            break;
        }
    }

    // Ensure that both a name and type were found
    if (strcmp(name, "") == 0 || strcmp(type, "") == 0) {
        log_fatal("Failed to get export data (Name: '%s', Type: '%s'. Line: '%s'", name, type, line);
    }

    // Create a new export and copy data
    export_t export = {};
    strcpy(export.name, name);
    strcpy(export.type, type);

    // Add exports to the class
    if (class->export_count >= MAX_CLASS_EXPORTS) {
        log_fatal("Cannot have more than %d exports in a class.", MAX_CLASS_EXPORTS);
    }
    class->exports[class->export_count++] = export;
}


void class_write_exports(class_t* class, const char* source_path, FILE* file) {
    // Don't write anything if there's no data to generate
    // This prevents _bind_methods() from being generated in classes it shouldn't be
    if (class->export_count <= 0 && !class->generate) {
        write_text("// No data found for class", file);
        return;
    }

    // Add includes
    char buffer[2048] = {};
    snprintf(buffer, sizeof(buffer), "#include \"%s\"\nusing namespace godot;\n", source_path);
    write_text(buffer, file);

    // Add Bind methods
    write_text("namespace wander {\n", file);
    snprintf(buffer, sizeof(buffer), "\n\tvoid %s::_bind_methods() {\n", class->name);
    write_text(buffer, file);

    // Write all exports
    for (int i = 0; i < class->export_count; i++) {
        // Write groups 
        for (int j = 0; j < class->group_count; j++) {
            group_t group = class->groups[j];
            if (group.start_export_index == i) {
                snprintf(buffer, sizeof(buffer), "\t\tADD_GROUP(\"%s\", \"%s\");\n", group.name, group.prefix);
                write_text(buffer, file);
            }
        }

        for (int j = 0; j < class->subgroup_count; j++) {
            group_t group = class->subgroups[j];
            if (group.start_export_index == i) {
                snprintf(buffer, sizeof(buffer), "\t\tADD_SUBGROUP(\"%s\", \"%s\");\n", group.name, group.prefix);
                write_text(buffer, file);
            }
        }

        // Write export getter and setter reflection info
        export_t* export = &class->exports[i];
        snprintf(buffer, sizeof(buffer), "\t\tClassDB::bind_method(D_METHOD(\"get_%s\"), &%s::get_%s);\n", export->name, class->name, export->name);
        write_text(buffer, file);

        snprintf(buffer, sizeof(buffer), "\t\tClassDB::bind_method(D_METHOD(\"set_%s\", \"%s\"), &%s::set_%s);\n", export->name, export->name, class->name, export->name);
        write_text(buffer, file);

        // Remove all keywords from type
        char type[256] = {}; // New keywordless type
        int type_offset = 0;
        strcpy(type, export->type);

        for (int k = 0; k < keyword_count; k++) {
            if (strncmp(type, keywords[k], strlen(keywords[k]) - 1) == 0) {
                type_offset += strlen(keywords[k]) + 1;
            }
        }

        // Trim excess info from type and store info
        int type_len = strlen(type);
        b8 trimmed = false;
        b8 is_resource = false;
        b8 is_typed_array = false;
        while (!trimmed) {
            trimmed = true;

            // Remove godot::
            if (strncmp(type + type_offset, "godot::", 7) == 0) {
                type_offset += strlen("godot::");
                trimmed = false;
            }

            // Check for typed arrays
            if (strncmp(type + type_offset, "TypedArray<", 11) == 0) {
                type_offset += strlen("TypedArray<");
                is_typed_array = true;
                for (int j = type_len; j >= type_offset; j--) {
                    if (type[j] == '>') {
                        type_len = j;
                        break;
                    }
                }
                trimmed = false;
            }

            // Remove Ref<> and mark as resource
            if (strncmp(type + type_offset, "Ref<", 4) == 0) {
                type_offset += strlen("Ref<");
                is_resource = true;

                for (int j = type_len; j >= type_offset; j--) {
                    if (type[j] == '>') {
                        type_len = j;
                        break;
                    }
                }
                trimmed = false;
            }
        }

        // Check for nodes (Needs to be a pointer)
        b8 is_ptr = false;
        for (int j = 0; j < type_len; j++) {
            if (type[j] == '*') {
                type[j] = 0;
                is_ptr = true;
                break;
            }
        }

        // Get export type from type string
        // NOTE: Messy due to lack of string parsing in C.
        // Also requires branches depending on data type and variant type
        const char* _type = type + type_offset;
        type[type_len] = 0;

        export_type_t export_type = EXPORT_TYPE_NULL;
        if (is_typed_array) {
            if (is_resource) {
                export_type = EXPORT_TYPE_TYPED_RESOURCE_ARRAY;
            } else {
                export_type = EXPORT_TYPE_TYPED_ARRAY;
            }
        } else if (is_resource) {
            export_type = EXPORT_TYPE_RESOURCE;
        } else if (is_ptr) {
            export_type = EXPORT_TYPE_NODE;
        } else {
            for (int i = 0; i < export_type_alias_count; i++) {
                const export_type_alias_t* alias = &export_type_aliases[i];

                b8 found = false;
                for (int j = 0; j < alias->alias_count; j++) {
                    if (strcmp(_type, alias->aliases[j]) == 0) {
                        export_type = alias->type;
                        found = true;
                        break;
                    }
                }

                if (found) {
                    break;
                }
            }
        }

        if (export_type == EXPORT_TYPE_NULL) {
            log_fatal("Unrecognized export type '%s' ('%s' / '%s')\n", type, export->type, _type);
            abort();
        }

        add_property(export, export_type, _type, file);
    }

    // Close bind methods function
    write_text("\n\t}\n", file);

    // Write getter and setter functions
    for (int i = 0; i < class->export_count; i++) {
        export_t* export = &class->exports[i];
        snprintf(buffer, sizeof(buffer), 
                "\tvoid %s::set_%s(%s value) {\n\t\t%s = value; \n\t}\n\t%s %s::get_%s() const {\n\t\treturn %s;\n\t}\n",
                class->name,
                export->name, export->type, export->name,
                export->type, class->name, export->name, export->name);

        write_text(buffer, file);
    }

    // Close namespace
    write_text("}\n", file);
}

// Private functions
// Adds properties to godot editor
void add_property(export_t* export, export_type_t type, const char* type_str, FILE* file) {
    const char* variant_string = NULL;
    char buffer[2048] = {};
    switch (type) {
        case EXPORT_TYPE_RESOURCE:
            snprintf(buffer, sizeof(buffer), "\t\tADD_PROPERTY(PropertyInfo(Variant::OBJECT, \"%s\", PROPERTY_HINT_RESOURCE_TYPE, \"%s\"), \"set_%s\", \"get_%s\");\n", export->name, type_str, export->name, export->name);
            write_text(buffer, file);
            return;
        case EXPORT_TYPE_DOUBLE:
        case EXPORT_TYPE_FLOAT:
            variant_string = "FLOAT";
            break;
        case EXPORT_TYPE_NULL:
            log_error("Cannot add null property.");
            abort();
            return;
        case EXPORT_TYPE_INT:
            variant_string = "INT";
            break;
        case EXPORT_TYPE_NODE:
            snprintf(buffer, sizeof(buffer), "\t\tADD_PROPERTY(PropertyInfo(Variant::OBJECT, \"%s\", PROPERTY_HINT_NODE_TYPE, \"%s\"), \"set_%s\", \"get_%s\");\n", export->name, type_str, export->name, export->name);
            write_text(buffer, file);
            return;
        case EXPORT_TYPE_STRING:
            variant_string = "STRING";
            break;
        case EXPORT_TYPE_TYPED_ARRAY:
            snprintf(buffer, 
                    sizeof(buffer),
                    "\t\tADD_PROPERTY(PropertyInfo(Variant::ARRAY, \"%s\", PROPERTY_HINT_TYPE_STRING, String::num(Variant::OBJECT) + \"/\" + String::num(PROPERTY_HINT_RESOURCE_TYPE) + \":%s\"), \"set_%s\", \"get_%s\");\n", 
                    export->name,
                    type_str,
                    export->name,
                    export->name);
            write_text(buffer, file);
            return;
        case EXPORT_TYPE_TYPED_RESOURCE_ARRAY:
            snprintf(buffer, 
                    sizeof(buffer),
                    "\t\tADD_PROPERTY(PropertyInfo(Variant::ARRAY, \"%s\", PROPERTY_HINT_TYPE_STRING, String::num(Variant::OBJECT) + \"/\" + String::num(PROPERTY_HINT_RESOURCE_TYPE) + \":%s\"), \"set_%s\", \"get_%s\");\n", 
                    export->name,
                    type_str,
                    export->name,
                    export->name);
            write_text(buffer, file);
            return;
        case EXPORT_TYPE_BOOL:
            variant_string = "BOOL";
            break;
        case EXPORT_TYPE_VECTOR2:
            variant_string = "VECTOR2";
            break;
        case EXPORT_TYPE_VECTOR2I:
            variant_string = "VECTOR2I";
            break;
        case EXPORT_TYPE_RECT2:
            variant_string = "RECT2";
            break;
        case EXPORT_TYPE_RECT2I:
            variant_string = "RECT2I";
            break;
        case EXPORT_TYPE_VECTOR3:
            variant_string = "VECTOR3";
            break;
        case EXPORT_TYPE_VECTOR3I:
            variant_string = "VECTOR3I";
            break;
        case EXPORT_TYPE_TRANSFORM2D:
            variant_string = "TRANSFORM2D";
            break;
        case EXPORT_TYPE_VECTOR4:
            variant_string = "VECTOR4";
            break;
        case EXPORT_TYPE_VECTOR4I:
            variant_string = "VECTOR4I";
            break;
        case EXPORT_TYPE_PLANE:
            variant_string = "PLANE";
            break;
        case EXPORT_TYPE_QUATERNION:
            variant_string = "QUATERNION";
            break;
        case EXPORT_TYPE_AABB:
            variant_string = "AABB";
            break;
        case EXPORT_TYPE_BASIS:
            variant_string = "BASIS";
            break;
        case EXPORT_TYPE_TRANSFORM3D:
            variant_string = "TRANSFORM3D";
            break;
        case EXPORT_TYPE_PROJECTION:
            variant_string = "PROJECTION";
            break;
        case EXPORT_TYPE_COLOR:
            variant_string = "COLOR";
            break;
        case EXPORT_TYPE_STRING_NAME:
            variant_string = "STRING_NAME";
            break;
        case EXPORT_TYPE_NODE_PATH:
            variant_string = "NODE_PATH";
            break;
        case EXPORT_TYPE_RID:
            variant_string = "RID";
            break;
        case EXPORT_TYPE_OBJECT:
            variant_string = "OBJECT";
            break;
        case EXPORT_TYPE_CALLABLE:
            variant_string = "CALLABLE";
            break;
        case EXPORT_TYPE_SIGNAL:
            variant_string = "SIGNAL";
            break;
        case EXPORT_TYPE_DICTIONARY:
            variant_string = "DICTIONARY";
            break;
        case EXPORT_TYPE_ARRAY:
            variant_string = "ARRAY";
            break;
        case EXPORT_TYPE_PACKED_BYTE_ARRAY:
            variant_string = "PACKED_BYTE_ARRAY";
            break;
        case EXPORT_TYPE_PACKED_INT32_ARRAY:
            variant_string = "PACKED_INT32_ARRAY";
            break;
        case EXPORT_TYPE_PACKED_INT64_ARRAY:
            variant_string = "PACKED_INT64_ARRAY";
            break;
        case EXPORT_TYPE_PACKED_FLOAT32_ARRAY:
            variant_string = "PACKED_FLOAT32_ARRAY";
            break;
        case EXPORT_TYPE_PACKED_FLOAT64_ARRAY:
            variant_string = "PACKED_FLOAT64_ARRAY";
            break;
        case EXPORT_TYPE_PACKED_STRING_ARRAY:
            variant_string = "PACKED_STRING_ARRAY";
            break;
        case EXPORT_TYPE_PACKED_VECTOR2_ARRAY:
            variant_string = "PACKED_VECTOR2_ARRAY";
            break;
        case EXPORT_TYPE_PACKED_VECTOR3_ARRAY:
            variant_string = "PACKED_VECTOR3_ARRAY";
            break;
        case EXPORT_TYPE_PACKED_COLOR_ARRAY:
            variant_string = "PACKED_COLOR_ARRAY";
            break;
        case EXPORT_TYPE_PACKED_VECTOR4_ARRAY:
            variant_string = "PACKED_VECTOR4_ARRAY";
            break;
    }

    // Writes variants since they only change one string
    if (variant_string) {
        snprintf(buffer, sizeof(buffer), "\t\tADD_PROPERTY(PropertyInfo(Variant::%s, \"%s\"), \"set_%s\", \"get_%s\");\n", variant_string, export->name, export->name, export->name);
        write_text(buffer, file);
    }
}

void parse_group(const char* line, char** out_name, char** out_prefix) {
    // Find (
    int line_length = strlen(line);
    int start = 0;

    for (int i = 0; i < line_length; i++) {
        const char c = line[i];
        if (c == '(') {
            start = i + 1;
            break;
        }
    }

    // Find ',' (marks end of name)
    int name_end = start;
    for (int i = start; i < line_length; i++) {
        const char c = line[i];
        if (c == ',') {
            name_end = i - 1;
            break;
        }
    }

    // Find start of prefix
    int prefix_start = name_end + 2;
    for (int i = prefix_start; i < line_length; i++) {
        const char c = line[i];
        if (c != ' ') {
            prefix_start = i;
            break;
        }
    }

    // Find end of prefix
    int prefix_end = prefix_start;
    for (int i = prefix_start; i < line_length; i++) {
        const char c = line[i];
        if (c == ')') {
            prefix_end = i - 1;
            break;
        }
    }

    // Create strings
    int name_length = name_end - start + 2;
    int prefix_length = prefix_end - prefix_start + 2;

    *out_name = malloc(name_length);
    *out_prefix = malloc(prefix_length);

    strncpy(*out_name, line + start, name_length);
    strncpy(*out_prefix, line + prefix_start, prefix_length);

    (*out_name)[name_length - 1] = 0;
    (*out_prefix)[prefix_length - 1] = 0;
}
