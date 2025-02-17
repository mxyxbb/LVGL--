#include "lvgl/lvgl.h"
#include "mymath.h"
#include "rtc.h"
#include "BMP280/bmp280.h"
#include "adc.h"
#include "st7735/st7735.h"
		
#define LV_ANIM_TIME_DEFAULT 200



/*RTC时间*/
static RTC_TimeTypeDef RTC_Time;
static RTC_TimeTypeDef RTC_TimeLast;
static RTC_DateTypeDef RTC_Date;

/*此页面窗口*/
static lv_obj_t * appWindow;

/*背景图片*/
static lv_obj_t * imgBg;

/*气压计信息*/
static lv_obj_t * labelBMP;

/*电池信息*/
static lv_obj_t * labelBatt;

/*时间信息*/
static lv_obj_t * labelDate;

/*时间容器*/
static lv_obj_t * contTime;

/*秒指示灯LED*/
static lv_obj_t * ledSec[2];

/*时间标签1、2，交替使用实现上下滑动效果*/
static lv_obj_t * labelTime_Grp[4];
static lv_obj_t * labelTime_Grp2[4];

/*运动图标*/
static lv_obj_t * imgRun;

/*计步次数标签*/
static lv_obj_t * labelStepCnt;

/*时间更新任务句柄*/
static lv_task_t * taskTimeUpdate;
static lv_task_t * taskDateUpdate;
static lv_task_t * taskBatUpdate;


/*状态栏更新任务句柄*/
static lv_task_t * taskTopBarUpdate;

//mxy add
/**
  * @brief  为对象添加动画
  * @param  obj:对象地址
  * @param  a:动画控制器地址
  * @param  exec_cb:控制对象属性的函数地址
  * @param  start:动画的开始值
  * @param  end:动画的结束值
  * @param  time:动画的执行时间
  * @param  ready_cb:动画结束事件回调
  * @param  path_cb:动画曲线
  * @retval 无
  */
void lv_obj_add_anim(
    lv_obj_t * obj, lv_anim_t * a,
    lv_anim_exec_xcb_t exec_cb, 
    int32_t start, int32_t end,    
    uint16_t time,
    lv_anim_ready_cb_t ready_cb,
    lv_anim_path_cb_t path_cb
)
{
    lv_anim_t a_tmp;
    if(a == NULL)
    {
        lv_anim_init(&a_tmp);
        a = &a_tmp;
    }

    a->var = obj;
    a->start = start;
    a->end = end;
    a->exec_cb = exec_cb;
    a->path_cb = path_cb;
    a->ready_cb = ready_cb;
    a->time = time;
    lv_anim_create(a);
}

#define LV_OBJ_ADD_ANIM(obj,attr,target,time)\
do{\
    static lv_anim_t a;\
    lv_obj_add_anim(\
        (obj), &a,\
        (lv_anim_exec_xcb_t)lv_obj_set_##attr,\
        lv_obj_get_##attr(obj),\
        (target),\
        (time),\
				NULL,\
				lv_anim_path_ease_out\
    );\
}while(0)
/**
  * @brief  将一个数字(浮点型)从一个范围重新映射到另一个区域
  * @param  x: 要映射的数字
  * @param  in_min: 值的当前范围的下界
  * @param  in_max: 值的当前范围的上界
  * @param  out_min: 值的目标范围的下界
  * @param  out_max: 值目标范围的上界
  * @retval 映射的值(double)
  */
