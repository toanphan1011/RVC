/*
****************************************************************************
PROJECT : RCAR-M3 WM Evaluation
FILE    : $Id: r_wm_d1hx_test_02.c 10755 2016-10-27 15:29:40Z michael.golczewski $
============================================================================ 
DESCRIPTION
Main functions of the D1H test suite
============================================================================
                            C O P Y R I G H T                            
============================================================================
                           Copyright (c) 2015
                                  by 
                       Renesas Electronics (Europe) GmbH. 
                           Arcadiastrasse 10
                          D-40472 Duesseldorf
                               Germany
                          All rights reserved.
============================================================================
Purpose: only for testing

DISCLAIMER                                                                   
This software is supplied by Renesas Electronics Corporation and is only     
intended for use with Renesas products. No other uses are authorized. This   
software is owned by Renesas Electronics Corporation and is protected under  
all applicable laws, including copyright laws.                               
THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING  
THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT      
LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE   
AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.          
TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS       
ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE  
FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR   
ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE  
BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.                             
Renesas reserves the right, without notice, to make changes to this software 
and to discontinue the availability of this software. By using this software,
you agree to the additional terms and conditions found by accessing the      
following link:                                                              
http://www.renesas.com/disclaimer *                                          
Copyright (C) 2015 Renesas Electronics Corporation. All rights reserved.     

****************************************************************************
*/

/*******************************************************************************
  Title: tests 10:  Test the WM Display DISCOM configuration

  Implementation of the application tests for R-Car H3/M3 WM.
*/

/*******************************************************************************
  Section: Includes
*/

#pragma ghs startnomisra
#include <INTEGRITY.h>
#include <util/error_string.h>
#include <bsp.h>
#pragma ghs endnomisra
#include "common.h"

#include "r_mmgr_config.h"
#include "r_mmgr_api.h"
#include "r_config_wm.h"
#include "r_config_vspd.h"
#include "r_ddb_api.h"
#include "r_wm_api.h"
#include "r_wm_d1hx_test_common.h"
//#include "001_VGA_YUV420.h"

#include "../../libs/vspd_usr/device/r_vspd_device_macro.h"

/*******************************************************************************
  Section: Defines
*/

/*******************************************************************************
  Define: LOC_DISPLAY
  
  Default display unit used in this test
*/
#define LOC_DISPLAY       R_WM_DEV_DU1



/*******************************************************************************
  Variable: locWindow
  
  Array where the window information are stored.
  
*/
static r_wm_Window_t *locWindows;


/*******************************************************************************
    Section: Local Functions prototypes
*/
   
static r_wm_DiscomCfg_t   loc_DiscomCfg = {0}; 
static Task               children[R_VSP_MACRO_NUM];
    
    

static uint32_t DiscomErrCnt[R_VSP_MACRO_NUM]={0};

/*******************************************************************************
    Section: Local Functions
*/

static void loc_ErrTask0(void)
{
    static uint32_t run = 1; 
       
    while(run)
    {
        if (R_WM_ERR_OK == R_WM_DiscomWait(0, &loc_DiscomCfg))
        { 
            DiscomErrCnt[0]++; 
            R_WM_DiscomIntEnable(0, &loc_DiscomCfg, 0);     
        }
    }
    Exit(0);
}
static void loc_ErrTask1(void)
{
    static uint32_t run = 1; 
       
    while(run)
    {
        if (R_WM_ERR_OK == R_WM_DiscomWait(1, &loc_DiscomCfg)) 
        {
            DiscomErrCnt[1]++;  
            R_WM_DiscomIntEnable(1, &loc_DiscomCfg, 0);     
        }
    }
    Exit(0);
}
static void loc_ErrTask2(void)
{
    static uint32_t run = 1; 
       
    while(run)
    {
        if (R_WM_ERR_OK == R_WM_DiscomWait(2, &loc_DiscomCfg))
        { 
            DiscomErrCnt[2]++;  
            R_WM_DiscomIntEnable(2, &loc_DiscomCfg, 0);     
        }
    }
    Exit(0);
}
#ifdef RCAR_H3
static void loc_ErrTask3(void)
{
    static uint32_t run = 1; 
       
    while(run)
    {
        if (R_WM_ERR_OK == R_WM_DiscomWait(3, &loc_DiscomCfg))
        {   
            DiscomErrCnt[3]++;  
            R_WM_DiscomIntEnable(3, &loc_DiscomCfg, 0);
        }
    }
    Exit(0);
}
#endif

