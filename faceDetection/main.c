/*
 * Copyright (C) 2017 GreenWaves Technologies
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 *
 */

#include <stdio.h>
#include "FaceDetKernels.h"
#include "cascade.h"
#include "face_cascade.h"
#include <limits.h>

#include "ili9341.h"
#include "Gap8.h"
#include "ImageDraw.h"
#include "ImgIO.h"
#include "setup.h"

#define LCD_OFF_X 40
#define LCD_OFF_Y 60

#define STACK_SIZE      1500
#define MOUNT           1
#define UNMOUNT         0
#define CID             0

#define FROM_FILE 1
#define SAVE_IMAGE 1
#define VERBOSE 1


RT_L2_DATA unsigned short *image16;
L2_MEM unsigned char *ImageIn;
L2_MEM unsigned char *ImageIn1;
L2_MEM unsigned char *ImageInOUT;
L2_MEM unsigned char *ImageOut;
L2_MEM unsigned int *ImageIntegral;
L2_MEM unsigned int *SquaredImageIntegral;

RT_FC_TINY_DATA rt_camera_t *camera1;
RT_FC_TINY_DATA rt_cam_conf_t cam1_conf;
RT_FC_TINY_DATA rt_spim_conf_t conf;


typedef struct cascade_answers{
	int x;
	int y;
	int w;
	int h;
	int score;
}cascade_reponse_t;

typedef struct ArgCluster {
	unsigned char*   	ImageIn;
	unsigned char*   	OutCamera;
	unsigned int 		Win;
	unsigned int 		Hin;
	unsigned char* 		ImageOut;
	unsigned int 		Wout;
	unsigned int 		Hout;
	unsigned int* 		ImageIntegral;
	unsigned int* 		SquaredImageIntegral;
	unsigned short *		image16;
	cascade_reponse_t* reponses;
	unsigned char num_reponse;
	int* output_map;
	cascade_t* model;
	unsigned int cycles;
	rt_perf_t *perf;	
} ArgCluster_T;


typedef struct {
  unsigned int	L2_Addr;
  unsigned int 	Size;
  unsigned int	imgReady;
  unsigned int	camReady;
} ImgDescriptor;

RT_L2_DATA ImgDescriptor ImageOutHeader;

static rt_perf_t *cluster_perf;

void camera_cristal_enable(){

  gap8SetOnePadAlternate(PLP_PAD_TIMER0_CH0, PLP_PAD_GPIO_ALTERNATE1);
  gap8SetGPIODir(gap8GiveGpioNb(PLP_PAD_TIMER0_CH0), GPIO_DIR_OUT);
  
  gap8WriteGPIO(gap8GiveGpioNb(PLP_PAD_TIMER0_CH0), 0x0);
  rt_time_wait_us(3000);

  gap8WriteGPIO(gap8GiveGpioNb(PLP_PAD_TIMER0_CH0), 0x1);
  rt_time_wait_us(3000);

}


void GPIO_2_en(){

  gap8SetOnePadAlternate(PLP_PAD_TIMER0_CH2, PLP_PAD_GPIO_ALTERNATE1);
  gap8SetGPIODir(gap8GiveGpioNb(PLP_PAD_TIMER0_CH2), GPIO_DIR_OUT);
  
}


static void cam_param_conf(rt_cam_conf_t *_conf){
  _conf->resolution = QVGA;
  _conf->format = HIMAX_MONO_COLOR;
  _conf->fps = fps30;
  _conf->slice_en = DISABLE;
  _conf->shift = 0;
  _conf->frameDrop_en = DISABLE;
  _conf->frameDrop_value = 0;
  _conf->cpiCfg = UDMA_CHANNEL_CFG_SIZE_16;
}

static void gray8_to_RGB565_tile(unsigned char *input,unsigned short *output,int width, int height,int x, int y,int img_width){

    for(int i=0;i<height;i++){
    	for(int j=0;j<width;j++){
    	output[(i*width)+j] = ((input[((i+y)*img_width)+j+x] >> 3 ) << 3) | ((input[((i+y)*img_width)+j+x] >> 5) ) | (((input[((i+y)*img_width)+j+x] >> 2 ) << 13) )|   ((input[((i+y)*img_width)+j+x] >> 3) <<8);
        }
    }
}

