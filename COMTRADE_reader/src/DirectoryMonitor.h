#pragma once
#include <regex>
#include <sstream>
#include <filesystem>
#include <fstream>
#include <string_view>
#include <iostream>
#include <string>
#include <algorithm> 
#include <cassert> 
#include <optional>
#include <conio.h>
#include <set>
#include <vector>

using std::filesystem::path;
using std::filesystem::file_status;
using namespace std::literals;

namespace DirectoryMonitor {

	/*Монитор директории отслеживет переданную ему папку на предмет наличия COMTRADE
	создаёт архив и вектор путей со всеми обнаруженными COMTRADE*/
	class DirectoryMonitor {
	public:

		// принимает на вход путь к папке
		DirectoryMonitor(const path& directory);

		// установка пути к папке для отслеживания
		void SetDirectory(const path directory);
		void SetDirectory(const std::istringstream strm);
		void SetDirectory(const std::string directory_str);

		//создать вектор из списка COMTRADE-файлов
		void SetComtradeFiles(const std::filesystem::path& directory);

		// установка пути к архивной папке для отслеживания
		void SetArchiveDirectory(const std::string directory_str);

		// начать мониторинг папки на предмет появления COMTRADE
		void StartMonitoring();

		// возвращает путь к исходной папке
		path GetRawDirectory();

		// возвращает путь к архивной папке
		path GetArchiveDirectory();

		// возвращает вектор путей к COMTRADE
		std::vector<std::filesystem::path> GetComtradeFilesPath();

		// возвращает кол-во путей к COMTRADE
		size_t GetComtradeFilesPathSize();

		// принимает на вход поток, задаваемый параметром dst, и выводит в него дерево файлов и каталогов этой папки
		void PrintTree(std::ostream& dst);

		//вывести в поток названия всех файлов
		void PrintComtradeFilesNames(std::ostream& dst);

		//создать архив и переместить в него все COMTRADE
		void CreateArchive(const std::optional<path>& parent);

	private:
		path raw_directory_;
		path archive_directory_;
		std::vector<std::filesystem::path> comtrade_files_;

		void PrintTree(std::ostream& dst, const path& p, const file_status& status, int offset);

		void CreateCopyOfPath(const path& p_child);

		void CreateArchive(const path& p, const file_status& status);

		void SetComtradeFiles(const std::filesystem::path& p, const std::filesystem::file_status status);

		void FindComtradeFiles(const std::filesystem::path& directory);

		void FindComtradeFiles(const std::filesystem::path& p, const std::filesystem::file_status status);

	};
	
} // namespace DirectoryMonitor