#define DISCOM_DEFAULT_OFFSET_X        (0u)
#define DISCOM_DEFAULT_OFFSET_Y        (0u)
//#define DISCOM_DEFAULT_TEST_TIME       (100u)
#define DISCOM_DEFAULT_TEST_TIME       (5u)

char_t *discom_execute(uint32_t Unit, uint32_t VspUnit, r_wm_DiscomCfg_t * DiscomCfg, r_mmgr_MemBlock_t *Mem, uint32_t IsSuccess, uint32_t IsInterrupt)
{
    uint32_t ExpectedCRC    = 0u;
    uint32_t errors         = 0u;
    uint32_t remain         = 0u;
    uint32_t val            = 0u;
    uint32_t status         = 0u;
     char *mode_name[2]     = {"Polling", "Interrupt"};
    uint32_t res_ent        = 0u;
    char *result_name[2]    = {"Success", "Error"};
    r_wm_Error_t wm_err     = R_WM_ERR_OK;
    uint32_t ret            = 0u;

    //discom config 
    DiscomCfg->Param.h_offset   = DISCOM_DEFAULT_OFFSET_X;
    DiscomCfg->Param.v_offset   = DISCOM_DEFAULT_OFFSET_Y;
    DiscomCfg->Param.alpha_mode = R_WM_DISCOM_ALPHA_PIXEL;
    DiscomCfg->Param.alpha      = 0;
    DiscomCfg->Param.threshold  = 0;
    
    if (R_WM_ERR_OK != wm_err)
    {
        printf("[discom_execute] Failed to init  Discom, WM Error = %d\n", wm_err);
        ret++;
    }
    wm_err = R_WM_ConfigDiscom(Unit, DiscomCfg);
    if (R_WM_ERR_OK != wm_err)
    {
        printf("[discom_execute] Failed to set up Discom, WM Error = %d\n", wm_err);
        ret++;
    }
   // Start the DISCOM once and stop, then get the initial CRC value.
    wm_err = R_WM_DiscomSetCRC(Unit, DiscomCfg, 0);
    if (R_WM_ERR_OK != wm_err)
    {
        printf("[discom_execute] Failed to set initial CRC, WM Error = %d\n", wm_err);
        ret++;
    }
    wm_err = R_WM_DiscomIntEnable(Unit, DiscomCfg, 0);     
    if (R_WM_ERR_OK != wm_err)
    {
        printf("[discom_execute] Failed to disable DISCOM Interrupt, WM Error = %d\n", wm_err);
        ret++;
    }
    wm_err = R_WM_DiscomEnable(Unit, DiscomCfg, 1);
    if (R_WM_ERR_OK != wm_err)
    {
        printf("[discom_execute] Failed to enable DISCOM, WM Error = %d\n", wm_err);
        ret++;
    }
    sleep(1u);

    wm_err = R_WM_DiscomGetCRC(Unit, DiscomCfg, &ExpectedCRC);
    if (R_WM_ERR_OK != wm_err)
    {
        printf("[discom_execute] Failed to get calculated CRC, WM Error = %d\n", wm_err);
        ret++;
    }
    wm_err = R_WM_DiscomEnable(Unit, DiscomCfg, 0);
    if (R_WM_ERR_OK != wm_err)
    {
        printf("[discom_execute] Failed to disable DISCOM, WM Error = %d\n", wm_err);
        ret++;
    }

    wm_err = R_WM_DiscomClear(Unit, DiscomCfg);
    if (R_WM_ERR_OK != wm_err)
    {
        printf("[discom_execute] Failed to clear DISCOM, WM Error = %d\n", wm_err);
        ret++;
    }

    // Main part - Start DISCOM and check the display frame in specified seconds.
    wm_err = R_WM_DiscomSetCRC(Unit, DiscomCfg, ExpectedCRC);
    if (R_WM_ERR_OK != wm_err)
    {
        printf("[discom_execute] Failed to set expected CRC, WM Error = %d\n", wm_err);
        ret++;
    }
    wm_err = R_WM_DiscomEnable(Unit, DiscomCfg, 1);
    if (R_WM_ERR_OK != wm_err)
    {
        printf("[discom_execute] Failed to enable again DISCOM, WM Error = %d\n", wm_err);
        ret++;
    }
    sleep(1u);
    
    if(IsInterrupt == 1)
    {
        DiscomErrCnt[VspUnit] = 0; 
        wm_err = R_WM_DiscomIntEnable(Unit, DiscomCfg, 1);     
        if (R_WM_ERR_OK != wm_err)
        {
            printf("[discom_execute] Failed to enable DISCOM interupt, WM Error = %d\n", wm_err);
            ret++;
        }
    }
    if(IsSuccess == 0)
    {
//	    if(remain == 10u)
    	{
    	    Address start_addr,offset=0u;
    	    Address mem_addr;
    	    start_addr = Mem->VmrStartAddr;
    
    	    mem_addr = start_addr+offset;
    	    val = *((uint32_t *)(mem_addr));
    	    *((uint32_t *)(mem_addr)) = ~val;  //modified pixel data
    	}
    }

    for (remain = DISCOM_DEFAULT_TEST_TIME; remain > 0u; remain--) 
    {
    	// Check DISCOM status in every VSYNC.
        R_WM_DevWaitForEvent(Unit, R_WM_EVENT_VBLANK);
    
    	if(IsInterrupt == 0)
    	{
    	    wm_err = R_WM_DiscomGetStatus(Unit, DiscomCfg, &status);
            if (R_WM_ERR_OK != wm_err)
            {
                printf("[discom_execute] Failed to get DISCOM status, WM Error = %d\n", wm_err);
                ret++;
            }
    
    	    if ((status & 1u) != 0u) 
    	    {
    		    //printf("DISCOM: Detected CRC mismatch, 0x%08x vs 0x%08x.\n", ExpectedCRC, param.CCRCR);
    		    errors++;
    
                wm_err = R_WM_DiscomClear(Unit, DiscomCfg);
                if (R_WM_ERR_OK != wm_err)
                {
                    printf("[discom_execute] Failed to clear DISCOM Status, WM Error = %d\n", wm_err);
                    ret++;
                }
    		    break;
    	    }
    	}
    	else
    	{
    	    if(DiscomErrCnt[Unit] > 0)
    	    {
    		    errors++;
    		    break;
    	    }
	    }
    }
 
 
    if(IsInterrupt == 1)
    {
        wm_err = R_WM_DiscomIntEnable(Unit, DiscomCfg, 0);     
        if (R_WM_ERR_OK != wm_err)
        {
            printf("[discom_execute] Failed to disable DISCOM interupt, WM Error = %d\n", wm_err);
            ret++;
        }
    }
    wm_err = R_WM_DiscomEnable(Unit, DiscomCfg, 0);
    if (R_WM_ERR_OK != wm_err)
    {
        printf("[discom_execute] Failed to disable again DISCOM, WM Error = %d\n", wm_err);
        ret++;
    }
    wm_err = R_WM_DiscomClear(Unit, DiscomCfg);
    if (R_WM_ERR_OK != wm_err)
    {
        printf("[discom_execute] Failed to again clear DISCOM, WM Error = %d\n", wm_err);
        ret++;
    }

    if(((IsSuccess == 1) && (errors == 0)) || ((IsSuccess == 0) && (errors > 0)))
    {
	    res_ent=0;
    }
    else
    {
	    res_ent=1;
    }

    if((DiscomCfg->Target == R_WM_DISCOM_TGT_BLEND) || (DiscomCfg->Target == R_WM_DISCOM_TGT_BLENDSUB))
    {
    	if(errors > 0)
    	{
    	    printf("DISCOM: Test %s layer=blend mismatch detect.(%s mode)\n", result_name[res_ent], mode_name[IsInterrupt]);
    	}
    	else
    	{
    	    printf("DISCOM: Test %s layer=blend no mismatch.(%s mode)\n", result_name[res_ent], mode_name[IsInterrupt]);
    	}
    }
    else
    {
    	if(errors > 0)
    	{
    	    printf("DISCOM: Test %s layer=%d mismatch detect.(%s mode)\n", result_name[res_ent], DiscomCfg->Target, mode_name[IsInterrupt]);
    	}
    	else
    	{
    	    printf("DISCOM: Test %s layer=%d no mismatch.(%s mode)\n", result_name[res_ent], DiscomCfg->Target, mode_name[IsInterrupt]);
    	}
    }
    if ((0 != ret) || (0 != res_ent))
    {
         return (char_t*)"Err";
    }
    else
    {
        return NULL;
    }
}
/*******************************************************************************
  Function: loc_prepareLayerGeom
  
  This function calculate the layer size and position on screen
  
*/
static void loc_prepareLayerGeom (const r_wm_SurfaceType_t     Type,
                                  const r_wm_WinBufAllocMode_t BufMode,
                                  const r_wm_WinColorFmt_t     ColorFmt,
                                  const uint32_t               BufNum,
                                  const uint32_t               layerNum,
                                  const uint32_t               screenWidth,
                                  const uint32_t               screenHeight,
                                  r_wm_Window_t              * Win
                                  )
{
    uint32_t i;

    for (i = 0; layerNum > i; i++)
    {
        memset(&Win[i], 0, sizeof(r_wm_Window_t));
        Win[i].ColorFmt  = ColorFmt;
        Win[i].PosX      = i * (screenWidth  / (2*layerNum));
        Win[i].PosY      = i * (screenHeight / (2*layerNum));
        Win[i].PosZ      = i; 
        Win[i].Pitch     = screenWidth  ;
        Win[i].Width     = screenWidth  ;
        Win[i].Height    = screenHeight ;
        Win[i].Alpha     = 0xff; /* Opaque*/
        
        Win[i].Surface.BufNum  = BufNum;
        Win[i].Surface.Type    = Type;
        Win[i].Surface.BufMode = BufMode;
    }
}


