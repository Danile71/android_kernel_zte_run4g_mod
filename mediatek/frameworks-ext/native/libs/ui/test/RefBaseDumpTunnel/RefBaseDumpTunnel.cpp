#include <stdio.h>
#include <time.h>
#include <utils/StrongPointer.h>
#include "RefBaseDumpTunnel.h"

using namespace android;

void RefBaseMonitorTest_Assignment(int testCnt, struct timespec *tsS, struct timespec *tsE ) {
    sp<RefBaseTest> sp1 = new RefBaseTest();
    sp<RefBaseTest> sp2;
    //printf("[1]");
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, tsS);
    //printf("with RefBaseTracking....S:%d(ns)\n",tsS.tv_nsec);
    for (int i = 0; i < testCnt; i++) {
         sp2 = sp1;
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, tsE);
}

void RefBaseMonitorTest_CtorDtor(int testCnt, struct timespec *tsS, struct timespec *tsE ) {
    RefBaseTest *rbtmp;
    //printf("[2]");
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, tsS);
    //printf("with RefBaseTracking....S:%d(ns)\n",tsS.tv_nsec);
    for (int i = 0; i < testCnt; i++) {
        rbtmp = new RefBaseTest();
        delete rbtmp;
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, tsE);
    //printf("with RefBaseTracking....E:%d(ns)\n",tsE.tv_nsec);
}

void RefBaseMonitorTest_CtorDtorAssgnment(int testCnt, struct timespec *tsS, struct timespec *tsE ) {
    sp<RefBaseTest> sp1 = new RefBaseTest();
    //printf("[3]");
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, tsS);
    //printf("with RefBaseTracking....S:%d(ns)\n",tsS.tv_nsec);
    for (int i = 0; i < testCnt; i++) {
        sp1 = new RefBaseTest();
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, tsE);
    //printf("with RefBaseTracking....E:%d(ns)\n",tsE.tv_nsec);
}

void RefBaseMonitorSubTest (int testRound, int testCntPerRound, void (*testFunc)(int, struct timespec *, struct timespec *)) {
    struct timespec tsS;
    struct timespec tsE;
    int tmp;
    int totalCnt;
    int timeWithTracking = 0;

    for(int j = 0; j < testRound; j++) {
        //printf("Rnd: %d",j);
        (*testFunc)(testCntPerRound, &tsS, &tsE);

        tmp = tsE.tv_nsec - tsS.tv_nsec;
        if (tmp < 0) {
            tmp = 1000000000L + tsE.tv_nsec - tsS.tv_nsec;
        }
        timeWithTracking += tmp;
        //printf("with RefBaseTracking....D:%d(ns)\n",tmp);
    }
    totalCnt = testRound * testCntPerRound;
    printf("%12d(ns) = TotalTime: %12d(ns) / TestCnt: %d \n", timeWithTracking / totalCnt, timeWithTracking, totalCnt);

}

void RefBaseMonitorTest(int recCount, int testRound, int testCntPerRound) {
    int tmp = recCount;
    if (recCount == 0) {
        RefBaseMonitorSubTest(testRound, testCntPerRound, RefBaseMonitorTest_Assignment);
        RefBaseMonitorSubTest(testRound, testCntPerRound, RefBaseMonitorTest_CtorDtor);
        RefBaseMonitorSubTest(testRound, testCntPerRound, RefBaseMonitorTest_CtorDtorAssgnment);
    } else {
        recCount--;
        RefBaseMonitorTest(recCount, testRound, testCntPerRound);
    }
    printf("(stack:%d)", tmp);
}

int main(int argc, char** argv) {
    int recLv;
    int testRound;
    int testCntPerRound;
    int recCntTime;

    if (argc == 5) {
        sscanf(argv[1], "%d", &recLv);
        sscanf(argv[2], "%d", &recCntTime);
        sscanf(argv[3], "%d", &testRound);
        sscanf(argv[4], "%d", &testCntPerRound);
        if ((recLv >= 0) && (recCntTime > 0) && (testRound > 0) && (testCntPerRound > 0)) {
            for (int i = 0; i < recCntTime; i++) {
                printf("\nrec[ n + %d ]\n", i + recLv);
                RefBaseMonitorTest(i + recLv, testRound, testCntPerRound);
            }
            printf("\n\n");
            return 0;
        }
    }
    printf("test-RefBaseDumpTunnel recLv recTimes testRnd testCntPerRnd\n");
    return 0;
}


