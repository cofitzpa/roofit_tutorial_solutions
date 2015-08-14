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

void combined_fits(){
  TFile* inputFile = TFile::Open("dataset.root");
  TTree* inputTree = (TTree*)inputFile->Get("modelData");
  
  RooRealVar obs_mass("mass","mass",5300,5400,"MeV/c^{2}");
  RooRealVar obs_time("time","time",0.0,12.0,"ps");
  
  RooDataSet *data = new RooDataSet("data","data",inputTree,RooArgList(obs_mass,obs_time));

  RooRealVar Nsig("Nsig","Nsig",0.0,data->numEntries());
  RooRealVar Nbkg("Nbkg","Nbkg",0.0,data->numEntries());

  // THE SIGNAL TIME PDF
  // First, we specify the lifetime parameter as a RooRealVar 
  // with a range as usual:
  RooRealVar signal_lifetime("signal_lifetime","signal_lifetime",0.0,5.0,"ps");
  
  RooFormulaVar signal_exponent("signal_exponent","signal_exponent","-1.0/@0",RooArgList(signal_lifetime));
  RooAbsPdf* signal_time = new RooExponential("signal_time","signal_time",obs_time,signal_exponent);  
  
  RooRealVar bkg_lifetime("bkg_lifetime","bkg_lifetime",0.0,3.0,"ps");
  RooFormulaVar bkg_exponent("bkg_exponent","bkg_exponent","-1.0/@0",RooArgList(bkg_lifetime));
  RooAbsPdf* bkg_time = new RooExponential("bkg_time","bkg_time",obs_time,bkg_exponent);

  // Now the mass PDFs again
  RooRealVar signal_mass_mean("signal_mass_mean","signal_mass_mean",5300,5400,"MeV/c^{2}");
  RooRealVar signal_mass_sigma("signal_mass_sigma","signal_mass_sigma",0.0,20.0,"MeV/c^{2}");
  RooAbsPdf* signal_mass = new RooGaussian("signal_mass","signal_mass",obs_mass,signal_mass_mean,signal_mass_sigma);

  RooRealVar bkg_mass_p0("bkg_mass_p0","bkg_mass_p0",5500);
  RooRealVar bkg_mass_p1("bkg_mass_p1","bkg_mass_p1",-2.0,0.0);
  RooAbsPdf* bkg_mass = new RooPolynomial("bkg_mass","bkg_mass",obs_mass,RooArgList(bkg_mass_p0,bkg_mass_p1));

  RooAbsPdf *mass_pdf = new RooAddPdf("mass_pdf","mass_pdf",RooArgList(*signal_mass,*bkg_mass),RooArgList(Nsig,Nbkg));
  RooAbsPdf *time_pdf = new RooAddPdf("time_pdf","time_pdf",RooArgList(*signal_time,*bkg_time),RooArgList(Nsig,Nbkg));

  //Now we create a RooAddPdf, telling it to use these yield terms 
  // and the signal and background pdfs in the same order: 

  //We make a product PDF of the two signal PDFs first. This 2D PDF completely 
  //specifies the signal in time and mass:
  RooAbsPdf* signal = new RooProdPdf("signal","signal",*signal_mass,*signal_time);
  
  //Likewise for the background:
  RooAbsPdf* bkg = new RooProdPdf("bkg","bkg",*bkg_mass,*bkg_time);
  
  
  // And now we make an AddPdf of the signal + background as before, 
  // but this time in both dimensions using the 2D PDFs:
  RooAbsPdf* model = new RooAddPdf("model","model",RooArgList(*signal,*bkg),RooArgList(Nsig,Nbkg));
  
  RooFitResult *mass_result = mass_pdf->fitTo(*data,Extended(),Save());
  RooFitResult *time_result = time_pdf->fitTo(*data,Extended(),Save());

  //Now we fit to the full model:
  RooFitResult *model_result = model->fitTo(*data,Extended(),Save());
  
  //Because we saved the fit results, we can print these sequentially and compare: 
  cout << "------------------ FIT RESULT FOR MASS ONLY --------------" << endl;
  mass_result->Print();
  
  cout << "------------------ FIT RESULT FOR TIME ONLY --------------" << endl;
  time_result->Print();
  
  cout << "----------------- FIT RESULT FOR THE 2D MODEL ------------" << endl;
  model_result->Print();
}
