// Code to compare the tracking performances: Truth seeding vs real seeding
// Shyam Kumar; shyam.kumar@ba.infn.it; shyam055119@gmail.com

#include "TGraphErrors.h"
#include "TF1.h"
#include "TRandom.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TMath.h"
#include <vector>
#include "TAxis.h"              // Ensure that TAxis is included
#include "TGraph.h"             // Ensure that TGraph is included
#include "TFormula.h"           // Ensure that TFormula is included
#include <cstdlib>

#define mpi 0.139  // 1.864 GeV/c^2

void draw_req_Mom(double etamin, double etamax, double xmin=0., double xmax=0.);
TGraphErrors* draw_directory(TString directory, TString particle = "pi-",double etamin=-1.0, double etamax=1.0, double range =0.3, 
//some new arguments
const Int_t nfiles=1,
std::vector<double> mom={0}, //note that this change is fine since [] is overloaded in vector
Bool_t drawreq=1, TString epic ="", TString eicrecon = "") // name = p, pt for getting p or pt dependence fitted results
{
	gSystem->cd(directory);
	std::vector<double> momV_real, momresolV_real, err_momresolV_real;
	momV_real.clear(); momresolV_real.clear(); err_momresolV_real.clear();
	  
	
	TFile *fmom_real[nfiles];
	
    //fit to the histogram will be a gaussian
    TF1 *func_real = new TF1("func_real","gaus",-0.5,0.5);
	
    for (int i =0; i<nfiles; ++i){

	 fmom_real[i] = TFile::Open(Form("realseed/pi-/mom/Performances_mom_%1.1f_mom_resol_realseed_%s.root",mom[i],particle.Data()));

	 TH1D *hist_real = (TH1D*) fmom_real[i]->Get(Form("hist_mom_%1.1f_%1.1f_pmax_%1.1f",mom[i],etamin,etamax));
	 hist_real->Rebin(2);
	 hist_real->SetName(Form("realseed_hist_mom_%1.1f_%1.1f_eta_%1.1f_%s",mom[i],etamin,etamax,particle.Data()));
	 hist_real->SetTitle(Form("Realistic Seed (%s): Momentum = %1.1f && %1.1f<#eta<%1.1f;#Delta p/p; Entries(a.u.)",particle.Data(),mom[i],etamin,etamax));
	 
	 double mu_real = hist_real->GetMean(); 
	 double sigma_real = hist_real->GetStdDev();
         hist_real->GetXaxis()->SetRangeUser(-1.0*range,1.0*range);
         func_real->SetRange(mu_real-2.0*sigma_real,mu_real+2.0*sigma_real); // fit with in 2 sigma range
	 hist_real->Fit(func_real,"NR+");
	 mu_real = func_real->GetParameter(1); 
	 sigma_real = func_real->GetParameter(2);
	 func_real->SetRange(mu_real-2.0*sigma_real,mu_real+2.0*sigma_real);
	 hist_real->Fit(func_real,"R+");
	 float real_par2 = func_real->GetParameter(2)*100;
	 float real_par2_err = func_real->GetParError(2)*100;
	 momV_real.push_back(mom[i]);
	 momresolV_real.push_back(real_par2);
	 err_momresolV_real.push_back(real_par2_err);
	} // all files
	
    const int size_real = momV_real.size();
	double p_real[size_real], err_p_real[size_real], sigma_p_real[size_real], err_sigma_p_real[size_real]; 
	
	for (int i=0; i<size_real; i++){
	p_real[i] = momV_real.at(i);
        sigma_p_real[i] = momresolV_real.at(i);
	err_sigma_p_real[i] = err_momresolV_real.at(i);
	err_p_real[i] = 0.;
	}

    TGraphErrors *gr2 = new TGraphErrors(size_real,p_real,sigma_p_real,err_p_real,err_sigma_p_real);
    gr2->SetName("gr_realseed");
	gr2->SetMarkerStyle(34);
	gr2->SetMarkerSize(1.0);
	gr2->SetTitle(";p (GeV/c);#sigmap/p");
	gr2->GetXaxis()->CenterTitle();
	gr2->GetYaxis()->CenterTitle();

	//leave the directory
	gSystem->cd("..");

	return gr2;
}