static void gray8_to_RGB565_half(unsigned char *input,unsigned short *output,int width, int height){

	for(int i=0;i<height;i++){
    	for(int j=0;j<width;j++){
    	output[((i/2)*(width/2))+(j/2)] = ((input[((i)*width)+j] >> 3 ) << 3) | ((input[((i)*width)+j] >> 5) ) | (((input[((i)*width)+j] >> 2 ) << 13) )|   ((input[((i)*width)+j] >> 3) <<8);

        }
    }
}
static void gray8_to_RGB565(unsigned char *input,unsigned short *output,int width, int height){
  for(int i=0;i<width*height;i++){
    output[i] = ((input[i] >> 3 ) << 3) | ((input[i] >> 5) ) | (((input[i] >> 2 ) << 13) )|   ((input[i] >> 3) <<8);

  }
}

void reset_camera(){
    //Usign GPIO1 to reset Camera
    gap8SetOnePadAlternate(PLP_PAD_TIMER0_CH1, PLP_PAD_GPIO_ALTERNATE1);
    gap8SetGPIODir(gap8GiveGpioNb(PLP_PAD_TIMER0_CH1), GPIO_DIR_OUT);
    gap8WriteGPIO(gap8GiveGpioNb(PLP_PAD_TIMER0_CH1), 0x0);
    rt_time_wait_us(1000);
    gap8WriteGPIO(gap8GiveGpioNb(PLP_PAD_TIMER0_CH1), 0x1);
    rt_time_wait_us(1000);
}

rt_spim_t *init_spi_lcd(){
    rt_spim_conf_init(&conf);
    conf.wordsize = RT_SPIM_WORDSIZE_8;
    conf.big_endian = 1;
    conf.max_baudrate = 50000000;
    conf.polarity = 0;
    conf.phase = 0;
    conf.id = 1;
    conf.cs = 0;

    rt_spim_t *spim = rt_spim_open(NULL, &conf, NULL);
    if (spim == NULL) return NULL;

    return spim;
}

void add_gwt_logo(rt_spim_t *spim){
	setCursor(30,0);
	writeText(spim,"GreenWaves\0",3);
	setCursor(10,3*8);
	writeText(spim,"Technologies\0",3);
}

cascade_t *getFaceCascade(){

	cascade_t *face_cascade;

	face_cascade = (cascade_t*) rt_alloc( RT_ALLOC_CL_DATA, sizeof(cascade_t));
	if(face_cascade==0){
		printf("Error allocatin model thresholds...");
		return 0;
	}
	single_cascade_t  **model_stages = (single_cascade_t  **) rt_alloc( RT_ALLOC_CL_DATA, sizeof(single_cascade_t*)*CASCADE_TOTAL_STAGES);

	face_cascade->stages_num = CASCADE_TOTAL_STAGES;
	face_cascade->thresholds = (signed short *) rt_alloc( RT_ALLOC_CL_DATA, sizeof(signed short )*face_cascade->stages_num);
	if(face_cascade->thresholds==0){
		printf("Error allocatin model thresholds...");
		return 0;
	}

	for(int a=0;a<face_cascade->stages_num;a++)
		face_cascade->thresholds[a] = model_thresholds[a];

	switch(CASCADE_TOTAL_STAGES){
		case 25:
			model_stages[24] = &stage_24;
		case 24:
			model_stages[23] = &stage_23;
		case 23:
			model_stages[22] = &stage_22;
		case 22:
			model_stages[21] = &stage_21;
		case 21:
			model_stages[20] = &stage_20;
		case 20:
			model_stages[19] = &stage_19;
		case 19:
			model_stages[18] = &stage_18;
		case 18:
			model_stages[17] = &stage_17;
		case 17:
			model_stages[16] = &stage_16;
		case 16:
			model_stages[15] = &stage_15;
		case 15:
			model_stages[14] = &stage_14;
		case 14:
			model_stages[13] = &stage_13;
		case 13:
			model_stages[12] = &stage_12;
		case 12:
			model_stages[11] = &stage_11;
		case 11:
			model_stages[10] = &stage_10;
		case 10:
			model_stages[9] = &stage_9;
		case 9:
			model_stages[8] = &stage_8;
		case 8:
			model_stages[7] = &stage_7;
		case 7:
			model_stages[6] = &stage_6;
		case 6:
			model_stages[5] = &stage_5;
		case 5:
			model_stages[4] = &stage_4;
		case 4:
			model_stages[3] = &stage_3;
		case 3:
			model_stages[2] = &stage_2;
		case 2:
			model_stages[1] = &stage_1;
		case 1:
			model_stages[0] = &stage_0;
		case 0:
			break;
	}

	face_cascade->stages = model_stages;
	return face_cascade;

}


