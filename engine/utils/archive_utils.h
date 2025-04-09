#include <archive.h>
#include <archive_entry.h>
// #include <minizip/unzip.h>
#include "unzip.h"
#include <trantor/utils/Logger.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "logging_utils.h"

namespace archive_utils {
inline bool UnzipFile(const std::string& input_zip_path,
                      const std::string& destination_path);
inline bool UntarFile(const std::string& input_tar_path,
                      const std::string& destination_path,
                      bool ignore_parent_dir = false);

inline bool ExtractArchive(const std::string& input_path,
                           const std::string& destination_path,
                           bool ignore_parent_dir = false) {
  if (input_path.find(".zip") != std::string::npos) {
    return UnzipFile(input_path, destination_path);
  } else if (input_path.find(".tar") != std::string::npos ||
             input_path.find(".tar.gz") != std::string::npos) {
    return UntarFile(input_path, destination_path, ignore_parent_dir);
  } else {
    LOG_ERROR << "Unsupported file type: " << input_path << "\n";
    return false;
  }
}

inline bool UnzipFile(const std::string& input_zip_path,
                      const std::string& destination_path) {
  unzFile zip_file = unzOpen(input_zip_path.c_str());
  if (!zip_file) {
    LOG_ERROR << "Failed to open zip file: " << input_zip_path << "\n";
    return false;
  }

  std::filesystem::create_directories(destination_path);

  if (unzGoToFirstFile(zip_file) != UNZ_OK) {
    LOG_ERROR << "Error opening first file in zip" << "\n";
    unzClose(zip_file);
    return false;
  }

  do {
    unz_file_info file_info;
    char file_name[256];
    if (unzGetCurrentFileInfo(zip_file, &file_info, file_name,
                              sizeof(file_name), nullptr, 0, nullptr,
                              0) != UNZ_OK) {
      LOG_ERROR << "Failed to get file info" << "\n";
      unzClose(zip_file);
      return false;
    }

    std::string full_path = destination_path + "/" + file_name;

    if (file_name[strlen(file_name) - 1] == '/') {
      std::filesystem::create_directories(full_path);
    } else {
      std::filesystem::create_directories(
          std::filesystem::path(full_path).parent_path());

      if (unzOpenCurrentFile(zip_file) != UNZ_OK) {
        LOG_ERROR << "Failed to open file in zip: " << file_name << "\n";
        unzClose(zip_file);
        return false;
      }

      std::ofstream outFile(full_path, std::ios::binary);
      if (!outFile.is_open()) {
        LOG_ERROR << "Failed to create file: " << full_path << "\n";
        unzCloseCurrentFile(zip_file);
        unzClose(zip_file);
        return false;
      }

      char buffer[8192];
      int bytes_read;
      while ((bytes_read =
                  unzReadCurrentFile(zip_file, buffer, sizeof(buffer))) > 0) {
        outFile.write(buffer, bytes_read);
      }

      outFile.close();
      unzCloseCurrentFile(zip_file);
    }
  } while (unzGoToNextFile(zip_file) == UNZ_OK);

  unzClose(zip_file);
  LOG_INFO << "Extracted successfully " << input_zip_path << " to "
           << destination_path << "\n";
  return true;
}

inline bool UntarFile(const std::string& input_tar_path,
                      const std::string& destination_path,
                      bool ignore_parent_dir) {
  struct archive* tar_archive = archive_read_new();
  archive_read_support_format_tar(tar_archive);
  archive_read_support_filter_gzip(tar_archive);

  if (archive_read_open_filename(tar_archive, input_tar_path.c_str(), 10240) !=
      ARCHIVE_OK) {
    LOG_ERROR << "Failed to open tar file: " << input_tar_path << "\n";
    archive_read_free(tar_archive);
    return false;
  }

  std::filesystem::create_directories(destination_path);
  struct archive_entry* entry;
  while (archive_read_next_header(tar_archive, &entry) == ARCHIVE_OK) {
    const char* current_file = archive_entry_pathname(entry);
    auto file_in_tar_path =
        std::filesystem::path(destination_path) / current_file;
    auto file_name = std::filesystem::path(file_in_tar_path).filename();
    auto output_path = std::filesystem::path(destination_path) / file_name;
    std::string full_path = destination_path + "/" + current_file;

    if (archive_entry_filetype(entry) == AE_IFDIR) {
      if (!ignore_parent_dir) {
        std::filesystem::create_directories(full_path);
      }
    } else {
      auto final_output_path =
          ignore_parent_dir ? output_path.string() : full_path;

      std::ofstream out_file(final_output_path, std::ios::binary);
      if (!out_file.is_open()) {
        LOG_ERROR << "Failed to create file: " << full_path << "\n";
        archive_read_free(tar_archive);
        return false;
      }

      const void* buff;
      size_t size;
      la_int64_t offset;
      while (archive_read_data_block(tar_archive, &buff, &size, &offset) ==
             ARCHIVE_OK) {
        out_file.write(static_cast<const char*>(buff), size);
      }

      out_file.close();
    }

    archive_entry_clear(entry);
  }

  archive_read_free(tar_archive);
  CTL_INF("Extracted successfully " << input_tar_path << " to "
                                    << destination_path << "\n");
  return true;
}
}  // namespace archive_utils
