#pragma once
// Class to take care of the correlation determination and plotting for question 3
// Author: Jochen jens Heinrich 2022

#include <iostream>
#include "TH2.h"
#include "TMath.h"
#include "TCanvas.h"
#include "TAxis.h"
#include "TColor.h"
#include "TF1.h"
#include "TLegend.h"

using namespace std;

class NC_Correlator {

public:

  NC_Correlator() {}
  ~NC_Correlator() {}
  float determineCorrelation(vector<pair<float,float>> parameter, string label, int x_bins, float x_low, float x_high, string par_name);
  float determineCorrelation(vector<pair<string,float>> parameter, string label, int x_bins, string par_name);
  void makeCorrelationSummaryGraph(vector<pair<string,float>> correlations);

private:

  void styleHist(TH2 *hist, string x_label, string y_label, string z_label);
  float plotCorrelation(TH2 *hist, string label, int x_bins, float x_low, float x_high, string par_name);

};


float NC_Correlator::determineCorrelation(vector<pair<float,float>> parameter, string label, int x_bins, float x_low, float x_high, string par_name) {

  // Creating and filling a 2D histogram
  TH2 *hist = new TH2F("hist","",x_bins,x_low,x_high,13,0.5,13.5);

  for (int i = 0; i < parameter.size(); ++i) 
	hist->Fill(parameter[i].first, parameter[i].second, 1.0);

  return plotCorrelation(hist, label, x_bins, x_low, x_high, par_name);
}


float NC_Correlator::determineCorrelation(vector<pair<string,float>> parameter, string label, int x_bins, string par_name) {

  // Creating and filling a 2D histogram
  TH2 *hist = new TH2F("hist","",x_bins,0.0,x_bins,13,0.5,13.5);

  for (int i = 0; i < parameter.size(); ++i) 
	hist->Fill(parameter[i].first.c_str(), parameter[i].second, 1.0);

  return plotCorrelation(hist, label, x_bins, 0.0, x_bins, par_name);
}


float NC_Correlator::plotCorrelation(TH2 *hist, string label, int x_bins, float x_low, float x_high, string par_name) {

  // Initialise canvas and graph for plotting
  TCanvas *canvas = new TCanvas("canvas","",0,0,800,600);

  // Make plot look a bit nicer
  styleHist(hist, label, "Number of menstruation cycles", "Pregnancies");

  // Calculate the mean (and standard deviation) for each x-bin individually and put in a hist
  TH1 *hist_average = new TH1F("hist_average", "", x_bins, x_low, x_high);
  for (int i_bin_x = 1; i_bin_x <= hist->GetNbinsX(); ++i_bin_x) {
	int sum_pregnancies = 0;
	int sum_cycles = 0;
	for (int i_bin_y = 1; i_bin_y <= hist->GetNbinsY(); ++i_bin_y) {
		sum_pregnancies += hist->GetBinContent(i_bin_x, i_bin_y);
		sum_cycles += hist->GetBinContent(i_bin_x, i_bin_y) * hist->GetYaxis()->GetBinCenter(i_bin_y);
	}
	if (sum_pregnancies > 0) {
		hist_average->SetBinContent(i_bin_x, (float) sum_cycles / (float) sum_pregnancies);
		hist_average->SetBinError(i_bin_x, ((float) sum_cycles / (float) sum_pregnancies) / sqrt((float) sum_pregnancies));
	}
  }
  hist_average->SetMarkerStyle(20);
  hist_average->SetMarkerSize(1);
  hist_average->SetLineColor(1);

  // Draw graph
  hist->Draw("colz");
  hist_average->Draw("pesame");

  // Use in-built function to get correlation (loss of precision because based on binned date, but acceptable for here)
  double correlation_factor = hist->GetCorrelationFactor();
  TLatex latex;
  latex.SetTextSize(0.035);
  char c_text[35];
  sprintf(c_text, "Correlation factor: %4.2f", correlation_factor);
  latex.DrawLatexNDC(0.11,0.93,c_text);

  // Fit hist and stylise fit function
  TF1 *fit;
  if (fabs(correlation_factor) > 0.10) fit = new TF1("fit", "[0]+[1]*x", x_low, x_high);
  else if (fabs(correlation_factor) <= 0.10) fit = new TF1("fit", "[0]", x_low, x_high);
  fit->SetLineColor(1);
  fit->SetLineWidth(3);
  hist_average->Fit("fit", "R");
  fit->Draw("same");

  // Save plot as pdf
  char output_name[80];
  sprintf(output_name, "correlation_%s.pdf", par_name.c_str());
  canvas->Print(output_name, "pdf");

  return correlation_factor;
}

