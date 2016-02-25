
#if defined(BUILD_LK)
#include <string.h>
#else
#include <linux/string.h>
#endif


#if defined(BUILD_LK)
#include "cust_gpio_usage.h"
#include <platform/mt_gpio.h>
#else
#include "cust_gpio_usage.h"
#include <mach/mt_gpio.h>
#endif

#include "lcm_drv.h"
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  										(480)
#define FRAME_HEIGHT 										(854)
#define LCM_ID       (0x00)
#define REGFLAG_DELAY             							0XFB
#define REGFLAG_END_OF_TABLE      							0xFA   // END OF REGISTERS MARKER

#define LCM_DSI_CMD_MODE						0

#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
#endif
#define LCM_TDT			0

static unsigned int lcm_esd_test = FALSE;      ///only for ESD test
//set LCM IC ID
#define LCM_ID_ILI9806E 	(0x980604)
//#define GPIO_LCM_ID1	(GPIO81 | 0x80000000)
//#define GPIO_LCM_ID2	(GPIO82 | 0x80000000)
#define GPIO_LCM_ID1	GPIO81_LCD_ID1
#define GPIO_LCM_ID2	GPIO82_LCD_ID2

//#define LCM_ESD_DEBUG


/*--------------------------LCD module explaination begin---------------------------------------*/

//LCD module explaination				//Project		Custom		W&H		Glass	degree	data		HWversion


/*--------------------------LCD module explaination end----------------------------------------*/


// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	        lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)											lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

struct LCM_setting_table {
    unsigned char cmd;
    unsigned char count;
    unsigned char para_list[64];
};


static struct LCM_setting_table lcm_initialization_setting[] = {

	//****************************************************************************//
	//****************************** Page 1 Command ******************************//
	//****************************************************************************
	{0xFF,5,{0xFF,0x98,0x06,0x04,0x01}},     // Change to Page 1
		
	{0x08,1,{0x10}},               // output SDA
		
	{0x21,1,{0x01}},               // DE = 1 Active
		
	{0x30,1,{0x01}},               // 480 X 854
		
	{0x31,1,{0x00}},               // 2-dot Inversion
		
	{0x40,1,{0x18}},               // DDVDH/DDVDL 
		
	{0x41,1,{0x77}},               // DDVDH/DDVDL CP
		
	{0x42,1,{0x02}},               // VGH/VGL 
		
	{0x43,1,{0x09}},               // VGH CP 
		
	{0x44,1,{0x06}},               // VGL CP
		
	{0x45,1,{0x06}},
		
	{0x50,1,{0x78}},               // VGMP
		
	{0x51,1,{0x78}},               // VGMN
		
	{0x52,1,{0x00}},                 //Flicker
		
	{0x53,1,{0x54}},                 //Flicker
		
	{0x57,1,{0x50}},             
		
	{0x60,1,{0x07}},               // SDTI
		
	{0x61,1,{0x00}},              // CRTI
		
	{0x62,1,{0x07}},               // EQTI
		
	{0x63,1,{0x00}},              // PCTI
		
	//++++++++++++++++++ Gamma Setting ++++++++++++++++++//
	{0xA0,1,{0x00}},  // Gamma 0 
		
	{0xA1,1,{0x08}},  // Gamma 4 
		
	{0xA2,1,{0x13}},  // Gamma 8
		
	{0xA3,1,{0x0F}},  // Gamma 16
		
	{0xA4,1,{0x09}},  // Gamma 24
		
	{0xA5,1,{0x16}},  // Gamma 52
		
	{0xA6,1,{0x0B}},  // Gamma 80
		
	{0xA7,1,{0x08}},  // Gamma 108
		
	{0xA8,1,{0x04}},  // Gamma 147
		
	{0xA9,1,{0x0B}},  // Gamma 175
		
	{0xAA,1,{0x06}},  // Gamma 203
		
	{0xAB,1,{0x08}},  // Gamma 231
		
	{0xAC,1,{0x0D}},  // Gamma 239
		
	{0xAD,1,{0x36}},  // Gamma 247
		
