#define _CRT_SECURE_NO_WARNINGS

#include "InputReader.h"

namespace rng = std::ranges; // с++20
namespace view = rng::views; // с++20

comtrade::Date_time_stamp detail_comtrade_IO::GetDigitDate(const std::string Date) {
    static std::regex RxDateTime("^(\\d{1,2})/(\\d{1,2})/(\\d{4}|\\d{2}),(\\d{1,2}):(\\d{1,2}):(\\d{1,2})+(\\.\\d+)");
    std::smatch MatchDateTime;
    if (std::regex_search(Date, MatchDateTime, RxDateTime)) {
        setlocale(LC_ALL, "en_us");
        return {
            .day = stoi(MatchDateTime[1]),
            .mon = stoi(MatchDateTime[2]),
            .year = stoi(MatchDateTime[3]),
            .hour = stoi(MatchDateTime[4]),
            .min = stoi(MatchDateTime[5]),
            .sec = (stoi(MatchDateTime[6]) + stod('0' + std::string(MatchDateTime[7]))),
        };
    }
    else {
        setlocale(0, "");
        std::cerr << "Дата в cfg/cff файле не корректна\n"s;
    }
    return {};
}

std::vector<std::string_view> detail_comtrade_IO::SplitIntoWordsByComma(std::string_view str) {
    std::vector<std::string_view> result;
    const int64_t pos_end = str.npos;
    while (true) {
        int64_t comma = str.find(',');
        if (comma != 0) {
            result.push_back(comma == pos_end ? str.substr(0) : str.substr(0, comma));
        }
        if (comma == pos_end) {
            break;
        }
        else {
            str.remove_prefix(comma + 1);
        }
    }
    return result;
}

std::vector<int> detail_comtrade_IO::SplitIntoNumByComma(std::string& text) {
    erase(text, ' ');
    std::vector<int> result;
    std::string word;
    for (const char c : text) {
        if (c == ',') {
            //if (!word.empty()) {
            result.push_back(stoi(word));
            word.clear();
            //}
        }
        else {
            word += c;
        }
    }
    //if (!word.empty()) {
    result.push_back(stoi(word));
    //}
    return result;
}

std::vector<std::string> detail_comtrade_IO::SplitIntoWordsByComma(std::string& text, REMOVE_SPACE_MODE mode) {
    if (mode == REMOVE_SPACE_MODE::ON) {
        erase(text, ' ');
    }
    std::vector<std::string> words;
    std::string word;
    for (const char c : text) {
        if (c == ',') {
            //if (!word.empty()) {
                words.push_back(word);
                word.clear();
            //}
        }
        else {
            word += c;
        }
    }
    //if (!word.empty()) {
        words.push_back(word);
    //}
    return words;
}

comtrade::AnalogChannel detail_comtrade_IO::ParseAnalogChFromCFG(std::string& text, comtrade::COMTRADE_REV_YEAR REV_YEAR) {
    comtrade::AnalogChannel result;
    std::vector<std::string> raw_an_ch_line = SplitIntoWordsByComma(text, detail_comtrade_IO::REMOVE_SPACE_MODE::OFF);
    int i = 0;
    result.An = std::stoi(raw_an_ch_line.at(i++));
    result.ch_id = raw_an_ch_line.at(i++);
    result.ph = raw_an_ch_line.at(i++);
    result.ccbm = raw_an_ch_line.at(i++);
    result.uu = raw_an_ch_line.at(i++);
    setlocale(LC_ALL, "en_us");
    result.a = stod(raw_an_ch_line.at(i++));
    result.b = stod(raw_an_ch_line.at(i++));
    result.skew = stod(raw_an_ch_line.at(i++));
    result.min = stoi(raw_an_ch_line.at(i++));
    result.max = stoi(raw_an_ch_line.at(i++));
    if (REV_YEAR > comtrade::COMTRADE_REV_YEAR::REV_YEAR_1991) {
        result.primary = stod(raw_an_ch_line.at(i++));
        result.secondary = stod(raw_an_ch_line.at(i++));
        result.PS = raw_an_ch_line.at(i++).find("P"s) != std::string::npos ? comtrade::SCALING_IDENTIFIER::PRIMARY : comtrade::SCALING_IDENTIFIER::SECONDARY;
    }
    setlocale(0, "");
    return result;
}

