#pragma once

#include "../../lilac/Macros.hpp"
#include "../../lilac/Result.hpp"
#include <string>
#include "types.hpp"
#include <filesystem>

namespace lilac::utils {
    LILAC_DLL Result<std::string> file_read_string(std::string            const& path);
    LILAC_DLL Result<std::string> file_read_string(std::wstring           const& path);
    LILAC_DLL Result<std::string> file_read_string(std::filesystem::path  const& path);
    LILAC_DLL Result<byte_array>  file_read_binary(std::string            const& path);
    LILAC_DLL Result<byte_array>  file_read_binary(std::wstring           const& path);
    LILAC_DLL Result<byte_array>  file_read_binary(std::filesystem::path  const& path);

    LILAC_DLL Result<>    file_write_string(std::string           const& path, std::string const& data);
    LILAC_DLL Result<>    file_write_string(std::wstring          const& path, std::string const& data);
    LILAC_DLL Result<>    file_write_string(std::filesystem::path const& path, std::string const& data);
    LILAC_DLL Result<>    file_write_binary(std::string           const& path, byte_array  const& data);
    LILAC_DLL Result<>    file_write_binary(std::wstring          const& path, byte_array  const& data);
    LILAC_DLL Result<>    file_write_binary(std::filesystem::path const& path, byte_array  const& data);

    LILAC_DLL Result<>    directory_create(std::string const& path);
    LILAC_DLL Result<>    directory_create_all(std::string const& path);
    LILAC_DLL Result<std::vector<std::string>> directory_list(std::string const& path);
}
