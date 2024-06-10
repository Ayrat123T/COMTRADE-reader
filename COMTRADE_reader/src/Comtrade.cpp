#include "Comtrade.h"

std::ostream& operator<<(std::ostream& out_stream, const comtrade::Date_time_stamp& dts) {
	return out_stream << dts.day << "/" << dts.mon << "/" << dts.year << ", " << dts.hour << ':' << dts.min << ':' << dts.sec;
}

std::ostream& operator<<(std::ostream& out_stream, const comtrade::FILE_TYPE& ft) {
	return out_stream << (ft == comtrade::FILE_TYPE::ASCII ? "ASCII" : "BINARY");
}

std::ostream& operator<<(std::ostream& out_stream, const comtrade::time_code& tc) {
	return out_stream << tc.hour << ':' << tc.min;
}

std::ostream& operator<<(std::ostream& out_stream, const std::vector<comtrade::AnalogChannel>& analog_channels) {
	if (!analog_channels.empty()) {
		out_stream << "\t\tanalog channel index number - identifer:" << std::endl;
		for (const auto& value : analog_channels) {
			out_stream << "\t\t" << value.An << " - " << value.ch_id << std::endl;
		}
	}
	else {
		out_stream << "empty" << std::endl;
	}
	return out_stream;
}

std::ostream& operator<<(std::ostream& out_stream, const std::vector<comtrade::DigitalChannel>& digital_channels) {
	if (!digital_channels.empty()) {
		out_stream << "\t\tdigital channel index number - identifer:" << std::endl;
		for (const auto& value : digital_channels) {
			out_stream << "\t\t" << value.Dn << " - " << value.ch_id << std::endl;
		}
	}
	else {
		out_stream << "empty" << std::endl;
	}
	return out_stream;
}

std::ostream& operator<<(std::ostream& out_stream, const std::vector<std::pair<double, int>>& samp_to_endsamp) {
	for (const auto& [key, value] : samp_to_endsamp) {
		out_stream << "Sample rate in Hertz: " << key << ", last sample number at sample rate: " << value << std::endl;
	}
	return out_stream;
}

void comtrade::Comtrade::PrintCFG(std::ostream& out_stream) {
	out_stream  << "comtrade file name: " << comtrade_file_name_ << std::endl
				<< "station name: " << cfg_.station_name << std::endl
				<< "rec dev id: " << cfg_.rec_dev_id << std::endl
				<< "rev year: " << cfg_.rev_year << std::endl
				<< "total number of channels: " << cfg_.TT << std::endl
				<< "number of analog channels: " << cfg_._A << std::endl
				<< "number of status channels: " << cfg_._D << std::endl
				<< "\tanalog channels:\n" << cfg_.analog_channels
				<< "\tdigital channels:\n" << cfg_.digital_channels
				<< "Line frequency: " << cfg_.lf << std::endl
				<< "number of sampling rates: " << cfg_.nrates << std::endl
				<< "sample rate in Hertz to last sample number at sample rate: " << cfg_.samp_to_endsamp
				<< "start time stamp: " << cfg_.start_time_stamp << std::endl
				<< "end time stamp: " << cfg_.end_time_stamp << std::endl
				<< "file type: " << cfg_.ft << std::endl;
	if (cfg_.rev_year > 1991) {
		out_stream	<< "multiplication factor for the time: " << cfg_.timemult << std::endl;
		if (cfg_.rev_year == 2013) {
			out_stream	<< "time code: " << cfg_.tc << std::endl
						<< "local code: " << cfg_.local_code << std::endl
						<< "TIME QUALITY CODE: " << cfg_.tmq_code << std::endl
						<< "LEAP SECOND IND: " << cfg_.leap_sec << std::endl;
		}
	}
}

void comtrade::Comtrade::PrintDatInfo(std::ostream& out_stream) {
	double sample_rate_sum = std::transform_reduce(cfg_.samp_to_endsamp.begin(),
	cfg_.samp_to_endsamp.end(), 0, std::plus{}, [](const auto sample) {
	return std::get<0>(sample);
	});
	out_stream << "number of sample numbers in dat: " << dat_.n.size() << std::endl
			   << "duration of signals in dat: " << double(dat_.time.back() - dat_.time.front()) / sample_rate_sum / 1000.0 << " c\n"s
			   << "number of analog channels in dat: " << dat_.analog_values.size() << std::endl
			   << "number of digital channels in dat: " << dat_.digital_values.size() << std::endl;
}

bool comtrade::Comtrade::CheckComtrade() {
	if (cfg_._A != dat_.analog_values.size()) {
		std::cerr << "В " << comtrade_file_name_
			<< " количество аналоговых каналов в cfg (" << cfg_._A <<")\n"s
			<< "не совпадает с фактическим в dat (" << dat_.analog_values.size() <<")\n"s;
		return false;
	} else if (cfg_._D != dat_.digital_values.size()) {
		std::cerr << "В " << comtrade_file_name_
			<< " количество дискретных каналов в cfg (" << cfg_._D <<")\n"s
			<< "не совпадает с фактическим в dat (" << dat_.digital_values.size() <<")\n"s;
		return false;
	}
	int sample_number_sum = std::transform_reduce(cfg_.samp_to_endsamp.begin(), cfg_.samp_to_endsamp.end(), 0, std::plus{}, [](const auto sample) {
		return std::get<1>(sample);
		});
	if (sample_number_sum != dat_.n.size()) {
		std::cerr << "В " << comtrade_file_name_
			<< " cуммарное число выборок в cfg (" << sample_number_sum <<")\n"s
			<< "не совпадает с фактическим в dat (" << dat_.n.size() <<")\n"s;
		return false;
	}
	for (int i = 0; i < dat_.analog_values.size(); i++) {
		if (sample_number_sum != dat_.analog_values[i].size()) {
			std::cerr << "В " << comtrade_file_name_
				<< " cуммарное число выборок в cfg (" << sample_number_sum <<")\n"s
				<< "не совпадает с фактическим в dat (" << dat_.analog_values[i].size() <<")\n"s
				<< "в аналоговом канале № "s << 1 + i << "\n"s;
			return false;
		}
	}
	for (int i = 0; i < dat_.digital_values.size(); i++) {
		if (sample_number_sum != dat_.digital_values[i].size()) {
			std::cerr << "В " << comtrade_file_name_
				<< " cуммарное число выборок в cfg (" << sample_number_sum <<")\n"s
				<< "не совпадает с фактическим в dat (" << dat_.analog_values[i].size() <<")\n"s
				<< "в дискретном канале № "s << 1 + i << "\n"s;
			return false;
		}
	}
	return true;
}

comtrade::CFG comtrade::Comtrade::GetCfg() {
	return cfg_;
}

comtrade::DAT comtrade::Comtrade::GetDat() {
	return dat_;
}