static int biggest_cascade_stage(cascade_t *cascade){

	//Calculate cascade bigger layer
	int biggest_stage_size=0;
	int cur_layer;

	for (int i=0; i<cascade->stages_num; i++) {
		
		cur_layer = sizeof(cascade->stages[i]->stage_size) + 
					sizeof(cascade->stages[i]->rectangles_size) + 
					(cascade->stages[i]->stage_size*
						(sizeof(cascade->stages[i]->thresholds) +
						 sizeof(cascade->stages[i]->alpha1) +
						 sizeof(cascade->stages[i]->alpha2) +
						 sizeof(cascade->stages[i]->rect_num) 
						 )
					) +
					(cascade->stages[i]->rectangles_size*sizeof(cascade->stages[i]->rectangles)) +
					((cascade->stages[i]->rectangles_size/4)*sizeof(cascade->stages[i]->weights));

		if(cur_layer>biggest_stage_size)
			biggest_stage_size=cur_layer;
		//printf ("Stage size: %d\n",cur_layer);
	}

	return biggest_stage_size;
}



//Permanently Store a scascade stage to L1
single_cascade_t* sync_copy_cascade_stage_to_l1(single_cascade_t* cascade_l2){

	rt_dma_copy_t DmaR_Evt1;

	single_cascade_t* cascade_l1;
	cascade_l1 = (single_cascade_t* )rt_alloc( RT_ALLOC_CL_DATA, sizeof(single_cascade_t));

	cascade_l1->stage_size = cascade_l2->stage_size;
	cascade_l1->rectangles_size = cascade_l2->rectangles_size;


	cascade_l1->thresholds     = (short*)rt_alloc( RT_ALLOC_CL_DATA, sizeof(short)*cascade_l2->stage_size);
	rt_dma_memcpy((unsigned int) cascade_l2->thresholds, (unsigned int) cascade_l1->thresholds, sizeof(short)*cascade_l1->stage_size, RT_DMA_DIR_EXT2LOC, 0, &DmaR_Evt1);
	rt_dma_wait(&DmaR_Evt1);


	cascade_l1->alpha1         = (short*)rt_alloc( RT_ALLOC_CL_DATA, sizeof(short)*cascade_l2->stage_size);
	rt_dma_memcpy((unsigned int) cascade_l2->alpha1, (unsigned int) cascade_l1->alpha1, sizeof(short)*cascade_l1->stage_size, RT_DMA_DIR_EXT2LOC, 0, &DmaR_Evt1);
	rt_dma_wait(&DmaR_Evt1);

	cascade_l1->alpha2         = (short*)rt_alloc( RT_ALLOC_CL_DATA, sizeof(short)*cascade_l2->stage_size);
	rt_dma_memcpy((unsigned int) cascade_l2->alpha2, (unsigned int) cascade_l1->alpha2, sizeof(short)*cascade_l1->stage_size, RT_DMA_DIR_EXT2LOC, 0, &DmaR_Evt1);
	rt_dma_wait(&DmaR_Evt1);

	cascade_l1->rect_num       = (unsigned  short*)rt_alloc( RT_ALLOC_CL_DATA, sizeof(unsigned short)*((cascade_l2->stage_size)+1));
	rt_dma_memcpy((unsigned int) cascade_l2->rect_num, (unsigned int) cascade_l1->rect_num, sizeof(unsigned short)*(cascade_l1->stage_size+1), RT_DMA_DIR_EXT2LOC, 0, &DmaR_Evt1);
	rt_dma_wait(&DmaR_Evt1);

	
	cascade_l1->weights    = (signed char*)rt_alloc( RT_ALLOC_CL_DATA, sizeof(signed char)*(cascade_l2->rectangles_size/4));
	rt_dma_memcpy((unsigned int) cascade_l2->weights, (unsigned int) cascade_l1->weights, sizeof(signed char)*(cascade_l2->rectangles_size/4), RT_DMA_DIR_EXT2LOC, 0, &DmaR_Evt1);
	rt_dma_wait(&DmaR_Evt1);


	cascade_l1->rectangles = (char*)rt_alloc( RT_ALLOC_CL_DATA, sizeof(char)*cascade_l2->rectangles_size);
	rt_dma_memcpy((unsigned int) cascade_l2->rectangles, (unsigned int) cascade_l1->rectangles, sizeof(char)*cascade_l2->rectangles_size, RT_DMA_DIR_EXT2LOC, 0, &DmaR_Evt1);
	rt_dma_wait(&DmaR_Evt1);

	if(cascade_l1->rectangles==0)
		printf("Allocation Error...\n");

	return cascade_l1;
}