/*******************************************************************************
  Function: loc_RenderToFb
  
  This function write to the off screen framebuffer and call the swap function when ready
  
*/
static r_wm_Error_t loc_RenderToFb (uint32_t Unit, r_wm_Window_t * Win,
                                    const uint32_t  colour
                                    )
{
    r_wm_Error_t        err;
    r_mmgr_MemBlock_t * rect = NULL;
    
    do
    {
        err = R_WM_DevWaitForEvent(Unit, R_WM_EVENT_VBLANK);
        if (R_WM_ERR_OK != err)
        {
            printf("[loc_RenderToFb] Wait for Video sync err = %d\n", err);
            return err;
        }
        /* Not used: just to change the status of the buffers (that are written by the Texture side) */
        rect = (r_mmgr_MemBlock_t *)R_WM_WindowNewDrawBufGet(Unit, Win);
    } while (NULL == rect);
    
    R_WM_Test_FillColorRGB(rect, colour, Win);

    return err;
}

#define DISCOM_TEST_DRVS (1u)
#define DISCOM_TEST_LAYERS (5u)
#define DISCOM_TEST_LAYER_NUM (DISCOM_TEST_DRVS*DISCOM_TEST_LAYERS)
static uint32_t MyColours[DISCOM_TEST_LAYERS] = {
    0xFFFF0000u, //red, 
    0xF0FF8000u, //orange 
    0xF4FFFF00u, //yellow, 
    0xF800FF00u, //green, 
    0x800000FFu //blue 
    //0xFFFFFFFFu
};     


