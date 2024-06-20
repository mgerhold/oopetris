

#include "oopetris_wrapper.h"

#include <core/core.hpp>
#include <cstring>
#include <recordings/recordings.hpp>


bool oopetris_is_recording_file(const char* file_path) {

    if (file_path == nullptr) {
        return false;
    }

    if (not std::filesystem::exists(file_path)) {
        return false;
    }

    auto parsed = recorder::RecordingReader::from_path(file_path);

    return parsed.has_value();
}


struct OOPetrisRecordingReturnValueImpl {
    bool is_error;
    union {
        OOPetrisRecordingInformation* information;
        char* error;
    } value;
};


static OOPetrisRecordingReturnValue*
construct_error_from_cstr_impl(OOPetrisRecordingReturnValue* return_value, const char* value, size_t length) {

    auto* alloced_str = static_cast<char*>(malloc(length + 1));

    if (alloced_str == nullptr) {
        free(return_value);
        return nullptr;
    }

    strcpy(alloced_str, value);

    return_value->is_error = true;
    return_value->value.error = alloced_str;

    return return_value;
}

static OOPetrisRecordingReturnValue*
construct_error_from_cstr(OOPetrisRecordingReturnValue* return_value, const char* value) {
    return construct_error_from_cstr_impl(return_value, value, strlen(value));
}

static OOPetrisRecordingReturnValue*
construct_error_from_string(OOPetrisRecordingReturnValue* return_value, const std::string& value) {
    return construct_error_from_cstr_impl(return_value, value.c_str(), value.size());
}

OOPetrisRecordingReturnValue* oopetris_get_recording_information(const char* file_path) {

    auto* return_value = static_cast<OOPetrisRecordingReturnValue*>(malloc(sizeof(OOPetrisRecordingReturnValue)));

    if (return_value == nullptr) {
        return nullptr;
    }

    if (file_path == nullptr) {
        return construct_error_from_cstr(return_value, "FilePath is NULL");
    }


    if (not std::filesystem::exists(file_path)) {
        std::string error = "File '";
        error += file_path;
        error += "' doesn't exist!";

        return construct_error_from_string(return_value, error);
    }

    auto parsed = recorder::RecordingReader::from_path(file_path);

    if (not parsed.has_value()) {
        std::string error = "An error occurred during parsing of the recording file '";
        error += file_path;
        error += "': ";
        error += parsed.error();

        return construct_error_from_string(return_value, error);
    }

    const auto recording_reader = std::move(parsed.value());


    //TODO
    return nullptr;
}

bool oopetris_is_error(OOPetrisRecordingReturnValue* value) {
    return value->is_error;
}

const char* oopetris_get_error(OOPetrisRecordingReturnValue* value) {
    return value->value.error;
}

OOPetrisRecordingInformation* oopetris_get_information(OOPetrisRecordingReturnValue* value) {
    return value->value.information;
}

void oopetris_free_recording_information(OOPetrisRecordingInformation** information) {
    //TODO: improve
    free(*information);
    *information = nullptr;
}

void oopetris_free_recording_value_only(OOPetrisRecordingReturnValue** information) {
    free(*information);
    *information = nullptr;
}

void oopetris_free_recording_value_whole(OOPetrisRecordingReturnValue** information) {
    if (oopetris_is_error(*information)) {
        free((*information)->value.error);
    } else {
        oopetris_free_recording_information(&((*information)->value.information));
    }

    oopetris_free_recording_value_only(information);
}

const char* oopetris_get_lib_version() {
    return utils::version();
}

OOPetrisGridProperties* oopetris_get_grid_properties() {
    auto* properties = static_cast<OOPetrisGridProperties*>(malloc(sizeof(OOPetrisGridProperties)));
    if (properties == nullptr) {
        return nullptr;
    }

    properties->height = grid::height_in_tiles;
    properties->width = grid::width_in_tiles;

    return properties;
}


void oopetris_free_grid_properties(OOPetrisGridProperties** properties) {
    free(*properties);
    *properties = nullptr;
}