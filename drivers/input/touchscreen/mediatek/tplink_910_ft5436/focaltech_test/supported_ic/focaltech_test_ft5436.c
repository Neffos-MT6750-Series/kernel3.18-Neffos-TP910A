/************************************************************************
* Copyright (C) 2012-2017, Focaltech Systems (R)��All Rights Reserved.
*
* File Name: Test_FT5X46.c
*
* Author: Software Development Team, AE
*
* Created: 2015-07-14
*
* Abstract: test item for FT5X46\FT5X46i\FT5526\FT3X17\FT5436\FT3X27\FT5526i\FT5416\FT5426\FT5435
*
************************************************************************/

/*****************************************************************************
* Included header files
*****************************************************************************/
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/slab.h>

#include "../include/focaltech_test_detail_threshold.h"
#include "../include/focaltech_test_supported_ic.h"
#include "../include/focaltech_test_main.h"
#include "../focaltech_test_config.h"
#include "../include/focaltech_ic_table.h"

#if (FTS_CHIP_TEST_TYPE ==FT5X46_TEST)

/*****************************************************************************
* Private constant and macro definitions using #define
*****************************************************************************/

/////////////////////////////////////////////////Reg
#define DEVIDE_MODE_ADDR    0x00
#define REG_LINE_NUM    0x01
#define REG_TX_NUM  0x02
#define REG_RX_NUM  0x03
#define REG_PATTERN_5422        0x53
#define REG_MAPPING_SWITCH      0x54
#define REG_TX_NOMAPPING_NUM        0x55
#define REG_RX_NOMAPPING_NUM      0x56
#define REG_NORMALIZE_TYPE      0x16
#define REG_ScCbBuf0    0x4E
#define REG_ScWorkMode  0x44
#define REG_ScCbAddrR   0x45
#define REG_RawBuf0 0x36
#define REG_WATER_CHANNEL_SELECT 0x09

/*****************************************************************************
* Private enumerations, structures and unions using typedef
*****************************************************************************/
enum WaterproofType {
    WT_NeedProofOnTest,
    WT_NeedProofOffTest,
    WT_NeedTxOnVal,
    WT_NeedRxOnVal,
    WT_NeedTxOffVal,
    WT_NeedRxOffVal,
};
/*****************************************************************************
* Static variables
*****************************************************************************/

static int m_RawData[TX_NUM_MAX][RX_NUM_MAX] = { {0} };
static int m_iTempRawData[TX_NUM_MAX * RX_NUM_MAX] = { 0 };
static unsigned char m_ucTempData[TX_NUM_MAX * RX_NUM_MAX * 2] = { 0 };

static bool m_bV3TP = false;
static int m_DifferData[TX_NUM_MAX][RX_NUM_MAX] = { {0} };
static int m_absDifferData[TX_NUM_MAX][RX_NUM_MAX] = { {0} };

/*****************************************************************************
* Global variable or extern global variabls/functions
*****************************************************************************/
extern struct stCfg_FT5X46_BasicThreshold g_stCfg_FT5X46_BasicThreshold;

/*****************************************************************************
* Static function prototypes
*****************************************************************************/
//////////////////////////////////////////////Communication function
static int StartScan(void);
static unsigned char ReadRawData(unsigned char Freq, unsigned char LineNum, int ByteNum, int *pRevBuffer);
static unsigned char GetPanelRows(unsigned char *pPanelRows);
static unsigned char GetPanelCols(unsigned char *pPanelCols);
static unsigned char GetTxSC_CB(unsigned char index, unsigned char *pcbValue);
static unsigned char GetRawData(void);
static unsigned char GetChannelNum(void);
static void Save_Test_Data(int iData[TX_NUM_MAX][RX_NUM_MAX], int iArrayIndex, unsigned char Row, unsigned char Col, unsigned char ItemCount);
static void ShowRawData(void);
static boolean GetTestCondition(int iTestType, unsigned char ucChannelValue);
static unsigned char GetChannelNumNoMapping(void);
static unsigned char SwitchToNoMapping(void);
static unsigned char WeakShort_GetAdcData(int AllAdcDataLen, int *pRevBuffer);

boolean FT5X46_StartTest(void);
int FT5X46_get_test_data(char *pTestData);  //pTestData, External application for memory, buff size >= 1024*80

unsigned char FT5X46_TestItem_EnterFactoryMode(void);
unsigned char FT5X46_TestItem_RawDataTest(bool * bTestResult);
unsigned char FT5X46_TestItem_SCapRawDataTest(bool * bTestResult);
unsigned char FT5X46_TestItem_SCapCbTest(bool * bTestResult);
unsigned char FT5X46_TestItem_PanelDifferTest(bool * bTestResult);
unsigned char FT5X46_TestItem_WeakShortTest(bool * bTestResult);

/************************************************************************
* Name: FT5X46_StartTest
* Brief:  Test entry. Determine which test item to test
* Input: none
* Output: none
* Return: Test Result, PASS or FAIL
***********************************************************************/
boolean FT5X46_StartTest()
{
    bool bTestResult = true;
    bool bTempResult = 1;
    unsigned char ReCode = 0;
    unsigned char ucDevice = 0;
    int iItemCount = 0;

    FTS_TEST_FUNC_ENTER();

    //--------------1. Init part
    if (InitTest() < 0) {
        FTS_TEST_ERROR("[focal] Failed to init test.");
        return false;
    }

    //--------------2. test item
    if (0 == g_TestItemNum)
        bTestResult = false;

    for (iItemCount = 0; iItemCount < g_TestItemNum; iItemCount++) {
        m_ucTestItemCode = g_stTestItem[ucDevice][iItemCount].ItemCode;

        ///////////////////////////////////////////////////////FT5X46_ENTER_FACTORY_MODE
        if (Code_FT5X46_ENTER_FACTORY_MODE == g_stTestItem[ucDevice][iItemCount].ItemCode) {

            ReCode = FT5X46_TestItem_EnterFactoryMode();
            if (ERROR_CODE_OK != ReCode || (!bTempResult)) {
                bTestResult = false;
                g_stTestItem[ucDevice][iItemCount].TestResult = RESULT_NG;
                break;          //if this item FAIL, no longer test.
            }
            else
                g_stTestItem[ucDevice][iItemCount].TestResult = RESULT_PASS;
        }

        ///////////////////////////////////////////////////////FT5X46_RAWDATA_TEST
        if (Code_FT5X46_RAWDATA_TEST == g_stTestItem[ucDevice][iItemCount].ItemCode) {

            ReCode = FT5X46_TestItem_RawDataTest(&bTempResult);
            if (ERROR_CODE_OK != ReCode || (!bTempResult)) {
                bTestResult = false;
                g_stTestItem[ucDevice][iItemCount].TestResult = RESULT_NG;
            }
            else
                g_stTestItem[ucDevice][iItemCount].TestResult = RESULT_PASS;
        }

        ///////////////////////////////////////////////////////FT5X46_SCAP_CB_TEST
        if (Code_FT5X46_SCAP_CB_TEST == g_stTestItem[ucDevice][iItemCount].ItemCode) {
            ReCode = FT5X46_TestItem_SCapCbTest(&bTempResult);
            if (ERROR_CODE_OK != ReCode || (!bTempResult)) {
                bTestResult = false;
                g_stTestItem[ucDevice][iItemCount].TestResult = RESULT_NG;
            }
            else
                g_stTestItem[ucDevice][iItemCount].TestResult = RESULT_PASS;
        }

        ///////////////////////////////////////////////////////FT5X46_SCAP_RAWDATA_TEST
        if (Code_FT5X46_SCAP_RAWDATA_TEST == g_stTestItem[ucDevice][iItemCount].ItemCode) {
            ReCode = FT5X46_TestItem_SCapRawDataTest(&bTempResult);
            if (ERROR_CODE_OK != ReCode || (!bTempResult)) {
                bTestResult = false;
                g_stTestItem[ucDevice][iItemCount].TestResult = RESULT_NG;
            }
            else
                g_stTestItem[ucDevice][iItemCount].TestResult = RESULT_PASS;
        }

        /////////////////////////////////////////////////////// Code_FT5X46_PANELDIFFER_TEST,
        if (Code_FT5X46_PANELDIFFER_TEST == g_stTestItem[ucDevice][iItemCount].ItemCode) {

            ReCode = FT5X46_TestItem_PanelDifferTest(&bTempResult);
            if (ERROR_CODE_OK != ReCode || (!bTempResult)) {
                bTestResult = false;
                g_stTestItem[ucDevice][iItemCount].TestResult = RESULT_NG;
            }
            else
                g_stTestItem[ucDevice][iItemCount].TestResult = RESULT_PASS;
        }
        ///////////////////////////////////////////////////////Code_FT5X46_WEAK_SHORT_CIRCUIT_TEST
        if (Code_FT5X46_WEAK_SHORT_CIRCUIT_TEST == g_stTestItem[ucDevice][iItemCount].ItemCode) {

            ReCode = FT5X46_TestItem_WeakShortTest(&bTempResult);
            if (ERROR_CODE_OK != ReCode || (!bTempResult)) {
                bTestResult = false;
                g_stTestItem[ucDevice][iItemCount].TestResult = RESULT_NG;
            }
            else
                g_stTestItem[ucDevice][iItemCount].TestResult = RESULT_PASS;
        }

    }

    //--------------3. End Part
    FinishTest();

    //--------------4. return result
    return bTestResult;
}

/************************************************************************
* Name: FT5X46_get_test_data
* Brief:  get data of test result
* Input: none
* Output: pTestData, the returned buff
* Return: the length of test data. if length > 0, got data;else ERR.
***********************************************************************/
int FT5X46_get_test_data(char *pTestData)
{
    if (NULL == pTestData) {
        FTS_TEST_ERROR(" pTestData == NULL ");
        return -1;
    }
    memcpy(pTestData, g_pStoreAllData, (g_lenStoreMsgArea + g_lenStoreDataArea));
    return (g_lenStoreMsgArea + g_lenStoreDataArea);
}

/************************************************************************
* Name: FT5X46_TestItem_EnterFactoryMode
* Brief:  Check whether TP can enter Factory Mode, and do some thing
* Input: none
* Output: none
* Return: Comm Code. Code = 0x00 is OK, else fail.
***********************************************************************/
unsigned char FT5X46_TestItem_EnterFactoryMode(void)
{
    unsigned char ReCode = ERROR_CODE_INVALID_PARAM;
    int iRedo = 5;              //If failed, repeat 5 times.
    int i;
    unsigned char chPattern = 0;

    FTS_TEST_FUNC_ENTER();

    SysDelay(150);
    for (i = 1; i <= iRedo; i++) {
        ReCode = EnterFactory();
        if (ERROR_CODE_OK != ReCode) {
            FTS_TEST_ERROR("Failed to Enter factory mode...");
            if (i < iRedo) {
                SysDelay(50);
                continue;
            }
        }
        else {
            break;
        }

    }
    SysDelay(300);

    if (ReCode != ERROR_CODE_OK) {
        return ReCode;
    }

    //After the success of the factory model, read the number of channels
    ReCode = GetChannelNum();

    ////////////set FIR,0:close,1:open
    //theDevice.m_cHidDev[m_NumDevice]->WriteReg(0xFB, 0);

    // to determine whether the V3 screen body
    ReCode = ReadReg(REG_PATTERN_5422, &chPattern);
    if (chPattern == 1) {
        m_bV3TP = true;
    }
    else {
        m_bV3TP = false;
    }

    return ReCode;
}

