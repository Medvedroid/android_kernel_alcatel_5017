#if defined(BUILD_LK)
#include <string.h>
#else
#include <linux/string.h>
#endif
#include "lcm_drv.h"

#if defined(BUILD_LK)
#include "cust_gpio_usage.h"
#include <platform/mt_gpio.h>
#else
#include "cust_gpio_usage.h"
#include <mach/mt_gpio.h>
#endif

// ---------------------------------------------------------------------------
//RGK add
// ---------------------------------------------------------------------------
//#include <cust_adc.h>        // zhoulidong  add for lcm detect
#define AUXADC_LCM_VOLTAGE_CHANNEL     0
#define AUXADC_ADC_FDD_RF_PARAMS_DYNAMIC_CUSTOM_CH_CHANNEL     1

#define MIN_VOLTAGE (0)     // zhoulidong  add for lcm detect
#define MAX_VOLTAGE (100)     // zhoulidong  add for lcm detect
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH                                          (480)
#define FRAME_HEIGHT                                         (854)

#define LCM_ID_OTM8019A                                    (0x8019a)
//#define GPIO_LCM_ID1	(GPIO81 | 0x80000000)
//#define GPIO_LCM_ID2	(GPIO82 | 0x80000000)
#define GPIO_LCM_ID1	GPIO81_LCD_ID1
#define GPIO_LCM_ID2	GPIO82_LCD_ID2

#define REGFLAG_DELAY             							0XFE
#define REGFLAG_END_OF_TABLE      							0xFFF   // END OF REGISTERS

#ifndef TRUE
    #define   TRUE     1
#endif

#ifndef FALSE
    #define   FALSE    0
#endif

 unsigned static int lcm_esd_test = FALSE;      ///only for ESD test

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util ;

#define SET_RESET_PIN(v)                                    (lcm_util.set_reset_pin((v)))

#define UDELAY(n)                                             (lcm_util.udelay(n))
#define MDELAY(n)                                             (lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------
#define dsi_set_cmdq_V3(para_tbl,size,force_update)        lcm_util.dsi_set_cmdq_V3(para_tbl,size,force_update)
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)            lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)        lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)                                        lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)                    lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)                                            lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)                lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

#define   LCM_DSI_CMD_MODE                            0


struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
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

// zhoulidong  add for lcm detect ,read adc voltage
extern int IMM_GetOneChannelValue(int dwChannel, int data[4], int* rawdata);


static struct LCM_setting_table lcm_initialization_setting[] = {

	{0x00,1,{0x00}},
	{0xFF,3,{0x80,0x19,0x01}},

	{0x00,1,{0x80}},
	{0xFF,2,{0x80,0x19}},

	{0x00,1,{0xC6}},
	{0xB0,1,{0x0B}},

	{0x00,1,{0x90}},
	{0xB3,1,{0x02}},

	{0x00,1,{0x92}},
	{0xB3,1,{0x40}},

	{0x00,1,{0xA6}},
	{0xB3,2,{0x00,0x01}},

	{0x00,1,{0x90}},
	{0xB6,1,{0xB4}},

	{0x00,1,{0x90}},
	{0xC0,6,{0x00,0x15,0x00,0x00,0x00,0x03}},

	{0x00,1,{0x98}},
	{0xC0,1,{0x00}},

	{0x00,1,{0xA3}},
	{0xC0,2,{0x05,0x15}},

	{0x00,1,{0xA9}},
	{0xC0,1,{0x06}},

	{0x00,1,{0xB2}},
	{0xC0,1,{0x30}},

	{0x00,1,{0xB4}},
	{0xC0,2,{0x77,0x48}},

	{0x00,1,{0xE1}},
	{0xC0,2,{0x40,0x18}},

	{0x00,1,{0x80}},
	{0xC1,2,{0x03,0x33}},

	{0x00,1,{0xA0}},
	{0xC1,1,{0xE8}},

	{0x00,1,{0xB0}},
	{0xC1,3,{0x20,0x00,0x00}},

	{0x00,1,{0x81}},
	{0xC4,2,{0x04,0xF2}},

	{0x00,1,{0x87}},
	{0xC4,1,{0x18}},

	{0x00,1,{0x89}},
	{0xC4,1,{0x08}},

	{0x00,1,{0x90}},
	{0xC5,3,{0x6E,0xD6,0x00}},

	{0x00,1,{0xC2}},
	{0xF5,1,{0x00}},

	{0x00,1,{0x80}},
	{0xCE,6,{0x8B,0x03,0x04,0x8A,0x03,0x04}},

	{0x00,1,{0x90}},
	{0xCE,12,{0x23,0x59,0x04,0x23,0x5A,0x04,0x33,0x5C,0x04,0x33,0x5D,0x04}},