int rect_intersect_area( unsigned short a_x, unsigned short a_y, unsigned short a_w, unsigned short a_h,
						 unsigned short b_x, unsigned short b_y, unsigned short b_w, unsigned short b_h ){

	#define MIN(a,b) ((a) < (b) ? (a) : (b))
    #define MAX(a,b) ((a) > (b) ? (a) : (b))

    int x = MAX(a_x,b_x);
    int y = MAX(a_y,b_y);

    int size_x = MIN(a_x+a_w,b_x+b_w) - x;
    int size_y = MIN(a_y+a_h,b_y+b_h) - y;

    if(size_x <=0 || size_x <=0)
        return 0;
    else
        return size_x*size_y; 

    #undef MAX
    #undef MIN
}


void non_max_suppress(cascade_reponse_t* reponses, int reponse_idx){

	int idx,idx_int;

    //Non-max supression
     for(idx=0;idx<reponse_idx;idx++){
        //check if rect has been removed (-1)
        if(reponses[idx].x==-1)
            continue;
 
        for(idx_int=0;idx_int<reponse_idx;idx_int++){

            if(reponses[idx_int].x==-1 || idx_int==idx)
                continue;

            //check the intersection between rects
            int intersection = rect_intersect_area(reponses[idx].x,reponses[idx].y,reponses[idx].w,reponses[idx].h,
               									   reponses[idx_int].x,reponses[idx_int].y,reponses[idx_int].w,reponses[idx_int].h);

            if(intersection >= NON_MAX_THRES){ //is non-max
                //supress the one that has lower score
                if(reponses[idx_int].score > reponses[idx].score){
                    reponses[idx].x = -1;
                    reponses[idx].y = -1;
                }
                else{
                    reponses[idx_int].x = -1;
                    reponses[idx_int].y = -1;
                }
            }
        }
    }
}

static void cluster_init(ArgCluster_T *ArgC){

	//printf ("Cluster Init start\n");
	FaceDet_L1_Memory = (char *) rt_alloc( RT_ALLOC_CL_DATA, _FaceDet_L1_Memory_SIZE);
	if (FaceDet_L1_Memory == 0) {
		printf("Failed to allocate %d bytes for L1_memory\n", _FaceDet_L1_Memory_SIZE); return ;
	}

	ArgC->output_map = rt_alloc(RT_ALLOC_L2_CL_DATA, sizeof(unsigned int)*(ArgC->Hout-24+1)*(ArgC->Wout-24+1));

	if(ArgC->output_map==0){
		printf("Error Allocating output buffer...");
		return;
	}
	//Get Cascade Model
	ArgC->model = getFaceCascade();

	int max_cascade_size = biggest_cascade_stage(ArgC->model);
	//printf("Max cascade size:%d\n",max_cascade_size);

	for(int i=0;i<CASCADE_STAGES_L1;i++)
		ArgC->model->stages[i] = sync_copy_cascade_stage_to_l1((ArgC->model->stages[i]));

	//Assigning space to cascade buffers

	ArgC->model->buffers_l1[0] = rt_alloc(RT_ALLOC_CL_DATA, max_cascade_size);
	ArgC->model->buffers_l1[1] = rt_alloc(RT_ALLOC_CL_DATA, max_cascade_size);

	if(ArgC->model->buffers_l1[0]==0 ){
		printf("Error allocating cascade buffer 0...\n");
	}
	if(
	ArgC->model->buffers_l1[1] == 0){
		printf("Error allocating cascade buffer 1...\n");
	}

	ArgC->reponses = (cascade_reponse_t*)rt_alloc(RT_ALLOC_L2_CL_DATA, sizeof(cascade_reponse_t)*MAX_NUM_OUT_WINS);

	#ifdef PERF_COUNT
	ArgC->perf = cluster_perf;
	// initialize the performance clock
	rt_perf_init(ArgC->perf );
	// Configure performance counters for counting the cycles
	rt_perf_conf(ArgC->perf , (1<<RT_PERF_CYCLES));
	//printf("Cluster core %d Launched, %d cores configuration\n", 1, rt_nb_pe());
	#endif

}