/************************************************************************
* Name: FT5X46_TestItem_RawDataTest
* Brief:  TestItem: RawDataTest. Check if MCAP RawData is within the range.
* Input: none
* Output: bTestResult, PASS or FAIL
* Return: Comm Code. Code = 0x00 is OK, else fail.
***********************************************************************/
unsigned char FT5X46_TestItem_RawDataTest(bool * bTestResult)
{
    unsigned char ReCode = 0;
    bool btmpresult = true;
    int RawDataMin;
    int RawDataMax;
    unsigned char ucFre;
    unsigned char strSwitch = 0;
    unsigned char OriginValue = 0xff;
    int index = 0;
    int iRow, iCol;
    int iValue = 0;
    int nRawDataOK = 0;

    FTS_TEST_INFO("\n\n==============================Test Item: -------- Raw Data  Test \n");
    ReCode = EnterFactory();
    if (ReCode != ERROR_CODE_OK) {
        FTS_TEST_ERROR("\n\n// Failed to Enter factory Mode. Error Code: %d", ReCode);
        goto TEST_ERR;
    }

    //Determine whether for v3 TP first, and then read the value of the 0x54
    //and check the mapping type is right or not,if not, write the register
    //rawdata test mapping before mapping 0x54=1;after mapping 0x54=0;
    if (m_bV3TP) {
        ReCode = ReadReg(REG_MAPPING_SWITCH, &strSwitch);
        if (ReCode != ERROR_CODE_OK) {
            FTS_TEST_ERROR("\n Read REG_MAPPING_SWITCH error. Error Code: %d", ReCode);
            goto TEST_ERR;
        }

        if (strSwitch != 0) {
            ReCode = WriteReg(REG_MAPPING_SWITCH, 0);
            if (ReCode != ERROR_CODE_OK) {
                FTS_TEST_ERROR("\n Write REG_MAPPING_SWITCH error. Error Code: %d", ReCode);
                goto TEST_ERR;
            }

            ReCode = GetChannelNum();
            if (ReCode != ERROR_CODE_OK) {
                FTS_TEST_ERROR("\n GetChannelNum error. Error Code: %d", ReCode);
                goto TEST_ERR;
            }
        }
    }

    //Line by line one after the rawdata value, the default 0X16=0
    ReCode = ReadReg(REG_NORMALIZE_TYPE, &OriginValue); // read the original value
    if (ReCode != ERROR_CODE_OK) {
        FTS_TEST_ERROR("\n Read  REG_NORMALIZE_TYPE error. Error Code: %d", ReCode);
        goto TEST_ERR;
    }

    if (g_ScreenSetParam.isNormalize == Auto_Normalize) {
        if (OriginValue != 1)   //if original value is not the value needed,write the register to change
        {
            ReCode = WriteReg(REG_NORMALIZE_TYPE, 0x01);
            if (ReCode != ERROR_CODE_OK) {
                FTS_TEST_ERROR("\n write  REG_NORMALIZE_TYPE error. Error Code: %d", ReCode);
                goto TEST_ERR;
            }
        }
        //Set Frequecy High

        FTS_TEST_DBG("\n=========Set Frequecy High\n");
        ReCode = WriteReg(0x0A, 0x81);
        if (ReCode != ERROR_CODE_OK) {
            FTS_TEST_ERROR("\n Set Frequecy High error. Error Code: %d", ReCode);
            goto TEST_ERR;
        }

        FTS_TEST_DBG("\n=========FIR State: ON");
        ReCode = WriteReg(0xFB, 1); //FIR OFF  0:close, 1:open
        if (ReCode != ERROR_CODE_OK) {
            FTS_TEST_ERROR("\n FIR State: ON error. Error Code: %d", ReCode);
            goto TEST_ERR;
        }

        //change register value before,need to lose 3 frame data
        for (index = 0; index < 3; ++index) {
            ReCode = GetRawData();
        }

        if (ReCode != ERROR_CODE_OK) {
            FTS_TEST_ERROR("\nGet Rawdata failed, Error Code: 0x%x", ReCode);
            goto TEST_ERR;
        }

        ShowRawData();

        ////////////////////////////////To Determine RawData if in Range or not
        for (iRow = 0; iRow < g_ScreenSetParam.iTxNum; iRow++) {
            for (iCol = 0; iCol < g_ScreenSetParam.iRxNum; iCol++) {
                if (g_stCfg_MCap_DetailThreshold.InvalidNode[iRow][iCol] == 0)
                    continue;   //Invalid Node
                RawDataMin = g_stCfg_MCap_DetailThreshold.RawDataTest_High_Min[iRow][iCol];
                RawDataMax = g_stCfg_MCap_DetailThreshold.RawDataTest_High_Max[iRow][iCol];
                iValue = m_RawData[iRow][iCol];
                if (iValue < RawDataMin || iValue > RawDataMax) {
                    btmpresult = false;
                    FTS_TEST_ERROR("rawdata test failure. Node=(%d,  %d), Get_value=%d,  Set_Range=(%d, %d) ", iRow + 1, iCol + 1, iValue, RawDataMin, RawDataMax);
                }
            }
        }

        //////////////////////////////Save Test Data
        Save_Test_Data(m_RawData, 0, g_ScreenSetParam.iTxNum, g_ScreenSetParam.iRxNum, 2);
    }
    else {
        if (OriginValue != 0)   //if original value is not the value needed,write the register to change
        {
            ReCode = WriteReg(REG_NORMALIZE_TYPE, 0x00);
            if (ReCode != ERROR_CODE_OK) {
                FTS_TEST_ERROR("\n write REG_NORMALIZE_TYPE error. Error Code: %d", ReCode);
                goto TEST_ERR;
            }
        }

        ReCode = ReadReg(0x0A, &ucFre);
        if (ReCode != ERROR_CODE_OK) {
            FTS_TEST_ERROR("\n Read frequency error. Error Code: %d", ReCode);
            goto TEST_ERR;
        }

        //Set Frequecy Low
        if (g_stCfg_FT5X46_BasicThreshold.RawDataTest_SetLowFreq) {
            FTS_TEST_DBG("\n=========Set Frequecy Low");
            ReCode = WriteReg(0x0A, 0x80);
            if (ReCode != ERROR_CODE_OK) {
                FTS_TEST_ERROR("\n write frequency error. Error Code: %d", ReCode);
                goto TEST_ERR;
            }

            //FIR OFF  0:close, 1:open

            FTS_TEST_DBG("\n=========FIR State: OFF\n");
            ReCode = WriteReg(0xFB, 0);
            if (ReCode != ERROR_CODE_OK) {
                FTS_TEST_ERROR("\n FIR State: OFF error. Error Code: %d", ReCode);
                goto TEST_ERR;
            }
            SysDelay(100);
            //change register value before,need to lose 3 frame data
            /*
               for (index = 0; index < 3; ++index )
               {
               ReCode = GetRawData();
               }
             */
            for (index = 0, nRawDataOK = 0; index < 10 && nRawDataOK < 3; ++index) {
                ReCode = GetRawData();
                if (ReCode != ERROR_CODE_OK) {
                    FTS_TEST_ERROR("Get Rawdata failed, index:%d. Error Code: 0x%x", index, ReCode);
                }
                else
                    nRawDataOK++;
            }

            if (ReCode != ERROR_CODE_OK) {
                FTS_TEST_ERROR("Get Rawdata failed, Error Code: 0x%x", ReCode);
                goto TEST_ERR;
            }
            ShowRawData();

            ////////////////////////////////To Determine RawData if in Range or not
            for (iRow = 0; iRow < g_ScreenSetParam.iTxNum; iRow++) {

                for (iCol = 0; iCol < g_ScreenSetParam.iRxNum; iCol++) {
                    if (g_stCfg_MCap_DetailThreshold.InvalidNode[iRow][iCol] == 0)
                        continue;   //Invalid Node
                    RawDataMin = g_stCfg_MCap_DetailThreshold.RawDataTest_Low_Min[iRow][iCol];
                    RawDataMax = g_stCfg_MCap_DetailThreshold.RawDataTest_Low_Max[iRow][iCol];
                    iValue = m_RawData[iRow][iCol];
                    if (iValue < RawDataMin || iValue > RawDataMax) {
                        btmpresult = false;
                        FTS_TEST_ERROR("rawdata test failure. Node=(%d,  %d), Get_value=%d,  Set_Range=(%d, %d) ", iRow + 1, iCol + 1, iValue, RawDataMin, RawDataMax);
                    }
                }
            }

            //////////////////////////////Save Test Data
            Save_Test_Data(m_RawData, 0, g_ScreenSetParam.iTxNum, g_ScreenSetParam.iRxNum, 1);
        }

        //Set Frequecy High
        if (g_stCfg_FT5X46_BasicThreshold.RawDataTest_SetHighFreq) {

            FTS_TEST_DBG("\n=========Set Frequecy High");
            ReCode = WriteReg(0x0A, 0x81);
            if (ReCode != ERROR_CODE_OK) {
                FTS_TEST_ERROR("\n Set Frequecy High error. Error Code: %d", ReCode);
                goto TEST_ERR;
            }

            //FIR OFF  0:close, 1:open

            FTS_TEST_DBG("\n=========FIR State: OFF\n");
            ReCode = WriteReg(0xFB, 0);
            if (ReCode != ERROR_CODE_OK) {
                FTS_TEST_ERROR("\n FIR State: OFF error. Error Code: %d", ReCode);
                goto TEST_ERR;
            }
            SysDelay(100);
            //change register value before,need to lose 3 frame data
            for (index = 0; index < 3; ++index) {
                ReCode = GetRawData();
            }

            if (ReCode != ERROR_CODE_OK) {
                FTS_TEST_ERROR("\nGet Rawdata failed, Error Code: 0x%x", ReCode);
                if (ReCode != ERROR_CODE_OK)
                    goto TEST_ERR;
            }
            ShowRawData();

            ////////////////////////////////To Determine RawData if in Range or not
            for (iRow = 0; iRow < g_ScreenSetParam.iTxNum; iRow++) {

                for (iCol = 0; iCol < g_ScreenSetParam.iRxNum; iCol++) {
                    if (g_stCfg_MCap_DetailThreshold.InvalidNode[iRow][iCol] == 0)
                        continue;   //Invalid Node
                    RawDataMin = g_stCfg_MCap_DetailThreshold.RawDataTest_High_Min[iRow][iCol];
                    RawDataMax = g_stCfg_MCap_DetailThreshold.RawDataTest_High_Max[iRow][iCol];
                    iValue = m_RawData[iRow][iCol];
                    if (iValue < RawDataMin || iValue > RawDataMax) {
                        btmpresult = false;
                        FTS_TEST_ERROR("rawdata test failure. Node=(%d,  %d), Get_value=%d,  Set_Range=(%d, %d) ", iRow + 1, iCol + 1, iValue, RawDataMin, RawDataMax);
                    }
                }
            }

            //////////////////////////////Save Test Data
            Save_Test_Data(m_RawData, 0, g_ScreenSetParam.iTxNum, g_ScreenSetParam.iRxNum, 2);
        }

    }

    ReCode = WriteReg(REG_NORMALIZE_TYPE, OriginValue); //set the origin value
    if (ReCode != ERROR_CODE_OK) {
        FTS_TEST_ERROR("\n Write REG_NORMALIZE_TYPE error. Error Code: %d", ReCode);
        goto TEST_ERR;
    }

    //set V3 TP the origin mapping value
    if (m_bV3TP) {
        ReCode = WriteReg(REG_MAPPING_SWITCH, strSwitch);
        if (ReCode != ERROR_CODE_OK) {
            FTS_TEST_ERROR("\n Write REG_MAPPING_SWITCH error. Error Code: %d", ReCode);
            goto TEST_ERR;
        }
    }

    TestResultLen += sprintf(TestResult + TestResultLen, " RawData Test is %s. \n\n", (btmpresult ? "OK" : "NG"));

    //-------------------------Result
    if (btmpresult) {
        *bTestResult = true;
        FTS_TEST_INFO("\n\n//RawData Test is OK!");
    }
    else {
        *bTestResult = false;
        FTS_TEST_INFO("\n\n//RawData Test is NG!");
    }
    return ReCode;

TEST_ERR:

    *bTestResult = false;
    FTS_TEST_INFO("\n\n//RawData Test is NG!");
    TestResultLen += sprintf(TestResult + TestResultLen, " RawData Test is NG. \n\n");
    return ReCode;

}

