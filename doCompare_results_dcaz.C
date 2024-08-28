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

void draw_req_DCA(double etamin, double etamax, double xmin=0., double xmax=0.);
TGraphErrors* draw_directory(TString directory, TString particle = "pi-",double etamin=-1.0, double etamax=1.0, Bool_t drawreq=1, TString epic ="24.06.0", TString eicrecon = "v1.14.0") // name = p, pt for getting p or pt dependence fitted results
{
   gSystem->cd(directory);
   const Int_t nptbins = 10;
   double pt[nptbins] ={0.2, 0.3, 0.5,1.0, 1.5, 2.0, 5.0, 8.0, 10., 15.0};
   Double_t variation = 0.1; // 10 % variation 
   std::vector<double> momV_real;
   std::vector<double> dcazresolV_real, err_dcazresolV_real;
   momV_real.clear(); dcazresolV_real.clear(); err_dcazresolV_real.clear();
   
   //Reading the root file
    TFile *fDCA_real;
    TGraphErrors *gr_dcaT_real, *gr_dcaZ_real;
	    
      fDCA_real = TFile::Open(Form("realseed/%s/dca/final_hist_dca_realseed.root",particle.Data()));
	 
	 // Truth seeding histograms
      TH3D *hist_d0z_real = (TH3D*) fDCA_real->Get("h_d0z_3d");
      
      // d0xy calculation for truth/real (binning are same in both cases)
     Int_t etamin_bin = hist_d0z_real->GetYaxis()->FindBin(etamin+0.0001);
     Int_t etamax_bin = hist_d0z_real->GetYaxis()->FindBin(etamax-0.0001);
     TF1 *func_real = new TF1("func_real","gaus",-0.5,0.5);
	
    for(int iptbin=0; iptbin<nptbins; ++iptbin){
    	
   TCanvas *cp = new TCanvas("cp","cp",1400,1000);
   cp->SetMargin(0.10, 0.05 ,0.1,0.07);
     
    double ptmin = (1.0-variation)*pt[iptbin]; // 10% range  
    double ptmax = (1.0+variation)*pt[iptbin]; // 10% range 
  
    Int_t ptmin_bin = hist_d0z_real->GetZaxis()->FindBin(ptmin+0.0001);
    Int_t ptmax_bin = hist_d0z_real->GetZaxis()->FindBin(ptmax-0.0001);
 
    TH1D *histd0z_real_1d = (TH1D*)hist_d0z_real->ProjectionX(Form("histd0z_real_eta%1.1f_%1.1f_pt%1.1f_%1.1f",etamin,etamax,ptmin,ptmax),etamin_bin,etamax_bin,ptmin_bin,ptmax_bin,"o");
    histd0z_real_1d->SetTitle(Form("d0_{z} (real): %1.1f <#eta< %1.1f && %1.2f <p_{T}< %1.2f",etamin,etamax,ptmin,ptmax));   
    histd0z_real_1d->SetName(Form("eta_%1.1f_%1.1f_d0z_real_pt_%1.1f",etamin,etamax,pt[iptbin])); 
   
   //if (histd0xy_real_1d->GetEntries()<100) continue;    
   double mu_real = histd0z_real_1d->GetMean(); 
   double sigma_real = histd0z_real_1d->GetStdDev();
   func_real->SetRange(mu_real-2.0*sigma_real,mu_real+2.0*sigma_real); // fit with in 2 sigma range
   histd0z_real_1d->Fit(func_real,"NR+");
   mu_real = func_real->GetParameter(1); 
   sigma_real = func_real->GetParameter(2);
   func_real->SetRange(mu_real-2.0*sigma_real,mu_real+2.0*sigma_real); // fit with in 2 sigma range
   histd0z_real_1d->Fit(func_real,"R+");
   float real_par2 = func_real->GetParameter(2)*10000; // cm to mum 10000 factor
   float real_par2_err = func_real->GetParError(2)*10000;
   momV_real.push_back(pt[iptbin]);
   dcazresolV_real.push_back(real_par2);
   err_dcazresolV_real.push_back(real_par2_err);
   }   // ptbin
	
     const int size_real = momV_real.size();
	double pt_real[size_real], err_pt_real[size_real], sigma_dcaz_real[size_real], err_sigma_dcaz_real[size_real]; 
	
	for (int i=0; i<size_real; i++){
	pt_real[i] = momV_real.at(i);
     sigma_dcaz_real[i] = dcazresolV_real.at(i);
	err_sigma_dcaz_real[i] = err_dcazresolV_real.at(i);
	err_pt_real[i] = 0.;
	}
	
     TGraphErrors *gr2 = new TGraphErrors(size_real,pt_real,sigma_dcaz_real,err_pt_real,err_sigma_dcaz_real);
     gr2->SetName("gr_realseed");
	gr2->SetMarkerStyle(34);
	gr2->SetMarkerColor(kRed);
	gr2->SetMarkerSize(2.0);
	gr2->SetTitle(";p_{z} (GeV/c); #sigma_{DCA_{z}} (#mum)");
	gr2->GetXaxis()->CenterTitle();
	gr2->GetYaxis()->CenterTitle();
	
     //leave the directory
	gSystem->cd("..");
	return gr2;
}

