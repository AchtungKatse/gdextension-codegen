#pragma once

#include <stdio.h>
static const char* keywords[] = {
    "class",
    "struct",
    "const",
};
static const int keyword_count = sizeof(keywords) / sizeof(const char*);

typedef struct export {
    char type[128];
    char name[64];
} export_t;

typedef enum export_type {
    EXPORT_TYPE_NULL,

    EXPORT_TYPE_DOUBLE,
    EXPORT_TYPE_RESOURCE,
    EXPORT_TYPE_NODE,
    EXPORT_TYPE_TYPED_ARRAY,
    EXPORT_TYPE_TYPED_RESOURCE_ARRAY,

    // Variant types
    // atomic types
    EXPORT_TYPE_BOOL,
    EXPORT_TYPE_INT,
    EXPORT_TYPE_FLOAT,
    EXPORT_TYPE_STRING,

    // math types
    EXPORT_TYPE_VECTOR2,
    EXPORT_TYPE_VECTOR2I,
    EXPORT_TYPE_RECT2,
    EXPORT_TYPE_RECT2I,
    EXPORT_TYPE_VECTOR3,
    EXPORT_TYPE_VECTOR3I,
    EXPORT_TYPE_TRANSFORM2D,
    EXPORT_TYPE_VECTOR4,
    EXPORT_TYPE_VECTOR4I,
    EXPORT_TYPE_PLANE,
    EXPORT_TYPE_QUATERNION,
    EXPORT_TYPE_AABB,
    EXPORT_TYPE_BASIS,
    EXPORT_TYPE_TRANSFORM3D,
    EXPORT_TYPE_PROJECTION,

    // misc types
    EXPORT_TYPE_COLOR,
    EXPORT_TYPE_STRING_NAME,
    EXPORT_TYPE_NODE_PATH,
    EXPORT_TYPE_RID,
    EXPORT_TYPE_OBJECT,
    EXPORT_TYPE_CALLABLE,
    EXPORT_TYPE_SIGNAL,
    EXPORT_TYPE_DICTIONARY,
    EXPORT_TYPE_ARRAY,

    // typed arrays
    EXPORT_TYPE_PACKED_BYTE_ARRAY,
    EXPORT_TYPE_PACKED_INT32_ARRAY,
    EXPORT_TYPE_PACKED_INT64_ARRAY,
    EXPORT_TYPE_PACKED_FLOAT32_ARRAY,
    EXPORT_TYPE_PACKED_FLOAT64_ARRAY,
    EXPORT_TYPE_PACKED_STRING_ARRAY,
    EXPORT_TYPE_PACKED_VECTOR2_ARRAY,
    EXPORT_TYPE_PACKED_VECTOR3_ARRAY,
    EXPORT_TYPE_PACKED_COLOR_ARRAY,
    EXPORT_TYPE_PACKED_VECTOR4_ARRAY,
} export_type_t;

typedef struct group {
    int start_export_index;
    const char* name;
    const char* prefix;
} group_t;

#define MAX_CLASS_EXPORTS 1024
#define MAX_GROUPS 128
#define MAX_SUBGROUPS 128
typedef struct class {
    char name[64];
    export_t exports[MAX_CLASS_EXPORTS];
    group_t groups[MAX_GROUPS];
    group_t subgroups[MAX_SUBGROUPS];
    int export_count;
    int group_count;
    int subgroup_count;
    int generate;
} class_t;

typedef struct export_type_alias {
    export_type_t type;
    int alias_count;
    const char** aliases;
} export_type_alias_t;