/************************************************************************
* Name: FT5X46_TestItem_SCapRawDataTest
* Brief:  TestItem: SCapRawDataTest. Check if SCAP RawData is within the range.
* Input: none
* Output: bTestResult, PASS or FAIL
* Return: Comm Code. Code = 0x00 is OK, else fail.
***********************************************************************/
unsigned char FT5X46_TestItem_SCapRawDataTest(bool * bTestResult)
{
    int i = 0;
    int RawDataMin = 0;
    int RawDataMax = 0;
    int Value = 0;
    boolean bFlag = true;
    unsigned char ReCode = 0;
    boolean btmpresult = true;
    int iMax = 0;
    int iMin = 0;
    int iAvg = 0;
    int ByteNum = 0;
    unsigned char wc_value = 0; //waterproof channel value
    unsigned char ucValue = 0;
    int iCount = 0;
//    int ibiggerValue = 0;

    FTS_TEST_INFO("\n\n==============================Test Item: -------- Scap RawData Test \n");
    //-------1.Preparatory work
    //in Factory Mode
    ReCode = EnterFactory();
    if (ReCode != ERROR_CODE_OK) {
        FTS_TEST_ERROR("\n\n// Failed to Enter factory Mode. Error Code: %d", ReCode);
        goto TEST_ERR;
    }

    //get waterproof channel setting, to check if Tx/Rx channel need to test
    ReCode = ReadReg(REG_WATER_CHANNEL_SELECT, &wc_value);
    if (ReCode != ERROR_CODE_OK) {
        FTS_TEST_ERROR("\n\n// Failed to read REG_WATER_CHANNEL_SELECT. Error Code: %d", ReCode);
        goto TEST_ERR;
    }

    //If it is V3 pattern, Get Tx/Rx Num again
    ReCode = SwitchToNoMapping();
    if (ReCode != ERROR_CODE_OK) {
        FTS_TEST_ERROR("\n\n// Failed to SwitchToNoMapping. Error Code: %d", ReCode);
        goto TEST_ERR;
    }

    //-------2.Get SCap Raw Data, Step:1.Start Scanning; 2. Read Raw Data
    ReCode = StartScan();
    if (ReCode != ERROR_CODE_OK) {
        FTS_TEST_ERROR("Failed to Scan SCap RawData! ");
        goto TEST_ERR;
    }
    for (i = 0; i < 3; i++) {
        memset(m_iTempRawData, 0, sizeof(m_iTempRawData));

        //water rawdata
        ByteNum = (g_ScreenSetParam.iTxNum + g_ScreenSetParam.iRxNum) * 2;
        ReCode = ReadRawData(0, 0xAC, ByteNum, m_iTempRawData);
        if (ReCode != ERROR_CODE_OK) {
            FTS_TEST_DBG("Failed to ReadRawData water! ");
            goto TEST_ERR;
        }

        memcpy(m_RawData[0 + g_ScreenSetParam.iTxNum], m_iTempRawData, sizeof(int) * g_ScreenSetParam.iRxNum);
        memcpy(m_RawData[1 + g_ScreenSetParam.iTxNum], m_iTempRawData + g_ScreenSetParam.iRxNum, sizeof(int) * g_ScreenSetParam.iTxNum);

        //No water rawdata
        ByteNum = (g_ScreenSetParam.iTxNum + g_ScreenSetParam.iRxNum) * 2;
        ReCode = ReadRawData(0, 0xAB, ByteNum, m_iTempRawData);
        if (ReCode != ERROR_CODE_OK) {
            FTS_TEST_ERROR("Failed to ReadRawData no water! ");
            goto TEST_ERR;
        }
        memcpy(m_RawData[2 + g_ScreenSetParam.iTxNum], m_iTempRawData, sizeof(int) * g_ScreenSetParam.iRxNum);
        memcpy(m_RawData[3 + g_ScreenSetParam.iTxNum], m_iTempRawData + g_ScreenSetParam.iRxNum, sizeof(int) * g_ScreenSetParam.iTxNum);
    }

    //-----3. Judge

    //Waterproof ON
    bFlag = GetTestCondition(WT_NeedProofOnTest, wc_value);
    if (g_stCfg_FT5X46_BasicThreshold.SCapRawDataTest_SetWaterproof_ON && bFlag) {
        iCount = 0;
        RawDataMin = g_stCfg_FT5X46_BasicThreshold.SCapRawDataTest_ON_Min;
        RawDataMax = g_stCfg_FT5X46_BasicThreshold.SCapRawDataTest_ON_Max;
        iMax = -m_RawData[0 + g_ScreenSetParam.iTxNum][0];
        iMin = 2 * m_RawData[0 + g_ScreenSetParam.iTxNum][0];
        iAvg = 0;
        Value = 0;

        bFlag = GetTestCondition(WT_NeedRxOnVal, wc_value);
        if (bFlag)
            FTS_TEST_DBG("Judge Rx in Waterproof-ON:");
        for (i = 0; bFlag && i < g_ScreenSetParam.iRxNum; i++) {
            if (g_stCfg_MCap_DetailThreshold.InvalidNode_SC[0][i] == 0)
                continue;
            RawDataMin = g_stCfg_MCap_DetailThreshold.SCapRawDataTest_ON_Min[0][i];
            RawDataMax = g_stCfg_MCap_DetailThreshold.SCapRawDataTest_ON_Max[0][i];
            Value = m_RawData[0 + g_ScreenSetParam.iTxNum][i];
            iAvg += Value;
            if (iMax < Value)
                iMax = Value;   //find the Max value
            if (iMin > Value)
                iMin = Value;   //fine the min value
            if (Value > RawDataMax || Value < RawDataMin) {
                btmpresult = false;
                FTS_TEST_ERROR("Failed. Num = %d, Value = %d, range = (%d, %d):", i + 1, Value, RawDataMin, RawDataMax);
            }
            iCount++;
        }

        bFlag = GetTestCondition(WT_NeedTxOnVal, wc_value);
        if (bFlag)
            FTS_TEST_DBG("Judge Tx in Waterproof-ON:");
        for (i = 0; bFlag && i < g_ScreenSetParam.iTxNum; i++) {
            if (g_stCfg_MCap_DetailThreshold.InvalidNode_SC[1][i] == 0)
                continue;
            RawDataMin = g_stCfg_MCap_DetailThreshold.SCapRawDataTest_ON_Min[1][i];
            RawDataMax = g_stCfg_MCap_DetailThreshold.SCapRawDataTest_ON_Max[1][i];
            Value = m_RawData[1 + g_ScreenSetParam.iTxNum][i];
            iAvg += Value;
            if (iMax < Value)
                iMax = Value;   //find the Max value
            if (iMin > Value)
                iMin = Value;   //fine the min value
            if (Value > RawDataMax || Value < RawDataMin) {
                btmpresult = false;
                FTS_TEST_ERROR("Failed. Num = %d, Value = %d, range = (%d, %d):", i + 1, Value, RawDataMin, RawDataMax);
            }
            iCount++;
        }
        if (0 == iCount) {
            iAvg = 0;
            iMax = 0;
            iMin = 0;
        }
        else
            iAvg = iAvg / iCount;

        FTS_TEST_DBG("SCap RawData in Waterproof-ON, Max : %d, Min: %d, Deviation: %d, Average: %d", iMax, iMin, iMax - iMin, iAvg);
        //////////////////////////////Save Test Data
//       ibiggerValue = g_ScreenSetParam.iTxNum>g_ScreenSetParam.iRxNum?g_ScreenSetParam.iTxNum:g_ScreenSetParam.iRxNum;
        Save_Test_Data(m_RawData, g_ScreenSetParam.iTxNum + 0, 2, g_ScreenSetParam.iRxNum, 1);
    }

    //Waterproof OFF
    bFlag = GetTestCondition(WT_NeedProofOffTest, wc_value);
    if (g_stCfg_FT5X46_BasicThreshold.SCapRawDataTest_SetWaterproof_OFF && bFlag) {
        iCount = 0;
        RawDataMin = g_stCfg_FT5X46_BasicThreshold.SCapRawDataTest_OFF_Min;
        RawDataMax = g_stCfg_FT5X46_BasicThreshold.SCapRawDataTest_OFF_Max;
        iMax = -m_RawData[2 + g_ScreenSetParam.iTxNum][0];
        iMin = 2 * m_RawData[2 + g_ScreenSetParam.iTxNum][0];
        iAvg = 0;
        Value = 0;

        bFlag = GetTestCondition(WT_NeedRxOffVal, wc_value);
        if (bFlag)
            FTS_TEST_DBG("Judge Rx in Waterproof-OFF:");
        for (i = 0; bFlag && i < g_ScreenSetParam.iRxNum; i++) {
            if (g_stCfg_MCap_DetailThreshold.InvalidNode_SC[0][i] == 0)
                continue;
            RawDataMin = g_stCfg_MCap_DetailThreshold.SCapRawDataTest_OFF_Min[0][i];
            RawDataMax = g_stCfg_MCap_DetailThreshold.SCapRawDataTest_OFF_Max[0][i];
            Value = m_RawData[2 + g_ScreenSetParam.iTxNum][i];
            iAvg += Value;

            //FTS_TEST_DBG("zaxzax3 Value %d RawDataMin %d  RawDataMax %d  ",  Value, RawDataMin, RawDataMax);
            //strTemp += str;
            if (iMax < Value)
                iMax = Value;
            if (iMin > Value)
                iMin = Value;
            if (Value > RawDataMax || Value < RawDataMin) {
                btmpresult = false;
                FTS_TEST_ERROR("Failed. Num = %d, Value = %d, range = (%d, %d):", i + 1, Value, RawDataMin, RawDataMax);
            }
            iCount++;
        }

        bFlag = GetTestCondition(WT_NeedTxOffVal, wc_value);
        if (bFlag)
            FTS_TEST_DBG("Judge Tx in Waterproof-OFF:");
        for (i = 0; bFlag && i < g_ScreenSetParam.iTxNum; i++) {
            if (g_stCfg_MCap_DetailThreshold.InvalidNode_SC[1][i] == 0)
                continue;

            Value = m_RawData[3 + g_ScreenSetParam.iTxNum][i];
            RawDataMin = g_stCfg_MCap_DetailThreshold.SCapRawDataTest_OFF_Min[1][i];
            RawDataMax = g_stCfg_MCap_DetailThreshold.SCapRawDataTest_OFF_Max[1][i];
            //FTS_TEST_DBG("zaxzax4 Value %d RawDataMin %d  RawDataMax %d  ",  Value, RawDataMin, RawDataMax);
            iAvg += Value;
            if (iMax < Value)
                iMax = Value;
            if (iMin > Value)
                iMin = Value;
            if (Value > RawDataMax || Value < RawDataMin) {
                btmpresult = false;
                FTS_TEST_ERROR("Failed. Num = %d, Value = %d, range = (%d, %d):", i + 1, Value, RawDataMin, RawDataMax);
            }
            iCount++;
        }
        if (0 == iCount) {
            iAvg = 0;
            iMax = 0;
            iMin = 0;
        }
        else
            iAvg = iAvg / iCount;

        FTS_TEST_DBG("SCap RawData in Waterproof-OFF, Max : %d, Min: %d, Deviation: %d, Average: %d", iMax, iMin, iMax - iMin, iAvg);
        //////////////////////////////Save Test Data
//       ibiggerValue = g_ScreenSetParam.iTxNum>g_ScreenSetParam.iRxNum?g_ScreenSetParam.iTxNum:g_ScreenSetParam.iRxNum;
        Save_Test_Data(m_RawData, g_ScreenSetParam.iTxNum + 2, 2, g_ScreenSetParam.iRxNum, 2);
    }
    //-----4. post-stage work
    if (m_bV3TP) {
        ReCode = ReadReg(REG_MAPPING_SWITCH, &ucValue);
        if (ReCode != ERROR_CODE_OK) {
            FTS_TEST_ERROR("\n Read REG_MAPPING_SWITCH error. Error Code: %d", ReCode);
            goto TEST_ERR;
        }

        if (0 != ucValue) {
            ReCode = WriteReg(REG_MAPPING_SWITCH, 0);
            SysDelay(10);
            if (ReCode != ERROR_CODE_OK) {
                FTS_TEST_ERROR("Failed to switch mapping type!\n ");
                btmpresult = false;
            }
        }

        //Only self content will be used before the Mapping, so the end of the test items, need to go after Mapping
        ReCode = GetChannelNum();
        if (ReCode != ERROR_CODE_OK) {
            FTS_TEST_ERROR("\n GetChannelNum error. Error Code: %d", ReCode);
            goto TEST_ERR;
        }
    }

    TestResultLen += sprintf(TestResult + TestResultLen, " SCap RawData Test is %s. \n\n", (btmpresult ? "OK" : "NG"));

    //-----5. Test Result
    if (btmpresult) {
        *bTestResult = true;
        FTS_TEST_INFO("\n\n//SCap RawData Test is OK!");
    }
    else {
        *bTestResult = false;
        FTS_TEST_INFO("\n\n//SCap RawData Test is NG!");
    }
    return ReCode;

TEST_ERR:
    *bTestResult = false;
    FTS_TEST_INFO("\n\n//SCap RawData Test is NG!");
    TestResultLen += sprintf(TestResult + TestResultLen, " SCap RawData Test is NG. \n\n");
    return ReCode;
}