	{0xAE,1,{0x27}},  // Gamma 251
		
	{0xAF,1,{0x00}},  // Gamma 255
		
	///==============Nagitive
	{0xC0,1,{0x00}},  // Gamma 0 
		
	{0xC1,1,{0x06}},  // Gamma 4
		
	{0xC2,1,{0x0F}},  // Gamma 8
		
	{0xC3,1,{0x0F}},  // Gamma 16
		
	{0xC4,1,{0x08}},  // Gamma 24
		
	{0xC5,1,{0x16}},  // Gamma 52
		
	{0xC6,1,{0x05}},  // Gamma 80
		
	{0xC7,1,{0x04}},  // Gamma 108
		
	{0xC8,1,{0x04}},  // Gamma 147
		
	{0xC9,1,{0x08}},  // Gamma 175
		
	{0xCA,1,{0x06}},  // Gamma 203
		
	{0xCB,1,{0x02}},  // Gamma 231
		
	{0xCC,1,{0x08}},  // Gamma 239
		
	{0xCD,1,{0x25}},  // Gamma 247
		
	{0xCE,1,{0x27}},  // Gamma 251
		
	{0xCF,1,{0x00}},  // Gamma 255
		


	//****************************************************************************//
	//****************************** Page 6 Command ******************************//
	//****************************************************************************//
	{0xFF,5,{0xFF,0x98,0x06,0x04,0x06}},     // Change to Page 6
		
	{0x00,1,{0x23}},
		
	{0x01,1,{0x0A}},
		
	{0x02,1,{0x00}},    
		
	{0x03,1,{0x04}},
		
	{0x04,1,{0x01}},
		
	{0x05,1,{0x01}},
		
	{0x06,1,{0x80}},    
		
	{0x07,1,{0x08}},
		
	{0x08,1,{0x09}},   //0A
		
	{0x09,1,{0x00}},    //
		
	{0x0A,1,{0x00}},    
		
	{0x0B,1,{0x00}},    
		
	{0x0C,1,{0x01}},
		
	{0x0D,1,{0x01}},
		
	{0x0E,1,{0x00}},
		
	{0x0F,1,{0x00}},
		
	{0x10,1,{0x7E}},
		
	{0x11,1,{0xF3}},
		
	{0x12,1,{0x04}},
		
	{0x13,1,{0x00}},
		
	{0x14,1,{0x00}},
		
	{0x15,1,{0xC0}},
		
	{0x16,1,{0x08}},
		
	{0x17,1,{0x00}},
		
	{0x18,1,{0x00}},
		
	{0x19,1,{0x00}},
		
	{0x1A,1,{0x00}},
		
	{0x1B,1,{0x00}},
		
	{0x1C,1,{0x00}},
		
	{0x1D,1,{0x00}},
		
		
	{0x20,1,{0x01}},
		
	{0x21,1,{0x23}},
		
	{0x22,1,{0x45}},
		
	{0x23,1,{0x67}},
		
	{0x24,1,{0x01}},
		
	{0x25,1,{0x23}},
		
	{0x26,1,{0x45}},
		
	{0x27,1,{0x67}},
		
		
	{0x30,1,{0x13}},
		
	{0x31,1,{0x22}},
		
	{0x32,1,{0x22}},
		
	{0x33,1,{0x96}},
		
	{0x34,1,{0xDA}},
		
	{0x35,1,{0xAB}},
		
	{0x36,1,{0xBC}},
		
	{0x37,1,{0xCD}},
		
	{0x38,1,{0x22}},
		
	{0x39,1,{0xFE}},
		
	{0x3A,1,{0xEF}},
		
	{0x3B,1,{0x22}},
		
	{0x3C,1,{0x68}},
		
	{0x3D,1,{0x22}},
		
	{0x3E,1,{0x22}},
		
	{0x3F,1,{0x89}},
		
	{0x40,1,{0x22}},
		
	{0x52,1,{0x10}},
		
	{0x53,1,{0x12}},


