#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

// Use directives to avoid repeatedly writing std::
using std::cin;
using std::cout;
using std::endl;
using std::getline;
using std::ifstream;
using std::stoi;
using std::string;

void loger(const string &log) { cout << "  DEBUG: " << log << endl; }

void notif(const string &body) {
  string cmd =
      "su -lp 2000 -c \"cmd notification post -S bigtext -t 'zcharge' 'Tag' '" +
      body + "'\"";
  system(cmd.c_str());
}

int read_bat_temp() {
  ifstream file("/sys/class/power_supply/battery/temp");
  int temp;
  file >> temp;
  return temp;
}

int read_capacity() {
  ifstream file("/sys/class/power_supply/battery/capacity");
  int capacity;
  file >> capacity;
  return capacity;
}

string read_charging_state() {
  ifstream file("/sys/class/power_supply/battery/status");
  string status;
  file >> status;
  return status;
}

void switch_off() {
  loger("Switching off charging");
  // Add your implementation here
}

void switch_on() {
  loger("Switching on charging");
  // Add your implementation here
}

void limiter_service(const string &conf) {
  int recharging_limit, capacity_limit, temp_limit;

  ifstream config(conf);
  string line;
  while (getline(config, line)) {
    if (line.find("recharging_limit") != string::npos)
      recharging_limit = stoi(line.substr(line.find('=') + 2));
    else if (line.find("capacity_limit") != string::npos)
      capacity_limit = stoi(line.substr(line.find('=') + 2));
    else if (line.find("temperature_limit") != string::npos)
      temp_limit = stoi(line.substr(line.find('=') + 2));
  }

  while (true) {
    int capacity = read_capacity();
    string charging_state = read_charging_state();

    if (charging_state == "Charging" && capacity >= capacity_limit) {
      std::this_thread::sleep_for(std::chrono::seconds(30));
      switch_off();
    }

    if (read_bat_temp() >= temp_limit) {
      switch_off();
      while (read_bat_temp() > temp_limit - 10) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
      }

      if (capacity < capacity_limit) {
        switch_on();
      }
    }

    if (capacity <= recharging_limit) {
      switch_on();
    }

    std::this_thread::sleep_for(std::chrono::seconds(5));
  }
}

int main() {
  string conf = "/path/to/your/config/file";

  std::thread service_thread(limiter_service, conf);

  pid_t pid = getpid();
  cout << "zcharge service activated with PID " << pid << endl;

  service_thread.join();

  return 0;
}