/************************************************************************
* Name: FT5X46_TestItem_SCapCbTest
* Brief:  TestItem: SCapCbTest. Check if SCAP Cb is within the range.
* Input: none
* Output: bTestResult, PASS or FAIL
* Return: Comm Code. Code = 0x00 is OK, else fail.
***********************************************************************/
unsigned char FT5X46_TestItem_SCapCbTest(bool * bTestResult)
{
    int i, /* j, iOutNum, */ index, Value, CBMin, CBMax;
    boolean bFlag = true;
    unsigned char ReCode;
    boolean btmpresult = true;
    int iMax, iMin, iAvg;
    unsigned char wc_value = 0;
    unsigned char ucValue = 0;
    int iCount = 0;
//   int ibiggerValue = 0;

    FTS_TEST_INFO("\n\n==============================Test Item: -----  Scap CB Test \n");
    //-------1.Preparatory work
    //in Factory Mode
    ReCode = EnterFactory();
    if (ReCode != ERROR_CODE_OK) {
        FTS_TEST_ERROR("\n\n// Failed to Enter factory Mode. Error Code: %d", ReCode);
        goto TEST_ERR;
    }

    //get waterproof channel setting, to check if Tx/Rx channel need to test
    ReCode = ReadReg(REG_WATER_CHANNEL_SELECT, &wc_value);
    if (ReCode != ERROR_CODE_OK) {
        FTS_TEST_ERROR("\n Read REG_WATER_CHANNEL_SELECT error. Error Code: %d", ReCode);
        goto TEST_ERR;
    }

    //If it is V3 pattern, Get Tx/Rx Num again
    bFlag = SwitchToNoMapping();
    if (bFlag) {
        FTS_TEST_ERROR("Failed to SwitchToNoMapping! ReCode = %d. ", ReCode);
        goto TEST_ERR;
    }

    //-------2.Get SCap Raw Data, Step:1.Start Scanning; 2. Read Raw Data
    ReCode = StartScan();
    if (ReCode != ERROR_CODE_OK) {
        FTS_TEST_ERROR("Failed to Scan SCap RawData!ReCode = %d. ", ReCode);
        goto TEST_ERR;
    }

    for (i = 0; i < 3; i++) {
        memset(m_RawData, 0, sizeof(m_RawData));
        memset(m_ucTempData, 0, sizeof(m_ucTempData));

        //waterproof CB
        ReCode = WriteReg(REG_ScWorkMode, 1);   //ScWorkMode:  1:waterproof 0:Non-waterproof
        if (ReCode != ERROR_CODE_OK) {
            FTS_TEST_ERROR("Get REG_ScWorkMode Failed!");
            goto TEST_ERR;
        }

        ReCode = StartScan();
        if (ReCode != ERROR_CODE_OK) {
            FTS_TEST_ERROR("StartScan Failed!");
            goto TEST_ERR;
        }

        ReCode = WriteReg(REG_ScCbAddrR, 0);
        if (ReCode != ERROR_CODE_OK) {
            FTS_TEST_ERROR("Write REG_ScCbAddrR Failed!");
            goto TEST_ERR;
        }

        ReCode = GetTxSC_CB(g_ScreenSetParam.iTxNum + g_ScreenSetParam.iRxNum + 128, m_ucTempData);
        if (ReCode != ERROR_CODE_OK) {
            FTS_TEST_ERROR("GetTxSC_CB Failed!");
            goto TEST_ERR;
        }

        for (index = 0; index < g_ScreenSetParam.iRxNum; ++index) {
            m_RawData[0 + g_ScreenSetParam.iTxNum][index] = m_ucTempData[index];
        }
        for (index = 0; index < g_ScreenSetParam.iTxNum; ++index) {
            m_RawData[1 + g_ScreenSetParam.iTxNum][index] = m_ucTempData[index + g_ScreenSetParam.iRxNum];
        }

        //Non-waterproof rawdata
        ReCode = WriteReg(REG_ScWorkMode, 0);   //ScWorkMode:  1:waterproof 0:Non-waterproof
        if (ReCode != ERROR_CODE_OK) {
            FTS_TEST_ERROR("Get REG_ScWorkMode Failed!");
            goto TEST_ERR;
        }

        ReCode = StartScan();
        if (ReCode != ERROR_CODE_OK) {
            FTS_TEST_ERROR("StartScan Failed!");
            goto TEST_ERR;
        }

        ReCode = WriteReg(REG_ScCbAddrR, 0);
        if (ReCode != ERROR_CODE_OK) {
            FTS_TEST_ERROR("Write REG_ScCbAddrR Failed!");
            goto TEST_ERR;
        }

        ReCode = GetTxSC_CB(g_ScreenSetParam.iTxNum + g_ScreenSetParam.iRxNum + 128, m_ucTempData);
        if (ReCode != ERROR_CODE_OK) {
            FTS_TEST_ERROR("GetTxSC_CB Failed!");
            goto TEST_ERR;
        }
        for (index = 0; index < g_ScreenSetParam.iRxNum; ++index) {
            m_RawData[2 + g_ScreenSetParam.iTxNum][index] = m_ucTempData[index];
        }
        for (index = 0; index < g_ScreenSetParam.iTxNum; ++index) {
            m_RawData[3 + g_ScreenSetParam.iTxNum][index] = m_ucTempData[index + g_ScreenSetParam.iRxNum];
        }

        if (ReCode != ERROR_CODE_OK) {
            FTS_TEST_ERROR("Failed to Get SCap CB!");
        }
    }

    if (ReCode != ERROR_CODE_OK)
        goto TEST_ERR;

    //-----3. Judge

    //Waterproof ON
    bFlag = GetTestCondition(WT_NeedProofOnTest, wc_value);
    if (g_stCfg_FT5X46_BasicThreshold.SCapCbTest_SetWaterproof_ON && bFlag) {
        FTS_TEST_DBG("SCapCbTest in WaterProof On Mode:  ");

        iMax = -m_RawData[0 + g_ScreenSetParam.iTxNum][0];
        iMin = 2 * m_RawData[0 + g_ScreenSetParam.iTxNum][0];
        iAvg = 0;
        Value = 0;
        iCount = 0;

        bFlag = GetTestCondition(WT_NeedRxOnVal, wc_value);
        if (bFlag)
            FTS_TEST_DBG("SCap CB_Rx:  ");
        for (i = 0; bFlag && i < g_ScreenSetParam.iRxNum; i++) {
            if (g_stCfg_MCap_DetailThreshold.InvalidNode_SC[0][i] == 0)
                continue;
            CBMin = g_stCfg_MCap_DetailThreshold.SCapCbTest_ON_Min[0][i];
            CBMax = g_stCfg_MCap_DetailThreshold.SCapCbTest_ON_Max[0][i];
            Value = m_RawData[0 + g_ScreenSetParam.iTxNum][i];
            iAvg += Value;

            if (iMax < Value)
                iMax = Value;   //find the Max Value
            if (iMin > Value)
                iMin = Value;   //find the Min Value
            if (Value > CBMax || Value < CBMin) {
                btmpresult = false;
                FTS_TEST_ERROR("Failed. Num = %d, Value = %d, range = (%d, %d):", i + 1, Value, CBMin, CBMax);
            }
            iCount++;
        }

        bFlag = GetTestCondition(WT_NeedTxOnVal, wc_value);
        if (bFlag)
            FTS_TEST_DBG("SCap CB_Tx:  ");
        for (i = 0; bFlag && i < g_ScreenSetParam.iTxNum; i++) {
            if (g_stCfg_MCap_DetailThreshold.InvalidNode_SC[1][i] == 0)
                continue;
            CBMin = g_stCfg_MCap_DetailThreshold.SCapCbTest_ON_Min[1][i];
            CBMax = g_stCfg_MCap_DetailThreshold.SCapCbTest_ON_Max[1][i];
            Value = m_RawData[1 + g_ScreenSetParam.iTxNum][i];
            iAvg += Value;
            if (iMax < Value)
                iMax = Value;
            if (iMin > Value)
                iMin = Value;
            if (Value > CBMax || Value < CBMin) {
                btmpresult = false;
                FTS_TEST_ERROR("Failed. Num = %d, Value = %d, range = (%d, %d):", i + 1, Value, CBMin, CBMax);
            }
            iCount++;
        }

        if (0 == iCount) {
            iAvg = 0;
            iMax = 0;
            iMin = 0;
        }
        else
            iAvg = iAvg / iCount;

        FTS_TEST_DBG("SCap CB in Waterproof-ON, Max : %d, Min: %d, Deviation: %d, Average: %d", iMax, iMin, iMax - iMin, iAvg);
        //////////////////////////////Save Test Data
//       ibiggerValue = g_ScreenSetParam.iTxNum>g_ScreenSetParam.iRxNum?g_ScreenSetParam.iTxNum:g_ScreenSetParam.iRxNum;
        Save_Test_Data(m_RawData, g_ScreenSetParam.iTxNum + 0, 2, g_ScreenSetParam.iRxNum, 1);
    }

    bFlag = GetTestCondition(WT_NeedProofOffTest, wc_value);
    if (g_stCfg_FT5X46_BasicThreshold.SCapCbTest_SetWaterproof_OFF && bFlag) {
        FTS_TEST_DBG("SCapCbTest in WaterProof OFF Mode:  ");
        iMax = -m_RawData[2 + g_ScreenSetParam.iTxNum][0];
        iMin = 2 * m_RawData[2 + g_ScreenSetParam.iTxNum][0];
        iAvg = 0;
        Value = 0;
        iCount = 0;

        bFlag = GetTestCondition(WT_NeedRxOffVal, wc_value);
        if (bFlag)
            FTS_TEST_DBG("SCap CB_Rx:  ");
        for (i = 0; bFlag && i < g_ScreenSetParam.iRxNum; i++) {
            if (g_stCfg_MCap_DetailThreshold.InvalidNode_SC[0][i] == 0)
                continue;
            CBMin = g_stCfg_MCap_DetailThreshold.SCapCbTest_OFF_Min[0][i];
            CBMax = g_stCfg_MCap_DetailThreshold.SCapCbTest_OFF_Max[0][i];
            Value = m_RawData[2 + g_ScreenSetParam.iTxNum][i];
            iAvg += Value;

            if (iMax < Value)
                iMax = Value;
            if (iMin > Value)
                iMin = Value;
            if (Value > CBMax || Value < CBMin) {
                btmpresult = false;
                FTS_TEST_ERROR("Failed. Num = %d, Value = %d, range = (%d, %d):", i + 1, Value, CBMin, CBMax);
            }
            iCount++;
        }

        bFlag = GetTestCondition(WT_NeedTxOffVal, wc_value);
        if (bFlag)
            FTS_TEST_DBG("SCap CB_Tx:  ");
        for (i = 0; bFlag && i < g_ScreenSetParam.iTxNum; i++) {
            //if( m_ScapInvalide[1][i] == 0 )      continue;
            if (g_stCfg_MCap_DetailThreshold.InvalidNode_SC[1][i] == 0)
                continue;
            CBMin = g_stCfg_MCap_DetailThreshold.SCapCbTest_OFF_Min[1][i];
            CBMax = g_stCfg_MCap_DetailThreshold.SCapCbTest_OFF_Max[1][i];
            Value = m_RawData[3 + g_ScreenSetParam.iTxNum][i];

            iAvg += Value;
            if (iMax < Value)
                iMax = Value;
            if (iMin > Value)
                iMin = Value;
            if (Value > CBMax || Value < CBMin) {
                btmpresult = false;
                FTS_TEST_ERROR("Failed. Num = %d, Value = %d, range = (%d, %d):", i + 1, Value, CBMin, CBMax);
            }
            iCount++;
        }

        if (0 == iCount) {
            iAvg = 0;
            iMax = 0;
            iMin = 0;
        }
        else
            iAvg = iAvg / iCount;

        FTS_TEST_DBG("SCap CB in Waterproof-OFF, Max : %d, Min: %d, Deviation: %d, Average: %d", iMax, iMin, iMax - iMin, iAvg);
        //////////////////////////////Save Test Data
        //     ibiggerValue = g_ScreenSetParam.iTxNum>g_ScreenSetParam.iRxNum?g_ScreenSetParam.iTxNum:g_ScreenSetParam.iRxNum;
        Save_Test_Data(m_RawData, g_ScreenSetParam.iTxNum + 2, 2, g_ScreenSetParam.iRxNum, 2);
    }
    //-----4. post-stage work
    if (m_bV3TP) {
        ReCode = ReadReg(REG_MAPPING_SWITCH, &ucValue);
        if (ReCode != ERROR_CODE_OK) {
            FTS_TEST_ERROR("\n Read REG_MAPPING_SWITCH error. Error Code: %d", ReCode);
            goto TEST_ERR;
        }

        if (0 != ucValue) {
            ReCode = WriteReg(REG_MAPPING_SWITCH, 0);
            SysDelay(10);
            if (ReCode != ERROR_CODE_OK) {
                FTS_TEST_DBG("Failed to switch mapping type!\n ");
                btmpresult = false;
            }
        }

        //Only self content will be used before the Mapping, so the end of the test items, need to go after Mapping
        ReCode = GetChannelNum();
        if (ReCode != ERROR_CODE_OK) {
            FTS_TEST_ERROR("\n GetChannelNum error. Error Code: %d", ReCode);
            goto TEST_ERR;
        }
    }

    TestResultLen += sprintf(TestResult + TestResultLen, " SCap CB Test is %s. \n\n", (btmpresult ? "OK" : "NG"));

    //-----5. Test Result

    if (btmpresult) {
        *bTestResult = true;
        FTS_TEST_INFO("\n\n//SCap CB Test Test is OK!");
    }
    else {
        *bTestResult = false;
        FTS_TEST_INFO("\n\n//SCap CB Test Test is NG!");
    }
    return ReCode;

TEST_ERR:

    *bTestResult = false;
    FTS_TEST_INFO("\n\n//SCap CB Test Test is NG!");
    TestResultLen += sprintf(TestResult + TestResultLen, " SCap CB Test is NG. \n\n");
    return ReCode;
}