//char_t* test_discom_du(uint32_t duch, r_vsp_Discom_Select_t Select)
static uint32_t loc_Test_Discom(uint32_t DuUnit, uint32_t VspUnit, uint32_t LayerNumMax, uint32_t WidthMax, uint32_t HeightMax)
{
    char *ptr;
    uint32_t ret = 0; 
    uint32_t Width;
    uint32_t Height;
    uint32_t layerIndex;
    uint32_t Select = 0; 
    r_wm_Error_t        wm_err;
    r_wm_VspdInfo_t vspd_info; 
    
	Width = WidthMax/2;
	Height = HeightMax/2;
    /* Allocate the windows locally */
    locWindows = (r_wm_Window_t *)malloc(sizeof(r_wm_Window_t) * LayerNumMax);
    loc_prepareLayerGeom(R_WM_SURFACE_FB,
                         R_WM_WINBUF_ALLOC_INTERNAL,
                         R_WM_COLORFMT_ARGB8888,
                         1,
                         LayerNumMax,
                         Width,
                         Height,
                         locWindows);
    
    /* Create the windows using the WM and enable the layers*/
    for (layerIndex = 0; layerIndex < LayerNumMax; layerIndex ++)
    {
        wm_err = R_WM_Test_CreateWindowSurface(DuUnit, &locWindows[layerIndex]);
        if (R_WM_ERR_OK != wm_err)
        {
            printf("[R_WM_Test_CreateWindowSurface] Failed to create window layer %d\n, WM Error = %d\n", layerIndex, wm_err);
            ret = 1;
            goto quit;
        }
        
        wm_err = loc_RenderToFb(DuUnit,&locWindows[layerIndex], (MyColours[layerIndex]));
        if (R_WM_ERR_OK != wm_err)
        { 
            printf("[Test_WindowSettings] Failed to Write to FB window layer %d\n, WM Error = %d\n", layerIndex, wm_err);
            ret = 1;
            goto quit;
        }
                
        wm_err = R_WM_WindowEnable(DuUnit, &locWindows[layerIndex]);
        if (R_WM_ERR_OK != wm_err)
        { 
            printf("[Test_WindowSettings] Failed to Enable window layer %d\n, WM Error = %d\n", layerIndex, wm_err);
            ret = 1;
            goto quit;
        }
    }
 	printf("DISCOM(success) test for VSPD%d(UIF%d).\n",VspUnit,4);

	for(layerIndex = 0u; layerIndex < DISCOM_TEST_LAYERS; layerIndex++)
	{
		//discom for input image
		loc_DiscomCfg.UifSelect = (r_wm_DiscomUif_t)Select; 
		loc_DiscomCfg.Target    = (r_wm_DiscomTarget_t)locWindows[layerIndex].PosZ; 
		loc_DiscomCfg.Param.h_size    = locWindows[layerIndex].Width; 
		loc_DiscomCfg.Param.v_size    = locWindows[layerIndex].Height;  
		
		ptr = discom_execute(DuUnit, VspUnit, &loc_DiscomCfg, locWindows[layerIndex].Surface.Buffer[0], 1, 0);
		if(ptr != NULL){
            ret = 1;
            goto quit;
		}
	}
	loc_DiscomCfg.Param.v_size     = WidthMax; 
    loc_DiscomCfg.Param.h_size    = HeightMax;  
    
    layerIndex--; 
 	//discom for blend image
 	R_WM_VspdInfoGet(DuUnit, &vspd_info); 
    if (0 == vspd_info.BlendUnit)
    {
        loc_DiscomCfg.Target    = R_WM_DISCOM_TGT_BLEND; 
		ptr = discom_execute(DuUnit, VspUnit, &loc_DiscomCfg, locWindows[layerIndex].Surface.Buffer[0], 1, 0);
		ptr = discom_execute(DuUnit, VspUnit, &loc_DiscomCfg, locWindows[layerIndex].Surface.Buffer[0], 1, 1);
    }
    else
    {
		loc_DiscomCfg.Target    = R_WM_DISCOM_TGT_BLENDSUB; 
		ptr = discom_execute(DuUnit, VspUnit, &loc_DiscomCfg, locWindows[layerIndex].Surface.Buffer[0], 1, 0);
		ptr = discom_execute(DuUnit, VspUnit, &loc_DiscomCfg, locWindows[layerIndex].Surface.Buffer[0], 1, 1);
    }
	if(ptr != NULL)
	{
        ret = 1;
 
    }


	printf("DISCOM(failure) test for VSPD%d(UIF%d).\n",VspUnit,4);

	for(layerIndex = 0u; layerIndex < DISCOM_TEST_LAYERS; layerIndex++)
	{
		//discom for input image
		loc_DiscomCfg.Param.h_size  = locWindows[layerIndex].Width; 
		loc_DiscomCfg.Param.v_size  = locWindows[layerIndex].Height;  
		loc_DiscomCfg.UifSelect     = (r_wm_DiscomUif_t)Select; 
		loc_DiscomCfg.Target        = (r_wm_DiscomTarget_t)locWindows[layerIndex].PosZ; 
		ptr = discom_execute(DuUnit, VspUnit, &loc_DiscomCfg, locWindows[layerIndex].Surface.Buffer[0], 0, 0);
		if(ptr != NULL)
		{
            ret = 1;
 
	    }
	}
	loc_DiscomCfg.Param.v_size     = WidthMax; 
    loc_DiscomCfg.Param.h_size    = HeightMax;  
    
    layerIndex--; 

	//discom for blend image
    if (0 == vspd_info.BlendUnit)
    {
		loc_DiscomCfg.Target    = R_WM_DISCOM_TGT_BLEND; 
		ptr = discom_execute(DuUnit, VspUnit, &loc_DiscomCfg, locWindows[layerIndex].Surface.Buffer[0], 0, 0);
		ptr = discom_execute(DuUnit, VspUnit, &loc_DiscomCfg, locWindows[layerIndex].Surface.Buffer[0], 0, 1);
    }
    else
    {
		loc_DiscomCfg.Target    = R_WM_DISCOM_TGT_BLENDSUB; 
		ptr = discom_execute(DuUnit, VspUnit, &loc_DiscomCfg, locWindows[layerIndex].Surface.Buffer[0], 0, 0);
		ptr = discom_execute(DuUnit, VspUnit, &loc_DiscomCfg, locWindows[layerIndex].Surface.Buffer[0], 0, 1);
    }

	if(ptr != NULL)
	{
            ret = 1;
 	}
quit:
    for (layerIndex = 0; layerIndex < LayerNumMax; layerIndex ++)
    {
        wm_err = R_WM_Test_DeleteWindowSurface(DuUnit, &locWindows[layerIndex]);
    }
    free(locWindows);
    return ret;
}


