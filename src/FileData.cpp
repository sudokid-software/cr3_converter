#include <converter/FileData.hpp>
#include <sstream>

FileData::FileData(std::string name, int width, int height) noexcept
    : _name(std::move(name)), _width(width), _height(height) {}

[[nodiscard]] std::string FileData::get_json() const {
  std::ostringstream json;
  json << R"({"fileName": ")" << _name << R"(","width":)" << _width
       << R"(,"height":)" << _height << "}";
  return json.str();
}

FileData::~FileData(){};