comtrade::DigitalChannel detail_comtrade_IO::ParseDiscreteChFromCFG(std::string& text, comtrade::COMTRADE_REV_YEAR REV_YEAR) {
    comtrade::DigitalChannel result;
    std::vector<std::string> raw_dig_ch_line = SplitIntoWordsByComma(text, detail_comtrade_IO::REMOVE_SPACE_MODE::OFF);
    int i = 0;
    result.Dn = stoi(raw_dig_ch_line.at(i++));
    result.ch_id = raw_dig_ch_line.at(i++);
    if (REV_YEAR > comtrade::COMTRADE_REV_YEAR::REV_YEAR_1991) {
        result.ph = raw_dig_ch_line.at(i++);
        result.ccbm = raw_dig_ch_line.at(i++);
    }
    result.y = raw_dig_ch_line.at(i++).find("1"s) != std::string::npos;
    return result;
}

comtrade::Comtrade Comtrade_IO::ReadComtrade(const path& cfg_file) {
    comtrade::Comtrade comtrade;
    comtrade.SetFileName(cfg_file.stem().string());
    if (!cfg_file.empty() /*&& !dat_file.empty()*/) {
        static std::regex RxCfgDat("^(.*)\\.([^\\.]+)$");
        std::smatch m;
        const std::string cfg_file_str = cfg_file.string();
        if (std::regex_search(cfg_file_str, m, RxCfgDat)) {
            if (m[2] == "cfg"s) {
                std::string dat_file_path = m[1].str() + ".dat"s;
                comtrade.SetCFG(detail_comtrade_IO::ReadCFG(cfg_file, NULL));
                if (std::filesystem::exists(dat_file_path)) {
                    comtrade.SetDAT(detail_comtrade_IO::ReadDAT(dat_file_path, comtrade.GetCfg(), NULL)); 
                } else {
                    throw std::invalid_argument("dat файла по указанному пути cfg не существует\n"s);
                    std::cerr << "dat файла по указанному пути cfg не существует\n"s;
                }
            } else if (m[2] == "cff"s) {
                std::ifstream raw_cfg_file_in; // поток для чтения
                std::string line;
                raw_cfg_file_in.open(cfg_file, std::ios::in | std::ios::binary); // окрываем файл для чтения
                size_t start_cfg_pos;
                std::getline(raw_cfg_file_in, line); 
                if (line.find("file type: CFG"s) != line.npos) {
                    start_cfg_pos = raw_cfg_file_in.tellg();
                    comtrade.SetCFG(detail_comtrade_IO::ReadCFG(cfg_file, start_cfg_pos));
                }
                while (std::getline(raw_cfg_file_in, line)) {
                    if (line.find("file type: DAT ASCII"s) !=line.npos
                        || line.find("file type: DAT BINARY"s) !=line.npos) {
                        comtrade.SetDAT(detail_comtrade_IO::ReadDAT(cfg_file, comtrade.GetCfg(), raw_cfg_file_in.tellg()));
                        break;                       
                    }
                }             
                /*else if (line.find("file type: INF"s) || line.find("file type:INF"s)) {
                    end_cfg_pos = raw_cfg_file_in.tellg();
                }*/
            } else {
                throw std::format_error("ошибка в формате файла cfg/cff\n"s);
                std::cerr << "ошибка в формате файла от cfg/cff\n"s;
            } 
        } else {
            throw std::format_error("расширение comtrade отличается от cfg/cff\n"s);
            std::cerr << "расширение comtrade отличается от cfg/cff\n"s;
        }
    } else {
        throw std::invalid_argument("cfg файла по указанному пути не существует\n"s);
        std::cerr << "cfg файла по указанному пути не существует\n"s;
    }
    return comtrade;
}

