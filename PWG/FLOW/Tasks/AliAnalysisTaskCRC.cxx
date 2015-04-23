/*************************************************************************
 * Copyright(c) 1998-2008, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

/********************************
 * analysis task for CRC        *
 *                              *
 * author: Jacopo Margutti      *
 *         (margutti@nikhef.nl) *
 * ******************************/

class TFile;
class TString;
class TList;
class AliAnalysisTaskSE;

#include "Riostream.h"
#include "AliAODEvent.h"
#include "AliAODHeader.h"
#include "AliCentrality.h"
#include "AliFlowEventSimple.h"
#include "AliAnalysisTaskCRC.h"
#include "AliFlowAnalysisCRC.h"

#include "AliLog.h"

using std::cout;
using std::endl;
ClassImp(AliAnalysisTaskCRC)

//================================================================================================================

AliAnalysisTaskCRC::AliAnalysisTaskCRC(const char *name, Bool_t useParticleWeights):
AliAnalysisTaskSE(name),
fEvent(NULL),
fQC(NULL),
fListHistos(NULL),
fBookOnlyBasicCCH(kTRUE),
fFillMultipleControlHistograms(kFALSE),
fHarmonic(0),
fApplyCorrectionForNUA(kFALSE),
fApplyCorrectionForNUAVsM(kFALSE),
fPropagateErrorAlsoFromNIT(kFALSE),
fCalculateDiffFlow(kTRUE),
fCalculate2DDiffFlow(kFALSE),
fCalculateDiffFlowVsEta(kTRUE),
fStoreDistributions(kFALSE),
fCalculateCumulantsVsM(kFALSE),
fCalculateAllCorrelationsVsM(kFALSE),
fCalculateMixedHarmonics(kFALSE),
fCalculateMixedHarmonicsVsM(kFALSE),
fStoreControlHistograms(kFALSE),
fMinimumBiasReferenceFlow(kTRUE),
fForgetAboutCovariances(kFALSE),
fStoreVarious(kFALSE),
fExactNoRPs(0),
fUse2DHistograms(kFALSE),
fFillProfilesVsMUsingWeights(kTRUE),
fUseQvectorTerms(kFALSE),
fnBinsMult(10000),
fMinMult(0.),
fMaxMult(10000.),
fUseParticleWeights(useParticleWeights),
fUsePhiWeights(kFALSE),
fUsePtWeights(kFALSE),
fUseEtaWeights(kFALSE),
fUseTrackWeights(kFALSE),
fUsePhiEtaWeights(kFALSE),
fWeightsList(NULL),
fMultiplicityWeight(NULL),
fMultiplicityIs(AliFlowCommonConstants::kRP),
fnBinsForCorrelations(10000),
fUseBootstrap(kFALSE),
fUseBootstrapVsM(kFALSE),
fnSubsamples(10),
fCalculateCRC(kTRUE),
fCalculateCRCPt(kFALSE),
fCalculateCRCBck(kFALSE),
fUseNUAforCRC(kFALSE),
fUseCRCRecenter(kFALSE),
fCRCEtaMin(0.),
fCRCEtaMax(0.),
fQVecList(NULL),
fCRCRunBinMin(0),
fCRCRunBinMax(1)
{
 // constructor
 AliDebug(2,"AliAnalysisTaskCRC::AliAnalysisTaskCRC(const char *name, Bool_t useParticleWeights)");
 
 // Define input and output slots here
 // Input slot #0 works with an AliFlowEventSimple
 DefineInput(0, AliFlowEventSimple::Class());
 
 // Output slot #0 is reserved
 // Output slot #1 writes into a TList container
 DefineOutput(1, TList::Class());
 
 // Event weights:
 fMultiplicityWeight = new TString("combinations");
 
 // b) Initialize default min and max values of correlations:
 //    (Remark: The default values bellow were chosen for v2=5% and M=500)
 fMinValueOfCorrelation[0] = -0.015; // <2>_min
 fMaxValueOfCorrelation[0] = 0.03; // <2>_max
 fMinValueOfCorrelation[1] = -0.6e-3; // <4>_min
 fMaxValueOfCorrelation[1] = 0.07; // <4>_max
 fMinValueOfCorrelation[2] = -0.08e-3; // <6>_min
 fMaxValueOfCorrelation[2] = 0.015; // <6>_max
 fMinValueOfCorrelation[3] = -20.e-6; // <8>_min
 fMaxValueOfCorrelation[3] = 0.003; // <8>_max
 
 // c) Initialize default min and max values of correlation products:
 //    (Remark: The default values bellow were chosen for v2=5% and M=500)
 fMinValueOfCorrelationProduct[0] = -15.e-6; // <2><4>_min
 fMaxValueOfCorrelationProduct[0] = 0.02; // <2><4>_max
 
 // d) Initialize default min and max values of q-vector terms:
 fMinValueOfQvectorTerms[0] = 0.;
 fMaxValueOfQvectorTerms[0] = 30.;
 fMinValueOfQvectorTerms[1] = 0.;
 fMaxValueOfQvectorTerms[1] = 20.;
 fMinValueOfQvectorTerms[2] = 0.;
 fMaxValueOfQvectorTerms[2] = 200.;
 fMinValueOfQvectorTerms[3] = -30.;
 fMaxValueOfQvectorTerms[3] = 80.;
 
}

