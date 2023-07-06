#include <iostream>
#include <filesystem>
#include <string>
#include <algorithm>
#include <libraw/libraw.h>
#include <regex>
#include "src/ImageData.h"

// Write manifest file
bool write_manifest(
        const std::string &manifest_file_path, const std::vector<FileData *> &full_files_list,
        const std::vector<FileData *> &gallery_files_list, const std::vector<FileData *> &thumbnail_files_list
) {
    // Create manifest file
    std::ofstream output_file(manifest_file_path);
    if (!output_file.is_open()) {
        std::cout << "Failed to open file for writing " << manifest_file_path << "\n";
        return false;
    }

    int file_count = static_cast<int>(full_files_list.size());
    int final_file_index = file_count - 1;

    // write each full file to manifest
    output_file << "{\"full\": [";
    for (int i = 0; i < file_count; ++i) {
        std::string json = full_files_list[i]->get_json();
        if (i != final_file_index) {
            output_file << json << ",";
        } else {
            output_file << json << "],";
        }
    }

    // write each gallery file to manifest
    output_file << "\"gallery\": [";
    for (int i = 0; i < file_count; ++i) {
        if (i != final_file_index) {
            output_file << gallery_files_list[i]->get_json() << ",";
        } else {
            output_file << gallery_files_list[i]->get_json() << "],";
        }
    }

    // write each thumbnail file to manifest
    output_file << "\"thumbnail\": [";
    for (int i = 0; i < file_count; ++i) {
        if (i != final_file_index) {
            output_file << thumbnail_files_list[i]->get_json() << ",";
        } else {
            output_file << thumbnail_files_list[i]->get_json() << "]}";
        }
    }

    // Close manifest file
    output_file.close();
    std::cout << "Data written to file successfully " << manifest_file_path << "\n";
    return true;
}

