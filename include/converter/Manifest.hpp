#pragma once

#include <converter/ImageData.hpp>
#include <libraw/libraw.h>

#include <string>
#include <vector>

class Manifest final {
public:
  Manifest() = delete;
  explicit Manifest(std::string &filepath,
                    std::vector<ImageData> data) noexcept;

  [[nodiscard]] bool write();
  [[nodiscard]] bool thumbnail(LibRaw &ImageProcessor, std::string &output_file,
                               int thumbnail_index);

  ~Manifest();

private:
  std::string _manifest_file_path;
  std::vector<ImageData> _image_data;
};