/************************************************************************
* Name: FT5X46_TestItem_PanelDifferTest
* Brief:  TestItem: PanelDifferTest. Check if Panel Differ is within the range.
* Input: none
* Output: bTestResult, PASS or FAIL
* Return: Comm Code. Code = 0x00 is OK, else fail.
***********************************************************************/
unsigned char FT5X46_TestItem_PanelDifferTest(bool * bTestResult)
{
    int index = 0;
    int iRow = 0, iCol = 0;
    int iValue = 0;
    unsigned char ReCode = 0, strSwitch = -1;
    bool btmpresult = true;
    int iMax, iMin;             //, iAvg;
    int maxValue = 0;
    int minValue = 32767;
    int AvgValue = 0;
    int InvalidNum = 0;
    int i = 0, j = 0;

    unsigned char OriginRawDataType = 0xff;
    unsigned char OriginFrequecy = 0xff;
    unsigned char OriginFirState = 0xff;

    FTS_TEST_INFO("\r\n\r\n\r\n==============================Test Item: -------- Panel Differ Test  \r\n\r\n");

    ReCode = EnterFactory();
    SysDelay(20);
    if (ReCode != ERROR_CODE_OK) {
        FTS_TEST_ERROR("\n\n// Failed to Enter factory Mode. Error Code: %d", ReCode);
        goto TEST_ERR;
    }

    //Determine whether for v3 TP first, and then read the value of the 0x54
    //and check the mapping type is right or not,if not, write the register
    //rawdata test mapping��?before mapping��o0x54=1;after mapping��o0x54=0;
    if (m_bV3TP) {
        ReCode = ReadReg(REG_MAPPING_SWITCH, &strSwitch);
        if (ReCode != ERROR_CODE_OK) {
            FTS_TEST_ERROR("\n Read REG_MAPPING_SWITCH error. Error Code: %d", ReCode);
            goto TEST_ERR;
        }

        if (strSwitch != 0) {
            ReCode = WriteReg(REG_MAPPING_SWITCH, 0);
            if (ReCode != ERROR_CODE_OK) {
                FTS_TEST_ERROR("\n Write REG_MAPPING_SWITCH error. Error Code: %d", ReCode);
                goto TEST_ERR;
            }
            //  MUST get channel number again after write reg mapping switch.
            ReCode = GetChannelNum();
            if (ReCode != ERROR_CODE_OK) {
                FTS_TEST_ERROR("\n GetChannelNum error. Error Code: %d", ReCode);
                goto TEST_ERR;
            }

            ReCode = GetRawData();
        }
    }

    ///////////
    FTS_TEST_DBG("\r\n=========Set Auto Equalization:\r\n");
    ReCode = ReadReg(REG_NORMALIZE_TYPE, &OriginRawDataType);   //Read the original value
    if (ReCode != ERROR_CODE_OK) {
        FTS_TEST_ERROR("Read  REG_NORMALIZE_TYPE error. Error Code: %d", ReCode);
        btmpresult = false;
        goto TEST_ERR;
    }

    if (OriginRawDataType != 0) {
        ReCode = WriteReg(REG_NORMALIZE_TYPE, 0x00);
        SysDelay(50);
        if (ReCode != ERROR_CODE_OK) {
            btmpresult = false;
            FTS_TEST_ERROR("Write reg REG_NORMALIZE_TYPE Failed.");
            goto TEST_ERR;
        }
    }
    ///////////

    //Set Frequecy High
    FTS_TEST_DBG("=========Set Frequecy High");
    ReCode = ReadReg(0x0A, &OriginFrequecy);    //Read the original value
    if (ReCode != ERROR_CODE_OK) {
        FTS_TEST_ERROR("Read reg 0x0A error. Error Code: %d", ReCode);
        btmpresult = false;
        goto TEST_ERR;
    }

    ReCode = WriteReg(0x0A, 0x81);
    SysDelay(10);
    if (ReCode != ERROR_CODE_OK) {
        btmpresult = false;
        FTS_TEST_ERROR("Write reg 0x0A Failed.");
        goto TEST_ERR;
    }

    FTS_TEST_DBG("=========FIR State: OFF");
    ReCode = ReadReg(0xFB, &OriginFirState);    //Read the original value
    if (ReCode != ERROR_CODE_OK) {
        FTS_TEST_ERROR("Read reg 0xFB error. Error Code: %d", ReCode);
        btmpresult = false;
        goto TEST_ERR;
    }
    ReCode = WriteReg(0xFB, 0);
    SysDelay(50);
    if (ReCode != ERROR_CODE_OK) {
        FTS_TEST_ERROR("Write reg 0xFB Failed.");
        btmpresult = false;
        goto TEST_ERR;
    }
    ReCode = GetRawData();

    //change register value before,need to lose 3 frame data��?4th frame data is valid
    for (index = 0; index < 4; ++index) {
        ReCode = GetRawData();
        if (ReCode != ERROR_CODE_OK) {
            FTS_TEST_ERROR("GetRawData Failed.");
            btmpresult = false;
            goto TEST_ERR;
        }
    }

    ////Differ = RawData * 1/10
    for (i = 0; i < g_ScreenSetParam.iTxNum; i++) {
        for (j = 0; j < g_ScreenSetParam.iRxNum; j++) {
            m_DifferData[i][j] = m_RawData[i][j] / 10;
        }
    }

    ////////////////////////////////To show value
#if 1
    FTS_TEST_DBG("PannelDiffer :\n");
    for (iRow = 0; iRow < g_ScreenSetParam.iTxNum; iRow++) {
        FTS_TEST_DBG("\nRow%2d:    ", iRow + 1);
        for (iCol = 0; iCol < g_ScreenSetParam.iRxNum; iCol++) {
            //  if(g_stCfg_MCap_DetailThreshold.InvalidNode[iRow][iCol] == 0)continue;//Invalid Node

            iValue = m_DifferData[iRow][iCol];
            FTS_TEST_DBG("%4d,  ", iValue);
        }
        FTS_TEST_DBG("\n");
    }
    FTS_TEST_DBG("\n");
#endif

    ///whether threshold is in range
    ////////////////////////////////To Determine  if in Range or not

    for (iRow = 0; iRow < g_ScreenSetParam.iTxNum; iRow++)  //  iRow = 1
    {
        for (iCol = 0; iCol < g_ScreenSetParam.iRxNum; iCol++) {
            if (g_stCfg_MCap_DetailThreshold.InvalidNode[iRow][iCol] == 0)
                continue;       //Invalid Node

            iValue = m_DifferData[iRow][iCol];
            iMin = g_stCfg_MCap_DetailThreshold.PanelDifferTest_Min[iRow][iCol];
            iMax = g_stCfg_MCap_DetailThreshold.PanelDifferTest_Max[iRow][iCol];
            //FTS_TEST_DBG(" Node=(%d,  %d), Get_value=%d,  Set_Range=(%d, %d) \n", iRow+1, iCol+1, iValue, iMin, iMax);
            if (iValue < iMin || iValue > iMax) {
                btmpresult = false;
                FTS_TEST_ERROR("Out Of Range.  Node=(%d,  %d), Get_value=%d,  Set_Range=(%d, %d) \n", iRow + 1, iCol + 1, iValue, iMin, iMax);
            }
        }
    }
    /////////////////////////// end determine

    ////////////////////
    ////>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>get test data ,and save to .csv file
    FTS_TEST_DBG("PannelDiffer ABS:\n");
    for (i = 0; i < g_ScreenSetParam.iTxNum; i++) {
        //  strTemp += "\r\n";
        FTS_TEST_DBG("\n");
        for (j = 0; j < g_ScreenSetParam.iRxNum; j++) {

            FTS_TEST_DBG("%ld,", abs(m_DifferData[i][j]));
            m_absDifferData[i][j] = abs(m_DifferData[i][j]);

            if (NODE_AST_TYPE == g_stCfg_MCap_DetailThreshold.InvalidNode[i][j] || NODE_INVALID_TYPE == g_stCfg_MCap_DetailThreshold.InvalidNode[i][j]) {
                InvalidNum++;
                continue;
            }
            maxValue = max(maxValue, m_DifferData[i][j]);
            minValue = min(minValue, m_DifferData[i][j]);
            AvgValue += m_DifferData[i][j];
        }
    }
    FTS_TEST_DBG("\n");
    Save_Test_Data(m_absDifferData, 0, g_ScreenSetParam.iTxNum, g_ScreenSetParam.iRxNum, 1);
    ////<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<get test data ,and save to .csv file

    AvgValue = AvgValue / (g_ScreenSetParam.iTxNum * g_ScreenSetParam.iRxNum - InvalidNum);
    FTS_TEST_DBG("PanelDiffer:Max: %d, Min: %d, Avg: %d ", maxValue, minValue, AvgValue);

    ReCode = WriteReg(REG_NORMALIZE_TYPE, OriginRawDataType);   //set to original value
    ReCode = WriteReg(0x0A, OriginFrequecy);    //set to original value
    ReCode = WriteReg(0xFB, OriginFirState);    //set to original value

    //set V3 TP the origin mapping value
    if (m_bV3TP) {
        ReCode = WriteReg(REG_MAPPING_SWITCH, strSwitch);
        if (ReCode != ERROR_CODE_OK) {
            FTS_TEST_ERROR("Failed to restore mapping type!");
            btmpresult = false;
        }
    }
    /////////////
    TestResultLen += sprintf(TestResult + TestResultLen, " Panel Differ Test is %s. \n\n", (btmpresult ? "OK" : "NG"));

    ///////////////////////////-------------------------Result
    if (btmpresult) {
        *bTestResult = true;
        FTS_TEST_INFO("		//Panel Differ Test is OK!");
    }
    else {
        *bTestResult = false;
        FTS_TEST_INFO("		//Panel Differ Test is NG!");
    }
    return ReCode;

TEST_ERR:

    *bTestResult = false;
    FTS_TEST_INFO("		//Panel Differ Test is NG!");
    TestResultLen += sprintf(TestResult + TestResultLen, " Panel Differ Test is NG. \n\n");
    return ReCode;
}