int main([[maybe_unused]] int argc, char *argv[]) {
    // Get first passed in argument
    std::string raw_image_directory = argv[1];
    // Drop last character if it's /
    if (raw_image_directory.back() == '/') {
        raw_image_directory.pop_back();
    }

    if (!std::filesystem::exists(raw_image_directory)) {
        std::cout << "Raw image directory does not exist: " << raw_image_directory << "\n";
        return 1;
    }

    std::string folder_name = raw_image_directory.substr(raw_image_directory.find_last_of("/\\") + 1);

    // Get second passed in argument
    std::string output_directory = argv[2];
    // Drop last character if it's /
    if (output_directory.back() == '/') {
        output_directory.pop_back();
    }
    // Add folder name
    output_directory += "/" + folder_name;

    std::string full_path = output_directory + "/full";
    std::string gallery_path = output_directory + "/gallery";
    std::string thumbnail_path = output_directory + "/thumbnail";

    // Create output directories
    std::filesystem::create_directories(full_path);
    if (!std::filesystem::exists(full_path)) {
        std::cout << "Failed to create directory: " << full_path << "\n";
        return 1;
    }

    std::filesystem::create_directories(gallery_path);
    if (!std::filesystem::exists(gallery_path)) {
        std::cout << "Failed to create directory: " << gallery_path << "\n";
        return 1;
    }

    std::filesystem::create_directories(thumbnail_path);
    if (!std::filesystem::exists(thumbnail_path)) {
        std::cout << "Failed to create directory: " << thumbnail_path << "\n";
        return 1;
    }

    // Get all files in directory
    const std::filesystem::directory_iterator directory_list = std::filesystem::directory_iterator(raw_image_directory);

    // Initialize a list of file names
    std::vector<std::string> image_paths;
    for (const auto &file: directory_list) {
        // Add file name to list
        image_paths.push_back(file.path());
    }

    // Create LibRaw ImageProcessor
    LibRaw ImageProcessor(0);

    // Get total number of files to process
    const int file_count = static_cast<int>(image_paths.size());
    int index = 0;
    int skip_count = 0;
    int error_count = 0;

    // Print number of files to process
    std::cout << "Processing " << file_count << " files" << "\n";

    std::vector<ImageData *> errors;

    std::vector<FileData *> full_files_list;
    std::vector<FileData *> gallery_files_list;
    std::vector<FileData *> thumbnail_files_list;

    auto start = std::chrono::steady_clock::now();
    // Process all found images
    for (const std::string &image_path: image_paths) {
        // Get current file name
        std::string image_name = image_path.substr(image_path.find_last_of("/\\") + 1);

        // Drop extension from file name
        image_name = image_name.substr(0, image_name.find_last_of('.'));

        // Regex test to see if file name is formatted correctly (IMG_0000)
        if (!std::regex_match(image_name.c_str(), std::regex("^IMG_[0-9]{4}$"))) {
            // Skip of file name is not formatted correctly
            std::cout << "Skipping: " << image_name << " " << image_path << "\"" << "\n";
            skip_count++;
            continue;
        }

        ImageData *image_data = new ImageData(image_name, image_path, output_directory);
        try {
            image_data->get_image_number();
        } catch (std::exception &e) {
            // Log error
            std::cout << "Couldn't process image number for : " << image_path << "\n";
            continue;
        }

        try {
            image_data->try_init();
            image_data->write_thumbnails();
        } catch (std::exception &e) {
            // Log error
            std::cout << "Error Processing: " << e.what() << "\n";
            errors.push_back(image_data);
            continue;
        }

        // std::cout << "Processing: " << ++index << "/" << file_count << " - " << image_path << "\n";

        // Copy image data to new object, so we can free the image data memory
        FileData* full_file_data = new FileData(image_data->full.name, image_data->full.number, image_data->full.width,
                                           image_data->full.height);
        full_files_list.push_back(full_file_data);

        FileData* gallery_file_data = new FileData(image_data->gallery.name, image_data->gallery.number,
                                              image_data->gallery.width, image_data->gallery.height);
        gallery_files_list.push_back(gallery_file_data);

        FileData* thumbnail_file_data = new FileData(image_data->thumbnail.name, image_data->thumbnail.number,
                                                image_data->thumbnail.width, image_data->thumbnail.height);
        thumbnail_files_list.push_back(thumbnail_file_data);

        // Free image_data
        free(image_data);
        ++index;
    }

    // Try to write errors to file
    for (ImageData *image_data: errors) {
        try {
            image_data->try_init();
            image_data->write_thumbnails();

            // Copy image data to new object, so we can free the image data memory
            FileData full_file_data = image_data->full;
            full_files_list.push_back(&image_data->full);

            FileData gallery_file_data = image_data->gallery;
            gallery_files_list.push_back(&image_data->gallery);

            FileData thumbnail_file_data = image_data->thumbnail;
            thumbnail_files_list.push_back(&thumbnail_file_data);

            free(image_data);
        } catch (std::exception &e) {
            // Log error
            std::cout << "Failed to process " << image_data->name << ": " << e.what() << std::endl;
            free(image_data);
            error_count++;
            continue;
        }
    }

    // Sort full images
    std::sort(full_files_list.begin(), full_files_list.end());
    // Sort gallery images
    std::sort(gallery_files_list.begin(), gallery_files_list.end());
    // Sort thumbnail images
    std::sort(thumbnail_files_list.begin(), thumbnail_files_list.end());

    // Write manifest file
    std::string manifest_path = output_directory + "/manifest.json";
    write_manifest(manifest_path, full_files_list, gallery_files_list, thumbnail_files_list);

    auto end = std::chrono::steady_clock::now();
    auto diff = end - start;
    std::cout << "Processed: " << index << " Skipped: " << skip_count << " Errored: " << error_count
              << " files in " << std::chrono::duration<double, std::milli>(diff).count() << "\n";

    // Free memory full files
    for (FileData *file_data: full_files_list) {
        free(file_data);
    }

    // Free memory gallery files
    for (FileData *file_data: gallery_files_list) {
        free(file_data);
    }

    // Free memory thumbnail files
    for (FileData *file_data: thumbnail_files_list) {
        free(file_data);
    }

    return 0;
}