std::vector<comtrade::Comtrade> Comtrade_IO::ReadComtrades(const std::vector<std::filesystem::path>& comtrade_files_path) {
    std::vector<comtrade::Comtrade> comtrade_files;
    std::transform(comtrade_files_path.begin(), comtrade_files_path.end(), std::back_inserter(comtrade_files),
    [](auto cfg_file) {return ReadComtrade(cfg_file);});
    return comtrade_files;
}

comtrade::CFG detail_comtrade_IO::ReadCFG(const path& cfg_file, std::optional<const size_t> start_cfg_pos) {
    setlocale(0, "");
    comtrade::CFG cfg;
	std::ifstream raw_cfg_in; // поток для чтения
	raw_cfg_in.open(cfg_file); // окрываем файл для чтения
    if (start_cfg_pos.has_value()) { // если открываем cff вместо cfg, начинаем читать cfg файл со второй строчки
        raw_cfg_in.seekg(start_cfg_pos.value());
    }
    if (raw_cfg_in.is_open()) {
        std::string line;
        for (int line_no = 1; getline(raw_cfg_in, line); line_no++) {
            switch (line_no) {
                case 1: { // обрабатываем 1-ю строку cfg (уникальное название регистратора, уникальный номер регистратора, версию стандарта
                    std::vector<std::string> first_line = SplitIntoWordsByComma(line, REMOVE_SPACE_MODE::ON);
                    cfg.station_name = first_line.at(0);
                    std::ofstream raw_cfg_out;
                    cfg.rec_dev_id = first_line.at(1);
                    if (first_line.size() > 2) {
                        cfg.rev_year = comtrade::COMTRADE_REV_YEAR(stoi(first_line.at(2)));
                    }
                    break;
                }
                case 2: { // обрабатываем 2-ю cfg строку (общее кол-во каналов + измерения + дискреты)
                    std::vector<std::string> second_line = SplitIntoWordsByComma(line, REMOVE_SPACE_MODE::ON);
                    cfg.TT = std::stoi(second_line.at(0));
                    erase(second_line[1], 'A');
                    erase(second_line[2], 'D');
                    cfg._A = stoi(second_line.at(1));
                    cfg._D = stoi(second_line.at(2));
                    break;
                }
                case 3: { // обрабатываем 3-ю++ строку cfg (векторы измерений + дискретов)
                    while (line_no - 3 < cfg._A) { // вектор измерении
                        cfg.analog_channels.push_back(ParseAnalogChFromCFG(line, cfg.rev_year));
                        getline(raw_cfg_in, line);
                        line_no++;
                    } 
                    while (line_no - 3 - cfg._A < cfg._D) { // векторы дискретов
                        cfg.digital_channels.push_back(ParseDiscreteChFromCFG(line, cfg.rev_year));
                        getline(raw_cfg_in, line);
                        line_no++;
                    }
                    break;
                }
            }
            switch (line_no - 2 - cfg.TT) // обрабатываем строки cfg с частотой
            {
                case 1: { // обрабатываем строку с частотой cfg
                    cfg.lf = stod(line);
                    break;
                }
                case 2: { // обрабатываем строку с кол-вом различных скоростей дискретизации в файле данных
                    cfg.nrates = stoi(line);
                    break;
                }
                case 3: {
                    while (line_no - 5 - cfg.TT < cfg.nrates) { // обрабатываем строки с частотой дискретизации в Гц и номером последней выборки для данной скорости
                        std::vector<std::string> samp_to_endsamp_line = SplitIntoWordsByComma(line, REMOVE_SPACE_MODE::ON);
                        cfg.samp_to_endsamp.push_back({ stod(samp_to_endsamp_line.at(0)) , stoi(samp_to_endsamp_line.at(1)) });
                        getline(raw_cfg_in, line);
                        line_no++;
                    }
                }
            }
            switch (line_no - 4 - cfg.TT - cfg.nrates) { // обрабатываем остальные строки cfg со временем и версией стандарта
                case 1: {
                    cfg.start_time_stamp = GetDigitDate(line); // отметка времени для первого значен-ия в файле данных дата/время
                    break;
                }
                case 2: {
                    cfg.end_time_stamp = GetDigitDate(line); // отметка времени для момента пуска значен-ия в файле данных дата/время
                    setlocale(0, "");
                    break;
                }
                case 3: { // обрабатываем строку с типом файла
                    if (line.find("ASCII"s) != std::string::npos) {
                        cfg.ft = comtrade::FILE_TYPE::ASCII; //тип dat файла - текстовый
                    }
                    else if (line.find("BINARY"s) != std::string::npos) {
                        cfg.ft = comtrade::FILE_TYPE::BINARY; //тип dat файла - бинарный
                    }
                    else {
                        throw std::format_error("Тип dat-файла в cfg/cff файле указан некорректно\n"s);
                        std::cerr << "Тип dat-файла в cfg/cff файле указан некорректно\n"s;
                    }
                    break;
                }
                if (cfg.rev_year > comtrade::COMTRADE_REV_YEAR::REV_YEAR_1991) {
                    case 4: { // обрабатываем строку с коэффициентом умножения для поля разницы во времени (временной метки) в файле данных
                        erase(line, ' ');
                        cfg.timemult = stoi(line);
                        break;
                    }
                    if (cfg.rev_year > comtrade::COMTRADE_REV_YEAR::REV_YEAR_1999) {
                        case 5: { // обрабатываем строку со сдвигом времени UTC
                            std::vector<std::string> time_to_local = SplitIntoWordsByComma(line, REMOVE_SPACE_MODE::ON);
                            static std::regex RxTimeCodeToLocalCode(R"/(s*([+-]?[0-9]+)h?([0-9]*)?)/");
                            std::smatch MatchTimeCodeToLocalCode;
                            if ((std::regex_search(time_to_local[0], MatchTimeCodeToLocalCode, RxTimeCodeToLocalCode))) {
                                setlocale(LC_ALL, "en_us");
                                cfg.tc.hour = stoi(MatchTimeCodeToLocalCode[1]);
                                if (MatchTimeCodeToLocalCode[2].length() > 0) {
                                    cfg.tc.min = stoi(MatchTimeCodeToLocalCode[2]);
                                }
                            }
                            else {
                                setlocale(0, "");
                                std::cerr << "UTC в cfg/cff файле не корректен\n"s;
                            }
                            if ((std::regex_search(time_to_local[1], MatchTimeCodeToLocalCode, RxTimeCodeToLocalCode))) {
                                setlocale(LC_ALL, "en_us");
                                cfg.local_code.hour = stoi(MatchTimeCodeToLocalCode[1]);
                                if (MatchTimeCodeToLocalCode[2].length() > 0) {
                                    cfg.local_code.min = stoi(MatchTimeCodeToLocalCode[2]);
                                }
                            }
                            else {
                                setlocale(0, "");
                                std::cerr << "Cдвиг по времени относительно UTC в cfg/cff файле не корректен\n"s;
                            }
                            break;
                        }
                        case 6: { // обрабатываем строку с полем качества времени в устройстве и доп. секундой
                            std::vector<std::string> tmq_to_leap = SplitIntoWordsByComma(line, REMOVE_SPACE_MODE::ON);
                            cfg.tmq_code = comtrade::TIME_QUALITY_CODE(stoi(tmq_to_leap.at(0)));
                            cfg.leap_sec = comtrade::LEAP_SECOND_IND(stoi(tmq_to_leap.at(1)));
                            break;
                        }
                    }
                }
            }
        }
    }
    raw_cfg_in.close(); // закрываем файл
    return cfg;
}

