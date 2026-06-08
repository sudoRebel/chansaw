#include "process_manager.h"
#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <dirent.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

struct procStruct {
  std::string PID;
  std::string procName;
  double vmRss;
};

std::vector<procStruct> Process;

void procList() {
  DIR *dir = opendir("/proc");

  if (dir == NULL) {
    perror("Could not open /proc");
    return;
  }

  struct dirent *entry;

  while ((entry = readdir(dir)) != NULL) {
    double kiloByte;

    if (isdigit(entry->d_name[0])) {
      std::string pid = entry->d_name;
      std::string statusFilePath = "/proc/" + pid + "/status";

      std::ifstream statusFile(statusFilePath);

      if (!statusFile.is_open()) {
        continue;
      }

      std::string vmRss = "0";
      std::string procName = "Unknown";
      std::string line;

      while (getline(statusFile, line)) {
        size_t charIndexForName = line.find("Name:");

        if (charIndexForName != std::string::npos) {
          std::string rawName = line.substr(charIndexForName + 5);
          size_t firstChar = rawName.find_first_not_of(" \t\r\n");
          size_t lastChar = rawName.find_last_not_of(" \t\r\n");

          if (firstChar != std::string::npos && lastChar != std::string::npos) {
            procName = rawName.substr(firstChar, lastChar - (firstChar - 1));
          }
        }

        size_t charIndexForVmRSS = line.find("VmRSS:");

        if (charIndexForVmRSS != std::string::npos) {
          std::string rawName = line.substr(charIndexForVmRSS + 6);
          size_t firstChar = rawName.find_first_not_of(" \t\r\n");
          size_t lastChar = rawName.find_last_not_of(" \t\r\n");

          if (firstChar != std::string::npos && lastChar != std::string::npos) {
            vmRss = rawName.substr(firstChar, lastChar - (firstChar - 1));
          }
        }
      }
      statusFile.close();

      try {
        kiloByte = stod(vmRss);
      } catch (const std::invalid_argument &e) {
        kiloByte = 0;
      }

      procStruct p;
      p.PID = pid;
      p.procName = procName;
      p.vmRss = kiloByte;

      Process.push_back(p);
    }
  }

  sort(Process.begin(), Process.end(),
       [](const procStruct &a, const procStruct &b) { return a.vmRss > b.vmRss; });

  for (const auto &p : Process) {

    std::cout << std::left << "PID: " << std::setw(10) << p.PID
              << " Name: " << std::setw(35) << p.procName << " RAM: ";

    if (p.vmRss >= 1099511627776.0) {
      double petaByte = p.vmRss / 1099511627776.0;
      std::cout << petaByte << " PB\n";
    } else if (p.vmRss >= 1073741824.0) {
      double teraByte = p.vmRss / 1073741824.0;
      std::cout << teraByte << " TB\n";
    } else if (p.vmRss >= 1048576.0) {
      double gigaByte = p.vmRss / 1048576.0;
      std::cout << gigaByte << " GB\n";
    } else if (p.vmRss >= 1024.0) {
      double megaByte = p.vmRss / 1024.0;
      std::cout << megaByte << " MB\n";
    } else if (p.vmRss >= 1.0) {
      std::cout << p.vmRss << " kB\n";
    } else {
      double bytes = p.vmRss * 1024.0;
      std::cout << bytes << " Byte\n";
    }
  }

  Process.clear();
  closedir(dir);
}
