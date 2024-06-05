#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <functional>
#include <numeric>
#include <ranges>
#include <stdexcept>

using namespace std::literals;

namespace comtrade {

	enum COMTRADE_REV_YEAR { //версия стандарта COMTRADE
		REV_YEAR_1991 = 1991,
		REV_YEAR_1999 = 1999,
		REV_YEAR_2013 = 2013
	};

	struct Date_time_stamp { //штамп времени
		int day = 31; //день месяца от 1 до 31
		int mon = 12; // месяц от 0 до 11
		int year = 1970; // год, начиная с 1900
		int hour = 0; // часы от 0 до 23
		int min = 0; // минуты от 0 до 59
		double sec = 0.0; // секунды
	};

	enum FILE_TYPE { //тип dat файла
		ASCII,
		BINARY
	};

	struct time_code { // сдвиг по времени относительно UTC
		int hour = +3; // часы от 0 до 23
		int min = 0; // минуты от 0 до 59
	};

	enum TIME_QUALITY_CODE { //индикатор качества времени в устройстве(задан в DEC)
		NORMAL_OPERATION_CLOCK_LOCKED,
		CLOCK_UNLOCKED_TIME_WITHIN_10eMINUS9s,
		CLOCK_UNLOCKED_TIME_WITHIN_10eMINUS8s,
		CLOCK_UNLOCKED_TIME_WITHIN_10eMINUS7s,
		CLOCK_UNLOCKED_TIME_WITHIN_10eMINUS6s,
		CLOCK_UNLOCKED_TIME_WITHIN_10eMINUS5s,
		CLOCK_UNLOCKED_TIME_WITHIN_10eMINUS4s,
		CLOCK_UNLOCKED_TIME_WITHIN_10eMINUS3s,
		CLOCK_UNLOCKED_TIME_WITHIN_10eMINUS2s,
		CLOCK_UNLOCKED_TIME_WITHIN_10eMINUS1s,
		CLOCK_UNLOCKED_TIME_WITHIN_1s,
		CLOCK_UNLOCKED_TIME_WITHIN_10s,
		FAULT_CLOCK_FAILURE_TIME_NOT_RELIABLE
	};

	enum LEAP_SECOND_IND { //индикатор дополнительной секунды в устройстве
		NO_LEAP_SEC_IN_REC,
		LEAP_SEC_ADD_IN_REC,
		LEAP_SEC_SUB_IN_REC,
		TIME_SRS_NO_CAPABILITY_TO_ADDRESS_LEAP_SEC
	};

	enum SCALING_IDENTIFIER { // ИДЕНТИФИКАТОР МАСШТАБИРОВАНИЯ 
		PRIMARY,
		SECONDARY
	};

	struct AnalogChannel { // Analog channel information
		int An; // analog channel index number
		std::string ch_id; // identifer
		std::string ph; // phase identification
		std::string ccbm; // circuit component being monitored
		std::string uu; // units
		double a; // multiplier
		double b; // offset adder
		double skew; // time skew from start of sample period
		int min; // range minimum data value for data values of this channel
		int max; // range maximum data value for data values	of this channel
		double primary; // voltage or current transformer ratio primary factor
		double secondary; // voltage or current transformer ratio secondary factor
		SCALING_IDENTIFIER PS; // primary or secondary data scaling identifier
	};

	struct DigitalChannel { // Digital channel information
		int Dn; // status channel index number
		std::string ch_id; // channel name
		std::string ph; // phase identification
		std::string ccbm; // circuit component being monitored
		bool y; // normal state of status channel
	};

	struct CFG { // The confguration file
		std::string station_name; // name of the substation location
		std::string rec_dev_id; // identifcation number or name of the recording device
		COMTRADE_REV_YEAR rev_year = COMTRADE_REV_YEAR::REV_YEAR_1991; // year of the standard revision
		int TT; // total number of channels
		int _A; // number of analog channels followed by identifier A
		int _D; // number of status channels followed by identifier D
		std::vector <AnalogChannel> analog_channels;
		std::vector <DigitalChannel> digital_channels;
		double lf; // Line frequency
		int nrates = 1; // number of sampling rates in the data file
		std::vector <std::pair<double, int>> samp_to_endsamp; // sample rate in Hertz to last sample number at sample rate
		Date_time_stamp start_time_stamp; // first one is for the time of the first data value in the data file
		Date_time_stamp end_time_stamp; // second one is for the time of the trigger point
		FILE_TYPE ft; // file type
		int timemult = 1; // multiplication factor for the time differential (timestamp) field in the data file (коэффициент умножения для поля разницы во времени (временной метки) в файле данных)
		time_code tc; // UTC
		time_code local_code; // сдвиг по времени относительно UTC
		TIME_QUALITY_CODE tmq_code = TIME_QUALITY_CODE::NORMAL_OPERATION_CLOCK_LOCKED; // индикатор качества времени в устройстве (задан в DEC)
		LEAP_SECOND_IND leap_sec = LEAP_SECOND_IND::NO_LEAP_SEC_IN_REC; //индикатор дополнительной секунды в устройстве
	};

	struct DAT {
		std::vector<unsigned int> n; // sample number
		std::vector<unsigned int> time; // time stamp
		std::vector<std::vector<int>> analog_values;
		std::vector<std::vector<bool>> digital_values;
	};

	// COMTRADE file
	class Comtrade {
	public:

		Comtrade() = default;
		Comtrade(const CFG& cfg, const DAT& dat) : cfg_(cfg), dat_(dat) {}

		void SetFileName(const std::string& comtrade_file_name) { comtrade_file_name_ = comtrade_file_name; }

		void SetCFG(const CFG& cfg) { cfg_ = cfg; }

		void SetDAT(const DAT& dat) { dat_ = dat; }

		void PrintCFG(std::ostream& out_stream);

		void PrintDatInfo(std::ostream& out_stream);

		bool CheckComtrade();

		CFG GetCfg();
		
		DAT GetDat();

	private:
		CFG cfg_;
		DAT dat_;
		std::string comtrade_file_name_;
	};

} //namespace comtrade