/************************************************************************
* Name: FT5X46_TestItem_WeakShortTest
* Brief:  TestItem: WeakShortTest. Check if Tp is short or not .
* Input: none
* Output: bTestResult, PASS or FAIL
* Return: Comm Code. Code = 0x00 is OK, else fail.
***********************************************************************/

unsigned char FT5X46_TestItem_WeakShortTest(bool * bTestResult)
{

    unsigned char ReCode = ERROR_CODE_COMM_ERROR;
    int i = 0;
    bool btmpresult = true;
    int iAllAdcDataNum = 63;
    int iMaxTx = 35;
    unsigned char iTxNum, iRxNum, iChannelNum;
    int iClbData_Ground, iClbData_Mutual, iOffset, iRsen, iCCRsen;
    unsigned char IcValue = 0;
    unsigned char strSwitch = 1;
    bool bCapShortTest = false;

    int *iAdcData = NULL;

    int fKcal = 0;              //  float fKcal = 0;
    int *fMShortResistance = NULL, *fGShortResistance = NULL;   //  loat *fMShortResistance, *fGShortResistance;
    int iDoffset = 0, iDsen = 0, iDrefn = 0;
    int iMin_CG = 0;
    int iCount = 0;
    int iMin_CC = 0;
    int iDCal = 0;
    int iMa = 0;

    FTS_TEST_INFO("\n\n\n\n==============================Test Item: -----  Weak Short-Circuit Test \r\n\r\n");

    ReCode = EnterWork();       //
    if (ReCode != ERROR_CODE_OK) {
        FTS_TEST_ERROR(" EnterWork failed.. Error Code: %d", ReCode);
        btmpresult = false;
        goto TEST_ERR;
    }
    SysDelay(200);

    //read 0xb1, <=0x05||==0xff:version E     =0x06 || others:version F
    ReCode = ReadReg(0xB1, &IcValue);   //Get IC type
    if (ReCode != ERROR_CODE_OK) {
        FTS_TEST_ERROR("\n Read 0xB1 IcValue error. Error Code: %d\n", ReCode);
        btmpresult = false;
        goto TEST_ERR;
    }
    else
        FTS_TEST_DBG(" IcValue:0x%02x.  \n", IcValue);

    iRsen = 57;
    //New test item only for Weak Short Test
    iCCRsen = g_stCfg_FT5X46_BasicThreshold.WeakShortTest_CC_Rsen;
    bCapShortTest = g_stCfg_FT5X46_BasicThreshold.WeakShortTest_CapShortTest;
    FTS_TEST_DBG(" iCCRsen:%d.  \n", iCCRsen);
    FTS_TEST_DBG(" bCapShortTest:%d.  \n", bCapShortTest);

    ReCode = EnterFactory();
    SysDelay(100);
    if (ReCode != ERROR_CODE_OK) {
        FTS_TEST_DBG(" EnterFactory failed.. Error Code: %d", ReCode);
        btmpresult = false;
        goto TEST_ERR;
    }

    //Determine whether for v3 TP first, and then read the value of the 0x54
    //and check the mapping type is right or not,if not, write the register
    //weakshort test mapping,  before mapping 0x54=1;after mapping 0x54=0;
    if (m_bV3TP) {
        ReCode = ReadReg(REG_MAPPING_SWITCH, &strSwitch);
        if (strSwitch != 1) {
            ReCode = WriteReg(REG_MAPPING_SWITCH, 1);
            SysDelay(20);
            if (ReCode != ERROR_CODE_OK) {
                FTS_TEST_ERROR("\r\nFailed to restore mapping type!\r\n ");
                btmpresult = false;
            }
            GetChannelNumNoMapping();

            iTxNum = g_ScreenSetParam.iTxNum;
            iRxNum = g_ScreenSetParam.iRxNum;
        }
    }
    else {
        //TxNum register 0x02(Read Only)
        //RxNum register 0x03(Read Only)
        ReCode = ReadReg(0x02, &iTxNum);    //Get Tx
        ReCode = ReadReg(0x03, &iRxNum);    //Get Rx
        FTS_TEST_DBG("Newly acquired TxNum:%d, RxNum:%d", iTxNum, iRxNum);
    }

    if (ReCode != ERROR_CODE_OK) {
        FTS_TEST_ERROR("ReCode  error. Error Code: %d\n", ReCode);
        btmpresult = false;
        goto TEST_ERR;
    }

    iChannelNum = iTxNum + iRxNum;
    iMaxTx = iTxNum;
    iAllAdcDataNum = 1 + (1 + iTxNum + iRxNum) * 2; //The total channel number + calibration data + channel calibration data + Offset

    for (i = 0; i < 5; i++) {
        ReCode = StartScan();
        if (ReCode != ERROR_CODE_OK) {
            FTS_TEST_ERROR("StartScan Failed!\n");
            //btmpresult = false;
            SysDelay(100);
        }
        else {
            FTS_TEST_DBG("StartScan OK!\n");
            break;
        }
    }
    if (i >= 5) {
        FTS_TEST_ERROR("StartScan Failed for several times.!\n");
        btmpresult = false;
        goto TEST_ERR;
    }

    iAdcData = fts_malloc(iAllAdcDataNum * sizeof(int));
    memset(iAdcData, 0, iAllAdcDataNum);    //  memset(iAdcData, 0, sizeof(iAdcData));

    for (i = 0; i < 5; i++) {
        memset(iAdcData, 0, iAllAdcDataNum);

        FTS_TEST_DBG("WeakShort_GetAdcData times: %d", i);

        ReCode = WeakShort_GetAdcData(iAllAdcDataNum * 2, iAdcData);
        SysDelay(50);
        if (ReCode != ERROR_CODE_OK) {
            continue;
        }
        else                    //  adc function ok
        {
            if (0 == iAdcData[0] && 0 == iAdcData[1]) {
                continue;
            }

            else
                break;
        }
    }
    if (i >= 5) {
        FTS_TEST_ERROR("WeakShort_GetAdcData or ADC data error. tried times: %d", i);
        btmpresult = false;
        goto TEST_ERR;
    }

    iOffset = iAdcData[0];
    iClbData_Ground = iAdcData[1];
    iClbData_Mutual = iAdcData[2 + iChannelNum];

    //  show value.
#if 0
    ///////////////////////Channel and Ground
    ///////////////////////Channel and Channel
    for (i = 0; i < iAllAdcDataNum /*iChannelNum */ ; i++) {
        if (i <= (iChannelNum + 1)) {
            if (i == 0)
                FTS_TEST_DBG("\n\n\nOffset %02d: %4d,	\n", i, iAdcData[i]);
            else if (i == 1)    /*if(i <= iMaxTx) */
                FTS_TEST_DBG("Ground %02d: %4d,	\n", i, iAdcData[i]);
            else if (i <= (iMaxTx + 1))
                FTS_TEST_DBG("Tx%02d: %4d,	", i - 1, iAdcData[i]);
            else if (i <= (iChannelNum + 1))
                FTS_TEST_DBG("Rx%02d: %4d,	", i - iMaxTx - 1, iAdcData[i]);

            if (i % 10 == 0)
                FTS_TEST_DBG("\n");

        }
        else {
            if (i == (iChannelNum + 2))
                FTS_TEST_DBG("\n\n\nMultual %02d: %4d,	\n", i, iAdcData[i]);
            else if (i <= (iMaxTx) + (iChannelNum + 2))
                FTS_TEST_DBG("Tx%02d: %4d,	", i - (iChannelNum + 2), iAdcData[i]);
            else if (i < iAllAdcDataNum)
                FTS_TEST_DBG("Rx%02d: %4d,	", i - iMaxTx - (iChannelNum + 2), iAdcData[i]);

            if (i % 10 == 0)
                FTS_TEST_DBG("\n");

        }
    }
    FTS_TEST_DBG("\r\n");
    //print all Adc value
#endif

    fMShortResistance = fts_malloc(iChannelNum * sizeof(int));  //  fMShortResistance = new float[iChannelNum];
    memset(fMShortResistance, 0, iChannelNum);
    fGShortResistance = fts_malloc(iChannelNum * sizeof(int));  //      fGShortResistance = new float[iChannelNum];
    memset(fGShortResistance, 0, iChannelNum);

    /////////////////////////////////////////////Channel and Ground
    iMin_CG = g_stCfg_FT5X46_BasicThreshold.WeakShortTest_CG;

    iDoffset = iOffset - 1024;
    iDrefn = iClbData_Ground;
    fKcal = 1;                  // 1.0;

    for (i = 0; i < iChannelNum; i++) {
        iDsen = iAdcData[i + 2];
        if ((2047 + iDoffset) - iDsen <= 0) //<=0 PASS
        {
            continue;
        }

        if (IcValue <= 0x05 || IcValue == 0xff) {

            fGShortResistance[i] = (iDsen - iDoffset + 410) * 25 * fKcal / (2047 + iDoffset - iDsen) - 3;
        }
        else {
            if (iDrefn - iDsen <= 0)    //<=0 PASS
            {
                fGShortResistance[i] = iMin_CG;
                FTS_TEST_DBG("%02d  ", fGShortResistance[i]);

                continue;
            }

            fGShortResistance[i] = (((iDsen - iDoffset + 384) / (iDrefn - iDsen) * 57) - 1);    //( ( iDsen - iDoffset + 384 ) * iRsen / (/*temp*/iDrefn - iDsen) ) * fKcal - 1.2;
        }
        if (fGShortResistance[i] < 0)
            fGShortResistance[i] = 0;

        if ((iMin_CG > fGShortResistance[i]) || (iDsen - iDoffset < 0)) //<=0 ShortResistance = 0
        {
            iCount++;
            if (i + 1 <= iMaxTx)
                FTS_TEST_DBG("Tx%02d: %02d (k��),	", i + 1, fGShortResistance[i]);
            else
                FTS_TEST_DBG("Rx%02d: %02d (k��),	", i + 1 - iMaxTx, fGShortResistance[i]);
            if (iCount % 10 == 0)
                FTS_TEST_DBG("\n");
        }

    }

    //print Channel and Ground value
    if (iCount > 0) {

        btmpresult = false;
    }

    /////////////////////////////////////////////Channel and Channel
    iMin_CC = g_stCfg_FT5X46_BasicThreshold.WeakShortTest_CC;

    if ((IcValue == 0x06 || IcValue < 0xff) && iRsen != iCCRsen) {
        iRsen = iCCRsen;
    }
    iDoffset = iOffset - 1024;
    iDrefn = iClbData_Mutual;
    fKcal = 1.0;
    iCount = 0;
    iDCal = max(iDrefn, 116 + iDoffset);

    for (i = 0; i < iChannelNum; i++) {
        iDsen = iAdcData[i + iChannelNum + 3];
        if (IcValue <= 0x05 || IcValue == 0xff) {
            if (iDsen - iDrefn < 0)
                continue;
        }
        //use new formula
        if (IcValue <= 0x05 || IcValue == 0xff) {
            iMa = iDsen - iDCal;
            iMa = iMa ? iMa : 1;
            fMShortResistance[i] = ((2047 + iDoffset - iDCal) * 24 / iMa - 27) * fKcal - 6;
        }
        else {
            if (iDrefn - iDsen <= 0)    //<=0 PASS
            {
                fMShortResistance[i] = iMin_CC;
                FTS_TEST_DBG("%02d  ", fMShortResistance[i]);
                continue;
            }

            fMShortResistance[i] = (iDsen - iDoffset - 123) * iRsen * fKcal / (iDrefn - iDsen /*temp */ ) - 2;
        }

        if (fMShortResistance[i] < 0 && fMShortResistance[i] >= -240)
            fMShortResistance[i] = 0;
        else if (fMShortResistance[i] < -240)
            continue;

        if (fMShortResistance[i] <= 0 || fMShortResistance[i] < iMin_CC) {
            iCount++;
            if (i + 1 <= iMaxTx)
                FTS_TEST_DBG("Tx%02d: %02d(k��),	", i + 1, fMShortResistance[i]);
            else
                FTS_TEST_DBG("Rx%02d: %02d(k��),	", i + 1 - iMaxTx, fMShortResistance[i]);

            if (iCount % 10 == 0)
                FTS_TEST_DBG("\n");

        }

    }

    if (iCount > 0 && !bCapShortTest) {

        btmpresult = false;
    }

    //take "cap short test" and the first test will be wrong
    if (bCapShortTest && iCount) {
        FTS_TEST_DBG(" bCapShortTest && iCount.  need to add ......");
    }

    //set V3 TP the origin mapping value
    if (m_bV3TP) {
        ReCode = WriteReg(REG_MAPPING_SWITCH, strSwitch);
        SysDelay(50);
        if (ReCode != ERROR_CODE_OK) {
            FTS_TEST_ERROR("Failed to restore mapping type!\r\n ");
            btmpresult = false;
        }
        ReCode = GetChannelNum();
        if (ReCode != ERROR_CODE_OK) {
            FTS_TEST_ERROR("\n GetChannelNum error. Error Code: %d", ReCode);
            goto TEST_ERR;
        }

        ReCode = GetRawData();
    }

TEST_ERR:

    if (NULL != iAdcData) {
        fts_free(iAdcData);
        iAdcData = NULL;
    }

    if (NULL != fMShortResistance) {
        fts_free(fMShortResistance);
        fMShortResistance = NULL;
    }

    if (NULL != fGShortResistance) {
        fts_free(fGShortResistance);
        fGShortResistance = NULL;
    }

    TestResultLen += sprintf(TestResult + TestResultLen, " Weak Short Test is %s. \n\n", (btmpresult ? "OK" : "NG"));

    if (btmpresult) {
        FTS_TEST_INFO("\r\n\r\n//Weak Short Test is OK.");
        *bTestResult = true;
    }
    else {
        FTS_TEST_INFO("\r\n\r\n//Weak Short Test is NG.");
        *bTestResult = false;
    }

    return ReCode;
}

