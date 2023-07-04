#pragma once

#include <iostream>
#include <string>

class FileData final {
public:
  FileData() = delete;
  explicit FileData(std::string name, int width, int height) noexcept;

  [[nodiscard]] std::string get_json() const;

  ~FileData();

private:
  std::string _name;
  int _width;
  int _height;
}

;