const static export_type_alias_t export_type_aliases[] = {
    (export_type_alias_t) {
        .type = EXPORT_TYPE_NULL,
        .alias_count = 1,
        .aliases = (const char*[]) {
        },
    },
    (export_type_alias_t) {
        .type = EXPORT_TYPE_NULL,
        .alias_count = 1,
        .aliases = (const char*[]) {
            "null",
        },
    },

    (export_type_alias_t) {
        .type = EXPORT_TYPE_DOUBLE,
        .alias_count = 2,
        .aliases = (const char*[]) {
            "double",
            "f64",
        },
    },
    // (export_type_alias_t) {
    //     .type = EXPORT_TYPE_RESOURCE,
    //     .alias_count = 1,
    //     .aliases = (const char*[]) {
    //         "resource",
    //     },
    // },
    // (export_type_alias_t) {
    //     .type = EXPORT_TYPE_NODE,
    //     .alias_count = 1,
    //     .aliases = (const char*[]) {
    //         "node",
    //     },
    // },
    // (export_type_alias_t) {
    //     .type = EXPORT_TYPE_TYPED_ARRAY,
    //     .alias_count = 1,
    //     .aliases = (const char*[]) {
    //         "typed_array",
    //     },
    // },
    // (export_type_alias_t) {
    //     .type = EXPORT_TYPE_TYPED_RESOURCE_ARRAY,
    //     .alias_count = 1,
    //     .aliases = (const char*[]) {
    //         "typed_resource_array",
    //     },
    // },

    // Variant types
    // atomic types
    (export_type_alias_t) {
        .type = EXPORT_TYPE_BOOL,
        .alias_count = 1,
        .aliases = (const char*[]) {
            "bool",
        },
    },
    (export_type_alias_t) {
        .type = EXPORT_TYPE_INT,
        .alias_count = 3,
        .aliases = (const char*[]) {
            "int",
            "u32",
            "s32",
        },
    },
    (export_type_alias_t) {
        .type = EXPORT_TYPE_FLOAT,
        .alias_count = 2,
        .aliases = (const char*[]) {
            "float",
            "f32",
        },
    },
    (export_type_alias_t) {
        .type = EXPORT_TYPE_STRING,
        .alias_count = 1,
        .aliases = (const char*[]) {
            "String",
        },
    },

    // math types
    (export_type_alias_t) {
        .type = EXPORT_TYPE_VECTOR2,
        .alias_count = 1,
        .aliases = (const char*[]) {
            "Vector2",
        },
    },
    (export_type_alias_t) {
        .type = EXPORT_TYPE_VECTOR2I,
        .alias_count = 1,
        .aliases = (const char*[]) {
            "Vector2i",
        },
    },
    (export_type_alias_t) {
        .type = EXPORT_TYPE_RECT2,
        .alias_count = 1,
        .aliases = (const char*[]) {
            "Rect2",
        },
    },
    (export_type_alias_t) {
        .type = EXPORT_TYPE_RECT2I,
        .alias_count = 1,
        .aliases = (const char*[]) {
            "Rect2i",
        },
    },
    (export_type_alias_t) {
        .type = EXPORT_TYPE_VECTOR3,
        .alias_count = 1,
        .aliases = (const char*[]) {
            "Vector3",
        },
    },
    (export_type_alias_t) {
        .type = EXPORT_TYPE_VECTOR3I,
        .alias_count = 1,
        .aliases = (const char*[]) {
            "Vector3i",
        },
    },
    (export_type_alias_t) {
        .type = EXPORT_TYPE_TRANSFORM2D,
        .alias_count = 1,
        .aliases = (const char*[]) {
            "Transform2D",
        },
    },
    (export_type_alias_t) {
        .type = EXPORT_TYPE_VECTOR4,
        .alias_count = 1,
        .aliases = (const char*[]) {
            "Vector4",
        },
    },
    (export_type_alias_t) {
        .type = EXPORT_TYPE_VECTOR4I,
        .alias_count = 1,
        .aliases = (const char*[]) {
            "Vector4i",
        },
    },
    (export_type_alias_t) {
        .type = EXPORT_TYPE_PLANE,
        .alias_count = 1,
        .aliases = (const char*[]) {
            "Plane",
        },
    },
    (export_type_alias_t) {
        .type = EXPORT_TYPE_QUATERNION,
        .alias_count = 1,
        .aliases = (const char*[]) {
            "Quaternion",
        },
    },
    (export_type_alias_t) {
        .type = EXPORT_TYPE_AABB,
        .alias_count = 1,
        .aliases = (const char*[]) {
            "AABB",
        },
    },
    (export_type_alias_t) {
        .type = EXPORT_TYPE_BASIS,
        .alias_count = 1,
        .aliases = (const char*[]) {
            "Basis",
        },
    },
    (export_type_alias_t) {
        .type = EXPORT_TYPE_TRANSFORM3D,
        .alias_count = 1,
        .aliases = (const char*[]) {
            "Transform3D",
        },
    },
    (export_type_alias_t) {
        .type = EXPORT_TYPE_PROJECTION,
        .alias_count = 1,
        .aliases = (const char*[]) {
            "Projection",
        },
    },

    // misc types
    (export_type_alias_t) {
        .type = EXPORT_TYPE_COLOR,
        .alias_count = 1,
        .aliases = (const char*[]) {
            "Color",
        },
    },
    (export_type_alias_t) {
        .type = EXPORT_TYPE_STRING_NAME,
        .alias_count = 1,
        .aliases = (const char*[]) {
            "StringName",
        },
    },
    (export_type_alias_t) {
        .type = EXPORT_TYPE_NODE_PATH,
        .alias_count = 1,
        .aliases = (const char*[]) {
            "NodePath",
        },
    },
    (export_type_alias_t) {
        .type = EXPORT_TYPE_RID,
        .alias_count = 1,
        .aliases = (const char*[]) {
            "RID",
        },
    },
    (export_type_alias_t) {
        .type = EXPORT_TYPE_OBJECT,
        .alias_count = 1,
        .aliases = (const char*[]) {
            "Object",
        },
    },
    (export_type_alias_t) {
        .type = EXPORT_TYPE_CALLABLE,
        .alias_count = 1,
        .aliases = (const char*[]) {
            "Callable",
        },
    },
    (export_type_alias_t) {
        .type = EXPORT_TYPE_SIGNAL,
        .alias_count = 1,
        .aliases = (const char*[]) {
            "Signal",
        },
    },
    (export_type_alias_t) {
        .type = EXPORT_TYPE_DICTIONARY,
        .alias_count = 1,
        .aliases = (const char*[]) {
            "Dictionary",
        },
    },
    (export_type_alias_t) {
        .type = EXPORT_TYPE_ARRAY,
        .alias_count = 1,
        .aliases = (const char*[]) {
            "Array",
        },
    },

    // typed arrays
    (export_type_alias_t) {
        .type = EXPORT_TYPE_PACKED_BYTE_ARRAY,
        .alias_count = 1,
        .aliases = (const char*[]) {
            "PackedByteArray",
        },
    },
    (export_type_alias_t) {
        .type = EXPORT_TYPE_PACKED_INT32_ARRAY,
        .alias_count = 1,
        .aliases = (const char*[]) {
            "PackedInt32Array",
        },
    },
    (export_type_alias_t) {
        .type = EXPORT_TYPE_PACKED_INT64_ARRAY,
        .alias_count = 1,
        .aliases = (const char*[]) {
            "PackedInt64Array",
        },
    },
    (export_type_alias_t) {
        .type = EXPORT_TYPE_PACKED_FLOAT32_ARRAY,
        .alias_count = 1,
        .aliases = (const char*[]) {
            "PackedFloat32Array",
        },
    },
    (export_type_alias_t) {
        .type = EXPORT_TYPE_PACKED_FLOAT64_ARRAY,
        .alias_count = 1,
        .aliases = (const char*[]) {
            "PackedFloat64Array",
        },
    },
    (export_type_alias_t) {
        .type = EXPORT_TYPE_PACKED_STRING_ARRAY,
        .alias_count = 1,
        .aliases = (const char*[]) {
            "PackedStringArray",
        },
    },
    (export_type_alias_t) {
        .type = EXPORT_TYPE_PACKED_VECTOR2_ARRAY,
        .alias_count = 1,
        .aliases = (const char*[]) {
            "PackedVector2Array",
        },
    },
    (export_type_alias_t) {
        .type = EXPORT_TYPE_PACKED_VECTOR3_ARRAY,
        .alias_count = 1,
        .aliases = (const char*[]) {
            "PackedVector3Array",
        },
    },
    (export_type_alias_t) {
        .type = EXPORT_TYPE_PACKED_COLOR_ARRAY,
        .alias_count = 1,
        .aliases = (const char*[]) {
            "PackedColorArray",
        },
    },
    (export_type_alias_t) {
        .type = EXPORT_TYPE_PACKED_VECTOR4_ARRAY,
        .alias_count = 1,
        .aliases = (const char*[]) {
            "PackedVector4Array",
        },
    },
};
const static int export_type_alias_count = sizeof(export_type_aliases) / sizeof(export_type_alias_t);

void class_write_exports(class_t* class, const char* source_path, FILE* file);
void class_parse_line(class_t* class, const char* line);
