#include <iostream>
#include <vector>
#include <filesystem>
#include <regex>
#include <string>
#include "TFile.h"
#include "TTree.h"
#include "TLeaf.h"
#include "TH1F.h"
#include "TGraph.h"
#include "TCanvas.h"

std::string treeString = "events";
std::string realleafString = "CentralCKFSeededTrackParameters.type";
std::string truthleafString = "CentralCKFTrackParameters.type";

const char* treeName = treeString.c_str();
const char* realleafName = realleafString.c_str();
const char* truthleafName = truthleafString.c_str();

void ReconEfficiency(TString DataDir)
{
    //move into the data dir
	gSystem->cd(DataDir);

    // Get all files matching the pattern
    std::vector<std::string> fileNames = getFiles("tracking_output");

    // Create a histogram to store the number of entries from each file
    TH1F *hist = new TH1F("leafEntries", "Number of Entries in Leaf;File Index;Entries", fileNames.size(), 0, fileNames.size());

    // Extract leaf entries from each file and fill the histogram
    for (size_t i = 0; i < fileNames.size(); ++i) {
        int entryCount = getLeafEntryCount(fileNames[i].c_str(), treeName, leafName);
        if (entryCount >= 0) {
            hist->SetBinContent(i+1, entryCount);
        }
    }

    // Draw the graph
    TCanvas *c1 = new TCanvas("c1", "Efficiency", 800, 600);

    //graph to combine real and truth seeding
    TMultiGraph *mgEff;
    TLegend *lEff;
    mgEff = new TMultiGraph("mgEff",";p (GeV/c); Efficiency %");
	lmom = new TLegend(0.65,0.80,0.90,0.93);

    TGraphErrors *gr2 = new TGraphErrors(size_real,p_real,sigma_p_real,err_p_real,err_sigma_p_real);
    gr2->SetName("gr_realseed");
	gr2->SetMarkerStyle(34);
	gr2->SetMarkerSize(2.5);
	gr2->SetTitle(";p (GeV/c);#sigmap/p");
	gr2->GetXaxis()->CenterTitle();
	gr2->GetYaxis()->CenterTitle();

    //save the graph drawn on the canvas
    c1->SaveAs("ReconstructionEfficiency.png");

    //leave the directory
	gSystem->cd("..");
}


// Function to extract the number after "tracking_output_" in the filename
int extractNumber(const std::string& filename) {
    std::regex pattern("tracking_output_(\\d+)");
    std::smatch matches;
    if (std::regex_search(filename, matches, pattern) && matches.size() > 1) {
        return std::stoi(matches[1].str());
    }
    return -1; // Return -1 if the number is not found
}


int countLeafEntries(const char* filename, const char* treeName, const char* leafName) {
    // Open the ROOT file
    TFile *file = TFile::Open(filename);
    if (!file || file->IsZombie()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }
    
    // Access the TTree
    TTree *tree = dynamic_cast<TTree*>(file->Get(treeName));
    if (!tree) {
        std::cerr << "Error accessing tree: " << treeName << std::endl;
        file->Close();
        return;
    }
    
    // Use TTree::Draw to count the number of non-zero entries in the specified leaf
    TString drawCommand = TString::Format("%s>>htemp", leafName);
    tree->Draw(drawCommand, "", "goff");
    
    // Access the histogram created by TTree::Draw
    TH1 *htemp = (TH1*)gDirectory->Get("htemp");
    if (!htemp) {
        std::cerr << "Error accessing histogram for leaf: " << leafName << std::endl;
        file->Close();
        return;
    }
    
    // Get the number of non-zero entries
    int leafEntryCount = htemp->GetEntries();
    
    std::cout << "Number of entries in leaf " << leafName << ": " << leafEntryCount << std::endl;

    // Clean up
    delete htemp;
    file->Close();

    return leafEntryCount;
}

// Function to get all files matching the pattern
std::vector<std::string> getFiles(const std::string& pattern) {
    std::vector<std::string> fileNames;
    for (const auto& entry : std::filesystem::directory_iterator(".")) {
        if (entry.path().string().find(pattern) != std::string::npos) {
            fileNames.push_back(entry.path().string());
        }
    }
    return fileNames;
}