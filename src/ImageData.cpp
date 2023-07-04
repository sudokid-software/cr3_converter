#include <converter/ImageData.hpp>

ImageData::ImageData(FileData full, FileData gallery, FileData thumbnail,
                     int image_number) noexcept
    : _full(std::move(full)), _gallery(std::move(gallery)),
      _thumbnail(std::move(thumbnail)), _image_number(image_number){};

bool ImageData::operator<(const ImageData &other) const {
  return _image_number < other._image_number;
}

// Greater than operator overload
bool ImageData::operator>(const ImageData &other) const {
  return _image_number > other._image_number;
}

// Equal to operator overload
bool ImageData::operator==(const ImageData &other) const {
  return _image_number == other._image_number;
}

ImageData::~ImageData() {}
