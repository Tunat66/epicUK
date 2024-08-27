// Code to extract the Tracking Performances
// Shyam Kumar; INFN Bari, Italy
// shyam.kumar@ba.infn.it; shyam.kumar@cern.ch

#include "TGraphErrors.h"
#include "TF1.h"
#include "TRandom.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TMath.h"
#define mpi 0.139  // 1.864 GeV/c^2

void Tracking_Hits_phiz(TString filename="tracking_output.root",TString particle="pi-")
{ //slice z determintes where the slice of hits is going to be extracted

   // style of the plot
   gStyle->SetPalette(1);
   gStyle->SetOptTitle(1);
   gStyle->SetTitleOffset(.85,"X");gStyle->SetTitleOffset(.85,"Y");
   gStyle->SetTitleSize(.05,"X");gStyle->SetTitleSize(.05,"Y");
   gStyle->SetLabelSize(.04,"X");gStyle->SetLabelSize(.04,"Y");
   gStyle->SetHistLineWidth(2);
   gStyle->SetOptFit(1);
   gStyle->SetOptStat(1);
   
   
   bool debug=true;	 
   TFile* file = TFile::Open(filename.Data());
   
   if (!file) {printf("Tracking_Hits_xy: file not found !!!"); return;}
   TTreeReader myReader("events", file); // name of tree and file
   if (debug) cout<<"Filename: "<<file->GetName()<<"\t NEvents: "<<myReader.GetEntries()<<endl;

    // Create a new ROOT file to store the flattened tree
    TFile *outputFile = new TFile("flat.root", "RECREATE");

    // Create a new tree to store the flattened data
    TTree *outputTree = new TTree("flat_tree", "Tree with flattened arrays");

    // Hits information 
    TTreeReaderArray<Double_t> SiBarrelHits_posx(myReader, "SiBarrelHits.position.x");
    TTreeReaderArray<Double_t> SiBarrelHits_posy(myReader, "SiBarrelHits.position.y");
    TTreeReaderArray<Double_t> SiBarrelHits_posz(myReader, "SiBarrelHits.position.z");

    // Loop over the entries in the input tree
    
    auto hit_plot_1 = new TGraph();
    hit_plot_1->SetMarkerColor(kBlue);
    hit_plot_1->SetMarkerStyle(kFullCircle);
    hit_plot_1->SetMarkerSize(0.5);
    
    auto hit_plot_2 = new TGraph();
    hit_plot_2->SetMarkerColor(kRed);
    hit_plot_2->SetMarkerStyle(kFullCircle);
    hit_plot_2->SetMarkerSize(0.5);

    //so why is this while loop needed:
    //essentially, SiBarrelHits_posx etc are unflattened 2d arrays, the while loop traverses each element of an 
    //array. The elements of this array are also arrays, which are then handled by the for loop
    while (myReader.Next()) {
        // everything, for some reason, needs to be in this while loop
        Double_t *x_coord = new Double_t[SiBarrelHits_posx.GetSize()];
        Double_t *y_coord = new Double_t[SiBarrelHits_posy.GetSize()];
        
        for (size_t i = 0; i < SiBarrelHits_posx.GetSize(); ++i) {
            
            double x = static_cast<double>(SiBarrelHits_posx[i]);
            double y = static_cast<double>(SiBarrelHits_posy[i]);
            Double_t phi = atan2(y, x);
            cout << phi << endl;
            //some radius settings and coloring
            Double_t hit_radius = sqrt(x*x + y*y);
            if (hit_radius < 300) {
               hit_plot_2->AddPoint(SiBarrelHits_posz[i], hit_radius*phi);
            }
            else { 
               hit_plot_1->AddPoint(SiBarrelHits_posz[i], hit_radius*phi);
            }

            //x_coord[i] = SiBarrelHits_posx[i];
            //y_coord[i] = SiBarrelHits_posy[i];
        }
        
        delete[] x_coord;
        delete[] y_coord;
    }
    TMultiGraph *mg = new TMultiGraph();
    mg->Add(hit_plot_1);
    mg->Add(hit_plot_2);
    mg->GetXaxis()->SetTitle("z (mm)");
    mg->GetYaxis()->SetTitle("Unrolled position hit_phi*hit_r (mm)");
    
    mg->SetTitle("HitsUnrolledDetectors_L3New_L4New");
    mg->SetName("HitsUnrolledDetectors_phiz");
    mg->Draw("ap"); //a for all, p for a scatter plot
    // Write the output tree to the output file
    outputTree->Write();

    // Close the files
    outputFile->Close();

}




