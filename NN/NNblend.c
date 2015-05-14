////////////////////////////////////////////////////////////////////////////
// Neural Network Blender for Netflix Prize
// Author: Aakash Indurkhya
// Collaborators: 
// Date: 5/14/14
// Summary: Here I implement a neural network blender. I take in the probe
// set predictions from our various algorithms, and allow a neural network
// to determine the weights to apply for each algorithm in the test set 
// prediction. As recommended by the BellKor paper, I use 1000 epochs
// Otherwise, I use standard NN procedure.  
////////////////////////////////////////////////////////////////////////////

//#include <iostream.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string.h>


//// Data dependent settings ////
#define numInputs       3
#define numPatterns     1374739
//#define numPatterns     100
#define numPreds        2749898

////////////////////////////////////////////////////////////////////////////
//// User defineable settings ////
#define numHidden       100
const int numEpochs =   500;
double LR_IH;
double LR_HO;
#define NUMRUNS         5
////////////////////////////////////////////////////////////////////////////


//// functions ////
void initWeights();
void initData();
static inline void calcNet();
void WeightChangesHO();
void WeightChangesIH();
void calcOverallError();
void displayResults();
double getRand();
void predict();

//// variables ////
int patNum = 0;
double errThisPat = 0.0;
double outPred = 0.0;
double RMSerror = 0.0;

// the outputs of the hidden neurons
double hiddenVal[numHidden];

// the weights
double weightsIH[numInputs][numHidden];
double weightsHO[numHidden];

// the data
float trainInputs[numPatterns][numInputs];
float trainOutput[numPatterns];
float prediction[numPatterns][numInputs];

// data loading buffers


//==============================================================
//************** function definitions **************************
//==============================================================


//***********************************
// calculates the network output
static inline
void calcNet(void)
{
    //calculate the outputs of the hidden neurons
    //the hidden neurons are tanh
    int i = 0;
    for(i = 0;i<numHidden;i++) {
	      hiddenVal[i] = 0.0;
        for(int j = 0;j<numInputs;j++) {
            
	             hiddenVal[i] = hiddenVal[i] + (trainInputs[patNum][j] * weightsIH[j][i]);
      }
        hiddenVal[i] = tanh(hiddenVal[i]);
    }

    //calculate the output of the network
    //the output neuron is linear
    outPred = 0.0;

    for(i = 0;i<numHidden;i++) {
        outPred = outPred + hiddenVal[i] * weightsHO[i];
    }

    // outPred = tanh(outPred);

    //calculate the error
    errThisPat = outPred - trainOutput[patNum];
}

//***********************************
// calculates the network output
void predict(void)
{
    //calculate the outputs of the hidden neurons
    //the hidden neurons are tanh
    int i = 0;
    for(i = 0;i<numHidden;i++) {
        hiddenVal[i] = 0.0;
        for(int j = 0;j<numInputs;j++) {
           hiddenVal[i] = hiddenVal[i] + (prediction[patNum][j] * weightsIH[j][i]);
        }
        hiddenVal[i] = tanh(hiddenVal[i]);
    }

    //calculate the output of the network
    //the output neuron is linear
    outPred = 0.0;

    for(i = 0;i<numHidden;i++) {
        outPred = outPred + hiddenVal[i] * weightsHO[i];
    }
    // outPred = tanh(outPred);
}


//************************************
//adjust the weights hidden-output
void WeightChangesHO(void) {
     for(int k = 0;k<numHidden;k++) {
          double weightChange = LR_HO * errThisPat * hiddenVal[k];
          weightsHO[k] = weightsHO[k] - weightChange;
          //regularisation on the output weights
          if (weightsHO[k] < -5) {
              weightsHO[k] = -5;
          }
          else if (weightsHO[k] > 5) {
              weightsHO[k] = 5;
          }
     }
 }


//************************************
// adjust the weights input-hidden
void WeightChangesIH(void) {
    for(int i = 0;i<numHidden;i++) {
        for(int k = 0;k<numInputs;k++) {
            double x = 1 - (hiddenVal[i] * hiddenVal[i]);
            x = x * weightsHO[i] * errThisPat * LR_IH;
            x = x * trainInputs[patNum][k];
            double weightChange = x;
            weightsIH[k][i] = weightsIH[k][i] - weightChange;
        }
    }
}


//************************************
// generates a random number
double getRand(void) {
    return ((double)rand())/((double)RAND_MAX / 2);
}



//************************************
// set weights to random numbers 
void initWeights(void) {
 for(int j = 0;j<numHidden;j++){
      weightsHO[j] = (getRand() - 1)/sqrt(numPatterns);
      for(int i = 0;i<numInputs;i++){
          weightsIH[i][j] = (getRand() - 1)/sqrt(numPatterns);
          //printf("Weight = %f\n", weightsIH[i][j]);
      }
  }
}