void doCompare_results_mom(std::vector<TString> directory_list) 
{
	//change the following as required
	std::vector<double> mom={0.50, 0.75, 1.00, 1.25, 1.75, 2.00, 2.50, 3.00, 4.00, 5.00, 7.00, 8.50, 10.00};
	const Int_t nfiles = mom.size();
	TString particle = "pi-";
	double etamin=-1.0; //etamin
	double etamax=1.0; //etamax
	double range =0.3; //range, we rarely change this
	std::vector<Color_t> colors = {kRed, kBlue, kGreen, kCyan, kMagenta, kOrange, kBlack};
	
	//=== style of the plot=========
   	gStyle->SetPalette(1);
   	gStyle->SetOptTitle(1);
   	gStyle->SetTitleOffset(1.0,"XY");
   	gStyle->SetTitleSize(.04,"XY");
   	gStyle->SetLabelSize(.04,"XY");
   	gStyle->SetHistLineWidth(2);
   	gStyle->SetOptFit(1);
   	gStyle->SetOptStat(1);

	TString symbolname = "";
	if (particle == "pi-") symbolname = "#pi^{-}"; 
	else symbolname = particle;

	TF1 *f1=new TF1("f1","FitMomentumResolution",0.,30.0,2);
   	f1->SetParLimits(0,0.,0.1);	
   	f1->SetParLimits(1,0.,5.0);	
   
   	TCanvas *c_mom = new TCanvas("cmom","cmom",1400,1000);
   	c_mom->SetMargin(0.10, 0.05 ,0.1,0.05);
   	c_mom->SetGridy();
	
	//set up stuff about the graph
	TMultiGraph *mgMom; 
	TLegend *lmom; 
	mgMom = new TMultiGraph("mgMom",";p (GeV/c); #sigmap/p %");
	lmom = new TLegend(0.65,0.80,0.90,0.93);
	lmom->SetTextSize(0.03);
	lmom->SetBorderSize(0);
	lmom->SetHeader(Form("%s ePIC(%s/%s): %1.1f < #eta < %1.1f",symbolname.Data(),"","",etamin,etamax),"C");

	int i = 0;
	for(auto& directory : directory_list)
	{
		TGraphErrors* new_graph = draw_directory(directory, 
												 particle, 
												 etamin, 
												 etamax,
												 range,
												 nfiles, 
												 mom); //momentum array std::vector<double>
		new_graph->SetMarkerColor(colors[i]);
		mgMom->Add(new_graph);
		lmom->AddEntry(new_graph, directory); //add entry to the legend
		i = (i+1) % colors.size();
	}


	
	c_mom->cd();
	mgMom->GetXaxis()->SetRangeUser(0.40,15.2);
	// Reduce this range to better see the increase
	mgMom->GetYaxis()->SetRangeUser(0.,2.0);
	mgMom->Draw("AP");
	lmom->Draw("same");
	
	draw_req_Mom(etamin,etamax,0.,mgMom->GetXaxis()->GetXmax());
	c_mom->SaveAs(Form("mom_resol_%1.1f_eta_%1.1f.png",etamin,etamax));
}

//===Fit Momentum Resolution
float FitMomentumResolution(Double_t *x, Double_t *par)
{
  float func = sqrt(par[0]*par[0]*x[0]*x[0]+par[1]*par[1]);
  return func;
}

//From Yellow report from section 11.2.2

void draw_req_Mom(double etamin, double etamax, double xmin=0., double xmax=0.)
{

   TF1 *dd4hep_p;
   if (etamin >= -3.5 && etamax <= -2.5) dd4hep_p = new TF1("dd4hep_p", "TMath::Sqrt((0.1*x)^2+2.0^2)",xmin,xmax);
   else if (etamin >= -2.5 && etamax <= -1.0) dd4hep_p = new TF1("dd4hep_p", "TMath::Sqrt((0.05*x)^2+1.0^2)",xmin,xmax);
   else if (etamin >= -1.0 && etamax <= 1.0) dd4hep_p = new TF1("dd4hep_p", "TMath::Sqrt((0.05*x)^2+0.5^2)",xmin,xmax);
   else if (etamin >= 1.0 && etamax <= 2.5) dd4hep_p = new TF1("dd4hep_p", "TMath::Sqrt((0.05*x)^2+1.0^2)",xmin,xmax);
   else if (etamin >= 2.5 && etamax <= 3.5) dd4hep_p = new TF1("dd4hep_p", "TMath::Sqrt((0.1*x)^2+2.0^2)",xmin,xmax);
   else return;
   dd4hep_p->SetLineStyle(7);
   dd4hep_p->SetLineColor(kMagenta);
   dd4hep_p->SetLineWidth(3.0);
   dd4hep_p->Draw("same");

  TLegend *l= new TLegend(0.70,0.75,0.90,0.80);
  l->SetTextSize(0.03);
  l->SetBorderSize(0);
  l->AddEntry(dd4hep_p,"PWGReq","l");
  l->Draw("same");
 }
