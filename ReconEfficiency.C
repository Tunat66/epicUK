#include <iostream>
#include <vector>
#include <filesystem>
#include "TFile.h"
#include "TTree.h"
#include "TLeaf.h"
#include "TH1F.h"
#include "TCanvas.h"

void ReconEfficiency(std::filesystem::path DataDir)
{

}

void plotLeafEntries(const char* treeName, const char* leafName) {
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

    // Draw the histogram
    TCanvas *c1 = new TCanvas("c1", "Leaf Entries", 800, 600);
    hist->Draw();
    c1->SaveAs("leaf_entries.png");
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