	//****************************************************************************//
	{0xFF,5,{0xFF,0x98,0x06,0x04,0x07}},     // Change to Page 7

	{0x02,1,{0x77}},
		
	{0xE1,1,{0x79}}, 
		                  
	{0x17,1,{0x32}},  
		
	{0x68,1,{0x01}}, 
		
	{0x06,1,{0x13}},
	//****************************************************************************//
	{0xFF,5,{0xFF,0x98,0x06,0x04,0x00}},     // Change to Page 0
		
	{0x11,1,{0x00}},                 // Sleep-Out
	{REGFLAG_DELAY, 120, {}},		
	{0x29,1,{0x00}},                 // Display On
	{REGFLAG_DELAY, 50, {}},
	//{0xFF,5,{0xFF,0x98,0x06,0x04,0x06}},     // Change to Page 6

	{REGFLAG_END_OF_TABLE, 0x00, {}}  
 
};


static struct LCM_setting_table lcm_sleep_out_setting[] = {
	{0xFF,5,{0xFF,0x98,0x06,0x04,0x00}}, // Change to Page 0 
	{0x11,1,{0x00}}, // Sleep-Out 
	{REGFLAG_DELAY, 120, {}},//DELAY,120 
	{0x29,1,{0x00}}, // Display On 
	{REGFLAG_DELAY, 50, {}},//DELAY,50	
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
    	{0xFF,5,{0xFF,0x98,0x06,0x04,0x00}},
	// Display off sequence
	{0x28, 1, {0x00}},
        {REGFLAG_DELAY, 50, {}},

    	// Sleep Mode On
	{0x10, 1, {0x00}},
        {REGFLAG_DELAY, 120, {}},
	//{0xFF,	5,	{0xFF, 0x98, 0x06, 0x04, 0x01}},// Change to Page 1
	//{0x58,  1, 	{0x91}},//0x00 or 0x91;
	//{REGFLAG_DELAY, 200, {}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;

    for(i = 0; i < count; i++) {

        unsigned cmd;
        cmd = table[i].cmd;

        switch (cmd) {

            case REGFLAG_DELAY :
                MDELAY(table[i].count);
                break;

            case REGFLAG_END_OF_TABLE :
                break;

            default:
		dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
       	}
    }

}


// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
	memset(params, 0, sizeof(LCM_PARAMS));

	params->type   = LCM_TYPE_DSI;

	params->width  = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;

	// enable tearing-free
	//params->dbi.te_mode 			= LCM_DBI_TE_MODE_VSYNC_ONLY;
	//params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;
	params->dbi.te_mode 				= LCM_DBI_TE_MODE_DISABLED;

	#if (LCM_DSI_CMD_MODE)
	params->dsi.mode   = CMD_MODE;
	#else
	params->dsi.mode   = SYNC_PULSE_VDO_MODE;
	#endif

	// DSI
	/* Command mode setting */
	params->dsi.LANE_NUM				= LCM_TWO_LANE;
	//The following defined the fomat for data coming from LCD engine.
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

	// Highly depends on LCD driver capability.
	params->dsi.packet_size=256;

	// Video mode setting	

	// add by zhuqiang for FR437058 at 2013.4.25 begin
	params->dsi.intermediat_buffer_num = 2;	
	// add by zhuqiang for FR437058 at 2013.4.25 end
	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

	params->dsi.word_count=480*3;

	//here is for esd protect by legen
	//params->dsi.noncont_clock = true;
	//params->dsi.noncont_clock_period=2;
	params->dsi.lcm_ext_te_enable=false;
	//for esd protest end by legen

	//delete by zhuqiang 2013.3.4 
	//params->dsi.word_count=FRAME_WIDTH*3;	
	// add by zhuqiang for FR437058 at 2013.4.25 begin
	params->dsi.vertical_sync_active=6;  //4
	params->dsi.vertical_backporch=14;	//16
	params->dsi.vertical_frontporch=20;
	// add by zhuqiang for FR437058 at 2013.4.25 end
	params->dsi.vertical_active_line=FRAME_HEIGHT;

	//delete by zhuqiang 2013.3.4 
	//	params->dsi.line_byte=2180;		
	// add by zhuqiang for FR437058 at 2013.4.25 begin
	params->dsi.horizontal_sync_active=10;  
	//zrl modify for improve the TP reort point begin,130916
	params->dsi.horizontal_backporch=80;   //50  60 
	params->dsi.horizontal_frontporch=80;  //50   200
	//zrl modify for improve the TP reort point end,130916
	// add by zhuqiang for FR437058 at 2013.4.25 end
	params->dsi.horizontal_active_pixel = FRAME_WIDTH;	//added by zhuqiang 2013.3.4 
	params->dsi.HS_TRAIL= 7;  // 4.3406868

	params->dsi.CLK_TRAIL= 50;
	params->dsi.rgb_byte=(FRAME_WIDTH*3+6);		// NC

	params->dsi.horizontal_sync_active_word_count=20;	
	params->dsi.horizontal_backporch_word_count=200;
	params->dsi.horizontal_frontporch_word_count=200;

	*/
	/*
	// added by zhuqiang for one lane speed maximum is 270MHz start.20121024
	// Bit rate calculation
	params->dsi.pll_div1=35;		// fref=26MHz, fvco=fref*(div1+1)	(div1=0~63, fvco=500MHZ~1GHz)
	params->dsi.pll_div2=1;			// div2=0~15: fout=fvo/(2*div2) 300MHZ
	// added by zhuqiang for one lane speed maximum is 270MHz end.20121024
	*/
	/*
	// add by zhuqiang for FR437058 at 2013.4.25 begin
	//params->dsi.pll_div1=0;         //  div1=0,1,2,3;  div1_real=1,2,4,4
	//params->dsi.pll_div2=2;         // div2=0,1,2,3;div2_real=1,2,4,4
	//params->dsi.fbk_div =0x1E;              // fref=26MHz,  fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)

	// add by zhuqiang for FR437058 at 2013.4.25 end
	*/
	params->dsi.PLL_CLOCK = 208; //modified by zhuqiang for PR683317
	//params->dsi.ssc_range = 7;   //modified by zhuqiang for PR683317
	//params->dsi.ssc_disable = 0; //modified by zhuqiang for PR683317
		
}

//legen add for detect lcm vendor
static bool lcm_select_panel(void)
{
	int value=0;

	//printk("\t\t 9806e [lcm_select_panel]\n");

	mt_set_gpio_mode(GPIO_LCM_ID1,GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LCM_ID1, GPIO_DIR_IN);
	mt_set_gpio_pull_enable(GPIO_LCM_ID1, GPIO_PULL_DISABLE);
	mt_set_gpio_mode(GPIO_LCM_ID2,GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LCM_ID2, GPIO_DIR_IN);
	mt_set_gpio_pull_enable(GPIO_LCM_ID2, GPIO_PULL_DISABLE);

	MDELAY(10);
	value = mt_get_gpio_in(GPIO_LCM_ID1)<<1 | mt_get_gpio_in(GPIO_LCM_ID2);
	if(value)
		return value;

	return LCM_TDT;
}
//legen add end 

static void lcm_init(void)
{
    unsigned int data_array[16];

    SET_RESET_PIN(1);
    MDELAY(20);
    SET_RESET_PIN(0);
    MDELAY(20);
    SET_RESET_PIN(1);
    MDELAY(120);

    push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);

}


static void lcm_suspend(void)
{
	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);	
	SET_RESET_PIN(0);
	MDELAY(20);
	SET_RESET_PIN(1);
}


static void lcm_resume(void)
{
	lcm_init();
      // add by zhuqiang for FR437058 at 2013.4.26 end
	//push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0>>8)&0xFF);
	unsigned char x0_LSB = (x0&0xFF);
	unsigned char x1_MSB = ((x1>>8)&0xFF);
	unsigned char x1_LSB = (x1&0xFF);
	unsigned char y0_MSB = ((y0>>8)&0xFF);
	unsigned char y0_LSB = (y0&0xFF);
	unsigned char y1_MSB = ((y1>>8)&0xFF);
	unsigned char y1_LSB = (y1&0xFF);

	unsigned int data_array[16];

	data_array[0]= 0x00053902;
	data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
	data_array[2]= (x1_LSB);
	data_array[3]= 0x00053902;
	data_array[4]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[5]= (y1_LSB);
	data_array[6]= 0x002c3909;

	dsi_set_cmdq(data_array, 7, 0);

}