/************************************************************************
* Name: GetPanelRows(Same function name as FT_MultipleTest)
* Brief:  Get row of TP
* Input: none
* Output: pPanelRows
* Return: Comm Code. Code = 0x00 is OK, else fail.
***********************************************************************/
static unsigned char GetPanelRows(unsigned char *pPanelRows)
{
    return ReadReg(REG_TX_NUM, pPanelRows);
}

/************************************************************************
* Name: GetPanelCols(Same function name as FT_MultipleTest)
* Brief:  get column of TP
* Input: none
* Output: pPanelCols
* Return: Comm Code. Code = 0x00 is OK, else fail.
***********************************************************************/
static unsigned char GetPanelCols(unsigned char *pPanelCols)
{
    return ReadReg(REG_RX_NUM, pPanelCols);
}

/************************************************************************
* Name: StartScan(Same function name as FT_MultipleTest)
* Brief:  Scan TP, do it before read Raw Data
* Input: none
* Output: none
* Return: Comm Code. Code = 0x00 is OK, else fail.
***********************************************************************/
static int StartScan(void)
{
    unsigned char RegVal = 0;
    unsigned char times = 0;
    const unsigned char MaxTimes = 250; //The longest wait 160ms
    unsigned char ReCode = ERROR_CODE_COMM_ERROR;

    ReCode = ReadReg(DEVIDE_MODE_ADDR, &RegVal);
    if (ReCode == ERROR_CODE_OK) {
        RegVal |= 0x80;         //Top bit position 1, start scan
        ReCode = WriteReg(DEVIDE_MODE_ADDR, RegVal);
        if (ReCode == ERROR_CODE_OK) {
            while (times++ < MaxTimes)  //Wait for the scan to complete
            {
                SysDelay(16);   //16ms
                ReCode = ReadReg(DEVIDE_MODE_ADDR, &RegVal);
                if (ReCode == ERROR_CODE_OK) {
                    if ((RegVal >> 7) == 0)
                        break;
                }
                else {
                    FTS_TEST_ERROR("StartScan read DEVIDE_MODE_ADDR error.");
                    break;
                }
            }
            if (times < MaxTimes)
                ReCode = ERROR_CODE_OK;
            else {
                ReCode = ERROR_CODE_COMM_ERROR;
                FTS_TEST_ERROR("times NOT < MaxTimes. error.");
            }
        }
        else
            FTS_TEST_ERROR("StartScan write DEVIDE_MODE_ADDR error.");
    }
    else
        FTS_TEST_ERROR("StartScan read DEVIDE_MODE_ADDR error.");
    return ReCode;

}

/************************************************************************
* Name: ReadRawData(Same function name as FT_MultipleTest)
* Brief:  read Raw Data
* Input: Freq(No longer used, reserved), LineNum, ByteNum
* Output: pRevBuffer
* Return: Comm Code. Code = 0x00 is OK, else fail.
***********************************************************************/
unsigned char ReadRawData(unsigned char Freq, unsigned char LineNum, int ByteNum, int *pRevBuffer)
{
    unsigned char ReCode = ERROR_CODE_COMM_ERROR;
    unsigned char I2C_wBuffer[3];
    int i, iReadNum;
    unsigned short BytesNumInTestMode1 = 0;

    iReadNum = ByteNum / BYTES_PER_TIME;

    if (0 != (ByteNum % BYTES_PER_TIME))
        iReadNum++;

    if (ByteNum <= BYTES_PER_TIME) {
        BytesNumInTestMode1 = ByteNum;
    }
    else {
        BytesNumInTestMode1 = BYTES_PER_TIME;
    }

    ReCode = WriteReg(REG_LINE_NUM, LineNum);   //Set row addr;

    if (ReCode != ERROR_CODE_OK) {
        FTS_TEST_ERROR("Failed to write REG_LINE_NUM! ");
        goto READ_ERR;
    }

    //***********************************************************Read raw data
    I2C_wBuffer[0] = REG_RawBuf0;   //set begin address
    if (ReCode == ERROR_CODE_OK) {
        focal_msleep(10);
        ReCode = Comm_Base_IIC_IO(I2C_wBuffer, 1, m_ucTempData, BytesNumInTestMode1);
        if (ReCode != ERROR_CODE_OK) {
            FTS_TEST_ERROR("read rawdata Comm_Base_IIC_IO Failed!1 ");
            goto READ_ERR;
        }
    }

    for (i = 1; i < iReadNum; i++) {
        if (ReCode != ERROR_CODE_OK)
            break;

        if (i == iReadNum - 1)  //last packet
        {
            focal_msleep(10);
            ReCode = Comm_Base_IIC_IO(NULL, 0, m_ucTempData + BYTES_PER_TIME * i, ByteNum - BYTES_PER_TIME * i);
            if (ReCode != ERROR_CODE_OK) {
                FTS_TEST_ERROR("read rawdata Comm_Base_IIC_IO Failed!2 ");
                goto READ_ERR;
            }
        }
        else {
            focal_msleep(10);
            ReCode = Comm_Base_IIC_IO(NULL, 0, m_ucTempData + BYTES_PER_TIME * i, BYTES_PER_TIME);

            if (ReCode != ERROR_CODE_OK) {
                FTS_TEST_ERROR("read rawdata Comm_Base_IIC_IO Failed!3 ");
                goto READ_ERR;
            }
        }

    }

    if (ReCode == ERROR_CODE_OK) {
        for (i = 0; i < (ByteNum >> 1); i++) {
            pRevBuffer[i] = (m_ucTempData[i << 1] << 8) + m_ucTempData[(i << 1) + 1];
            //if(pRevBuffer[i] & 0x8000)//have sign bit
            //{
            //  pRevBuffer[i] -= 0xffff + 1;
            //}
        }
    }

READ_ERR:
    return ReCode;

}

/************************************************************************
* Name: GetTxSC_CB(Same function name as FT_MultipleTest)
* Brief:  get CB of Tx SCap
* Input: index
* Output: pcbValue
* Return: Comm Code. Code = 0x00 is OK, else fail.
***********************************************************************/
unsigned char GetTxSC_CB(unsigned char index, unsigned char *pcbValue)
{
    unsigned char ReCode = ERROR_CODE_OK;
    unsigned char wBuffer[4];

    if (index < 128)            //single read
    {
        *pcbValue = 0;
        WriteReg(REG_ScCbAddrR, index);
        ReCode = ReadReg(REG_ScCbBuf0, pcbValue);
    }
    else                        //Sequential Read length index-128
    {
        WriteReg(REG_ScCbAddrR, 0);
        wBuffer[0] = REG_ScCbBuf0;
        ReCode = Comm_Base_IIC_IO(wBuffer, 1, pcbValue, index - 128);

    }

    return ReCode;
}

/************************************************************************
* Name: Save_Test_Data
* Brief:  Storage format of test data
* Input: int iData[TX_NUM_MAX][RX_NUM_MAX], int iArrayIndex, unsigned char Row, unsigned char Col, unsigned char ItemCount
* Output: none
* Return: none
***********************************************************************/
static void Save_Test_Data(int iData[TX_NUM_MAX][RX_NUM_MAX], int iArrayIndex, unsigned char Row, unsigned char Col, unsigned char ItemCount)
{
    int iLen = 0;
    int i = 0, j = 0;

    //Save  Msg (ItemCode is enough, ItemName is not necessary, so set it to "NA".)
    iLen = sprintf(g_pTmpBuff, "NA, %d, %d, %d, %d, %d, ", m_ucTestItemCode, Row, Col, m_iStartLine, ItemCount);
    memcpy(g_pMsgAreaLine2 + g_lenMsgAreaLine2, g_pTmpBuff, iLen);
    g_lenMsgAreaLine2 += iLen;

    m_iStartLine += Row;
    m_iTestDataCount++;

    //Save Data
    for (i = 0 + iArrayIndex; (i < Row + iArrayIndex) && (i < TX_NUM_MAX); i++) {
        for (j = 0; (j < Col) && (j < RX_NUM_MAX); j++) {
            if (j == (Col - 1)) //The Last Data of the Row, add "\n"
                iLen = sprintf(g_pTmpBuff, "%d, \n", iData[i][j]);
            else
                iLen = sprintf(g_pTmpBuff, "%d, ", iData[i][j]);

            memcpy(g_pStoreDataArea + g_lenStoreDataArea, g_pTmpBuff, iLen);
            g_lenStoreDataArea += iLen;
        }
    }

}

