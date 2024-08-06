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

void Tracking_Hits_xy(TString filename="tracking_output.root",TString particle="pi-", double mom=0.1, Double_t pTcut = 0.15, TString name = "")
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
    //if(!SiBarrelHits_posx.IsEmpty()) cout << "debug" << endl;
    //cout << SiBarrelHits_posx.At(0) << endl;
    cout << "debug2" << endl;
    // Variables to hold the flattened data
    Double_t flatVariable1;
    // Create branches in the output tree for the flattened data
    outputTree->Branch("variable1", &flatVariable1, "variable1/F");
    // Loop over the entries in the input tree
    
    auto hit_plot = new TGraph(10000);
    hit_plot->SetMarkerColor(kBlue);
    //hit_plot->SetMarkerStyle(kFullCircle);
    //so why is this while loop needed:
    //essentially, SiBarrelHits_posx etc are unflattened 2d arrays, the while loop traverses each element of an 
    //array. The elements of this array are also arrays, which are then handled by the for loop
    while (myReader.Next()) {
        // everything, for some reason, needs to be in this while loop
        //TGraph *hit_plot = new TGraph(SiBarrelHits_posx.GetSize());
        Double_t *x_coord = new Double_t[SiBarrelHits_posx.GetSize()];
        Double_t *y_coord = new Double_t[SiBarrelHits_posy.GetSize()];
        
        for (size_t i = 0; i < SiBarrelHits_posx.GetSize(); ++i) {
            hit_plot->AddPoint(SiBarrelHits_posx[i], SiBarrelHits_posy[i]);
            
            

            //x_coord[i] = SiBarrelHits_posx[i];
            //y_coord[i] = SiBarrelHits_posy[i];
        }
        
        delete[] x_coord;
        delete[] y_coord;
    }
    
    hit_plot->SetTitle(Form("HitsCrossSection_xy_L3New_L4New"));
    hit_plot->SetName(Form("HitsCrossSection_xy"));
    hit_plot->Draw("ap"); //a for all, p for a scatter plot
    // Write the output tree to the output file
    outputTree->Write();

    // Close the files
    outputFile->Close();

}