void NC_Correlator::styleHist(TH2 *hist, string x_label, string y_label, string z_label) {

  // Make a nice colour scheme
  Int_t palette_nominal[200];
  Double_t Red[]    = {0.9000, 0.4471};
  Double_t Green[]  = {0.9000, 0.0118};
  Double_t Blue[]   = {0.9000, 0.3647};
  Double_t Length[] = {0.0000, 1.0000};
  Int_t palette_n = TColor::CreateGradientColorTable(2,Length,Red,Green,Blue,200);
  for (int i = 0; i < 200; ++i) palette_nominal[i] = palette_n+i;

  hist->GetXaxis()->SetTitle(x_label.c_str());
  hist->GetYaxis()->SetTitle(y_label.c_str());
  hist->GetZaxis()->SetTitle(z_label.c_str());
  hist->SetTitle("");
  hist->SetStats(false);
  gPad->SetTopMargin(0.02);
  gPad->SetLeftMargin(0.08);
  gPad->SetRightMargin(0.12);

  return;
}


void NC_Correlator::makeCorrelationSummaryGraph(vector<pair<string,float>> correlations) {

  // Initialise canvas
  TCanvas *canvas_summary = new TCanvas("canvas_summary","",0,0,800,600);

  // Create summary histogram
  TH1 *hist = new TH1F("hist","",correlations.size(),0,correlations.size());

  // Fill histogram ordered according to correlation size
  int i_bin = 1;
  while (correlations.size() > 0) {
	float largest_corr = -1.0;
	int next_item = -1;
	for (int i = 0; i < correlations.size(); ++i) {
		if (fabs(correlations[i].second) > largest_corr) {
			largest_corr = fabs(correlations[i].second);
			next_item = i;
		}
	}
	hist->SetBinContent(i_bin,correlations[next_item].second);
        hist->GetXaxis()->SetBinLabel(i_bin,correlations[next_item].first.c_str());
	correlations.erase(correlations.begin() + next_item);
	++i_bin;
  }

  // Stylise plot
  gPad->SetTopMargin(0.02);
  gPad->SetLeftMargin(0.11);
  gPad->SetBottomMargin(0.135);
  gPad->SetRightMargin(0.09);
  hist->SetMaximum(0.20);
  hist->SetMinimum(-0.20);
  hist->GetYaxis()->SetTitle("Correlation factor");
  hist->GetXaxis()->SetLabelSize(0.05);
  hist->SetFillColor(1701);
  hist->SetStats(false);
  hist->SetBarWidth(0.8);
  hist->Draw("B");

  // Draw lines at 0 and +-10%
  TLine *line_zero = new TLine(0.0, 0.0, 11.0, 0.0);
  line_zero->SetLineWidth(2);
  line_zero->Draw();
  TLine *line_plus = new TLine(0.0, 0.1, 11.0, 0.1);
  line_plus->SetLineWidth(2);
  line_plus->SetLineStyle(2);
  line_plus->Draw();
  TLine *line_minus = new TLine(0.0, -0.1, 11.0, -0.1);
  line_minus->SetLineWidth(2);
  line_minus->SetLineStyle(2);
  line_minus->Draw();

  // Print canvas to file
  canvas_summary->Print("correlation_summary.pdf", "pdf");

  return;
}