/************************************************************************
* Name: GetChannelNum
* Brief:  Get Channel Num(Tx and Rx)
* Input: none
* Output: none
* Return: Comm Code. Code = 0x00 is OK, else fail.
***********************************************************************/
static unsigned char GetChannelNum(void)
{
    unsigned char ReCode;
    unsigned char rBuffer[1];   //= new unsigned char;

    //m_strCurrentTestMsg = "Get Tx Num...";
    ReCode = GetPanelRows(rBuffer);
    if (ReCode == ERROR_CODE_OK) {
        g_ScreenSetParam.iTxNum = rBuffer[0];
        if (g_ScreenSetParam.iTxNum > g_ScreenSetParam.iUsedMaxTxNum) {
            FTS_TEST_ERROR("Failed to get Tx number, Get num = %d, UsedMaxNum = %d", g_ScreenSetParam.iTxNum, g_ScreenSetParam.iUsedMaxTxNum);
            g_ScreenSetParam.iTxNum = 0;
            return ERROR_CODE_INVALID_PARAM;
        }
    }
    else {
        FTS_TEST_ERROR("Failed to get Tx number");
    }

    ///////////////m_strCurrentTestMsg = "Get Rx Num...";

    ReCode = GetPanelCols(rBuffer);
    if (ReCode == ERROR_CODE_OK) {
        g_ScreenSetParam.iRxNum = rBuffer[0];
        if (g_ScreenSetParam.iRxNum > g_ScreenSetParam.iUsedMaxRxNum) {
            FTS_TEST_ERROR("Failed to get Rx number, Get num = %d, UsedMaxNum = %d", g_ScreenSetParam.iRxNum, g_ScreenSetParam.iUsedMaxRxNum);
            g_ScreenSetParam.iRxNum = 0;
            return ERROR_CODE_INVALID_PARAM;
        }
    }
    else {
        FTS_TEST_ERROR("Failed to get Rx number");
    }

    return ReCode;

}

/************************************************************************
* Name: GetRawData
* Brief:  Get Raw Data of MCAP
* Input: none
* Output: none
* Return: Comm Code. Code = 0x00 is OK, else fail.
***********************************************************************/
static unsigned char GetRawData(void)
{
    unsigned char ReCode = ERROR_CODE_OK;
    int iRow = 0;
    int iCol = 0;

    //--------------------------------------------Enter Factory Mode
    ReCode = EnterFactory();
    if (ERROR_CODE_OK != ReCode) {
        FTS_TEST_DBG("Failed to Enter Factory Mode...");
        return ReCode;
    }

    //--------------------------------------------Check Num of Channel
    if (0 == (g_ScreenSetParam.iTxNum + g_ScreenSetParam.iRxNum)) {
        ReCode = GetChannelNum();
        if (ERROR_CODE_OK != ReCode) {
            FTS_TEST_DBG("Error Channel Num...");
            return ERROR_CODE_INVALID_PARAM;
        }
    }

    //--------------------------------------------Start Scanning
    FTS_TEST_DBG("Start Scan ...");
    ReCode = StartScan();
    if (ERROR_CODE_OK != ReCode) {
        FTS_TEST_ERROR("Failed to Scan ...");
        return ReCode;
    }

    //--------------------------------------------Read RawData, Only MCAP
    memset(m_RawData, 0, sizeof(m_RawData));
    ReCode = ReadRawData(1, 0xAA, (g_ScreenSetParam.iTxNum * g_ScreenSetParam.iRxNum) * 2, m_iTempRawData);
    for (iRow = 0; iRow < g_ScreenSetParam.iTxNum; iRow++) {
        for (iCol = 0; iCol < g_ScreenSetParam.iRxNum; iCol++) {
            m_RawData[iRow][iCol] = m_iTempRawData[iRow * g_ScreenSetParam.iRxNum + iCol];
        }
    }
    return ReCode;
}

/************************************************************************
* Name: ShowRawData
* Brief:  Show RawData
* Input: none
* Output: none
* Return: none.
***********************************************************************/
static void ShowRawData(void)
{
    int iRow, iCol;
    //----------------------------------------------------------Show RawData
    for (iRow = 0; iRow < g_ScreenSetParam.iTxNum; iRow++) {
        FTS_TEST_DBG("Tx%2d:  ", iRow + 1);
        for (iCol = 0; iCol < g_ScreenSetParam.iRxNum; iCol++) {
            FTS_TEST_DBG("%5d    ", m_RawData[iRow][iCol]);
        }
        FTS_TEST_DBG("\n ");
    }
}

/************************************************************************
* Name: GetChannelNumNoMapping
* Brief:  get Tx&Rx num from other Register
* Input: none
* Output: none
* Return: Comm Code. Code = 0x00 is OK, else fail.
***********************************************************************/
static unsigned char GetChannelNumNoMapping(void)
{
    unsigned char ReCode;
    unsigned char rBuffer[1];   //= new unsigned char;

    FTS_TEST_DBG("Get Tx Num...");
    ReCode = ReadReg(REG_TX_NOMAPPING_NUM, rBuffer);
    if (ReCode == ERROR_CODE_OK) {
        g_ScreenSetParam.iTxNum = rBuffer[0];
    }
    else {
        FTS_TEST_ERROR("Failed to get Tx number");
    }

    FTS_TEST_DBG("Get Rx Num...");
    ReCode = ReadReg(REG_RX_NOMAPPING_NUM, rBuffer);
    if (ReCode == ERROR_CODE_OK) {
        g_ScreenSetParam.iRxNum = rBuffer[0];
    }
    else {
        FTS_TEST_ERROR("Failed to get Rx number");
    }

    return ReCode;
}

/************************************************************************
* Name: SwitchToNoMapping
* Brief:  If it is V3 pattern, Get Tx/Rx Num again
* Input: none
* Output: none
* Return: Comm Code. Code = 0x00 is OK, else fail.
***********************************************************************/
static unsigned char SwitchToNoMapping(void)
{
    unsigned char chPattern = -1;
    unsigned char ReCode = ERROR_CODE_OK;
    unsigned char RegData = -1;
    ReCode = ReadReg(REG_PATTERN_5422, &chPattern); //
    if (ReCode != ERROR_CODE_OK) {
        FTS_TEST_ERROR("Switch To NoMapping Failed!");
        goto READ_ERR;
    }

    if (1 == chPattern)         // 1: V3 Pattern
    {
        RegData = -1;
        ReCode = ReadReg(REG_MAPPING_SWITCH, &RegData);
        if (ReCode != ERROR_CODE_OK) {
            FTS_TEST_ERROR("read REG_MAPPING_SWITCH Failed!");
            goto READ_ERR;
        }

        if (1 != RegData) {
            ReCode = WriteReg(REG_MAPPING_SWITCH, 1);   //0-mapping 1-no mampping
            if (ReCode != ERROR_CODE_OK) {
                FTS_TEST_ERROR("write REG_MAPPING_SWITCH Failed!");
                goto READ_ERR;
            }
            focal_msleep(20);
            ReCode = GetChannelNumNoMapping();

            if (ReCode != ERROR_CODE_OK) {
                FTS_TEST_ERROR("GetChannelNumNoMapping Failed!");
                goto READ_ERR;
            }
        }
    }

READ_ERR:
    return ReCode;
}

/************************************************************************
* Name: GetTestCondition
* Brief:  Check whether Rx or TX need to test, in Waterproof ON/OFF Mode.
* Input: none
* Output: none
* Return: true: need to test; false: Not tested.
***********************************************************************/
static boolean GetTestCondition(int iTestType, unsigned char ucChannelValue)
{
    boolean bIsNeeded = false;
    switch (iTestType) {
    case WT_NeedProofOnTest:   //Bit5:  0:test waterProof mode ;  1 not test waterProof mode
        bIsNeeded = !(ucChannelValue & 0x20);
        break;
    case WT_NeedProofOffTest:  //Bit7: 0: test normal mode  1:not test normal mode
        bIsNeeded = !(ucChannelValue & 0x80);
        break;
    case WT_NeedTxOnVal:
        //Bit6:  0 : test waterProof Rx+Tx  1:test waterProof single channel
        //Bit2:  0: test waterProof Tx only;  1:  test waterProof Rx only
        bIsNeeded = !(ucChannelValue & 0x40) || !(ucChannelValue & 0x04);
        break;
    case WT_NeedRxOnVal:
        //Bit6:  0 : test waterProof Rx+Tx  1 test waterProof single channel
        //Bit2:  0: test waterProof Tx only;  1:  test waterProof Rx only
        bIsNeeded = !(ucChannelValue & 0x40) || (ucChannelValue & 0x04);
        break;
    case WT_NeedTxOffVal:      //Bit1,Bit0:  00:test normal Tx; 10: test normal Rx+Tx
        bIsNeeded = (0x00 == (ucChannelValue & 0x03)) || (0x02 == (ucChannelValue & 0x03));
        break;
    case WT_NeedRxOffVal:      //Bit1,Bit0:  01: test normal Rx;    10: test normal Rx+Tx
        bIsNeeded = (0x01 == (ucChannelValue & 0x03)) || (0x02 == (ucChannelValue & 0x03));
        break;
    default:
        break;
    }
    return bIsNeeded;
}

//unsigned char WeakShort_GetAdcData( int AllAdcDataLen, int *pRevBuffer, int iCommMode,unsigned char chCmd/* = 0x01*/ )
static unsigned char WeakShort_GetAdcData(int AllAdcDataLen, int *pRevBuffer)
{
    unsigned char ReCode = ERROR_CODE_COMM_ERROR;
    int iReadDataLen = AllAdcDataLen;   //Offset*2 + (ClbData + TxNum + RxNum)*2*2
    unsigned char *pDataSend = NULL;

    unsigned char Data = 0xff;
    int i = 0;
    bool bAdcOK = false;

    FTS_TEST_FUNC_ENTER();

    pDataSend = fts_malloc(iReadDataLen + 1);
    if (pDataSend == NULL)
        return ERROR_CODE_ALLOCATE_BUFFER_ERROR;
    memset(pDataSend, 0, iReadDataLen + 1);

    ReCode = WriteReg(0x07, 0x01);  // Test weak short once,after host send an enable command
    if (ReCode != ERROR_CODE_OK) {
        FTS_TEST_ERROR("WriteReg error. \n");
        goto EndGetAdc;
    }

    SysDelay(100);

    for (i = 0; i < 100 * 5; i++)   // Data ready,and FW set reg 0x07 to 0.
    {
        SysDelay(10);
        ReCode = ReadReg(0x07, &Data);
        if (ReCode == ERROR_CODE_OK) {
            if (Data == 0) {
                bAdcOK = true;
                break;
            }
        }
    }

    if (!bAdcOK) {
        FTS_TEST_ERROR("ADC data NOT ready.  error.\n");
        ReCode = ERROR_CODE_COMM_ERROR;
        goto EndGetAdc;
    }
    SysDelay(300);              //  frank add. 20160517
    pDataSend[0] = 0xF4;

    ReCode = Comm_Base_IIC_IO(pDataSend, 1, pDataSend + 1, iReadDataLen);
    if (ReCode == ERROR_CODE_OK) {

        for (i = 0; i < iReadDataLen / 2; i++) {
            pRevBuffer[i] = (pDataSend[1 + 2 * i] << 8) + pDataSend[1 + 2 * i + 1];

        }

    }
    else {
        FTS_TEST_ERROR("Comm_Base_IIC_IO error. error:%d. \n", ReCode);
    }

EndGetAdc:
    if (pDataSend != NULL) {
        fts_free(pDataSend);
        pDataSend = NULL;
    }

    FTS_TEST_FUNC_EXIT();

    return ReCode;
}

#endif