static inline unsigned int __attribute__((always_inline)) ChunkSize(unsigned int X)

{
	unsigned int NCore;
	unsigned int Log2Core;
	unsigned int Chunk;

	NCore = rt_nb_pe();
	Log2Core = gap8_fl1(NCore);
	Chunk = (X>>Log2Core) + ((X&(NCore-1))!=0);
	return Chunk;
}


void gray8_to_RGB565_cluster_half(ArgCluster_T *ArgC){

  	unsigned int width = ArgC->Win;
  	unsigned int height = ArgC->Hin;

  	unsigned int CoreId = rt_core_id();
  	unsigned int ChunkCell = ChunkSize(height);
  	unsigned int First = CoreId*ChunkCell, Last  = Min(height, First+ChunkCell);
	for(unsigned int i=First;i<Last;i++){
   		for(unsigned int j=0;j<width;j++){
    	ArgC->image16[((i/2)*(width/2))+(j/2)] = ((ArgC->ImageIn[((i)*width)+j] >> 3 ) << 3) | ((ArgC->ImageIn[((i)*width)+j] >> 5) ) | (((ArgC->ImageIn[((i)*width)+j] >> 2 ) << 13) )|   ((ArgC->ImageIn[((i)*width)+j] >> 3) <<8);
		}
	}
}


void gray8_to_RGB565_cluster(ArgCluster_T *ArgC){

  unsigned int width = ArgC->Win;
  unsigned int height = ArgC->Hin;

  unsigned int CoreId = rt_core_id();
  unsigned int ChunkCell = ChunkSize(width*height);
  unsigned int First = CoreId*ChunkCell, Last  = Min(width*height, First+ChunkCell);

  for(unsigned int i=First;i<Last;i++){
    ArgC->image16[i] = ((ArgC->ImageIn[i] >> 3 ) << 3) | ((ArgC->ImageIn[i] >> 5) ) | (((ArgC->ImageIn[i] >> 2 ) << 13) )|   ((ArgC->ImageIn[i] >> 3) <<8);
  }

}

void copy_image(ArgCluster_T *ArgC){

  unsigned int width = ArgC->Win;
  unsigned int height = ArgC->Hin;

  unsigned int CoreId = rt_core_id();
  unsigned int ChunkCell = ChunkSize(width*height);
  unsigned int First = CoreId*ChunkCell, Last  = Min(width*height, First+ChunkCell);

  for(unsigned int i=First;i<Last;i++){
    ArgC->ImageIn[i] = ArgC->OutCamera[i];
  }

}

