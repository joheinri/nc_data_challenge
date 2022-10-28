#pragma once
// A simple class to hold all the input data in an easy-to-interact-with way
// Author: Jochen jens Heinrich 2022

#include <iostream>
#include <fstream>

using namespace std;

class NC_Users {

public:

  NC_Users() {}
  ~NC_Users() {}
  void readData(string file_name);
  int fillPregnantBefore(string label);
  int number_of_users() { return m_users.size(); }
  // Access handles
  int index(int n) { return m_users[n].index; }
  int age(int n) { return m_users[n].age; }
  int pregnant_before(int n) { return m_users[n].pregnant_before; }
  int n_cycles_trying(int n) { return m_users[n].n_cycles_trying; }
  float bmi(int n) { return m_users[n].bmi; }
  float dedication(int n) { return m_users[n].dedication; }
  float average_cycle_length(int n) { return m_users[n].average_cycle_length; }
  float cycle_length_std(int n) { return m_users[n].cycle_length_std; }
  float intercourse_frequency(int n) { return m_users[n].intercourse_frequency; }
  string country(int n) { return m_users[n].country; }
  string education(int n) { return m_users[n].education; }
  string sleeping_pattern(int n) { return m_users[n].sleeping_pattern; }
  string regular_cycle(int n) { return m_users[n].regular_cycle; }
  string outcome(int n) { return m_users[n].outcome; }

private:

  struct User {
	int index;
	int age;
	int pregnant_before;
	int n_cycles_trying;
	float bmi;
	float dedication;
	float average_cycle_length;
	float cycle_length_std;
	float intercourse_frequency;
	string country;
	string education;
	string sleeping_pattern;
	string regular_cycle;
	string outcome;
  };

  vector<User> m_users;

};


void NC_Users::readData(string file_name) {

  printf("Reading input data from file '%s'\n", file_name.c_str());
  ifstream file(file_name.c_str());
  int n_data_read = 0;

  // Read input file
  if (file.is_open()) {

	string prev_preg;

	// First line is labels only, so we ignore it
	string header;
	std::getline(file, header);

	// start reading data
	while (true) {
		User user;
		file >> user.index >> user.bmi >> user.age >> user.country >> prev_preg >> user.education >> user.sleeping_pattern >> user.n_cycles_trying >> user.outcome >> user.dedication >> user.average_cycle_length >> user.cycle_length_std >> user.regular_cycle >> user.intercourse_frequency;

		if (file.eof()) break;

		// Translate the number of previous pregnancies into integer format
		user.pregnant_before = fillPregnantBefore(prev_preg);

		m_users.push_back(user);

		++n_data_read;
        }
  }
  else {
	printf("...Could not open input data file '%s'\n", file_name.c_str());
	exit (EXIT_FAILURE);
  }

  printf("Read %d input lines from file %s and transferred %d entries into analysable format\n", n_data_read, file_name.c_str(), m_users.size());

  return;
}


// Function to translate written previous pregnancy data to integer
int NC_Users::fillPregnantBefore(string label) {
  if (label == "No,never") return 0;
  else if (label == "Yes,once") return 1;
  else if (label == "Yes,twice") return 2;
  else if (label == "Yes,3TimesOrMore") return 3;
  return -1;
}