void detail_comtrade_IO::Parse_ASCII_DAT(std::ifstream& raw_dat_in,
                                    const comtrade::CFG* cfg,
                                    comtrade::DAT* dat) {
    std::string line;
    int sample_number_sum = std::transform_reduce((*cfg).samp_to_endsamp.begin(), (*cfg).samp_to_endsamp.end(), 0, std::plus{}, [](const auto sample) {
    return std::get<1>(sample);
    });
    while (getline(raw_dat_in, line)) {
        std::vector<int> line_values = SplitIntoNumByComma(line);
        (*dat).n.push_back(line_values[0]); // заполняем номер выборки
        (*dat).time.push_back(line_values[1]); // заполняем время выборки
        for (int i = 0; i < (*cfg)._A; i++) {
            (*dat).analog_values[i].push_back(*(line_values.begin() + 2 + i));
        }
        for (int i = 0; i < (*cfg)._D; i++) {
            (*dat).digital_values[i].push_back(*(line_values.begin() + 2 + (*cfg)._A + i));
        }
        if ((*dat).n.back() == sample_number_sum) {
            break;
        }
    }
}

void detail_comtrade_IO::Parse_BINARY_DAT(std::ifstream& raw_dat_in,
const comtrade::CFG* cfg,
comtrade::DAT* dat) {
    size_t dat_line = 8 + 2 * (cfg->_A) + 2 * ceil(double(cfg->_D) / 16.0); //The length of the Þle will vary with the number of channels and the number of samples in the Þle. The number of bytes required for each sample in the Þle will be: (Ak ´ 2) + (2 ´ INT(Dm/16)) + 4 + 4.
    std::vector<unsigned char> buffer(dat_line);
    int sample_number_sum = std::transform_reduce((*cfg).samp_to_endsamp.begin(), (*cfg).samp_to_endsamp.end(), 0, std::plus{}, [](const auto sample) {
        return std::get<1>(sample);
    });
    while (raw_dat_in) {
        for (size_t i = 0; i < dat_line; ++i) {
            unsigned char c = raw_dat_in.get();
            buffer[i] = c;
        }
        dat->n.push_back(((buffer[3] & 0xff) << 24) | ((buffer[2] & 0xff) << 16) | ((buffer[1] & 0xff) << 8) | (buffer[0] & 0xff));
        dat->time.push_back(((buffer[7] & 0xff) << 24) | ((buffer[6] & 0xff) << 16) | ((buffer[5] & 0xff) << 8) | (buffer[4] & 0xff));
        for (int i = 0; i < cfg->_A; i ++) {
            int An = ((buffer[9 + i * 2] & 0xff) << 8) | (buffer[8 + i * 2] & 0xff);
            if (An > 32767) An -= 65536;
            dat->analog_values[i].push_back(An);
        }
        std::vector<bool> DnBytesStatus;
        for (int i = 0; i < 2 * ceil(double(cfg->_D) / 16.0); i++) {
            unsigned int DnByte = (buffer[8 + 2 * (cfg->_A)  + i] & 0xff);
            for (int j = 0; j < 8; j++) {
                DnBytesStatus.push_back(((DnByte & (1 << j)) != 0));
            }
        }
        for (int i = 0; i < cfg->_D; i++) {
            dat->digital_values[i].push_back(DnBytesStatus[i]);
        }
        if ((*dat).n.back() == sample_number_sum) {
            break;
        }
    }
}