/*******************************************************************************
  Function: loc_Test_DiscomParams
  
  This function test if the function R_WM_ScreenCmmSetProperty returns the expected errors
  when wrong input parameters are given.
  
*/
static uint32_t loc_Test_DiscomParams(void)
{
    uint32_t ret = 0;
    /* TO DO: implement the function parameter test for Discom Parameters*/
    return ret;
}

/*******************************************************************************
    Section: Global Functions
*/


/*******************************************************************************
    Function: R_WM_DiscomTest
    
    per default use DU 0
*/
uint32_t R_WM_DiscomTest(void) 
{
    r_wm_Error_t       wm_err = R_WM_ERR_OK;
    uint32_t           ret = 0;
    uint32_t           testRes = 0;
    uint32_t           i = 0; 
    uint32_t           dispUnit   = LOC_DISPLAY;
    uint32_t           LayerNumMax;
    uint32_t           PitchMax;
    uint32_t           WidthMax;
    uint32_t           HeightMax;
    int32_t            vspUnit;
    int32_t            vspDuLayer;
    r_wm_OutColorFmt_t ColorFormat;
    
    
    /* Initialise the WM for DU0 */
    wm_err = R_WM_Test_InitWM(dispUnit,
                              &LayerNumMax,
                              &PitchMax,
                              &WidthMax,
                              &HeightMax,
                              &ColorFormat);
    if (R_WM_ERR_OK != wm_err)
    {
        printf("[R_WM_Test_Windows] Failed to Init WM Dev %d\n, WM Error = %d\n", dispUnit, wm_err);
        ret = 1;
        goto quit;
    }
    
 
    /* Set screen, light green ;) */
    wm_err = R_WM_Test_InitScreen(dispUnit, 0,55,0); 

    if (R_WM_ERR_OK != wm_err)
    {
        printf("[R_WM_Test_Windows] Failed to setup Screen %d\n, WM Error = %d\n", dispUnit, wm_err);
        ret = 1;
        goto DisableWm;
    }
    
    /* get information on the display */
    wm_err = R_WM_DevInfoGet(dispUnit,
                             &LayerNumMax,
                             &PitchMax,
                             &WidthMax,
                             &HeightMax,
                             &ColorFormat,
                             &vspUnit,
                             &vspDuLayer);

    
    wm_err = R_WM_DevEventRegister(dispUnit, R_WM_EVENT_VBLANK, 0);
	
	loc_DiscomCfg.UifSelect = (r_wm_DiscomUif_t)0; 
	wm_err = R_WM_DiscomInit(dispUnit, &loc_DiscomCfg);
    if (R_WM_ERR_OK != wm_err)
    {
        printf("[discom_execute] Failed to Init Discom, WM Error = %d\n", wm_err);
        ret = 1;
        goto quit; 
    }
    
    
    CreateProtectedTask(1, (Address)loc_ErrTask0, 0x2000, "DiscomInt0", &children[0]);
    CreateProtectedTask(1, (Address)loc_ErrTask1, 0x2000, "DiscomInt1", &children[1]);
    CreateProtectedTask(1, (Address)loc_ErrTask2, 0x2000, "DiscomInt2", &children[2]);
#ifdef RCAR_H3
    CreateProtectedTask(1, (Address)loc_ErrTask3, 0x2000, "DiscomInt3", &children[3]);
#endif

	RunTask(children[vspUnit]);

#ifdef RCAR_H3
//    CreateProtectedTask(1, (Address)loc_ErrTask3, 0x2000, "DiscomInt3", &children[3]);
//	RunTask(children[3]);
#endif
    


    /* Test the WM Windows functions and their parameters */
    printf("\t[WM TEST]: Window functions parameter tests: \n");
    ret = loc_Test_DiscomParams();
    if(ret)
    {
        printf("\t\tFailed\n\n");
        testRes++; /* Failed */
    } else {
        printf("\t\tSuccess\n\n");
    }
    printf("\t[WM TEST]: Properties tests: \n");
    ret = loc_Test_Discom(dispUnit, vspUnit, LayerNumMax, WidthMax, HeightMax);
    if(ret)
    {
        printf("\t\tFailed\n\n");
        testRes++; /* Failed */
    } else {
        printf("\t\tSuccess\n\n");
    }

    wm_err = R_WM_DiscomDeInit(dispUnit);
    if (R_WM_ERR_OK != wm_err)
    {
        printf("[discom_execute] Failed to DeInit Discom, WM Error = %d\n", wm_err);
        ret = 1;
    }
	for (i = 0; i < R_VSP_MACRO_NUM; i++) 
	{
	    CloseTask(children[i]);
    }
    /* Disable Screen */
    wm_err |= R_WM_Test_DeInitScreen(dispUnit);
    
DisableWm:
    /* De - initialise the WM */
    wm_err |= R_WM_Test_DeInitWM(dispUnit);
    wm_err |= R_WM_DevDeinit(R_WM_DEV_TEXTURE);

quit:
    return ret;
}
