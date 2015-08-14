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

void toystudies(){
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

  RooAbsPdf* signal = new RooProdPdf("signal","signal",*signal_mass,*signal_time);
  RooAbsPdf* bkg = new RooProdPdf("bkg","bkg",*bkg_mass,*bkg_time);
  RooAbsPdf* model = new RooAddPdf("model","model",RooArgList(*signal,*bkg),RooArgList(Nsig,Nbkg));

  RooFitResult *model_result = model->fitTo(*data,Extended(),Save());
  
  RooMCStudy *toyStudy = new RooMCStudy(*model,RooArgList(obs_mass,obs_time),Extended(),FitOptions(Save(kTRUE)),Silence());
  UInt_t Ntoys = 50;
  toyStudy->generateAndFit(Ntoys);
  // That's it! Only 2 lines to automatically generate and fit Ntoys data 
  // samples with the parameters of the model fit result. 
  // Do you understand what the above lines are doing? 
  
  //Now that we've run our toys, we want to see what our expected 
  //sensitivity to the lifetime is. We'll plot the signal lifetime, 
  //its error and pull for the toys:  
  
  TCanvas *d = new TCanvas("toys","toys",1024,340);
  d->Divide(3);
  d->cd(1);
  RooPlot *value = toyStudy->plotParam(signal_lifetime,Bins(10));
  value->Draw();
  d->cd(2);
  RooPlot *error = toyStudy->plotError(signal_lifetime,Bins(10));
  error->Draw();
  d->cd(3);
  RooPlot *pull = toyStudy->plotPull(signal_lifetime,Bins(10),FitGauss(kTRUE));
  pull->Draw();
  d->Update();
  d->Print("toystudy1.png");
        
  //We can also extract the fit result for each individual fit, 
  //and look at correlations between fitted values:
  
  TCanvas *e = new TCanvas("toys2","toys2",1280,384);
  e->Divide(3);
  e->cd(1);
  gPad->SetLeftMargin(0.15) ; gPad->SetBottomMargin(0.15) ;
  
  // A 2D scatter showing how the signal lifetime and background lifetime fluctuate
  TH1* sig_vs_bkg_lifetime = toyStudy->fitParDataSet().createHistogram("sig_vs_bkg_lifetime",signal_lifetime,YVar(bkg_lifetime)) ;
  sig_vs_bkg_lifetime->SetTitle("Signal vs Background Lifetime");
  sig_vs_bkg_lifetime->Draw("box");
  
  e->cd(2);
  gPad->SetLeftMargin(0.15) ; gPad->SetBottomMargin(0.15) ;
  
  // Similarly for the yields:
  TH1* sig_vs_bkg_yield = toyStudy->fitParDataSet().createHistogram("sig_vs_bkg_yield",Nsig,YVar(Nbkg));
  sig_vs_bkg_yield->SetTitle("Signal vs Background Yield");
  sig_vs_bkg_yield->Draw("box");
  
  e->cd(3);
  gPad->SetLeftMargin(0.20); gPad->SetBottomMargin(0.15);
  
  // We extract the correlation matrix of each toy and average them to get the average correlation matrix across 50 toys:
  TH2* corrmatrix = toyStudy->fitResult(0)->correlationHist("Parameter Correlation matrix");
  for(UInt_t i=1; i<Ntoys; i++){
    TString name = "c";
    name += i;
    corrmatrix->Add(toyStudy->fitResult(i)->correlationHist(name));
  }
  corrmatrix->Scale(1.0/((double)Ntoys));
  corrmatrix->SetMinimum(-1.0);
  
  gStyle->SetOptStat(0);
  corrmatrix->Draw("colz");
  e->Draw();
  e->Print("toystudy2.png");
}
