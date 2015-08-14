#include "TStyle.h"
#include "RooGlobalFunc.h"
#include "RooRealVar.h"
#include "RooDataSet.h"
#include "RooPlot.h"
#include "RooAbsPdf.h"
#include "RooFitResult.h"
#include "RooMCStudy.h"
#include "RooGaussian.h"
#include "RooProdPdf.h"
#include "RooPolynomial.h"
#include "RooExponential.h"
#include "RooAddPdf.h"
#include "TH1.h"
#include "TH2.h"
#include "TFile.h"
#include "TTree.h"
#include "TPad.h"
#include "TCanvas.h"

using namespace RooFit;

void mass_fits(){
	TFile* inputFile = TFile::Open("dataset.root");
	TTree* inputTree = (TTree*)inputFile->Get("modelData");

	RooRealVar obs_mass("mass","mass",5300,5400,"MeV/c^{2}");
	RooRealVar obs_time("time","time",0.0,12.0,"ps");

	RooDataSet *data = new RooDataSet("data","data",inputTree,RooArgList(obs_mass,obs_time));

	//THE SIGNAL MASS PDF
	//To start you out, knowing that the signal PDF is a gaussian 
	//you're going to need two variables to specify it: 
	//A mean and a width. Uncomment these and complete the second line: 

	RooRealVar signal_mass_mean("signal_mass_mean","signal_mass_mean",5300,5400,"MeV/c^{2}");
	RooRealVar signal_mass_sigma("signal_mass_sigma","signal_mass_sigma",0.0,20.0,"MeV/c^{2}");

	//Now you'll need to specify the RooGaussian PDF. 
	//Have a look at http://root.cern.ch/root/html/RooGaussian.html 
	//to see what it expects
	//Complete this line, using the obs_mass observable and the 
	//mean, width you just finished off: 

	RooAbsPdf* signal_mass = new RooGaussian("signal_mass","signal_mass",obs_mass,signal_mass_mean,signal_mass_sigma);

	//THE BACKGROUND MASS PDF
	//It's now up to you to create the O(1) poly to fit the background with.
	//First you'll need to specify the coefficients for both O(0) and O(1)
	//Remember that the O(0) term should be _fixed_ and not floated in the fit.

	RooRealVar bkg_mass_p0("bkg_mass_p0","bkg_mass_p0",5500);
	RooRealVar bkg_mass_p1("bkg_mass_p1","bkg_mass_p1",-2.0,0.0);

	//Now finish this line off with a RooPolynomial (http://root.cern.ch/root/html/RooPolynomial.html)

	RooAbsPdf* bkg_mass = new RooPolynomial("bkg_mass","bkg_mass",obs_mass,RooArgList(bkg_mass_p0,bkg_mass_p1));


	//THE MASS FIT
	// Now we have a PDF describing the signal and another describing 
	// the background in the mass observable. We can combine these as 
	// a RooAddPdf with an additional 2 parameters describing the yields.
	// When we fit, assuming the PDFs are a good description of the data 
	// we will get back the signal mean, the width and the yield. 


	//We need two RooRealVars to specify the signal and background yields. 
	// As we don't know the yields a priori, we'll set the ranges for each 
	// to be from 0 to the number of events in the ntuple. Uncomment these:

	RooRealVar Nsig("Nsig","Nsig",0.0,data->numEntries());
	RooRealVar Nbkg("Nbkg","Nbkg",0.0,data->numEntries());

	//Now we create a RooAddPdf, telling it to use these yield terms 
	// and the signal and background pdfs in the same order: 
	RooAbsPdf *mass_pdf = new RooAddPdf("mass_pdf","mass_pdf",RooArgList(*signal_mass,*bkg_mass),RooArgList(Nsig,Nbkg));

	//Finally, the extended likelihood fit: We fit to the mass_pdf and save the fitresult: 
	RooFitResult *mass_result = mass_pdf->fitTo(*data,Extended(),Save());

	//We print the mass fit result to the screen: 
	mass_result->Print();

	//We update the mass plot from earlier to show the fit to the data.
	//the next few lines will plot the PDF and its components:
	TCanvas *c = new TCanvas("data","data",1024,512);
	c->Divide(2);
	c->cd(1);
	gPad->SetLeftMargin(0.15); gPad->SetBottomMargin(0.15);
	RooPlot *massplot = obs_mass.frame();
	data->plotOn(massplot);
	//The combined fit in solid blue:	
	mass_pdf->plotOn(massplot);
	//The signal component only in dashed blue:
	mass_pdf->plotOn(massplot,Components(*signal_mass),LineColor(kBlue),LineStyle(kDashed));
	//The background component only in dashed red: 
	mass_pdf->plotOn(massplot,Components(*bkg_mass),LineColor(kRed),LineStyle(kDashed));
	massplot->Draw();
	c->Update();

	c->cd(2);
	gPad->SetLeftMargin(0.15); gPad->SetBottomMargin(0.15);
	//Logarithmic axis as this looks better:
	gPad->SetLogy();
	RooPlot *timeplot = obs_time.frame();
	data->plotOn(timeplot);
	timeplot->GetYaxis()->SetTitleOffset(1.6);
	timeplot->Draw();
	c->Draw();
	c->Print("masspdfplot.png");
}
