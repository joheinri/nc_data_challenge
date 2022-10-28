#pragma once
// A simple class that takes care of the plotting of graphs and histograms for questions 1 and 2
// Author: Jochen jens Heinrich 2022

#include <iostream>
#include "TGraphErrors.h"
#include "TMath.h"
#include "TCanvas.h"
#include "TAxis.h"
#include "TColor.h"
#include "TF1.h"
#include "TLegend.h"
#include "TH1.h"
#include "TLine.h"
#include "TLatex.h"

using namespace std;

class NC_Plotter {

public:

  NC_Plotter() {}
  ~NC_Plotter() {}
  void PlotProbabilityOverCycles(const int size, float probability[], float probability_uncertainty[]);
  template<class T> void stylePlot(T *graph, string x_label, string y_label);
  void DrawHistogram(const int size, TH1 *hist);
  std::vector<float> getPercentiles(TF1 *func, const int size, std::vector<float> perc_values);

private:

  void stylePlot(TGraphErrors *graph, string x_label, string y_label);
};


void NC_Plotter::PlotProbabilityOverCycles(const int size, float probability[], float probability_uncertainty[]) {

  float cycle[size];
  float cycle_uncertainty[size];

  // Need to quickly fill cycle information
  for (int i = 0; i < size; ++i) {
	cycle[i] = i+1;
	cycle_uncertainty[i] = 0.0;
  }

  // Initialise canvas and graph for plotting
  TCanvas *canvas_overallProbability = new TCanvas("canvas_overallProbability","",0,0,800,600);
  TGraphErrors *g_overallProbability = new TGraphErrors(size, cycle, probability, cycle_uncertainty, probability_uncertainty);

  // Make plot look a bit nicer
  stylePlot<TGraphErrors>(g_overallProbability, "Number of menstruation cycles", "Pregnancy probability [%]");

  // Draw graph
  g_overallProbability->Draw("a3");
  g_overallProbability->Draw("lxsame");

  // Fit graph and stylise fit function
  TF1 *fit = new TF1("fit", "[0]*(1-exp(-1.0*[1]*x))", 1.0, size);
  fit->SetParameter(0, 74);
  fit->SetParameter(1, 0.2);
  fit->SetLineColor(1);
  fit->SetLineStyle(3);
  g_overallProbability->Fit("fit", "R");
  fit->Draw("same");

  printf("\nChance of getting pregnant within 13 cycles from fit is %4.3f\n\n", fit->Eval(13));
  // Add a legend to the canvas
  TLegend *legend = new TLegend(0.61,0.25,0.91,0.15);
  legend->SetTextSize(0.035);
  legend->SetBorderSize(0);
  legend->AddEntry(g_overallProbability, "Cummulative probability","fl");
  legend->AddEntry(fit, "Regression","l");
  legend->Draw();

  // Save plot as pdf
  canvas_overallProbability->Print("cummulativeProbability.pdf", "pdf");

  return;
}


// Function to define the style of plots
template<class T> void NC_Plotter::stylePlot(T *graph, string x_label, string y_label) {

  TColor *nc_colour = new TColor(1701, 0.4471, 0.0118, 0.3647);

  graph->GetXaxis()->SetTitle(x_label.c_str());
  graph->GetYaxis()->SetTitle(y_label.c_str());
  graph->SetLineWidth(3);
  graph->SetTitle("");
  graph->SetFillColor(17);
  graph->SetLineColor(1701);
  gPad->SetTopMargin(0.02);
  gPad->SetRightMargin(0.02);

  return;
}


void NC_Plotter::DrawHistogram(const int size, TH1 *hist) {

  // Create canvas and stylise histogram
  TCanvas *canvas_hist = new TCanvas("canvas_hist","",0,0,800,600);
  hist->Draw("hist");
  hist->SetStats(false);
  stylePlot<TH1>(hist, "Number of menstruation cycles", "Number of pregnancies");

  // Fit histogram
  TF1 *fit_hist = new TF1("fit_hist", "[0]*exp(-1.0*[1]*x)", 0.5, size);
  fit_hist->SetLineColor(1);
  fit_hist->SetLineWidth(2);
  fit_hist->SetLineStyle(2);
  hist->Fit("fit_hist", "R");
  fit_hist->Draw("same");

  // Because its interesting, let's get the percentile lines into the plot
  std::vector<float> perc_to_plot = {0.50, 0.80, 0.95};
  std::vector<float> percentiles = getPercentiles(fit_hist, size, perc_to_plot);
  std::vector<TLine*> lines;
  TLatex latex;
  latex.SetTextSize(0.032);
  latex.SetTextColor(kAzure);
  char c_text[35];
  for (int i_perc = 0; i_perc < perc_to_plot.size(); ++i_perc) {
	TLine *line = new TLine(percentiles[i_perc], 0.0, percentiles[i_perc], 250.0);
	printf("The %4.2f percentile corresponds to %4.2f cycles\n", perc_to_plot[i_perc], percentiles[i_perc]);
	line->SetLineColor(kAzure);
	line->SetLineWidth(2);
	lines.push_back(line);
	lines[i_perc]->Draw();
	sprintf(c_text, "#color[860]{%3.1f%%}", perc_to_plot[i_perc]*100.0);
	latex.DrawLatex(percentiles[i_perc]+0.2,240.0,c_text);
  }

  // Add a legend to the canvas
  TLegend *legend_hist = new TLegend(0.76,0.95,0.91,0.85);
  legend_hist->SetTextSize(0.035);
  legend_hist->SetBorderSize(0);
  legend_hist->AddEntry(hist, "Pregnancies","fl");
  legend_hist->AddEntry(fit_hist, "Regression","l");
  legend_hist->Draw();

  // Save plot as pdf
  canvas_hist->Print("pregnanciesInCycle.pdf", "pdf");

  return;
}


// Function to determine the x-axis value of the supplied percentiles
std::vector<float> NC_Plotter::getPercentiles(TF1 *func, const int size, std::vector<float> perc_values) {
  std::vector<float> percentiles;
  float integral = func->Integral(1.0, size);

  // This is a bit ugly, but it works; finding percentile values by integrating in increments
  float step_integral;
  for (float time = 0; time < size; time+=0.05) {
	step_integral = func->Integral(1.0, time);
	if (step_integral / integral > perc_values[0]) {
		percentiles.push_back(time);
		perc_values.erase(perc_values.begin());
	}
	if (perc_values.size() == 0) break;
  }

  return percentiles;
}
