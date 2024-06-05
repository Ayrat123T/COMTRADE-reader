#include "DirectoryMonitor.h"

void ResetStatus();

namespace DirectoryMonitor {

    DirectoryMonitor::DirectoryMonitor(const path& directory) {
        if (!std::filesystem::exists(directory)) {
            std::cerr << "invalid path!";
            exit(1); // выходим из программы
        } else {
            raw_directory_ = directory;
            archive_directory_ = raw_directory_.parent_path() / path{ raw_directory_.filename().string() + "_archive" };
        }
    }

	void DirectoryMonitor::SetDirectory(const path directory) {
        raw_directory_ = std::move(directory);
	}

	void DirectoryMonitor::SetDirectory(const std::istringstream strm) {
        raw_directory_ = { strm.str() };
	}

	void DirectoryMonitor::SetDirectory(const std::string directory_str) {
        raw_directory_ = { directory_str };
	}

    void DirectoryMonitor::SetArchiveDirectory(const std::string archive_directory_str) {
        archive_directory_ = { archive_directory_str };
    }

	path DirectoryMonitor::GetRawDirectory() {
		return raw_directory_;
	}

    path DirectoryMonitor::GetArchiveDirectory() {
        return archive_directory_;
    }

    std::vector<std::filesystem::path> DirectoryMonitor::GetComtradeFilesPath() {
        return comtrade_files_;
    }

    size_t DirectoryMonitor::GetComtradeFilesPathSize() {
        return comtrade_files_.size();
    }

	void DirectoryMonitor::StartMonitoring() {
        std::cout << "Выполняется мониторинг директории\n"s
            << "Для остановки мониторинга нажмите любую клавишу\n"s;
        while (true) {
            if (_kbhit()) {
                std::cout << "Мониторинг директории остановлен\n"s
                        << "Найдено "s << GetComtradeFilesPathSize() << " Comtrade-файлов\n"s;
                break;
                _getch();
            }
            FindComtradeFiles(raw_directory_);
        }
	}

    void DirectoryMonitor::FindComtradeFiles(const std::filesystem::path& directory) {
        std::error_code err;
        const std::filesystem::file_status status = std::filesystem::status(directory, err);
        if (err) {
            return;
        }
        FindComtradeFiles(directory, status);
    }

    void DirectoryMonitor::FindComtradeFiles(const std::filesystem::path& p, const std::filesystem::file_status status) {
        std::error_code err;
        std::vector<std::pair<std::filesystem::path, std::filesystem::file_status>> directories;
        if (status.type() == std::filesystem::file_type::directory) {
            for (const auto& dir_entry : std::filesystem::directory_iterator(p)) {
                const std::filesystem::path p_child = dir_entry.path();
                const std::filesystem::file_status status_child = std::filesystem::status(p_child, err);
                directories.push_back({ p_child , status_child });
            }
            for (const auto& directory : directories) {
                if (directory.second.type() != std::filesystem::file_type::directory
                    && (get<0>(directory).extension().string().find(".cff"sv) != -1
                    || get<0>(directory).extension().string().find(".cfg"sv) != -1
                    || get<0>(directory).extension().string().find(".dat"sv) != -1)) {
                    CreateCopyOfPath(get<0>(directory));
                }
                FindComtradeFiles(get<0>(directory), get<1>(directory));
            }
        }
    }

    void DirectoryMonitor::PrintTree(std::ostream& dst) {
        std::error_code err;
        const file_status status = std::filesystem::status(raw_directory_, err);
        int offset = 0;
        if (err) {
            return;
        }
        dst << raw_directory_.filename().string() << std::endl;
        PrintTree(dst, raw_directory_, status, offset);
    }

    void DirectoryMonitor::PrintTree(std::ostream& dst, const path& p, const file_status& status, int offset) {
        std::error_code err;
        if (status.type() == std::filesystem::file_type::directory) {
            offset += 2;
            for (const auto& dir_entry : std::filesystem::directory_iterator(p)) {
                const path p_child = dir_entry.path();
                const file_status status_child = std::filesystem::status(p_child, err);
                std::string extension = p_child.extension().string();
                dst << std::string(offset, ' ') << p_child.filename().string() << std::endl;
                PrintTree(dst, p_child, status_child, offset);
            }
        }
    }

    void DirectoryMonitor::PrintComtradeFilesNames(std::ostream& dst) {
        for (const auto & comtrade_file : comtrade_files_) {
            dst << comtrade_file.stem().string() << ";\n";
        }
    }

    void DirectoryMonitor::CreateArchive(const std::optional<path>& parent) {
        if (parent.has_value()) {
            archive_directory_ = parent.value();
        }
        std::error_code err;
        const file_status status = std::filesystem::status(raw_directory_, err);
        if (err) {
            return;
        }
        CreateArchive(raw_directory_, status);
    }

    void DirectoryMonitor::CreateArchive(const path& p, const file_status& status) {
        std::error_code err;
        if (status.type() == std::filesystem::file_type::directory) {
            for (const auto& dir_entry : std::filesystem::directory_iterator(p)) {
                const path p_child = dir_entry.path();
                const file_status status_child = std::filesystem::status(p_child, err);
                CreateCopyOfPath(p_child);
                CreateArchive(p_child, status_child);
            }
        }
    }

    void DirectoryMonitor::CreateCopyOfPath(const path& p_child) {
        std::string extension = p_child.extension().string();
        if (extension.find(".cff"sv) != -1
            || extension.find(".cfg"sv) != -1
            || extension.find(".dat"sv) != -1) {
            std::error_code err;
            path copy_archive = archive_directory_ / p_child.parent_path().stem();
            create_directories(copy_archive, err);
            std::filesystem::copy_options my_options = std::filesystem::copy_options::none;
            try {
                std::filesystem::copy(p_child, copy_archive / p_child.filename(), my_options);
                std::filesystem::remove(p_child);
                if (((copy_archive / p_child.filename())).extension().string().find(".cff"sv) != -1
                    || (copy_archive / p_child.filename()).extension().string().find(".cfg"sv) != -1) {
                    comtrade_files_.push_back(copy_archive / p_child.filename());
                }
            }
            catch (const std::filesystem::filesystem_error& e) {
                std::cout << e.what() << std::endl;   //Ошибки не отлавливаются, все завершается успешно.
            }
        }
    }

    void DirectoryMonitor::SetComtradeFiles(const std::filesystem::path& directory) {
        std::error_code err;
        const std::filesystem::file_status status = std::filesystem::status(directory, err);
        if (err) {
            return;
        }
        SetComtradeFiles(directory, status);
    }

    void DirectoryMonitor::SetComtradeFiles(const std::filesystem::path& p, const std::filesystem::file_status status) {
        std::error_code err;
        std::vector<std::pair<std::filesystem::path, std::filesystem::file_status>> directories;
        if (status.type() == std::filesystem::file_type::directory) {
            for (const auto& dir_entry : std::filesystem::directory_iterator(p)) {
                const std::filesystem::path p_child = dir_entry.path();
                const std::filesystem::file_status status_child = std::filesystem::status(p_child, err);
                directories.push_back({ p_child , status_child });
            }
            for (const auto& directory : directories) {
                if (directory.second.type() != std::filesystem::file_type::directory
                    && (get<0>(directory).extension().string().find(".cff"sv) != -1
                    || get<0>(directory).extension().string().find(".cfg"sv) != -1)) {
                    comtrade_files_.push_back(get<0>(directory));
                }
                SetComtradeFiles(get<0>(directory), get<1>(directory));
            }
        }
    }

} // namespace DirectoryMonitor