//================================================================================================================

AliAnalysisTaskCRC::AliAnalysisTaskCRC():
AliAnalysisTaskSE(),
fEvent(NULL),
fQC(NULL),
fListHistos(NULL),
fBookOnlyBasicCCH(kFALSE),
fFillMultipleControlHistograms(kFALSE),
fHarmonic(0),
fApplyCorrectionForNUA(kFALSE),
fApplyCorrectionForNUAVsM(kFALSE),
fPropagateErrorAlsoFromNIT(kFALSE),
fCalculateDiffFlow(kFALSE),
fCalculate2DDiffFlow(kFALSE),
fCalculateDiffFlowVsEta(kTRUE),
fStoreDistributions(kFALSE),
fCalculateCumulantsVsM(kFALSE),
fCalculateAllCorrelationsVsM(kFALSE),
fCalculateMixedHarmonics(kFALSE),
fCalculateMixedHarmonicsVsM(kFALSE),
fStoreControlHistograms(kFALSE),
fMinimumBiasReferenceFlow(kFALSE),
fForgetAboutCovariances(kFALSE),
fStoreVarious(kFALSE),
fExactNoRPs(0),
fUse2DHistograms(kFALSE),
fFillProfilesVsMUsingWeights(kTRUE),
fUseQvectorTerms(kFALSE),
fnBinsMult(0),
fMinMult(0.),
fMaxMult(0.),
fUseParticleWeights(kFALSE),
fUsePhiWeights(kFALSE),
fUsePtWeights(kFALSE),
fUseEtaWeights(kFALSE),
fUseTrackWeights(kFALSE),
fUsePhiEtaWeights(kFALSE),
fWeightsList(NULL),
fMultiplicityWeight(NULL),
fMultiplicityIs(AliFlowCommonConstants::kRP),
fnBinsForCorrelations(0),
fUseBootstrap(kFALSE),
fUseBootstrapVsM(kFALSE),
fnSubsamples(10),
fCalculateCRC(kTRUE),
fCalculateCRCPt(kFALSE),
fCalculateCRCBck(kFALSE),
fUseNUAforCRC(kFALSE),
fUseCRCRecenter(kFALSE),
fCRCEtaMin(0.),
fCRCEtaMax(0.),
fQVecList(NULL)
{
 // Dummy constructor
 AliDebug(2,"AliAnalysisTaskCRC::AliAnalysisTaskCRC()");
 
 // b) Initialize default min and max values of correlations:
 //    (Remark: The default values bellow were chosen for v2=5% and M=500)
 fMinValueOfCorrelation[0] = -0.015; // <2>_min
 fMaxValueOfCorrelation[0] = 0.03; // <2>_max
 fMinValueOfCorrelation[1] = -0.6e-3; // <4>_min
 fMaxValueOfCorrelation[1] = 0.07; // <4>_max
 fMinValueOfCorrelation[2] = -0.08e-3; // <6>_min
 fMaxValueOfCorrelation[2] = 0.015; // <6>_max
 fMinValueOfCorrelation[3] = -20.e-6; // <8>_min
 fMaxValueOfCorrelation[3] = 0.003; // <8>_max
 
 // c) Initialize default min and max values of correlation products:
 //    (Remark: The default values bellow were chosen for v2=5% and M=500)
 fMinValueOfCorrelationProduct[0] = -15.e-6; // <2><4>_min
 fMaxValueOfCorrelationProduct[0] = 0.02; // <2><4>_max
 
 // d) Initialize default min and max values of q-vector terms:
 fMinValueOfQvectorTerms[0] = 0.;
 fMaxValueOfQvectorTerms[0] = 30.;
 fMinValueOfQvectorTerms[1] = 0.;
 fMaxValueOfQvectorTerms[1] = 20.;
 fMinValueOfQvectorTerms[2] = 0.;
 fMaxValueOfQvectorTerms[2] = 200.;
 fMinValueOfQvectorTerms[3] = -30.;
 fMaxValueOfQvectorTerms[3] = 80.;
 
}

