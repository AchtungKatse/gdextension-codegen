# Overview 
This tool is used to automatically generate properties for Godot's GDExtension and reduce boilerplate required when using C++ and Godot. 

This can reduce the following code from: 
```cpp
// .h file
class GDExample {
    ...
        float property = 1.0f;
    float get_property() const;
    void set_property(float value);
};

// .cpp file
float GDExample::get_property() const {
    return property;
}
void GDExample::set_property(float value) {
    property = value;
}

void _bind_methods() {
    ClassDB::bind_method(D_METHOD("get_property"), &GDExample::get_property);
    ClassDB::bind_method(D_METHOD("set_property", "property"), &GDExample::set_property);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "property"), "set_property", "get_property");
}
```

to the following:
```cpp
// .h file
class GDExample {
    ...
        GD_EXPORT(float, property) = 1.0f;
};
```

The remainder of the required code is auto-generated.

# Features
- Automatic node, resource, and variant binding
- Group and subgroup emitting

# Getting Started
1. Add this repository as a git submodule.
```
git submodule add [githuburl here] [dependency path]
i.e.
git submodule add [url] dependencies/gdextension-codegen
```

2. Add the following to your CMakeLists.txt file
```cmake
add_subdirectory([submodule path]) # must match path from step 1

# Create a list of all output generated files
set(codegen_source "") 
# Adds a directory to output generated files
make_directory(${CMAKE_BINARY_DIR}/codegen)

# Find all header files with exports
    file(GLOB_RECURSE HEADER_FILES 
            ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
                                                 ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
                                                 )

                                                 foreach (file IN ITEMS ${HEADER_FILES}) 
# Modify source file name to output file path
cmake_path(GET file FILENAME file_name)
string(REPLACE ".h" ".cpp" file_name ${file_name})
string(REPLACE ".hpp" ".cpp" file_name ${file_name})
set(file_out_path "${CMAKE_BINARY_DIR}/codegen/${file_name}")

set(file_target ${file}-target)
string(REPLACE "/" "-" file_target ${file_target})

message("Generating ${file} -> ${file_out_path}")

# Create a target and command that converts the header to its generated source file
add_custom_target(${file_target} SOURCES ${file})
add_custom_command(
OUTPUT ${file_out_path}
MAIN_DEPENDENCY ${file}
COMMAND source_gen file "${file}" -o "${file_out_path}"
COMMENT "Generating source: ${file} -> ${file_out_path}"
BYPRODUCTS ${file_out_path}
DEPENDS ${file} source_gen
VERBATIM
)

# Add new source file to codegen source
set(codegen_source ${codegen_source} ${file_out_path})
endforeach()
```

This will automatically find all header files and emit generated code where required. Note, the include directory may need to be changed.

3. Add the generated files to your project's source
```cmake
# i.e. Change
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
# to 
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
```

gdextension-codegen will now run on all files that use the GD_EXPORT macro and emit matching property functions whenever header files change.

## Complete example CMakeLists.txt file
```cmake
# ==============================================
# Project Definition and options
# ==============================================
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(example_project)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/project/bin)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/deps/godot-cpp)

# Add source files
file(GLOB_RECURSE SOURCE_FILES 
${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

# Codegen
add_subdirectory(dependencies/gdextension-codegen)

# Create a list of all output generated files
set(codegen_source "") 
# Adds a directory to output generated files
make_directory(${CMAKE_BINARY_DIR}/codegen)

# Find all header files with exports
file(GLOB_RECURSE HEADER_FILES 
${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
)

foreach (file IN ITEMS ${HEADER_FILES}) 
# Modify source file name to output file path
cmake_path(GET file FILENAME file_name)
string(REPLACE ".h" ".cpp" file_name ${file_name})
string(REPLACE ".hpp" ".cpp" file_name ${file_name})
set(file_out_path "${CMAKE_BINARY_DIR}/codegen/${file_name}")

set(file_target ${file}-target)
string(REPLACE "/" "-" file_target ${file_target})

message("Generating ${file} -> ${file_out_path}")

# Create a target and command that converts the header to its generated source file
add_custom_target(${file_target} SOURCES ${file})
add_custom_command(
OUTPUT ${file_out_path}
MAIN_DEPENDENCY ${file}
COMMAND source_gen file "${file}" -o "${file_out_path}"
COMMENT "Generating source: ${file} -> ${file_out_path}"
BYPRODUCTS ${file_out_path}
DEPENDS ${file} source_gen
VERBATIM
)

# Add new source file to codegen source
set(codegen_source ${codegen_source} ${file_out_path})
endforeach()

# Define the link libraries
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${codegen_source})

# Define the include DIRs
target_include_directories(${PROJECT_NAME} PRIVATE
"${CMAKE_CURRENT_SOURCE_DIR}/include/"
"${CMAKE_CURRENT_SOURCE_DIR}/deps/godot-cpp/include"
)

target_link_libraries(${PROJECT_NAME} PRIVATE godot-cpp)
```

