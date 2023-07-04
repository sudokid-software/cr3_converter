#pragma once

#include <converter/FileData.hpp>
#include <vector>

class ImageData final {

public:
  ImageData() = delete;
  explicit ImageData(FileData full, FileData gallery, FileData thumbnail,
                     int image_number) noexcept;

  bool operator<(const ImageData &other) const;
  bool operator>(const ImageData &other) const;
  bool operator==(const ImageData &other) const;

  ~ImageData();

public:
  FileData _full;
  FileData _gallery;
  FileData _thumbnail;

private:
  int _image_number;
};