// scale a rating to be between -1 and 1
static inline
float scale(float r) {
    float result = ((r - 1.0) / 2.0) - 1.0;
    return result;
}

// rescale rating predictions such that they are between 1 and 5
static inline
float rescale(float r) {
    float result = 1.0 + ((r + 1.0) * 2.0);
    // return the rescaled result while dealing with nan results. 
    return result;
    //return (result <= 1.0 || result >= 5.0) ? 3.2 : result;
}

//************************************
// read in the data
void initData(void) {
    printf("INITIALISING DATA\n");

    FILE * fp0 = fopen("../stats/probe.dta", "r");
    FILE *fp1 = fopen("../SVDsimple/results/um_probe_f220_e060_t.dta", "r");
    FILE *fp2 = fopen("../SVDsimple/results/um_probe_f100_e040_t.dta", "r");

    char str0[20];
    char str1[20];
    char str2[20];
    char str3[20];

    int pat = 0; 

    while (fgets(str0, 20, fp0) != NULL && fgets(str1, 20, fp1) != NULL
        && fgets(str2, 20, fp2) != NULL) { //  && fgets(str3, 20, fp3) != NULL) {
        // all of the data here has been scaled to be in [-1, 1]
        // also each input list has an extra value of 1 to serve as the bias. 
        // pat
        // in the actual probe rating
        // Skip user, movie, and time in probe and just get rating
        strtok(str0, " ");
        strtok(NULL, " ");
        strtok(NULL, " ");
        trainOutput[pat] = scale(atof((char *) strtok(NULL, " ")));

        // load in the predicted ratings
        trainInputs[pat][0] = scale(atof((char *) strtok(str1, " ")));
        trainInputs[pat][1] = scale(atof((char *) strtok(str2, " ")));
        trainInputs[pat][2] = 1;       // bias


        pat++;
        if (pat + 1 == numPatterns)
          break;
    }


}


//************************************
// display results
void makePredictions(int resultNum) {
      // load in the test set
    printf("LOADING TEST SET PREDICTIONS\n");
    FILE *fp3 = fopen("../SVDsimple/results/um_test_f220_e060.dta", "r");
    FILE *fp4 = fopen("../SVDsimple/results/um_test_f100_e040.dta", "r");
    // FILE *fp6 = fopen("mavg/mavg_test.dta", "r");
    
    char blend[60];
    sprintf(blend, "f220_f100_NNBlend_%d_%d_result_%d_.dta", numHidden, numEpochs,resultNum);
    FILE *RESULT = fopen(blend, "w");
    char str4[20];
    char str5[20];
    char str6[20];


    int result = 0;
    while (fgets(str4, 20, fp3) != NULL && fgets(str5, 20, fp4) != NULL){
    // && fgets(str6, 20, fp6) != NULL) {
        // load in the prediction ratings for the test set
        prediction[result][0] = scale(atof((char *) strtok(str4, " ")));
        prediction[result][1] = scale(atof((char *) strtok(str5, " ")));
        // prediction[result][2] = scale(atof((char *) strtok(str6, " ")));
        prediction[result][2] = 1;

        result++;
    }

   for(int i = 0; i<numPreds; i++) {
        patNum = i;
        predict();
        // printf("prediction = %d neural model = %f\n", patNum+1, outPred);
        fprintf(RESULT, "%f\n", rescale(outPred));
   }

   fclose(RESULT);
}


//************************************
// calculate the overall error
void calcOverallError(void) {
     RMSerror = 0.0;
     for(int i = 0;i<numPatterns;i++) {
         patNum = i;
         calcNet();
         RMSerror += (errThisPat * errThisPat);
     }
     RMSerror = sqrt(RMSerror/numPatterns);
}



//==============================================================
//********** THIS IS THE MAIN PROGRAM **************************
//==============================================================


int main(void) {
  int resultNum;
  for (resultNum = 0; resultNum < NUMRUNS; resultNum++) {
   // seed random number function
   srand ( time(NULL) );

   // refresh learning rates
   //LR_IH =          0.0008;
   //LR_HO =          0.0003;
   LR_IH = 0.01;
   LR_HO = 0.01;
   // initiate the weights
   initWeights();

   // load in the data
   initData();



   // train the network
      for(int j = 0;j <= numEpochs;j++) {
          for(int i = 0;i<numPatterns;i++) {
              //select a pattern at random
              patNum = i;

              //calculate the current network output
              //and error for this pattern
              calcNet();


              WeightChangesHO();
              WeightChangesIH();
          }

          //change network weights
          LR_IH = LR_IH -  0.0000001;
          LR_HO = LR_HO -   0.0000001;

          //display the overall network error
          //after each epoch
          calcOverallError();

          printf("epoch = %d RMS Error = %f\n",j,RMSerror);
      }



    // training is now finished. Proceed with predictions
    makePredictions(resultNum);

    printf("BLENDING COMPLETED - run %d\n", resultNum);
  }

   system("PAUSE");
   return 0;
}