## Macro definitions
Finally, the following macros will need to be defined and included. It is recommended to add these to a PCH or common defines file.
```cpp
#define #define GD_EXPORT(type, name) \
void set_##name(type value); \
type get_##name() const; \
type name

    // These two macros add nothing to source files and are only used as markers by the code generator.
#define GD_GROUP(name, prefix)
#define GD_SUBGROUP(name, prefix)
```

to add use add this to a PCH to have it automatically included in all files, include the following snippet in the CMakeLists.txt file.
```cmake
target_precompile_headers(${PROJECT_NAME} PRIVATE
"$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_INCLUDE_DIR}/defines.h>"
)
```

# Usage
Simply use the GD_EXPORT(type, name) macro to all properties that you wish to be exported. Please note that any pointers need to be initialized to nullptr to prevent Godot from crashing.
GD_GROUP(name, prefix) and GD_SUBGROUP(name, prefix) can be used to group any following exported properties with an optional prefix.

## Example
```cpp
class GDExample : public Sprite2D {
GDCLASS(GDExample, Sprite2D)

    public:
    GD_GROUP(Objects, objects_); // Groups the following two exports with the objects_ prefix but not the misc_scene export
    GD_EXPORT(Node*, objects_node) = nullptr; // Pointers need to be initialized to null to prevent crashes
    GD_EXPORT(Ref<PackedScene>, objects_resource); 
    GD_EXPORT(Ref<PackedScene>, misc_scene); 

    GD_GROUP(Primatives, ); // Groups all following primatives regardless of their prefix
    GD_EXPORT(bool, _bool);
    GD_EXPORT(int, _int);
    GD_EXPORT(float, _float);
    GD_EXPORT(double, _double) = 0;

    GD_SUBGROUP(Strings, ); // Adds a subgroup to the primatives group
    GD_EXPORT(String, string_0);
    GD_EXPORT(String, string_1);
    GD_EXPORT(String, string_2);

    GD_GROUP(Other Variants, );
    GD_EXPORT(Vector2, vec2);
    GD_EXPORT(Vector2i, vec2i);
    GD_EXPORT(Rect2, rect);
    GD_EXPORT(Rect2i, recti);
    GD_EXPORT(Vector3, vec3);
    GD_EXPORT(Vector3i, vec3i);
    GD_EXPORT(Transform2D, transform2d);
    GD_EXPORT(Vector4, vec4);
    GD_EXPORT(Vector4i, vec4i);
    GD_EXPORT(Plane, plane);
    GD_EXPORT(Quaternion, quat);
    GD_EXPORT(AABB, aabb);
    GD_EXPORT(Basis, basis);
    GD_EXPORT(Transform3D, transform3d);
    GD_EXPORT(Projection, projection);
    GD_EXPORT(Color, color);
    GD_EXPORT(StringName, string_name);
    GD_EXPORT(NodePath, node_path);
    GD_EXPORT(RID, rid);
    GD_EXPORT(Callable, callable);
    GD_EXPORT(Signal, signal);
    GD_EXPORT(Dictionary, dictionary);
    GD_EXPORT(Array, array);

    GD_SUBGROUP(Packed Arrays, packed_); // Adds a subgroup with the packed_ prefix
    GD_EXPORT(PackedByteArray, packed_byte);
    GD_EXPORT(PackedInt32Array, packed_int);
    GD_EXPORT(PackedInt64Array, packed_int64);
    GD_EXPORT(PackedFloat32Array, packed_float);
    GD_EXPORT(PackedFloat64Array, packed_double);
    GD_EXPORT(PackedStringArray, packed_string);
    GD_EXPORT(PackedVector2Array, packed_vec2);
    GD_EXPORT(PackedVector3Array, packed_vec3);
    GD_EXPORT(PackedColorArray, packed_color);
    GD_EXPORT(PackedVector4Array, packed_vec4);

    protected:
    static void _bind_methods();

    public:
    void _process(double delta) override;
    };
```

# How it works
As mentioned above, this tool checks all header files in a project for the use of the GD_EXPORT macro or bind methods functions to identify classes that require codegen. Each of these files are then read line by line to find all exports and property groups and emit appropriate functions based on their types. 
This project assumes that
1. All node types are pointers (i.e. Node3D*)
2. All resource types are use the Ref<> class (i.e. Ref<PackedScene>)
3. All variants use their default names since typedef classes are not automatically recognized.

# Limitations
    Currently, the project does not support enum types and users cannot easily add to or modify the _bind_methods() function. 
