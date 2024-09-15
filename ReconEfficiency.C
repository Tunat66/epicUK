#pragma once
#include <iostream>
#include <vector>
#include <string> 
#include "TFile.h"
#include "TTree.h"
#include "TLeaf.h"
#include "TH1F.h"
#include "TGraph.h"
#include "TCanvas.h"
#include <windows.h>

std::string treeString = "events";
std::string realleafString = "CentralCKFSeededTrackParameters.type";
std::string truthleafString = "CentralCKFTrackParameters.type";

const char* treeName = treeString.c_str();
const char* realleafName = realleafString.c_str();
const char* truthleafName = truthleafString.c_str();

// Function to extract the number after "tracking_output_" in the filename
double extractNumber(const std::string& filename) {
    size_t pos = filename.find("tracking_output_");
    if (pos == std::string::npos) return -1;

    size_t start = pos + strlen("tracking_output_");
    size_t end = filename.find_first_not_of("0123456789.", start);

    std::string number = filename.substr(start, end - start);
    return std::stod(number);
}

std::vector<double> extractMomenta(const std::vector<std::string>& fileNames) {
    std::vector<double> mom(fileNames.size());
    
    for (size_t i = 0; i < fileNames.size(); ++i) {
        mom[i] = extractNumber(fileNames[i]);
    }
    
    return mom;
}

int getLeafEntryCount(const char* filename, const char* treeName, const char* leafName) {
    // Open the ROOT file
    TFile *file = TFile::Open(filename);
    if (!file || file->IsZombie()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return -1;
    }
    
    // Access the TTree
    TTree *tree = dynamic_cast<TTree*>(file->Get(treeName));
    if (!tree) {
        std::cerr << "Error accessing tree: " << treeName << std::endl;
        file->Close();
        return -1;
    }
    
    // Use TTree::Draw to count the number of non-zero entries in the specified leaf
    TString drawCommand = TString::Format("%s>>htemp", leafName);
    tree->Draw(drawCommand, "", "goff");
    
    // Access the histogram created by TTree::Draw
    TH1 *htemp = (TH1*)gDirectory->Get("htemp");
    if (!htemp) {
        std::cerr << "Error accessing histogram for leaf: " << leafName << std::endl;
        file->Close();
        return -1;
    }
    
    // Get the number of non-zero entries
    int leafEntryCount = htemp->GetEntries();
    
    std::cout << "Number of entries in leaf " << leafName << ": " << leafEntryCount << std::endl;

    // Clean up
    delete htemp;
    file->Close();

    return leafEntryCount;
}

std::vector<std::string> getFiles(const std::string& pattern) {
    std::vector<std::string> fileNames;
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile((pattern + "*").c_str(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        std::cerr << "Invalid directory handle or no files found." << std::endl;
        return fileNames;
    }

    do {
        if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            fileNames.push_back(findFileData.cFileName);
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
    return fileNames;
}

void ReconEfficiency(TString DataDir)
{
    int eventsnum = 10000; //change as required
    
    //move into the data dir
	gSystem->cd(DataDir);

    // Get all files matching the pattern
    std::vector<std::string> fileNames = getFiles("tracking_output");
    // Get the momenta from filename (this is a bit imperfect, it might be better later to read the momentum from the root file)
    std::vector<double> mom = extractMomenta(fileNames);
    
    //=== style of the plot=========
    gStyle->SetPalette(1);
    gStyle->SetOptTitle(1);
    gStyle->SetTitleOffset(1.0,"XY");
    gStyle->SetTitleSize(.04,"XY");
    gStyle->SetLabelSize(.04,"XY");
    gStyle->SetHistLineWidth(2);
    gStyle->SetOptFit(1);
    gStyle->SetOptStat(1);

    // Create the canvas to draw the graph
    TCanvas *c1 = new TCanvas("c1", "Efficiency", 800, 600);

    // Create graph
    TMultiGraph *mgEff;
    TLegend *lEff;
    mgEff = new TMultiGraph("mgEff",";p (GeV/c); Efficiency %");
	lEff = new TLegend(0.65,0.80,0.90,0.93);
    lEff->SetTextSize(0.03);
    lEff->SetBorderSize(0);
    lEff->SetHeader(Form("Reconstruction Efficiency"),"C");
  
    // Extract leaf entries from each file and fill the histogram
    std::vector<double> realentryCountVector;
    std::vector<double> truthentryCountVector;
    for (size_t i = 0; i < fileNames.size(); ++i) {
        int realentryCount = getLeafEntryCount(fileNames[i].c_str(), treeName, realleafName);
        int truthentryCount = getLeafEntryCount(fileNames[i].c_str(), treeName, truthleafName);
        //if (realentrycount < 0 && truthentryCount < 0) continue; //no need for this exception handling thingy
        realentryCountVector.push_back((double) realentryCount/eventsnum);
        truthentryCountVector.push_back((double) truthentryCount/eventsnum);
    }

    

    double truthentryCountArray[truthentryCountVector.size()];
    std::copy(truthentryCountVector.begin(), truthentryCountVector.end(), truthentryCountArray);

    double realentryCountArray[realentryCountVector.size()];
    std::copy(realentryCountVector.begin(), realentryCountVector.end(), realentryCountArray);

    double momArray[mom.size()];
    std::copy(mom.begin(), mom.end(), momArray);

    std::cout << realentryCountArray[1] << std::endl;

    //POSSIBLE ISSUE: the entries may be null, which causes the graph to skip a momentum and therefore the data
    //points to be shifted. Develop a more robust system to put entries into a vector
    TGraph *gr1 = new TGraph(truthentryCountVector.size(), momArray, truthentryCountArray);
    gr1->SetName("gr_truthseed");
	gr1->SetMarkerStyle(25);
	gr1->SetMarkerColor(kBlue);
	gr1->SetMarkerSize(2.0);
	gr1->SetTitle(";p (GeV/c);#sigmap/p");
	gr1->GetXaxis()->CenterTitle();
	gr1->GetYaxis()->CenterTitle();
	
    TGraph *gr2 = new TGraph(realentryCountVector.size(), momArray, realentryCountArray);
    gr2->SetName("gr_realseed");
	gr2->SetMarkerStyle(34);
	gr2->SetMarkerColor(kRed);
	gr2->SetMarkerSize(2.0);
	gr2->SetTitle(";p (GeV/c);#sigmap/p");
	gr2->GetXaxis()->CenterTitle();
	gr2->GetYaxis()->CenterTitle();
	
	mgEff->Add(gr1);
	mgEff->Add(gr2);
	mgEff->GetXaxis()->SetRangeUser(0.40,15.2);
	// Reduce this range to better see the increase
	//mgEff->GetYaxis()->SetRangeUser(0.,10.0);
	mgEff->GetYaxis()->SetRangeUser(0.,1.0);
	mgEff->Draw("AP");
	lEff->AddEntry(gr1,"Truth Seeding");
	lEff->AddEntry(gr2,"Realistic Seeding");
	lEff->Draw("same");
	mgEff->SetName(Form("recon_eff_-1.0_eta_1.0"));
	mgEff->Write();

    //save the graph drawn on the canvas
    c1->Update();
    c1->SaveAs("ReconstructionEfficiency.png");

    //leave the directory
	gSystem->cd("..");
}


