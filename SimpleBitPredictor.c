#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

int32_t microOpCount, sourceRegister1, sourceRegister2, destinationRegister;
char conditionRegister, TNnotBranch, loadStore, macroOperation[12], microOperation[23];
uint64_t instructionAddress, addressForMemoryOp, fallthroughPC, targetAddressTakenBranch;
int64_t immediate;

int oneBit=0, twoBit=0;

char oneBitPredictor(uint64_t instructionAddress, char TNnotBranch){

    int state=oneBit;

    if(oneBit==0 && TNnotBranch=='T')
        oneBit=1;
    if(oneBit==1 && TNnotBranch=='N')
        oneBit=0;

    if(state==0)
        return 'N';
    else
        return 'T';
    
}

char twoBitPredictor(uint64_t instructionAddress, char TNnotBranch){

    int state=twoBit;

    if(twoBit<3 && TNnotBranch=='T')
        twoBit+=1;
    if(twoBit>0 && TNnotBranch=='N')
        twoBit-=1;

    if(state<2)
        return 'N';
    if(state>1)
        return 'T';
}



int main()
{
    FILE *input=stdin;
    int num=0, count=1e5;
    int pTbT=0, pTbN=0, pNbT=0, pNbN=0, total;
    float accuracy;

    // char (*predictor)(uint64_t, char) = &oneBitPredictor;
    char (*predictor)(uint64_t, char) = &twoBitPredictor;

    while (true) {

        int result = fscanf(input, 
                            "%" SCNi32 "%" SCNx64 "%" SCNi32 "%" SCNi32 "%" SCNi32
                            " %c" " %c" " %c" "%" SCNi64 "%" SCNx64 "%" SCNx64 "%" SCNx64 "%11s" "%22s",
                            &microOpCount, &instructionAddress, &sourceRegister1, &sourceRegister2,
                            &destinationRegister, &conditionRegister, &TNnotBranch, &loadStore,
                            &immediate, &addressForMemoryOp, &fallthroughPC, &targetAddressTakenBranch,
                            macroOperation, microOperation);
                            
        if (result == EOF) break;

        if(TNnotBranch=='T')
            if( (*predictor)(instructionAddress, TNnotBranch) == 'T' ) pTbT++;
            else pNbT++;

        else if(TNnotBranch=='N')
            if( (*predictor)(instructionAddress, TNnotBranch) == 'N' ) pNbN++;
            else pTbN++;   

        num++;
        if(num==count) break;
    }

    total = pTbT+pTbN+pNbT+pNbN;
    accuracy = (float) (pTbT + pNbN) / total * 100.0;

    printf("total,\t pTbT,\t pTbN,\t pNbT,\t pNbN,\t accuracy\n %d,\t %d,\t %d,\t %d,\t %d,\t %.3f\n", total, pTbT, pTbN, pNbT, pNbN, accuracy);

}



