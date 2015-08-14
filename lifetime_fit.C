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

void lifetime_fit(){
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
  
  //Now we need to turn this into a derived variable of the 
  // type the PDF expects, in this case c = -1/tau. 
  // See http://root.cern.ch/root/html/RooFormulaVar.html and 
  // complete the line below: 
  RooFormulaVar signal_exponent("signal_exponent","signal_exponent","-1.0/@0",RooArgList(signal_lifetime));
  
  //Finally we give the PDF this term as the free parameter in the exponential PDF
  //Complete this line:
  RooAbsPdf* signal_time = new RooExponential("signal_time","signal_time",obs_time,signal_exponent);
  
  //THE BACKGROUND TIME PDF
  //The background in this case is "prompt"- It has a much smaller lifetime 
  //than the signal but can be considered as roughly exponential. 
  //We make an identical PDF to the signal one. Uncomment and complete:
  
  RooRealVar bkg_lifetime("bkg_lifetime","bkg_lifetime",0.0,3.0,"ps");
  RooFormulaVar bkg_exponent("bkg_exponent","bkg_exponent","-1.0/@0",RooArgList(bkg_lifetime));
  RooAbsPdf* bkg_time = new RooExponential("bkg_time","bkg_time",obs_time,bkg_exponent);
  
  //THE LIFETIME FIT
  // Now we can do the exact same thing as we did with the mass PDF, 
  // and fit for the lifetimes
  
  // We don't actually need new RooRealVars to specify the yields, 
  // we can recycle the old ones from the mass fit
  // Uncomment and complete the time PDF and the fit, being sure to save the fitresult: 
  RooAbsPdf *time_pdf = new RooAddPdf("time_pdf","time_pdf",RooArgList(*signal_time,*bkg_time),RooArgList(Nsig,Nbkg));
  RooFitResult *time_result = time_pdf->fitTo(*data,Extended(),Save());
  time_result->Print();
  
  TCanvas *c = new TCanvas("data","data",1024,512);
  c->Divide(2);
  c->cd(1);
        gPad->SetLeftMargin(0.15); gPad->SetBottomMargin(0.15);
        RooPlot *massplot = obs_mass.frame();
        data->plotOn(massplot);
        massplot->Draw();
        c->Update();

  //Now we update the time plot as we did for the mass:
  c->cd(2);
  gPad->SetLogy();
  gPad->SetLeftMargin(0.15); gPad->SetBottomMargin(0.15);
  RooPlot *timeplot = obs_time.frame();
  data->plotOn(timeplot);
  //The combined fit in solid blue:
  time_pdf->plotOn(timeplot);
  //The signal component only in dashed blue:
  time_pdf->plotOn(timeplot,Components(*signal_time),LineColor(kBlue),LineStyle(kDashed));
  //The background component only in dashed red: 
  time_pdf->plotOn(timeplot,Components(*bkg_time),LineColor(kRed),LineStyle(kDashed));
  timeplot->Draw();
  c->Update();
  c->Print("timepdfplot.png");
}