double fmap(double x, double in_min, double in_max, double out_min, double out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/**
  * @brief  创建背景图
  * @param  无
  * @retval 无
  */
static void ImgBg_Create()
{
    LV_IMG_DECLARE(MYimgBg);//ImgBg);
    imgBg = lv_img_create(appWindow, NULL);
    lv_img_set_src(imgBg, &MYimgBg);
    lv_obj_align(imgBg, NULL, LV_ALIGN_CENTER, 0, 0);
}
//static void ImgBg_Create()
//{
//    LV_IMG_DECLARE(ImgBg);
//		lv_obj_t * canvas = lv_canvas_create(appWindow, NULL);
//		static lv_color_t cbuf[LV_CANVAS_BUF_SIZE_TRUE_COLOR_ALPHA(160, 80)];
//		memset(cbuf, 0x00, sizeof(cbuf));
//		lv_canvas_set_buffer(canvas, cbuf, 160, 80, LV_IMG_CF_TRUE_COLOR_ALPHA);
//		lv_canvas_rotate(canvas, &ImgBg, 30, 25, 25, 25, 25);
//}


/**
  * @brief  状态栏更新
  * @param  task:任务句柄
  * @retval 无
  */
#define LV_SYMBOL_DEGREE_SIGN   "\xC2\xB0"
//BME280_configuration
extern BMP280_HandleTypedef bmp280;
float pressure, temperature, humidity;
static void Task_TopBarUpdate(lv_task_t * task)
{
    /*气压计状态更新*/
    bmp280_read_float(&bmp280, &temperature, &pressure, &humidity);
    lv_label_set_text_fmt(labelBMP, "% 2dC"LV_SYMBOL_DEGREE_SIGN" %d%%", (int)temperature, (int)humidity);

}

static void Task_BatUpdate(lv_task_t * task)
{
    /*读取电池电压*/
    float battVoltage = my_adc_read()*4.18;
    
    /*是否充电*/
//    bool Is_BattCharging = !digitalRead(BAT_CHG_Pin);

    /*电池图标组*/
    const char * battSymbol[] =
    {
        LV_SYMBOL_BATTERY_EMPTY,
        LV_SYMBOL_BATTERY_1,
        LV_SYMBOL_BATTERY_2,
        LV_SYMBOL_BATTERY_3,
        LV_SYMBOL_BATTERY_FULL
    };
    
    /*电压映射到图标索引*/
		int symIndex = fmap(
        battVoltage, 
        2.8f, 4.2f, 
        0, __Sizeof(battSymbol)
    );
    __LimitValue(symIndex, 0, __Sizeof(battSymbol) - 1);
    
//    if(Is_BattCharging)
//    {
//        /*充电动画效果*/
//        static uint8_t usage = 0;
//        usage++;
//        usage %= (symIndex + 1);
//        symIndex = usage;
//    }

    lv_label_set_text_fmt(labelBatt, "#FFFFFF %s#", battSymbol[symIndex]);
}

/**
  * @brief  创建状态栏
  * @param  无
  * @retval 无
  */
static void LabelTopBar_Create()
{
    LV_FONT_DECLARE(Segoe_20);
    labelBMP = lv_label_create(appWindow, NULL);
    static lv_style_t bmp_style;
    bmp_style = *lv_label_get_style(labelBMP, LV_LABEL_STYLE_MAIN);
    bmp_style.text.font = &Segoe_20;
    bmp_style.text.color = LV_COLOR_WHITE;
    lv_label_set_style(labelBMP, LV_LABEL_STYLE_MAIN, &bmp_style);
    lv_label_set_text(labelBMP, "-- C -- kPa");
    lv_obj_align(labelBMP, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
    lv_obj_set_auto_realign(labelBMP, true);
    
//    labelBatt = lv_label_create(appWindow, NULL);
//    lv_label_set_recolor(labelBatt, true);
//    lv_label_set_text(labelBatt, "#FFFFFF "LV_SYMBOL_BATTERY_EMPTY"#");
//    lv_obj_align(labelBatt, NULL, LV_ALIGN_IN_TOP_RIGHT, 0, 0);
//    lv_obj_set_auto_realign(labelBatt, true);
    
    taskTopBarUpdate = lv_task_create(Task_TopBarUpdate, 500, LV_TASK_PRIO_LOW, NULL);
    Task_TopBarUpdate(taskTopBarUpdate);
}
static void LabelBatBar_Create()
{ 
    labelBatt = lv_label_create(appWindow, NULL);
    lv_label_set_recolor(labelBatt, true);
    lv_label_set_text(labelBatt, "#FFFFFF "LV_SYMBOL_BATTERY_EMPTY"#");
    lv_obj_align(labelBatt, NULL, LV_ALIGN_IN_TOP_RIGHT, 0, 0);
    lv_obj_set_auto_realign(labelBatt, true);
    
    taskBatUpdate = lv_task_create(Task_BatUpdate, 2 * 1000, LV_TASK_PRIO_LOW, NULL);
    Task_BatUpdate(taskBatUpdate);
}

/**
  * @brief  滑动改变时间标签
  * @param  val_now:当前值
  * @param  val_last:上一次的值
  * @param  index:标签索引
  * @retval 无
  */
#define LABEL_TIME_CHECK_DEF(val_now,val_last,index)\
do{\
    /*当前值发生改变时*/\
    if((val_now) != (val_last))\
    {\
        /*标签对象*/\
        lv_obj_t * next_label;\
        lv_obj_t * now_label;\
        /*判断两个标签的相对位置，确定谁是下一个标签*/\
        if(lv_obj_get_y(labelTime_Grp2[index]) > lv_obj_get_y(labelTime_Grp[index]))\
        {\
            now_label = labelTime_Grp[index];\
            next_label = labelTime_Grp2[index];\
        }\
        else\
        {\
            now_label = labelTime_Grp2[index];\
            next_label = labelTime_Grp[index];\
        }\
        \
        lv_label_set_text_fmt(now_label, "%d", (val_last));\
        lv_label_set_text_fmt(next_label, "%d", (val_now));\
        /*对齐*/\
        lv_obj_align(next_label, now_label, LV_ALIGN_OUT_TOP_MID, 0, -10);\
        /*计算需要的Y偏移量*/\
        lv_coord_t y_offset = abs(lv_obj_get_y(now_label) - lv_obj_get_y(next_label));\
        /*滑动动画*/\
        LV_OBJ_ADD_ANIM(now_label, y, lv_obj_get_y(now_label) + y_offset, LV_ANIM_TIME_DEFAULT);\
        LV_OBJ_ADD_ANIM(next_label, y, lv_obj_get_y(next_label) + y_offset, LV_ANIM_TIME_DEFAULT);\
    }\
}while(0)

/**
  * @brief  时间标签更新
  * @param  无
  * @retval 无
  */
static void LabelTimeGrp_Update()
{
    /*获取RTC时间*/
    HAL_RTC_GetTime(&hrtc,&RTC_Time,RTC_FORMAT_BIN);
    /*分-个位*/
		LABEL_TIME_CHECK_DEF(RTC_Time.Minutes % 10,RTC_TimeLast.Minutes % 10, 3);
    /*分-十位*/
    LABEL_TIME_CHECK_DEF(RTC_Time.Minutes / 10,RTC_TimeLast.Minutes / 10, 2);
    
    /*时-个位*/
    LABEL_TIME_CHECK_DEF(RTC_Time.Hours % 10,RTC_TimeLast.Hours % 10, 1);
    /*时-十位*/
    LABEL_TIME_CHECK_DEF(RTC_Time.Hours / 10,RTC_TimeLast.Hours / 10, 0);
    
    RTC_TimeLast = RTC_Time;
}

/**
  * @brief  时间更新任务
  * @param  task:任务句柄
  * @retval 无
  */
static void Task_TimeUpdate(lv_task_t * task)
{
    /*时间标签状态更新*/
    LabelTimeGrp_Update();
    
    /*翻转LED状态*/
    lv_led_toggle(ledSec[0]);
    lv_led_toggle(ledSec[1]);
}
static void Task_DateUpdate(lv_task_t * task)
{  
    /*日期*/
		HAL_RTC_GetDate(&hrtc,&RTC_Date,RTC_FORMAT_BIN);
		const char* week_str[7] = { "MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"};
    int8_t index = RTC_Date.WeekDay - 1;
    __LimitValue(index, 0, 6);
    lv_label_set_text_fmt(labelDate, "%02d#FF0000 /#%02d %s", RTC_Date.Month, RTC_Date.Date, week_str[index]);
}
/**
  * @brief  创建日期标签
  * @param  无
  * @retval 无
  */
static void LabelDate_Create()
{
    LV_FONT_DECLARE(Segoe_20);//Morganite_36);
    labelDate = lv_label_create(appWindow, NULL);
    
    static lv_style_t bmp_style;
    bmp_style = *lv_label_get_style(labelDate, LV_LABEL_STYLE_MAIN);
    bmp_style.text.font = &Segoe_20;
    bmp_style.text.color = LV_COLOR_WHITE;
    lv_label_set_style(labelDate, LV_LABEL_STYLE_MAIN, &bmp_style);
    
    lv_label_set_recolor(labelDate, true);
    lv_label_set_text(labelDate, "01#FF0000 /#01 MON");
    lv_obj_align(labelDate, NULL, LV_ALIGN_IN_TOP_LEFT, 3, 10);
    lv_obj_set_auto_realign(labelDate, true);
		
		/*注册日期标签更新任务，执行周期500ms*/
    taskDateUpdate = lv_task_create(Task_DateUpdate, 800, LV_TASK_PRIO_MID, NULL);
    Task_DateUpdate(taskDateUpdate);
		
}

/**
  * @brief  创建时间标签
  * @param  无
  * @retval 无
  */
static void LabelTime_Create()
{
//		LabelDate_Create();
    
    /*contTime*/
    contTime = lv_cont_create(appWindow, NULL);
    lv_cont_set_style(contTime, LV_CONT_STYLE_MAIN, &lv_style_transp);
    lv_obj_set_size(contTime, 130, 80);
    lv_obj_align(contTime, NULL, LV_ALIGN_CENTER, 0, 0);
    
    /*ledSec*/
    static lv_style_t led_style;
    led_style = lv_style_pretty_color;
    led_style.body.main_color = LV_COLOR_RED;
    led_style.body.grad_color = LV_COLOR_RED;
    for(int i = 0; i < __Sizeof(ledSec); i++)
    {
        lv_obj_t * led = lv_led_create(contTime, NULL);
        lv_led_set_style(led, LV_LED_STYLE_MAIN, &led_style);
        lv_obj_set_size(led, 8, 10);
        lv_obj_align(led, NULL, LV_ALIGN_CENTER, 0, i == 0 ? -10 : 10);
        ledSec[i] = led;
    }
    
    /*labelTime*/
    LV_FONT_DECLARE(Morganite_100);
    static lv_style_t time_style;
    time_style = lv_style_plain;
    time_style.text.font = &Morganite_100;
    time_style.text.color = LV_COLOR_WHITE;
    const lv_coord_t x_mod[4] = {-45, -20, 20, 45};
    for(int i = 0; i < __Sizeof(labelTime_Grp); i++)
    {
        lv_obj_t * label = lv_label_create(contTime, NULL);
        lv_label_set_style(label, LV_LABEL_STYLE_MAIN, &time_style);
        lv_label_set_text(label, "0");
        lv_obj_align(label, NULL, LV_ALIGN_CENTER, x_mod[i], 0);
        labelTime_Grp[i] = label;
    }
    for(int i = 0; i < __Sizeof(labelTime_Grp2); i++)
    {
        lv_obj_t * label = lv_label_create(contTime, NULL);
        lv_label_set_style(label, LV_LABEL_STYLE_MAIN, &time_style);
        lv_label_set_text(label, "0");
        lv_obj_align(label, labelTime_Grp[i], LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
        labelTime_Grp2[i] = label;
    }

    /*时间清零*/
    memset(&RTC_TimeLast, 0, sizeof(RTC_TimeLast));
    
    /*注册时间标签更新任务，执行周期500ms*/
    taskTimeUpdate = lv_task_create(Task_TimeUpdate, 500, LV_TASK_PRIO_MID, NULL);
    Task_TimeUpdate(taskTimeUpdate);
}

/**
  * @brief  创建计步标签
  * @param  无
  * @retval 无
  */
static void LabelStep_Create()
{
    LV_IMG_DECLARE(ImgRun);
    imgRun = lv_img_create(appWindow, NULL);
    lv_img_set_src(imgRun, &ImgRun);
    lv_obj_align(imgRun, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 20, -20);
    
    LV_FONT_DECLARE(HandGotn_26);
    
    labelStepCnt = lv_label_create(appWindow, NULL);
    
    static lv_style_t step_style;
    step_style = *lv_label_get_style(labelStepCnt, LV_LABEL_STYLE_MAIN);
    step_style.text.font = &HandGotn_26;
    step_style.text.color = LV_COLOR_WHITE;
    lv_label_set_style(labelStepCnt, LV_LABEL_STYLE_MAIN, &step_style);
    
    lv_label_set_recolor(labelStepCnt, true);
    /*懒得做计步，先随便写个值*/
    lv_label_set_text(labelStepCnt, "#FF0000 /# 1255");
    lv_obj_align(labelStepCnt, imgRun, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
    lv_obj_set_auto_realign(labelStepCnt, true);
}

/**
  * @brief  页面初始化事件
  * @param  无
  * @retval 无
  */
//void DialPlateSetup()
//{
//    /*将此页面移到前台*/
//    lv_obj_move_foreground(appWindow);
//    
//    ImgBg_Create();
////    LabelTopBar_Create();
//    LabelTime_Create();
//    LabelStep_Create();
//    
////    Power_SetAutoLowPowerEnable(true);
//}
void DialPlateSetup()
{
		appWindow = lv_obj_create(lv_scr_act(), NULL);
		lv_obj_set_size(appWindow, 160, 80);
    /*将此页面移到前台*/
    lv_obj_move_foreground(appWindow);
    
    ImgBg_Create();
		LabelBatBar_Create();
//    LabelTopBar_Create();
    LabelTime_Create();
//    LabelStep_Create();
    
//    Power_SetAutoLowPowerEnable(true);
}
void LabelTopBarSetup()
{
		appWindow = lv_obj_create(lv_scr_act(), NULL);
		lv_obj_set_size(appWindow, 160, 80);
    /*将此页面移到前台*/
    lv_obj_move_foreground(appWindow);
    
    ImgBg_Create();
		LabelBatBar_Create();
    LabelTopBar_Create();
		LabelDate_Create();
//    LabelTime_Create();
//    LabelStep_Create();
    
//    Power_SetAutoLowPowerEnable(true);
}
/**
  * @brief  页面退出事件
  * @param  无
  * @retval 无
  */
void DialPlateExit()
{
    /*关任务*/
    lv_task_del(taskTimeUpdate);
//    lv_task_del(taskTopBarUpdate);
//    lv_task_del(taskDateUpdate);
	
    lv_task_del(taskBatUpdate);
    /*删除此页面上的子控件*/
    lv_obj_clean(appWindow);
    
    /*禁用自动关机*/
//    Power_SetAutoLowPowerEnable(false);
}
void DateExit()
{
    /*关任务*/
//    lv_task_del(taskTimeUpdate);
    lv_task_del(taskTopBarUpdate);
    lv_task_del(taskDateUpdate);
	
    lv_task_del(taskBatUpdate);
    /*删除此页面上的子控件*/
    lv_obj_clean(appWindow);
    
    /*禁用自动关机*/
//    Power_SetAutoLowPowerEnable(false);
}

///**
//  * @brief  页面事件
//  * @param  btn:发出事件的按键
//  * @param  event:事件编号
//  * @retval 无
//  */
//static void Event(void* btn, int event)
//{
//    /*当有按键点击或长按时*/
//    if(event == ButtonEvent::EVENT_ButtonClick || event == ButtonEvent::EVENT_ButtonLongPressed)
//    {
//        /*进入主菜单*/
//        page.PagePush(PAGE_MainMenu);
//    }
//}

///**
//  * @brief  页面注册
//  * @param  pageID:为此页面分配的ID号
//  * @retval 无
//  */
//void PageRegister_DialPlate(uint8_t pageID)
//{
//    /*获取分配给此页面的窗口*/
//    appWindow = AppWindow_GetCont(pageID);
//    
//    /*注册至页面调度器*/
//    page.PageRegister(pageID, Setup, NULL, Exit, Event);
//}