void doCompare_results_dcaz(std::vector<TString> directory_list) 
{
     //change the following as required
	TString particle = "pi-";
	double etamin=-1.0; //etamin
	double etamax=1.0; //etamax
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

     TF1 *f1=new TF1("f1","FitPointingAngle",0.,30.0,2);
     f1->SetParLimits(0,0.,50000);	
     f1->SetParLimits(1,0.,50000);	

     TCanvas *c_dcaz = new TCanvas("c_dcaz","c_dcaz",1400,1000);
     c_dcaz->SetMargin(0.10, 0.05 ,0.1,0.05);
     c_dcaz->SetGridy();

     TMultiGraph *mgDCAz; 
     TLegend *lDCAz; 
     mgDCAz = new TMultiGraph("mgDCAz",";p_{z} (GeV/c); #sigma_{DCA_{z}} (#mum)");
     lDCAz = new TLegend(0.65,0.80,0.90,0.93);
     lDCAz->SetTextSize(0.03);
     lDCAz->SetBorderSize(0);
     lDCAz->SetHeader(Form("%s ePIC: %1.1f < #eta < %1.1f",symbolname.Data(),etamin,etamax),"C");
     int i = 0;
     for(auto& directory : directory_list)
	{
          TGraphErrors* new_graph = draw_directory(directory, 
												 particle, 
												 etamin, 
												 etamax);
          new_graph->SetMarkerColor(colors[i]);
          mgDCAz->Add(new_graph);
		lDCAz->AddEntry(new_graph, directory);
          i = (i+1) % colors.size();
     }

     c_dcaz->cd();
     mgDCAz->GetXaxis()->SetRangeUser(0.18,mgDCAz->GetXaxis()->GetXmax()); 
	mgDCAz->GetYaxis()->SetRangeUser(0.0,100); 
     mgDCAz->Draw("AP");
     lDCAz->Draw("same");
     draw_req_DCA(etamin,etamax,0.,mgDCAz->GetXaxis()->GetXmax());
     c_dcaz->SaveAs(Form("dcaxy_resol_%1.1f_eta_%1.1f.png",etamin,etamax));
  
}

//From Yellow report from section 11.2.2
//===Fit Pointing Resolution
float FitPointingAngle(Double_t *x, Double_t *par)
{
  float func = sqrt((par[0]*par[0])/(x[0]*x[0])+par[1]*par[1]);
  return func;
}

void draw_req_DCA(double etamin, double etamax, double xmin=0., double xmax=0.)
{
  TF1 *PWGReq_DCA2D;
  if (etamin >= -3.5 && etamax <= -2.5) PWGReq_DCA2D = new TF1("PWGReq_DCA2D", "TMath::Sqrt((30./x)^2+40.0^2)",xmin,xmax);
  else if (etamin >= -2.5 && etamax <= -1.0) PWGReq_DCA2D = new TF1("PWGReq_DCA2D", "TMath::Sqrt((30./x)^2+20.0^2)",xmin,xmax);
  else if (etamin >= -1.0 && etamax <= 1.0) PWGReq_DCA2D = new TF1("PWGReq_DCA2D", "TMath::Sqrt((20./x)^2+5.0^2)",xmin,xmax);
  else if (etamin >= 1.0 && etamax <= 2.5) PWGReq_DCA2D = new TF1("PWGReq_DCA2D", "TMath::Sqrt((30./x)^2+20.0^2)",xmin,xmax);
  else if (etamin >= 2.5 && etamax <= 3.5) PWGReq_DCA2D = new TF1("PWGReq_DCA2D", "TMath::Sqrt((30./x)^2+40.0^2)",xmin,xmax);
  else return;
  PWGReq_DCA2D->SetLineStyle(7);
  PWGReq_DCA2D->SetLineWidth(3.0);
  PWGReq_DCA2D->SetLineColor(kBlue);
  PWGReq_DCA2D->Draw("same");
		
  TLegend *l= new TLegend(0.70,0.75,0.90,0.80);
  l->SetTextSize(0.03);
  l->SetBorderSize(0);
  l->AddEntry(PWGReq_DCA2D,"PWGReq","l");
  l->Draw("same");
}