	{0x00,1,{0xA0}},
	{0xCE,14,{0x38,0x09,0x03,0x5A,0x00,0x04,0x00,0x38,0x08,0x03,0x5B,0x00,0x04,0x00}},

	{0x00,1,{0xB0}},
	{0xCE,14,{0x38,0x07,0x03,0x5C,0x00,0x04,0x00,0x38,0x06,0x03,0x5D,0x00,0x04,0x00}},

	{0x00,1,{0xC0}},
	{0xCE,14,{0x38,0x05,0x03,0x5E,0x00,0x04,0x00,0x38,0x04,0x03,0x5F,0x00,0x04,0x00}},

	{0x00,1,{0xD0}},
	{0xCE,14,{0x38,0x03,0x03,0x60,0x00,0x04,0x00,0x38,0x02,0x03,0x61,0x00,0x04,0x00}},

	{0x00,1,{0xC0}},
	{0xCF,10,{0x3D,0x3D,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

	{0x00,1,{0x80}},
	{0xCB,10,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

	{0x00,1,{0x90}},
	{0xCB,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

	{0x00,1,{0xA0}},
	{0xCB,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

	{0x00,1,{0xB0}},
	{0xCB,10,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

	{0x00,1,{0xC0}},
	{0xCB,15,{0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x00,0x01,0x01,0x00,0x01,0x01,0x01,0x01}},

	{0x00,1,{0xD0}},
	{0xCB,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x00,0x01,0x01,0x00,0x01}},

	{0x00,1,{0xE0}},
	{0xCB,10,{0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00}},

	{0x00,1,{0xF0}},
	{0xCB,10,{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}},

	{0x00,1,{0x80}},
	{0xCC,10,{0x00,0x00,0x02,0x0A,0x0C,0x0E,0x10,0x00,0x21,0x22}},

	{0x00,1,{0x90}},
	{0xCC,15,{0x00,0x06,0x25,0x25,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0x25,0x25,0x05}},

	{0x00,1,{0xA0}},
	{0xCC,10,{0x00,0x21,0x22,0x00,0x0F,0x0D,0x0B,0x09,0x01,0x00,0x00,0x00,0x00,0x00,0x00}},

	{0x00,1,{0xB0}},
	{0xCC,10,{0x00,0x00,0x05,0x09,0x0F,0x0D,0x0B,0x00,0x21,0x22}},

	{0x00,1,{0xC0}},
	{0xCC,15,{0x00,0x01,0x25,0x25,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x25,0x25,0x02}},

	{0x00,1,{0xD0}},
	{0xCC,15,{0x00,0x21,0x22,0x00,0x0C,0x0E,0x10,0x0A,0x06,0x00,0x00,0x00,0x00,0x00,0x00}},

	{0x00,1,{0xC9}},
	{0xCF,2,{0x06,0x02}},

	{0x00,1,{0x00}},
	{0xD8,2,{0x67,0x67}},

	{0x00,1,{0x00}},
	{0xD9,1,{0x45}},

	{0x00,1,{0x00}},
	{0xE1,20, {0x01,0x02,0x05,0x12,0x24,0x38,0x43,0x77,0x68,0x7F,0x87,0x75,0x8D,0x77,0x7C,0x74,0x6D,0x5D,0x54,0x10}},

	{0x00,1,{0x00}},
	{0xE2,20,{0x01,0x02,0x05,0x12,0x24,0x38,0x43,0x77,0x68,0x7F,0x87,0x75,0x8D,0x77,0x7C,0x74,0x6D,0x5D,0x54,0x10}},

	/////////////////////////////////////////////////
	//add
	{0x00,1,{0x80}},
	{0xC4,1,{0x30}},	

	{0x00,1,{0x98}},
	{0xC0,1,{0x00}},	//

	{0x00,1,{0xa9}},
	{0xC0,1,{0x0A}},	//0x06

	{0x00,1,{0xb0}},
	{0xC1,3,{0x20,0x00,0x00}}, //

	{0x00,1,{0xe1}},
	{0xC0,2,{0x40,0x30}}, //0x40,0x18

	{0x00,1,{0x80}},
	{0xC1,2,{0x03,0x33}},

	{0x00,1,{0xA0}},
	{0xC1,1,{0xe8}},

	{0x00,1,{0x90}},
	{0xb6,1,{0xb4}},	//command fial

	{REGFLAG_DELAY, 10, {}},

	{0x00,1,{0x00}},
	{0xfb,1,{0x01}},
	/////////////////////////////////////////////////
	{0x00,1,{0x00}},
	{0xFF,3,{0xFF,0xFF,0xFF}},

	{0x35,	1,	{0x00}},
	{0x11,	1,	{0x00}},
	{REGFLAG_DELAY, 200, {}},
	{0x29,	1,	{0x00}},
	{REGFLAG_DELAY, 50, {}},
	// Setting ending by predefined flag
	{REGFLAG_END_OF_TABLE, 0x00, {}}

};

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

	params->dbi.te_mode 				= LCM_DBI_TE_MODE_DISABLED;

        #if (LCM_DSI_CMD_MODE)
        params->dsi.mode   = CMD_MODE;
        #else
        params->dsi.mode   = SYNC_PULSE_VDO_MODE; //SYNC_PULSE_VDO_MODE;//BURST_VDO_MODE;
        #endif

        // DSI
        /* Command mode setting */
        //1 Three lane or Four lane
        params->dsi.LANE_NUM                = LCM_TWO_LANE;
	//The following defined the fomat for data coming from LCD engine.
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

	// Highly depends on LCD driver capability.
	params->dsi.packet_size=256;
	// add by zhuqiang for FR437058 at 2013.4.25 begin
	params->dsi.intermediat_buffer_num = 2;	
	// add by zhuqiang for FR437058 at 2013.4.25 end
	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

	params->dsi.word_count=480*3;


	//here is for esd protect by legen
	params->dsi.lcm_int_te_monitor = FALSE; 
	params->dsi.lcm_int_te_period = 1; // Unit : frames 

	// Need longer FP for more opportunity to do int. TE monitor applicably. 
	if(params->dsi.lcm_int_te_monitor) 
		params->dsi.vertical_frontporch *= 2; 

	// Monitor external TE (or named VSYNC) from LCM once per 2 sec. (LCM VSYNC must be wired to baseband TE pin.) 
	params->dsi.lcm_ext_te_monitor = FALSE; 

	params->dsi.noncont_clock = true;
	params->dsi.noncont_clock_period=2;

	//for esd protest end by legen

        // Video mode setting
        params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

        params->dsi.vertical_sync_active                = 2;//4
        params->dsi.vertical_backporch                    = 16;//8
        params->dsi.vertical_frontporch                    = 16;//8
        params->dsi.vertical_active_line                = FRAME_HEIGHT;

        params->dsi.horizontal_sync_active                = 4;//6
        params->dsi.horizontal_backporch                = 46;//37
        params->dsi.horizontal_frontporch                = 46;//37
        params->dsi.horizontal_active_pixel                = FRAME_WIDTH;

        //params->dsi.LPX=8;

        // Bit rate calculation
        //1 Every lane speed
        //params->dsi.pll_select=1;
        //params->dsi.PLL_CLOCK  = LCM_DSI_6589_PLL_CLOCK_377;
        params->dsi.PLL_CLOCK=208;
       /* params->dsi.pll_div1=0;        // div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps
        params->dsi.pll_div2=0;        // div2=0,1,2,3;div1_real=1,2,4,4
        #if (LCM_DSI_CMD_MODE)
        params->dsi.fbk_div =7;
        #else
        params->dsi.fbk_div =7;    // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)
        #endif
	*/
        //params->dsi.compatibility_for_nvk = 1;        // this parameter would be set to 1 if DriverIC is NTK's and when force match DSI clock for NTK's

}


static void lcm_init(void)
{
	SET_RESET_PIN(1);
	MDELAY(100);
	SET_RESET_PIN(0);
	MDELAY(100);
	SET_RESET_PIN(1);
	MDELAY(200);

	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);

}


static struct LCM_setting_table  lcm_deep_sleep_mode_in_setting[] = {
	// Display off sequence
	{0x28, 1, {0x00}},
	{REGFLAG_DELAY, 20, {}},

	// Sleep Mode On
	{0x10, 1, {0x00}},
	{REGFLAG_DELAY, 50, {}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
static void lcm_suspend(void)
{
    push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);

    SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    MDELAY(20); // 1ms

    SET_RESET_PIN(1);
    MDELAY(120);
}


static void lcm_resume(void)
{
    lcm_init();

}

#if (LCM_DSI_CMD_MODE)
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
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0]= 0x00053902;
    data_array[1]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
    data_array[2]= (y1_LSB);
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0]= 0x002c3909;
    dsi_set_cmdq(data_array, 1, 0);

}
#endif





// zhoulidong  add for lcm detect (start)

static unsigned int lcm_compare_id(void)
{

	char id_high=0;
	char id_low=0;
	int id=0;

	int id_type=0;	
	unsigned char buffer[4]={0};
	unsigned int array[16]={0};

	mt_set_gpio_mode(GPIO_LCM_ID1,GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LCM_ID1, GPIO_DIR_IN);
	//mt_set_gpio_pull_select(GPIO_LCM_ID1,GPIO_PULL_DOWN);
	mt_set_gpio_pull_enable(GPIO_LCM_ID1, GPIO_PULL_DISABLE);// def 0
	
	mt_set_gpio_mode(GPIO_LCM_ID2,GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LCM_ID2, GPIO_DIR_IN);
	//mt_set_gpio_pull_select(GPIO_LCM_ID2,GPIO_PULL_DOWN);
	mt_set_gpio_pull_enable(GPIO_LCM_ID2, GPIO_PULL_DISABLE);//def 0

	MDELAY(10);
	id_type = mt_get_gpio_in(GPIO_LCM_ID1)<<1 | mt_get_gpio_in(GPIO_LCM_ID2);
	#ifdef BUILD_LK
	printf("[LLF] otm8019a [lcm_compare_id2   id_type  %d ]\n" , id_type);
	#else
	printk("[LLF] otm8019a [lcm_compare_id2   id_type  %d ]\n" , id_type);
	#endif

	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(200);

	array[0] = 0x00053700;
	dsi_set_cmdq(array, 1, 1);
	read_reg_v2(0xa1, buffer, 5);

	id_high = buffer[2];
	id_low = buffer[3];
	id = (id_high<<8) | id_low;


	#if defined(BUILD_LK)
	printf("OTM8019A lk %s \n", __func__);
	printf("%s id = 0x%08x \n", __func__, id);
	#else
	printk("OTM8019A kernel %s \n", __func__);
	printk("%s id = 0x%08x \n", __func__, id);
	#endif
	if (id_type == 1 ) //otm8019a_byd  source ,and ID_tpye is . 
	{
		return 1;
	}
	else
	{
		return  0 ;
	}

	//return 2;
}

// zhoulidong  add for lcm detect (start)
static unsigned int rgk_lcm_compare_id(void)
{
    int data[4] = {0,0,0,0};
    int res = 0;
    int rawdata = 0;
    int lcm_vol = 0;
#ifdef AUXADC_LCM_VOLTAGE_CHANNEL
    res = IMM_GetOneChannelValue(AUXADC_LCM_VOLTAGE_CHANNEL,data,&rawdata);
    if(res < 0)
    {
    #ifdef BUILD_LK
    printf("[adc_uboot]: get data error\n");
    #endif
    return 0;

    }
#endif

    lcm_vol = data[0]*1000+data[1]*10;

    #ifdef BUILD_LK
    printf("[adc_uboot]: lcm_vol= %d\n",lcm_vol);
    #endif

    if (lcm_vol>=MIN_VOLTAGE &&lcm_vol <= MAX_VOLTAGE)
    {
    return 1;
    }

    return 0;

}


// zhoulidong  add for lcm detect (end)

// zhoulidong add for eds(start)
static unsigned int lcm_esd_check(void)
{
    #ifdef BUILD_LK
        //printf("lcm_esd_check()\n");
    #else
        //printk("lcm_esd_check()\n");
    #endif
 #ifndef BUILD_LK
    char  buffer[3];
    int   array[4];

    if(lcm_esd_test)
    {
        lcm_esd_test = FALSE;
        return TRUE;
    }

    array[0] = 0x00013700;
    dsi_set_cmdq(array, 1, 1);

    read_reg_v2(0x0a, buffer, 1);
    if(buffer[0]==0x9c)
    {
        //#ifdef BUILD_LK
        //printf("%s %d\n FALSE", __func__, __LINE__);
        //#else
        //printk("%s %d\n FALSE", __func__, __LINE__);
        //#endif
        return FALSE;
    }
    else
    {
        //#ifdef BUILD_LK
        //printf("%s %d\n FALSE", __func__, __LINE__);
        //#else
        //printk("%s %d\n FALSE", __func__, __LINE__);
        //#endif
        return TRUE;
    }
 #endif

}

static unsigned int lcm_esd_recover(void)
{

    #ifdef BUILD_LK
        printf("lcm_esd_recover()\n");
    #else
        printk("lcm_esd_recover()\n");
    #endif

    lcm_init();

    return TRUE;
}


void cmd_push_table(struct LCM_setting_table *table, unsigned int count)
{
    push_table(table, count, 1);
}


LCM_DRIVER pixi45_otm8019a_fwvga_dsi_byd_lcm_drv =
{
    .name            = "pixi45_otm8019a_fwvga_dsi_byd",
    .set_util_funcs = lcm_set_util_funcs,
    .get_params     = lcm_get_params,
    .init           = lcm_init,
    .suspend        = lcm_suspend,
    .resume         = lcm_resume,
    .compare_id    = lcm_compare_id,
//    .esd_check = lcm_esd_check,
//    .esd_recover = lcm_esd_recover,
#if (LCM_DSI_CMD_MODE)
    .update         = lcm_update,
#endif
};

