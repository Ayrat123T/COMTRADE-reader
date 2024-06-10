#pragma once
#include <string>
#include <cstring>
#include <vector>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <regex>
#include <ranges>
#include <algorithm>
#include <stdexcept>
#include <format>
#include <optional>
#include <memory>
#include <array>
#include <cmath>
#include <cstdlib>
#include <cstdint>
#include <set>

#include "Comtrade.h"


using std::filesystem::path;
using std::filesystem::file_status;
using namespace std::literals;

// ввод и вывод COMTRADE
namespace Comtrade_IO {

	// прочитать Comtrade из потока
	comtrade::Comtrade ReadComtrade(const path& cfg_file);

	std::vector<comtrade::Comtrade> ReadComtrades(const std::vector<std::filesystem::path>& comtrade_files_path);

} // namespace Comtrade_IO

namespace detail_comtrade_IO {

	// удалять пробелы?
	enum REMOVE_SPACE_MODE {
		ON, // да
		OFF // нет
	};

	// возвращает дату из сырой строки с помощью регекса
	comtrade::Date_time_stamp GetDigitDate(const std::string Date);

	// парсинг аналогового канала из строки
	comtrade::AnalogChannel ParseAnalogChFromCFG(std::string& text, comtrade::COMTRADE_REV_YEAR REV_YEAR);

	// парсинг дискретного канала из строки
	comtrade::DigitalChannel ParseDiscreteChFromCFG(std::string& text, comtrade::COMTRADE_REV_YEAR REV_YEAR);

	// разделить строку по словам через запятую
	std::vector<std::string_view> SplitIntoWordsByComma(std::string_view str);

	// разделить строку по словам через запятую
	std::vector<std::string> SplitIntoWordsByComma(std::string& text, REMOVE_SPACE_MODE mode);

	// разделить строку по числам через запятую
	std::vector<int> SplitIntoNumByComma(std::string& text);

	// прочитать CFG-файл из потока
	comtrade::CFG ReadCFG(const path& cfg_file, std::optional<const size_t> start_cfg_pos);

	// прочитать DAT-файл из потока
	comtrade::DAT ReadDAT(const path& dat_file, const comtrade::CFG& cfg, std::optional<const size_t> start_dat_pos);

	// парсер для ASCII DAT файла
	void Parse_ASCII_DAT(std::ifstream& raw_dat_in, const comtrade::CFG* cfg, comtrade::DAT* dat);

	// парсер для BINARY DAT файла
	void Parse_BINARY_DAT(std::ifstream& raw_dat_in, const comtrade::CFG* cfg, comtrade::DAT* dat);

	//посчитать размер файла в байтах
	unsigned long long int GetFileSize(std::ifstream& file);

} // namespace detail_comtrade_IO