#include "code_gen/string_utils.h"
#include "code_gen/class_parser.h"
#include "code_gen/defines.h"

#include <string.h>

void write_text(const char* text, FILE* file) {
    fwrite(text, strlen(text), 1, file);
}

b8 char_valid_in_name(char c) {
    return  (c >= 'a' && c <= 'z') || 
            (c >= 'A' && c <= 'Z') || 
            (c >= '0' && c <= '9') || 
             c == '_' || c == ':';
}

