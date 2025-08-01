#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "code_gen/defines.h"
#include "code_gen/class_parser.h"
#include "code_gen/string_utils.h"

// Private data
const char* line_ends[] = { ";", "{", "}", "public:", "private:", "protected:", };
const int line_end_count = sizeof(line_ends) / sizeof(const char*);

int main(int argc, char** argv) {
    // Search args for input and output files
    const char* filename = NULL;
    const char* output_path = NULL;
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0) {
            output_path = argv[i + 1];
            i++;
        }
        if (strcmp(argv[i], "file") == 0) {
            filename = argv[i + 1];
            i++;
        }
    }

    // Ensure input and output paths were found, return error if not
    if (output_path == NULL) {
        return -1;
    }
    if (filename == NULL) {
        return -1;
    }

    // Open file
    FILE* input_file = fopen(filename, "r");
    if (!input_file) {
        return -1;
    }

    // Get input file length
    fseek(input_file, 0, SEEK_END);
    long file_length = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);

    // Allocate text buffers
    char* text = (char*)malloc(file_length);
    char* line_buffer = (char*)malloc(file_length); // Not required to be this long but is easy and relatively cheap

    // Read all text from file
    fread(text, file_length, 1, input_file);
    fclose(input_file);
    memset(line_buffer, 0, file_length);

    // Open type header file
    FILE* out_file = fopen(output_path, "w");

    // Read each line
    char class_heirarchy[16][256] = {}; // Need to save a heirarchy for subclasses to work
    int class_indents[16] = {};
    int class_index = -1;
    int indent = 0;


    class_t class = {};
    for (int i = 0, offset = 0; i < file_length; i++) {
        const char c = text[i];
        if (c == '\n') {
            continue;
        }

        // Skip whitespace
        if (offset == 0 && (c == ' ' || c == '\t')) {
            continue;
        }

        // Modify indent
        if (c == '{') {
            indent++;
        } else if (c == '}') {
            indent--;
            if (indent <= class_indents[class_index]) {
                if (class_index >= 0) {
                    class_write_exports(&class, filename, out_file);
                    strcpy(class.name, class_heirarchy[class_index - 1]);
                }
                class_index--;
            }
        }

        // Next keyword should be a class
        if (strncmp(text + i, "class ", 6) == 0) {
            int class_name_length = 0;
            for (int j = 0; j < file_length - i; j++) {
                char class_char = text[i + sizeof("class") + j];

                // Check if current char is not a valid character for a classname
                if (!char_valid_in_name(class_char)) {
                    class_name_length = j;
                    break;
                }
            }

            class_index++;
            class_indents[class_index] = indent;
            class_heirarchy[class_index][class_name_length] = 0;
            memcpy(class_heirarchy[class_index], text + i + sizeof("class"), class_name_length);
            memcpy(class.name, text + i + sizeof("class"), class_name_length);
            printf("Found class '%s'\n", class_heirarchy[class_index]);
            class.name[class_name_length] = 0;

            i += sizeof("class") + class_name_length;
            continue;
        }

        if (strncmp(text + i, "GD_EXPORT", 9) == 0) {
            offset = 0;
        }

        // Check for line end and parse if in class declaration
        b8 is_line_end = false;
        for (int j = 0; j < line_end_count; j++) {
            int end_length = strlen(line_ends[j]);
            if (file_length - i < end_length) {
                continue;
            }

            if (strncmp(text + i, line_ends[j], end_length) == 0) {
                i += end_length;
                is_line_end = true;
                line_buffer[offset++] = 0;
                offset = 0;

                if (class_index >= 0) {
                    class_parse_line(&class, line_buffer);
                }
                break;
            }
        }
        if (is_line_end) {
            continue;
        }

        // Build line
        line_buffer[offset++] = c;
    }

    fclose(out_file);
}