static void cluster_main(ArgCluster_T *ArgC)
{
	//printf ("Cluster master start\n");
	
	unsigned int Win=ArgC->Win, Hin=ArgC->Hin;
	unsigned int Wout, Hout;

	unsigned int i, MaxCore = rt_nb_pe();

	//create structure for output
	cascade_reponse_t* reponses = ArgC->reponses;
	for(int i=0;i<MAX_NUM_OUT_WINS;i++)reponses[i].x=-1;
	int reponse_idx = 0;

	Wout=64;
	Hout=48;

	int result;
	#if !FROM_FILE
	rt_team_fork(__builtin_pulp_CoreCount(), (void *) copy_image, (void *) ArgC);
	#endif

	#ifdef PERF_COUNT
	gap8_resethwtimer();
	unsigned int Ta = gap8_readhwtimer();
	#endif
	

#ifdef ENABLE_LAYER_1	
	ResizeImage_1(ArgC->ImageIn,ArgC->ImageOut,NULL);
	ProcessIntegralImage_1(ArgC->ImageOut,ArgC->ImageIntegral,NULL);
	ProcessSquaredIntegralImage_1(ArgC->ImageOut,ArgC->SquaredImageIntegral,NULL);	
	ProcessCascade_1(ArgC->ImageIntegral,ArgC->SquaredImageIntegral,ArgC->model, ArgC->output_map, NULL);

	for(unsigned int i=0;i<Hout-24+1;i+=DETECT_STRIDE)
		for(unsigned int j=0;j<Wout-24+1;j+=DETECT_STRIDE){
			
			result = ArgC->output_map[i*(Wout-24+1)+j];

			if(result!=0){
				reponses[reponse_idx].x       = (j*Win)/Wout;
				reponses[reponse_idx].y       = (i*Hin)/Hout;
				reponses[reponse_idx].w = (24*Win)/Wout;
				reponses[reponse_idx].h = (24*Hin)/Hout;
				reponses[reponse_idx].score   = result;
				reponse_idx++;
				//	printf("Face Found in %dx%d at X: %d, Y: %d - value: %d\n",Wout,Hout,j,i,result);
			}
	}
#endif

	Wout /= 1.25, Hout /= 1.25;

#ifdef ENABLE_LAYER_2
	ResizeImage_2(ArgC->ImageIn,ArgC->ImageOut,NULL);
	ProcessIntegralImage_2(ArgC->ImageOut,ArgC->ImageIntegral,NULL);
	ProcessSquaredIntegralImage_2(ArgC->ImageOut,ArgC->SquaredImageIntegral,NULL);
	ProcessCascade_2(ArgC->ImageIntegral,ArgC->SquaredImageIntegral,ArgC->model, ArgC->output_map, NULL);

	for(unsigned int i=0;i<Hout-24+1;i+=DETECT_STRIDE)
		for(unsigned int j=0;j<Wout-24+1;j+=DETECT_STRIDE){
			
			result = ArgC->output_map[i*(Wout-24+1)+j];

			if(result!=0){
				reponses[reponse_idx].x     = (j*Win)/Wout;
				reponses[reponse_idx].y     = (i*Hin)/Hout;
				reponses[reponse_idx].w 	= (24*Win)/Wout;
				reponses[reponse_idx].h 	= (24*Hin)/Hout;
				reponses[reponse_idx].score = result;
				reponse_idx++;
				//	printf("Face Found in %dx%d at X: %d, Y: %d - value: %d\n",Wout,Hout,j,i,result);
			}
	}
#endif

	Wout /= 1.25, Hout /= 1.25;

#ifdef ENABLE_LAYER_3
	ResizeImage_3(ArgC->ImageIn,ArgC->ImageOut,NULL);
	ProcessIntegralImage_3(ArgC->ImageOut,ArgC->ImageIntegral,NULL);
	ProcessSquaredIntegralImage_3(ArgC->ImageOut,ArgC->SquaredImageIntegral,NULL);
	ProcessCascade_3(ArgC->ImageIntegral,ArgC->SquaredImageIntegral,ArgC->model, ArgC->output_map, NULL);
	for(unsigned int i=0;i<Hout-24+1;i+=DETECT_STRIDE)
		for(unsigned int j=0;j<Wout-24+1;j+=DETECT_STRIDE){
			
			result = ArgC->output_map[i*(Wout-24+1)+j];

			if(result!=0){
				reponses[reponse_idx].x     = (j*Win)/Wout;
				reponses[reponse_idx].y     = (i*Hin)/Hout;
				reponses[reponse_idx].w 	= (24*Win)/Wout;
				reponses[reponse_idx].h 	= (24*Hin)/Hout;
				reponses[reponse_idx].score = result;
				reponse_idx++;
				//	printf("Face Found in %dx%d at X: %d, Y: %d - value: %d\n",Wout,Hout,j,i,result);
			}
	}
	
#endif

	non_max_suppress(reponses,reponse_idx);

	#ifdef PERF_COUNT
	unsigned int Ti = gap8_readhwtimer();
	ArgC->cycles = Ti-Ta;
	#endif
	
	unsigned int real_detections=0;
	ArgC->num_reponse=reponse_idx;
	for(int i=0;i<reponse_idx;i++)
		if(reponses[i].x!=-1){
			DrawRectangle(ArgC->ImageIn, Hin, Win,  reponses[i].x, reponses[i].y, reponses[i].w, reponses[i].h, 0);
			DrawRectangle(ArgC->ImageIn, Hin, Win,  reponses[i].x-1, reponses[i].y-1, reponses[i].w+2, reponses[i].h+2, 255);
			DrawRectangle(ArgC->ImageIn, Hin, Win,  reponses[i].x-2, reponses[i].y-2, reponses[i].w+4, reponses[i].h+4, 255);
			DrawRectangle(ArgC->ImageIn, Hin, Win,  reponses[i].x-3, reponses[i].y-3, reponses[i].w+6, reponses[i].h+6, 0);
			//DrawRectangle(ArgC->ImageIn, Hin, Win,  reponses[i].x-4, reponses[i].y-4, reponses[i].w+8, reponses[i].h+8, 255);
			real_detections++;
			//printf("x:%d y:%d w:%d h:%d\n",reponses[i].x, reponses[i].y, reponses[i].w, reponses[i].h);
		}
	//ResizeImage_1(ArgC->ImageIn,ArgC->ImageOut,NULL);


#if !FROM_FILE
	//Converting image to RGB 565 for LCD screen and binning image to half the size
	rt_team_fork(__builtin_pulp_CoreCount(), (void *) gray8_to_RGB565_cluster_half, (void *) ArgC);
#else
	#ifdef VERBOSE
	printf("Total detections: %d \n", real_detections);
	printf("[W:%d, H:%d] Done in %d cycles at %d cycles per pixel....\n", Win, Hin, Ti, Ti/(Win*Hin));
	#endif
#endif
}