comtrade::DAT detail_comtrade_IO::ReadDAT(const path& dat_file, const comtrade::CFG& cfg, std::optional<const size_t> start_dat_pos) {
    comtrade::DAT dat;
    std::ifstream raw_dat_in; // поток для чтения
    dat.analog_values.resize(cfg._A);
    dat.digital_values.resize(cfg._D);
    raw_dat_in.open(dat_file, std::ios::in | std::ios::binary); // окрываем файл для чтения
    if (start_dat_pos.has_value()) { // если открываем cff вместо dat, начинаем читать dat файл со нужной строчки
        raw_dat_in.seekg(start_dat_pos.value());
    }
    if (raw_dat_in.is_open()) {
        if (cfg.ft == comtrade::FILE_TYPE::ASCII) {
            detail_comtrade_IO::Parse_ASCII_DAT(raw_dat_in, &cfg, &dat);
        }
        else if (cfg.ft == comtrade::FILE_TYPE::BINARY) {
            detail_comtrade_IO::Parse_BINARY_DAT(raw_dat_in, &cfg, &dat);
        }
        else {
            std::cerr << "Формат dat файла не определён\n"s;
        }
    }
    return dat;
}

unsigned long long int detail_comtrade_IO::GetFileSize(std::ifstream& file) {
    file.seekg(0, std::ios::end);
    long long int size = abs(static_cast<double>(file.tellg()));
    file.seekg(0, std::ios::beg);
    return size;
}

