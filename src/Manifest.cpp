#include <converter/Manifest.hpp>

#include <fstream>

Manifest::Manifest(std::string &filepath, std::vector<ImageData> data) noexcept
: _manifest_file_path(filepath), _image_data(data) {}

[[nodiscard]] bool Manifest::write() {
    std::ofstream output_file(_manifest_file_path);
    if (!output_file.is_open()) {
        std::cout << "Failed to open file for writing " << _manifest_file_path
            << std::endl;
        return false;
    }

    int file_count = _image_data.size();
    int final_file_index = file_count - 1;

    // write each full file to manifest
    output_file << "{\"full\": [";
    for (int i = 0; i < file_count; ++i) {
        if (i != final_file_index) {
            output_file << _image_data[i]._full.get_json() << ",";
        } else {
            output_file << _image_data[i]._full.get_json() << "],";
        }
    }

    // write each gallery file to manifest
    output_file << "\"gallery\": [";
    for (int i = 0; i < file_count; ++i) {
        if (i != final_file_index) {
            output_file << _image_data[i]._gallery.get_json() << ",";
        } else {
            output_file << _image_data[i]._gallery.get_json() << "],";
        }
    }

    // write each thumbnail file to manifest
    output_file << "\"thumbnail\": [";
    for (int i = 0; i < file_count; ++i) {
        if (i != final_file_index) {
            output_file << _image_data[i]._thumbnail.get_json() << ",";
        } else {
            output_file << _image_data[i]._thumbnail.get_json() << "]}";
        }
    }

    // Close manifest file
    output_file.close();
    std::cout << "Data written to file successfully " << _manifest_file_path
        << std::endl;
}

[[nodiscard]] bool Manifest::thumbnail(LibRaw &ImageProcessor,
        std::string &output_file,
        int thumbnail_index) {
    // Get thumbnail data
    error_t unpack_response = ImageProcessor.unpack_thumb_ex(thumbnail_index);
    if (unpack_response != LIBRAW_SUCCESS) {
        std::cout << "Error unpacking thumbnail index: " << thumbnail_index
            << " File: " << output_file << " Error: " << unpack_response
            << std::endl;
        return false;
    }

    // Write thumbnail data to jpeg full path
    error_t write_response =
        ImageProcessor.dcraw_thumb_writer(output_file.c_str());
    if (write_response != LIBRAW_SUCCESS) {
        std::cout << "Error writing thumbnail index: " << thumbnail_index
            << " File: " << output_file << " Error: " << write_response
            << std::endl;
        return false;
    }

    return true;
}

Manifest::~Manifest() {}