//==========================================================================================================

void AliAnalysisTaskCRC::UserCreateOutputObjects()
{
 // Called at every worker node to initialize
 AliDebug(2,"AliAnalysisTaskCRC::UserCreateOutputObjects()");
 
 // Analyser:
 fQC = new AliFlowAnalysisCRC("AliFlowAnalysisCRC",fCRCRunBinMin,fCRCRunBinMax);
 
 // Common:
 fQC->SetBookOnlyBasicCCH(fBookOnlyBasicCCH);
 fQC->SetFillMultipleControlHistograms(fFillMultipleControlHistograms);
 fQC->SetHarmonic(fHarmonic);
 fQC->SetApplyCorrectionForNUA(fApplyCorrectionForNUA);
 fQC->SetApplyCorrectionForNUAVsM(fApplyCorrectionForNUAVsM);
 fQC->SetPropagateErrorAlsoFromNIT(fPropagateErrorAlsoFromNIT);
 fQC->SetCalculateDiffFlow(fCalculateDiffFlow);
 fQC->SetCalculate2DDiffFlow(fCalculate2DDiffFlow);
 fQC->SetCalculateDiffFlowVsEta(fCalculateDiffFlowVsEta);
 fQC->SetStoreDistributions(fStoreDistributions);
 fQC->SetCalculateCumulantsVsM(fCalculateCumulantsVsM);
 fQC->SetCalculateAllCorrelationsVsM(fCalculateAllCorrelationsVsM);
 fQC->SetCalculateMixedHarmonics(fCalculateMixedHarmonics);
 fQC->SetCalculateMixedHarmonicsVsM(fCalculateMixedHarmonicsVsM);
 fQC->SetStoreControlHistograms(fStoreControlHistograms);
 fQC->SetMinimumBiasReferenceFlow(fMinimumBiasReferenceFlow);
 fQC->SetForgetAboutCovariances(fForgetAboutCovariances);
 fQC->SetExactNoRPs(fExactNoRPs);
 fQC->SetCalculateCRC(fCalculateCRC);
 fQC->SetCalculateCRCPt(fCalculateCRCPt);
 fQC->SetCalculateCRCBck(fCalculateCRCBck);
 fQC->SetNUAforCRC(fUseNUAforCRC);
 fQC->SetUseCRCRecenter(fUseCRCRecenter);
 fQC->SetCRCEtaRange(fCRCEtaMin,fCRCEtaMax);
 // Multiparticle correlations vs multiplicity:
 fQC->SetnBinsMult(fnBinsMult);
 fQC->SetMinMult(fMinMult);
 fQC->SetMaxMult(fMaxMult);
 // Particle weights:
 if(fUseParticleWeights) {
  // Pass the flags to class:
  if(fUsePhiWeights){fQC->SetUsePhiWeights(fUsePhiWeights);}
  if(fUsePtWeights){fQC->SetUsePtWeights(fUsePtWeights);}
  if(fUseEtaWeights){fQC->SetUseEtaWeights(fUseEtaWeights);}
  if(fUseTrackWeights){fQC->SetUseTrackWeights(fUseTrackWeights);}
  if(fUsePhiEtaWeights){fQC->SetUsePhiEtaWeights(fUsePhiEtaWeights);}
  
  // Pass the list with weights to class:
  if(fWeightsList) fQC->SetWeightsList(fWeightsList);
 }
 // Event weights:
 if(!fMultiplicityWeight->Contains("combinations")) {
  fQC->SetMultiplicityWeight(fMultiplicityWeight->Data());
 }
 // Q Vector weights:
 if(fUseCRCRecenter) {
  if(fQVecList) fQC->SetCRCQVecWeightsList(fQVecList);
 }
 
 fQC->SetMultiplicityIs(fMultiplicityIs);
 fQC->SetnBinsForCorrelations(fnBinsForCorrelations);
 fQC->SetUse2DHistograms(fUse2DHistograms);
 fQC->SetFillProfilesVsMUsingWeights(fFillProfilesVsMUsingWeights);
 fQC->SetUseQvectorTerms(fUseQvectorTerms);
 
 // Store phi distribution for one event to illustrate flow:
 fQC->SetStoreVarious(fStoreVarious);
 
 // Initialize default min and max values of correlations:
 for(Int_t ci=0;ci<4;ci++) {
  fQC->SetMinValueOfCorrelation(ci,fMinValueOfCorrelation[ci]);
  fQC->SetMaxValueOfCorrelation(ci,fMaxValueOfCorrelation[ci]);
 }
 
 // Initialize default min and max values of correlation products:
 for(Int_t cpi=0;cpi<1;cpi++) {
  fQC->SetMinValueOfCorrelationProduct(cpi,fMinValueOfCorrelationProduct[cpi]);
  fQC->SetMaxValueOfCorrelationProduct(cpi,fMaxValueOfCorrelationProduct[cpi]);
 }
 
 // Initialize default min and max values of Q-vector terms:
 for(Int_t ci=0;ci<4;ci++) {
  fQC->SetMinValueOfQvectorTerms(ci,fMinValueOfQvectorTerms[ci]);
  fQC->SetMaxValueOfQvectorTerms(ci,fMaxValueOfQvectorTerms[ci]);
 }
 
 // Bootstrap:
 fQC->SetUseBootstrap(fUseBootstrap);
 fQC->SetUseBootstrapVsM(fUseBootstrapVsM);
 fQC->SetnSubsamples(fnSubsamples);
 
 fQC->Init();
 
 if(fQC->GetHistList()) {
  fListHistos = fQC->GetHistList();
 } else {
  Printf("ERROR: Could not retrieve histogram list (QC, Task::UserCreateOutputObjects()) !!!!");
 }
 
 PostData(1,fListHistos);
 
} // end of void AliAnalysisTaskCRC::UserCreateOutputObjects()

//================================================================================================================

void AliAnalysisTaskCRC::UserExec(Option_t *)
{
 // main loop (called for each event)
 fEvent = dynamic_cast<AliFlowEventSimple*>(GetInputData(0));
 
 // Q-cumulants
 if(fEvent) {
  fQC->SetRunNumber(fEvent->GetRun());
  fQC->Make(fEvent);
 } else {
  cout<<"WARNING: No input data (QC, Task::UserExec()) !!!!"<<endl;
  cout<<endl;
 }
 
 PostData(1,fListHistos);
}

//================================================================================================================

void AliAnalysisTaskCRC::Terminate(Option_t *)
{
 //accessing the merged output list:
 fListHistos = (TList*)GetOutputData(1);
 
 fQC = new AliFlowAnalysisCRC("AliFlowAnalysisCRC",fCRCRunBinMin,fCRCRunBinMax);
 
 if(fListHistos) {
  fQC->GetOutputHistograms(fListHistos);
  fQC->Finish();
  PostData(1,fListHistos);
 } else {
  cout<<" WARNING: histogram list pointer is empty (QC, Task::Terminate()) !!!!"<<endl;
  cout<<endl;
 }
 
} // end of void AliAnalysisTaskCRC::Terminate(Option_t *)





