// added by zhuqiang for lcd esd begin 2012.11.19

static unsigned int lcm_esd_check(void)
{
#ifndef BUILD_LK

       unsigned char buffer_vcom[4];

       unsigned char buffer_0a[1];

       unsigned int array[16];


       array[0]=0x00341500;

       dsi_set_cmdq(&array, 1, 1);

       array[0] = 0x00013700;

       dsi_set_cmdq(array, 1, 1);

      read_reg_v2(0x0A,buffer_0a, 1);

      array[0] = 0x00043700;

      dsi_set_cmdq(array, 1, 1);

      read_reg_v2(0xC5, buffer_vcom, 4);

      array[0]=0x00351500;

      dsi_set_cmdq(&array, 1, 1);
     //printk("lcm 0x0a is %x--------------\n", buffer_0a[0]);
     //printk("lcm 0xc5 is %x,%x,%x,%x--------------\n", buffer_vcom[0], buffer_vcom[1] ,buffer_vcom[2], buffer_vcom[3]);
    //  if ((buffer_vcom[0]==0x00)&&(buffer_vcom[1]==0x41)&&(buffer_vcom[3]==0x41)&&(buffer_0a[0]==0x9C)){

               return 0;


   //   }else{

  //            return 1;
  //    }
#endif
}


static unsigned int lcm_esd_recover(void)
{
#ifndef BUILD_LK


       lcm_init();

       return 1;

      #endif 
}

 // added by zhuqiang for lcd esd end 2012.11.19

static unsigned int lcm_compare_id(void)
{
      unsigned int id=0;
      unsigned int id1=0; 

	mt_set_gpio_mode(GPIO_LCM_ID1,GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LCM_ID1, GPIO_DIR_IN);
	//mt_set_gpio_pull_select(GPIO_LCM_ID1,GPIO_PULL_DOWN);
	mt_set_gpio_pull_enable(GPIO_LCM_ID1, GPIO_PULL_DISABLE);// def 0
        MDELAY(5);
	id = mt_get_gpio_in(GPIO_LCM_ID1) ;
 
   
	mt_set_gpio_mode(GPIO_LCM_ID2,GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LCM_ID2, GPIO_DIR_IN);
	//mt_set_gpio_pull_select(GPIO_LCM_ID2,GPIO_PULL_DOWN);
	mt_set_gpio_pull_enable(GPIO_LCM_ID2, GPIO_PULL_DISABLE);//def 0
        MDELAY(5);
	id1 = mt_get_gpio_in(GPIO_LCM_ID2) ;


 #if defined(BUILD_LK)
	 printf("llf_lk -- ili9806e %d,%d \n",id,id1);
#else
	 printk("llf_kernel -- ili9806e %d,%d \n",id,id1);
#endif
         
      if((id==0x00) && (id1==0x01))
      {
      	return 1;
      }
      else 
      {
      	return 0;
      }
}
// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER pixi45_ili9806e_fwvga_dsi_txd_lcm_drv =
{
        .name			= "pixi45_ili9806e_fwvga_dsi_txd",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
//#if (LCM_DSI_CMD_MODE)
//	.update         = lcm_update,
//	.set_backlight	= lcm_setbacklight,
//	.set_pwm        = lcm_setpwm,
//	.get_pwm        = lcm_getpwm,
   
      // added by zhuqiang for lcd esd begin 2012.11.19
//	.esd_check   = lcm_esd_check,
 //  	.esd_recover   = lcm_esd_recover,
      // added by zhuqiang for lcd esd end 2012.11.19
	.compare_id    = lcm_compare_id,
//#endif
};
