#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <windows.h>
#include <stdlib.h>
#include <math.h>

#include "InputReader.h"
#include "DirectoryMonitor.h"

using namespace std;
using namespace std::literals;

void TestSinglelaunch(const filesystem::path& comtrade_files_path) {
    DirectoryMonitor::DirectoryMonitor directory_monitor(filesystem::path{ comtrade_files_path });
    directory_monitor.PrintTree(cout);
    directory_monitor.SetComtradeFiles(directory_monitor.GetRawDirectory());
    std::vector<comtrade::Comtrade> comtrades = Comtrade_IO::ReadComtrades(directory_monitor.GetComtradeFilesPath());
    directory_monitor.PrintComtradeFilesNames(std::cout);
    /* расчёты */ 
    for (auto comtrade : comtrades) {
        comtrade.CheckComtrade();
        comtrade.PrintCFG(cout);
        comtrade.PrintDatInfo(cout);
        cout << '\n';
    } 
}

void TestSinglelaunchWithCreateArchive(const filesystem::path& comtrade_files_path) {
    DirectoryMonitor::DirectoryMonitor directory_monitor(filesystem::path{ comtrade_files_path });
    directory_monitor.PrintTree(cout);
    directory_monitor.CreateArchive(std::nullopt);
    std::vector<comtrade::Comtrade> comtrades = Comtrade_IO::ReadComtrades(directory_monitor.GetComtradeFilesPath());
    directory_monitor.PrintComtradeFilesNames(std::cout);
    /* расчёты */ 
    for (auto comtrade : comtrades) {
        comtrade.CheckComtrade();
        comtrade.PrintCFG(cout);
        comtrade.PrintDatInfo(cout);
        cout << '\n';
    } 
}

void TestMonitoring(const filesystem::path& comtrade_files_path) {
    DirectoryMonitor::DirectoryMonitor directory_monitor(filesystem::path{ comtrade_files_path });
    directory_monitor.PrintTree(cout);
    directory_monitor.StartMonitoring();
    std::vector<comtrade::Comtrade> comtrades = Comtrade_IO::ReadComtrades(directory_monitor.GetComtradeFilesPath());
    for (auto comtrade : comtrades) {
        comtrade.CheckComtrade();
        comtrade.PrintCFG(cout);
        comtrade.PrintDatInfo(cout);
        cout << '\n';
    }   
}

int main(int argc, char*argv[]) {
    TCHAR buffer[MAX_PATH];
    GetCurrentDirectory(sizeof(buffer),buffer); //Получить текущую деректорию
    const filesystem::path comtrade_files_path = path(buffer).parent_path() / "oscillograms"s; //Получить деректорию с COMTRADE файлами
    TestSinglelaunch(comtrade_files_path);
}