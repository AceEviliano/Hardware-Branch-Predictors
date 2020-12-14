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


uint64_t mask=0xf;
int FAsize = 2;
int *BHT;


uint64_t extractBits(uint64_t instructionAddress, uint64_t mask){

    /*
        Extracts the bits from PC address that are specified as 1 in the mask.
        Concatenates these bits and returns them as an integer.

        Example:
        mask = 0x33     , corresponds to '0011 0011' in binary
        PC = 0x93       , corresponds to '1001 0011' in binary
        result = 0x7    , corresponds to '0111' in binary

        The result was made from extracting last 2 bits from 1st byte and last 2 bits from 2nd byte
    */

    uint64_t imask = mask, jmask=1;
    uint64_t result = 0;

    for(int i=0; i<32; i++)
        if( (1<<i) & imask )
        {
            if( (1<<i) &  instructionAddress )
                result = result|jmask;

            jmask = jmask<<1;
        }

    return result;
}

int setBHT(uint64_t mask){

    /*
        This initializes the Branch History Table (BHT) defined globally.
        Branch History Table will be having 'n' bit predictors.
        Here n = 2^m, where 'm' is the number of bits that are on in the mask.
    */

    uint64_t m = mask;
    int sum=0;

    while(m>0)
    {
        sum+=m%2;
        m/=2;
    }

    BHT = (int *) calloc( (1<<sum), sizeof(int) );
    return 1<<sum;
}

char BHTPredictor(uint64_t instructionAddress, char TNnotBranch){

    uint64_t index;

    index = extractBits(instructionAddress, mask);
    int state = BHT[index];

    if( BHT[index]<(FAsize-1) && TNnotBranch=='T')
        BHT[index]+=1;
    if( BHT[index]>0 && TNnotBranch=='N')
        BHT[index]-=1;

    if( state < FAsize/2 )
        return 'N';
    if( state > (FAsize-1)/2 )
        return 'T';

}


int main()
{
    FILE *input=stdin;
    int num=0, count=1e5;
    int pTbT=0, pTbN=0, pNbT=0, pNbN=0, total;
    float accuracy;

    int size = setBHT(mask);  
    printf("\nSize of index : %d\n", size);
    char (*predictor)(uint64_t, char) = &BHTPredictor;

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