int main(int argc, char *argv[])

{

	unsigned int W = 324, H = 244;
	unsigned int Wout = 64, Hout = 48;
	unsigned int ImgSize = W*H;
	char out_perf_string[120];
	ArgCluster_T ClusterCall;

#if FROM_FILE
	char *Imagefile = "img51.pgm";
	char imageName[64];
	sprintf(imageName, "../../../testImages/%s", Imagefile);
#endif

	printf("Start faceDection Demo Application\n");

	if (rt_event_alloc(NULL, 16)) return -1;
	//Setting FC to 250MHz
	rt_freq_set(RT_FREQ_DOMAIN_FC, 250000000);

	// Activate the Cluster
	rt_cluster_mount(MOUNT, CID, 0, NULL);
	//Setting Cluster to 175MHz
	rt_freq_set(RT_FREQ_DOMAIN_CL, 175000000);

	// Allocate the memory of L2 for the performance structure
	cluster_perf = rt_alloc(RT_ALLOC_L2_CL_DATA, sizeof(rt_perf_t));
	if (cluster_perf == NULL) return -1;

	// Allocate some stacks for cluster in L1, rt_nb_pe returns how many cores exist.
	void *stacks = rt_alloc(RT_ALLOC_CL_DATA, STACK_SIZE*rt_nb_pe());
	if (stacks == NULL) return -1;

	ImageIn              = (unsigned char *) rt_alloc( RT_ALLOC_L2_CL_DATA, W*H*sizeof(unsigned char));	
	ImageIn1             = (unsigned char *) rt_alloc( RT_ALLOC_L2_CL_DATA, W*H*sizeof(unsigned char));	
	
	ImageOut             = (unsigned char *) rt_alloc( RT_ALLOC_L2_CL_DATA, (Wout*Hout)*sizeof(unsigned char));
	ImageIntegral        = (unsigned int *)  rt_alloc( RT_ALLOC_L2_CL_DATA, (Wout*Hout)*sizeof(unsigned int));
	SquaredImageIntegral = (unsigned int *)  rt_alloc( RT_ALLOC_L2_CL_DATA, (Wout*Hout)*sizeof(unsigned int));
	image16              = (unsigned short *)  rt_alloc( RT_ALLOC_L2_CL_DATA, ((W/2)*(H/2))*sizeof(unsigned short));
	
	if (ImageIn==0 || ImageOut==0 || ImageIn1==0 ) {
		printf("Failed to allocate Memory for Image (%d bytes)\n", ImgSize*sizeof(unsigned char));
		return 1;
	}
	if (ImageIntegral==0 || SquaredImageIntegral==0) {
		printf("Failed to allocate Memory for one or both Integral Images (%d bytes)\n", ImgSize*sizeof(unsigned int));
		return 1;
	}

	#if FROM_FILE
	rt_bridge_connect(NULL);

	{

		unsigned int Wi, Hi;
		if ((ReadImageFromFile(imageName, &Wi, &Hi, ImageIn, W*H*sizeof(unsigned char))==0) || (Wi!=W) || (Hi!=H))
		{
			printf("Failed to load image %s or dimension mismatch Expects [%dx%d], Got [%dx%d]\n", imageName, W, H, Wi, Hi);
			return 1;
		}
	}

#else
	

	camera_cristal_enable();
	cam_param_conf(&cam1_conf);
	camera1 = rt_camera_open("camera", &cam1_conf, 0);
	if (camera1 == NULL) return -1;
	
	//Init SPI for LCD Screen
	rt_spim_t *spim = init_spi_lcd();
	ILI9341_begin(spim);
	writeFillRect(spim, 0,0,240,320,ILI9341_WHITE);
	setRotation(spim,2);
	
	//Camera
	rt_cam_control(camera1, CMD_INIT, 0);
	rt_cam_control(camera1, CMD_START, 0);


	himaxRegWrite(camera1,0x1003, 0x00);             //  Black level target
    
    himaxRegWrite(camera1,AE_TARGET_MEAN, 0x8C);      //AE target mean [Def: 0x3C]
    himaxRegWrite(camera1,AE_MIN_MEAN,    0x3A);      //AE min target mean [Def: 0x0A]
    himaxRegWrite(camera1,INTEGRATION_H,  0xa8);      //Integration H [Def: 0x01]
    himaxRegWrite(camera1,INTEGRATION_L,  0x08);      //Integration L [Def: 0x08]
    himaxRegWrite(camera1,ANALOG_GAIN,    0x08);      //Analog Global Gain
    himaxRegWrite(camera1,DAMPING_FACTOR, 0x00);      //Damping Factor [Def: 0x20]
    himaxRegWrite(camera1,DIGITAL_GAIN_H, 0x01);      //Digital Gain High [Def: 0x01]
    himaxRegWrite(camera1,DIGITAL_GAIN_L, 0x00);      //Digital Gain Low [Def: 0x00]
    himaxRegWrite(camera1,SINGLE_THR_HOT, 0xA0);     //  single hot pixel th
    himaxRegWrite(camera1,SINGLE_THR_COLD,0x30);    //  single cold pixel th

	rt_event_t *event_0 = rt_event_get_blocking(NULL);
	//Configure the DMA in continuous mode
	camera1->conf.cpiCfg |= 1;
	
	rt_camera_capture (camera1, ImageIn1, W*H, event_0);
	
	rt_event_wait(event_0);
#endif
	
	ClusterCall.OutCamera            = ImageIn1;
	ClusterCall.ImageIn              = ImageIn;
	ClusterCall.Win                  = W;
	ClusterCall.Hin                  = H;
	ClusterCall.Wout                 = Wout;
	ClusterCall.Hout                 = Hout;
	ClusterCall.ImageOut             = ImageOut;
	ClusterCall.ImageIntegral        = ImageIntegral;
	ClusterCall.SquaredImageIntegral = SquaredImageIntegral;
	ClusterCall.image16              = image16;

#if !FROM_FILE

	add_gwt_logo(spim);
	setCursor(0,200);
	sprintf(out_perf_string,"1 Image/Sec: \n       uWatt @ 1.2V\n       uWatt @ 1.0V");
	writeText(spim,out_perf_string,2);
#endif

    //Cluster Init
	rt_cluster_call(NULL, CID, (void (*)(void *)) cluster_init, &ClusterCall, stacks, STACK_SIZE, STACK_SIZE, rt_nb_pe(), NULL);
	
#if !FROM_FILE	
  	while(1){
#endif
  		//Elaborating input image
		rt_cluster_call(NULL, CID, (void (*)(void *)) cluster_main, &ClusterCall, stacks, STACK_SIZE, STACK_SIZE, rt_nb_pe(), NULL);

#if !FROM_FILE	
		//Printing cycles to screen		
		sprintf(out_perf_string,"%d  \n%d  ", (int)((float)(1/(50000000.f/ClusterCall.cycles)) * 28000.f),(int)((float)(1/(50000000.f/ClusterCall.cycles)) * 16800.f));
  		setCursor(0,200+2*8);
  		writeText(spim,out_perf_string,2);
  		//Pushing image to screen
  		lcd_pushPixels(spim, LCD_OFF_X, LCD_OFF_Y, W/2,H/2, image16);
		
  	}
#endif

	#if SAVE_IMAGE
		sprintf(imageName, "../../../%s", Imagefile);
    	printf("imgName: %s\n", imageName);
    	WriteImageToFile(imageName,W,H,(ImageIn));	
	#endif
  	
	// Close the cluster
	rt_cluster_mount(UNMOUNT, CID, 0, NULL);
#if FROM_FILE	
	rt_bridge_disconnect(NULL);
#endif

	return 0;
}

