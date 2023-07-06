//
// Created by sudokid on 05/07/23.
//

#ifndef CR3_CONVERTER_IMAGEDATA_H
#define CR3_CONVERTER_IMAGEDATA_H

#include <iostream>
#include <sstream>
#include <utility>
#include <libraw/libraw.h>

struct FileData {
    std::string name;
    int width{};
    int height{};
    int number{};

    FileData() = default;

    FileData(
            std::string name, int number, int width, int height
    ) : name(std::move(name)), number(number), width(width), height(height) {}

    explicit FileData(std::string name) : name(std::move(name)) {}

    [[nodiscard]] std::string get_json() const {
        std::ostringstream json;
        json << R"({"fileName": ")" << name << R"(","width":)" << width << R"(,"height":)" << height << "}";
        return json.str();
    }

    // Less than operator overload
    bool operator<(const FileData &other) const {
        return number < other.number;
    }

    // Greater than operator overload
    bool operator>(const FileData &other) const {
        return number > other.number;
    }

    // Equal to operator overload
    bool operator==(const FileData &other) const {
        return number == other.number;
    }
};

enum ImageType {
    THUMBNAIL,
    GALLERY,
    FULL
};

class ImageData {
public:
    FileData full;
    FileData gallery;
    FileData thumbnail;

    int number {0};
    std::basic_string<char> name;
    std::string path;

    // Will throw error if image name doesn't end with numbers
    ImageData(std::basic_string<char> _name, std::string _path, const std::string &output_path)
            : name(std::move(_name)), path(std::move(_path)) {

        // Set full path
        full_path = output_path + "/full/" + name + "-full.jpg";
        gallery_path = output_path + "/gallery/" + name + "-gallery.jpg";
        thumbnail_path = output_path + "/thumbnail/" + name + "-thumbnail.jpg";
    }

    // Destructor
    ~ImageData() {
        ImageProcessor.recycle();
    }

    void get_image_number() {
        // Get everything after the _ in the file name
        std::string image_number_string = name.substr(name.find_first_of('_') + 1);
        number = std::stoi(image_number_string);
    }

    void try_init() {
        // Load the raw image
        int image_processor_response = ImageProcessor.open_file(path.c_str());
        if (image_processor_response != LIBRAW_SUCCESS) {
            std::cout << "Error opening file " << path << std::endl;
            throw std::runtime_error("Error opening file");
        }

        // Get the raw image data
        int image_processor_unpack_response = ImageProcessor.unpack();
        if (image_processor_unpack_response != LIBRAW_SUCCESS) {
            std::cout << "Error unpacking file " << path << std::endl;
            throw std::runtime_error("Error unpacking file");
        }

    };

    void write_thumbnails() {
        write_thumbnail(THUMBNAIL);
        write_thumbnail(GALLERY);
        write_thumbnail(FULL);
        ImageProcessor.free_image();
    }

protected:
    std::string full_path;
    std::string gallery_path;
    std::string thumbnail_path;

    LibRaw ImageProcessor{0};

    void write_thumbnail(ImageType thumbnail_index) {
        std::string output_file;
        std::string file_name;

        switch (thumbnail_index) {
            case THUMBNAIL:
                output_file = thumbnail_path;
                file_name = name + "-thumbnail.jpeg";
                break;
            case GALLERY:
                output_file = gallery_path;
                file_name = name + "-gallery.jpeg";
                break;
            case FULL:
                output_file = full_path;
                file_name = name + "-full.jpeg";
                break;
        }

        // Get thumbnail data
        error_t unpack_response = ImageProcessor.unpack_thumb_ex(thumbnail_index);
        if (unpack_response != LIBRAW_SUCCESS) {
            std::cout << "Error unpacking thumbnail index: " << thumbnail_index << " File: " << output_file
                      << " Error: " << unpack_response << std::endl;

            // Throw error
            throw std::runtime_error("Error unpacking thumbnail");
        }

        // Write thumbnail data to jpeg full path
        int write_response = ImageProcessor.dcraw_thumb_writer(output_file.c_str());
        if (write_response != LIBRAW_SUCCESS) {
            std::cout << "Error writing thumbnail index: " << thumbnail_index << " File: " << output_file
                      << " Error: " << write_response << std::endl;
            // Throw error
            throw std::runtime_error("Error writing thumbnail");
        }

        add_file_data(thumbnail_index, file_name);
    }

    void add_file_data(ImageType thumbnail_index, std::string &file_name) {
        switch (thumbnail_index) {
            case THUMBNAIL:
                thumbnail = FileData(
                        file_name,
                        number,
                        ImageProcessor.imgdata.sizes.iwidth,
                        ImageProcessor.imgdata.sizes.iheight
                );
                break;
            case GALLERY:
                gallery = FileData(
                        file_name,
                        number,
                        ImageProcessor.imgdata.sizes.iwidth,
                        ImageProcessor.imgdata.sizes.iheight
                );
                break;
            case FULL:
                full = FileData(
                        file_name,
                        number,
                        ImageProcessor.imgdata.sizes.raw_width,
                        ImageProcessor.imgdata.sizes.raw_height
                );
                break;
        }
    }
};

#endif //CR3_CONVERTER_IMAGEDATA_H
