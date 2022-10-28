// Main script to run the Natural Cycles data challenge
// Author: Jochen jens Heinrich 2022

#include <iostream>
#include "NC_User.h"
#include "NC_Plotter.h"
#include "NC_Correlator.h"

using namespace std;

void NC_DataChallenge() {

  // Initialise data handler
  NC_Users *nc_user = new NC_Users();
  nc_user->readData("data.list");

  // Initialise Plotter
  NC_Plotter *nc_plotter = new NC_Plotter();

  // Initialise Correlator
  NC_Correlator *nc_correlator = new NC_Correlator();

  const int n_cycles = 15;

////////////////////////////////////////////////////////////
// What is the chance of getting pregnant within 13 cycles?
////////////////////////////////////////////////////////////

  // Counter for number of women attempting the get pregnant within the indexed cycle
  int all_attempts[n_cycles] = {0};
  // Counter of women who became pregnant within the indexed cycle
  int pregnancies[n_cycles] = {0};

  // Loop over all data items
  for (int i = 0; i < nc_user->number_of_users(); ++i) {

	// Only consider women who are actively trying,i.e. intercourse_frequency > 0
	// FIXME Assume a lot of women do not log intercourse
	//if (nc_user->intercourse_frequency(i) == 0) continue;
	
	// Increase all_attempts counter for all cycles the woman used NC	
	for (int i_cycle = 0; i_cycle < nc_user->n_cycles_trying(i); ++i_cycle) 
		++all_attempts[i_cycle];
	// Note the cycle the woman got pregnant
	if (nc_user->outcome(i) == "pregnant") ++pregnancies[nc_user->n_cycles_trying(i)-1];

  }

  // From the counts calculate the cummulative probability for a pregnancy (with uncertainty)
  float cummulative_probability[n_cycles], cumm_uncertainty[n_cycles];
  float cumm_prob = 0.0, cumm_uncert = 0.0;

  for (int j = 0; j < n_cycles; ++j) {

	// Calculate cummulative probability for pregnancy
	float frac = (float) pregnancies[j] / (float) all_attempts[j];
	cumm_prob = cumm_prob+((1.0-cumm_prob)*frac);
	cummulative_probability[j] = cumm_prob * 100.0;

	// And also keep track of the associated uncertainty
	float frac_uncert_sq = frac*(1.0-frac)/((float) all_attempts[j]);
	cumm_uncert = sqrt( (1.0-cumm_prob)*(1.0-frac)*frac_uncert_sq + (1.0-frac)*(1.0-frac)*cumm_uncert*cumm_uncert);
	cumm_uncertainty[j] = cumm_uncert * 100.0;
  }

  // Hand over to plotting
  nc_plotter->PlotProbabilityOverCycles(n_cycles, cummulative_probability, cumm_uncertainty);

////////////////////////////////////////////////////////////
// How long does it usually take to get pregnant?
////////////////////////////////////////////////////////////

  // Create a histogram
  TH1 *hist_pregnancy_cycle = new TH1F("hist_pregnancy_cycle", "Cycles until pregnancy", n_cycles, 0.5, n_cycles+0.5);

  // Loop over all data items
  for (int i = 0; i < nc_user->number_of_users(); ++i) {
  
	// Women who did not get pregnant can be ignored for this question
	if (nc_user->outcome(i) == "not_pregnant") continue;

	// Fill histogram with the cycle numbers of the pregnancy
	// FIXME Could also do this in days, but only have cycle information for point of pregnancy so no need to pretend to have a more precise number than we do
	hist_pregnancy_cycle->Fill(nc_user->n_cycles_trying(i));
  }

  // Hand over histogram to plotter
  nc_plotter->DrawHistogram(n_cycles, hist_pregnancy_cycle);

////////////////////////////////////////////////////////////
// What factors impact the time it takes to get pregnant?
////////////////////////////////////////////////////////////

  // To determine the impact of a factor we can look at the correlation between the parameter and the time it takes to get pregnant
  vector<pair<float,float>> vec_p_bmi, vec_p_age, vec_p_pregnant_before, vec_p_dedication, vec_p_average_cycle_length, vec_p_cycle_length_std, vec_p_intercourse_frequency;
  vector<pair<string,float>> vec_p_country, vec_p_education, vec_p_sleeping_pattern, vec_p_regular_cycle;
  for (int i = 0; i < nc_user->number_of_users(); ++i) {

	// Again we ignore the data of women who did not succeed in getting pregnant
	if (nc_user->outcome(i) == "not_pregnant") continue;

	// Ignore data without parameter information, i.e. if the parameter is set to -1 and fill the vectors
	if (nc_user->bmi(i) > -1) 
		vec_p_bmi.push_back(make_pair<float,float>(nc_user->bmi(i),nc_user->n_cycles_trying(i)));
	if (nc_user->age(i) > -1) 
		vec_p_age.push_back(make_pair<float,float>(nc_user->age(i),nc_user->n_cycles_trying(i)));
	if (nc_user->country(i) != "-1") 
		vec_p_country.push_back(make_pair<string,float>(nc_user->country(i),nc_user->n_cycles_trying(i)));
	if (nc_user->pregnant_before(i) > -1) 
		vec_p_pregnant_before.push_back(make_pair<float,float>(nc_user->pregnant_before(i),nc_user->n_cycles_trying(i)));
	if (nc_user->education(i) != "-1") 
		vec_p_education.push_back(make_pair<string,float>(nc_user->education(i),nc_user->n_cycles_trying(i)));
	if (nc_user->sleeping_pattern(i) != "-1") 
		vec_p_sleeping_pattern.push_back(make_pair<string,float>(nc_user->sleeping_pattern(i),nc_user->n_cycles_trying(i)));
	if (nc_user->dedication(i) > -1) 
		vec_p_dedication.push_back(make_pair<float,float>(nc_user->dedication(i),nc_user->n_cycles_trying(i)));
	if (nc_user->average_cycle_length(i) > -1) 
		vec_p_average_cycle_length.push_back(make_pair<float,float>(nc_user->average_cycle_length(i),nc_user->n_cycles_trying(i)));
	if (nc_user->cycle_length_std(i) > -1) 
		vec_p_cycle_length_std.push_back(make_pair<float,float>(nc_user->cycle_length_std(i),nc_user->n_cycles_trying(i)));
	if (nc_user->regular_cycle(i) != "-1") 
		vec_p_regular_cycle.push_back(make_pair<string,float>(nc_user->regular_cycle(i),nc_user->n_cycles_trying(i)));
	if (nc_user->intercourse_frequency(i) > -1) 
		vec_p_intercourse_frequency.push_back(make_pair<float,float>(nc_user->intercourse_frequency(i),nc_user->n_cycles_trying(i)));

  }

  // Declare a vector that will hold variable names and correlations
  vector<pair<string,float>> vec_correlations;

  // Fill vector and plot all the correlation histograms while we are at it
  vec_correlations.push_back(make_pair<string,float>("BMI",
	nc_correlator->determineCorrelation(vec_p_bmi, "BMI", 25, 15.0, 40.0, "bmi")));
  vec_correlations.push_back(make_pair<string,float>("Age",
	nc_correlator->determineCorrelation(vec_p_age, "Age [years]", 23, 21.5, 44.5, "age")));
  vec_correlations.push_back(make_pair<string,float>("Country",
	nc_correlator->determineCorrelation(vec_p_country, "Country", 28, "country")));
  vec_correlations.push_back(make_pair<string,float>("pregnant_before",
	nc_correlator->determineCorrelation(vec_p_pregnant_before, "Number of previous pregnancies", 4, -0.5, 3.5, "pregnant_before")));
  vec_correlations.push_back(make_pair<string,float>("education",
	nc_correlator->determineCorrelation(vec_p_education, "Education", 5, "education")));
  vec_correlations.push_back(make_pair<string,float>("sleeping_pattern",
	nc_correlator->determineCorrelation(vec_p_sleeping_pattern, "Sleeping pattern", 5, "sleeping_pattern")));
  vec_correlations.push_back(make_pair<string,float>("dedication",
	nc_correlator->determineCorrelation(vec_p_dedication, "Dedication", 30, 0.0, 1.0, "dedication")));
  vec_correlations.push_back(make_pair<string,float>("average_cycle_length",
	nc_correlator->determineCorrelation(vec_p_average_cycle_length, "Average cycle length [days]", 20, 20.0, 40.0, "average_cycle_length")));
  vec_correlations.push_back(make_pair<string,float>("cycle_length_std",
	nc_correlator->determineCorrelation(vec_p_cycle_length_std, "Variation of cycle length [days]", 20, 0.0, 9.0, "cycle_length_std")));
  vec_correlations.push_back(make_pair<string,float>("regular_cycle",
	nc_correlator->determineCorrelation(vec_p_regular_cycle, "Regular Cycle", 2, "regular_cycle")));
  vec_correlations.push_back(make_pair<string,float>("intercourse_frequency",
	nc_correlator->determineCorrelation(vec_p_intercourse_frequency, "Intercourse frequency [per day]", 20, 0.0, 0.8, "intercourse_frequency")));

  // Lastly, plot a graph that shows the obtained correlation factors
  nc_correlator->makeCorrelationSummaryGraph(vec_correlations);

  cout << "Analysis completed successfully" << endl;
}


