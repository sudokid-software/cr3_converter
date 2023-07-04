#include <algorithm>
#include <converter/FileData.hpp>
#include <converter/ImageData.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <libraw/libraw.h>
#include <sstream>
#include <string>
#include <vector>

bool write_thumbnail(LibRaw &ImageProcessor, std::string &output_file,
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

int main([[maybe_unused]] int argc, char *argv[]) {
  // Get first passed in argument
  std::string raw_image_directory = argv[1];

  if (!std::filesystem::exists(raw_image_directory)) {
    std::cout << "Raw image directory does not exist: " << raw_image_directory
              << std::endl;
    return 1;
  }

  // Get folder name from passed in argument
  std::string folder_name =
      raw_image_directory.substr(raw_image_directory.find_last_of("/\\") + 1);

  // Get second passed in argument
  std::filesystem::path output_directory = argv[2];

  // Get all files in directory
  const std::filesystem::directory_iterator directory_list =
      std::filesystem::directory_iterator(raw_image_directory);

  // Initialize a list of file names
  std::vector<std::string> file_paths;
  for (const auto &file : directory_list) {
    // Add file name to list
    file_paths.push_back(file.path());
  }

  // Reverse order file_paths
  std::reverse(file_paths.begin(), file_paths.end());

  // Create full_folder
  std::string full_folder =
      output_directory.string() + "/" + folder_name + "/full/";
  std::string gallery_folder =
      output_directory.string() + "/" + folder_name + "/gallery/";
  std::string thumbnail_folder =
      output_directory.string() + "/" + folder_name + "/thumbnail/";

  // Create output directory
  std::filesystem::create_directories(full_folder);
  std::filesystem::create_directories(gallery_folder);
  std::filesystem::create_directories(thumbnail_folder);

  // Create LibRaw ImageProcessor
  LibRaw ImageProcessor(0);

  // Get total number of files to process
  const int file_count = static_cast<int>(file_paths.size());
  int index = 0;

  std::vector<ImageData> image_data_list;
  image_data_list.reserve(file_count);

  // Process all found images
  for (const std::string &file_path : file_paths) {
    // Get current file name
    std::string file_name = file_path.substr(file_path.find_last_of("/\\") + 1);
    // Drop extension from file name
    file_name = file_name.substr(0, file_name.find_last_of('.'));

    // Get everything after the _ in the file name
    int image_number =
        std::stoi(file_name.substr(file_name.find_first_of('_') + 1));

    // Load the raw image
    int image_processor_response = ImageProcessor.open_file(file_path.c_str());
    if (image_processor_response != LIBRAW_SUCCESS) {
      std::cout << "Error opening file " << file_path << std::endl;
      return 1;
    }

    // Get the raw image data
    int image_processor_unpack_response = ImageProcessor.unpack();
    if (image_processor_unpack_response != LIBRAW_SUCCESS) {
      std::cout << "Error unpacking file " << file_path << std::endl;
      return 1;
    }

    // Write thumbnail data to jpeg full path with string append
    std::string full_jpeg_path = full_folder + file_name + "-full.jpeg";
    write_thumbnail(ImageProcessor, full_jpeg_path, 2);
    FileData full_file_data =
        FileData(file_name + "-full.jpeg", ImageProcessor.imgdata.sizes.iwidth,
                 ImageProcessor.imgdata.sizes.iheight);

    // Write gallery thumbnail data to jpeg path
    std::string gallery_jpeg_path =
        gallery_folder + file_name + "-gallery.jpeg";
    write_thumbnail(ImageProcessor, gallery_jpeg_path, 1);
    FileData gallery_file_data = FileData(file_name + "-gallery.jpeg",
                                          ImageProcessor.imgdata.sizes.iwidth,
                                          ImageProcessor.imgdata.sizes.iheight);

    // Write thumbnail data to jpeg path
    std::string thumbnail_jpeg_path =
        thumbnail_folder + file_name + "-thumbnail.jpeg";
    write_thumbnail(ImageProcessor, thumbnail_jpeg_path, 0);
    FileData thumbnail_file_data = FileData(
        file_name + "-thumbnail.jpeg", ImageProcessor.imgdata.sizes.iwidth,
        ImageProcessor.imgdata.sizes.iheight);

    ImageData image_data = ImageData(full_file_data, gallery_file_data,
                                     thumbnail_file_data, image_number);
    image_data_list.push_back(image_data);
    std::cout << "Processing: " << ++index << "/" << file_count << " - "
              << file_name << std::endl;
    ImageProcessor.free_image();
  }

  // Sort image_data_list
  std::sort(image_data_list.begin(), image_data_list.end());

  // Write manifest file
  std::string manifest_path =
      output_directory.string() + "/" + folder_name + "/manifest.json";
  write_manifest(manifest_path, image_data_list);

  ImageProcessor.recycle();
  return 0;
}
