#include "WouoUI.h"
#include "WouoUI_Data.h" 

// 实例化全局对象
WouoUI_Class WouoUI;

// 屏幕驱动定义 (软件模拟SPI)
U8G2_SH1107_PIMORONI_128X128_F_4W_HW_SPI u8g2(U8G2_R0, OLED_CS, OLED_DC, OLED_RES);

// 磁贴变量
#define   TILE_B_FONT         u8g2_font_helvB24_tr        //磁贴大标题字体
#define   TILE_S_FONT         u8g2_font_HelvetiPixel_tr   //磁贴小标题字体
#define   TILE_B_TITLE_H      25                          //磁贴大标题字体高度
#define   TILE_S_TITLE_H      8                           //磁贴小标题字体高度
#define   TILE_ICON_H         48                          //磁贴图标高度
#define   TILE_ICON_W         48                          //磁贴图标宽度
#define   TILE_ICON_S         57                          //磁贴图标间距
#define   TILE_INDI_H         40                          //磁贴大标题指示器高度
#define   TILE_INDI_W         10                          //磁贴大标题指示器宽度
#define   TILE_INDI_S         57                          //磁贴大标题指示器上边距

// 列表变量
#define   LIST_FONT           u8g2_font_HelvetiPixel_tr   //列表字体
#define   LIST_TEXT_H         8                           //列表每行文字字体的高度
#define   LIST_LINE_H         16                          //列表单行高度
#define   LIST_TEXT_S         4                           //列表每行文字的上边距
#define   LIST_BAR_W          5                           //列表进度条宽度
#define   LIST_BOX_R          0.5f                        //列表选择框圆角

// 电压测量页面变量
#define   WAVE_SAMPLE         20                          //采集倍数
#define   WAVE_W              DISP_W                      //波形宽度
#define   WAVE_L              0                           //波形左边距
#define   WAVE_U              0                           //波形上边距
#define   WAVE_MAX            43                          //最大值
#define   WAVE_MIN            5                           //最小值
#define   WAVE_BOX_H          49                          //波形边框高度
#define   WAVE_BOX_W          DISP_W                      //波形边框宽度
#define   VOLT_FONT           u8g2_font_helvB24_tr        //电压数字字体
#define   VOLT_LIST_U_S       94                          //列表上边距
#define   VOLT_TEXT_BG_U_S    53                          //文字背景框上边距
#define   VOLT_TEXT_BG_H      33                          //文字背景框高度

// 选择框变量
#define   CHECK_BOX_L_S       95                          //选择框在每行的左边距
#define   CHECK_BOX_U_S       2                           //选择框在每行的上边距
#define   CHECK_BOX_F_W       12                          //选择框外框宽度
#define   CHECK_BOX_F_H       12                          //选择框外框高度
#define   CHECK_BOX_D_S       2                           //选择框里面的点距离外框的边距

// 弹窗变量
#define   WIN_FONT            u8g2_font_HelvetiPixel_tr   //弹窗字体
#define   WIN_H               32                          //弹窗高度
#define   WIN_W               102                         //弹窗宽度
#define   WIN_BAR_W           92                          //弹窗进度条宽度
#define   WIN_BAR_H           7                           //弹窗进度条高度
#define   WIN_Y               - WIN_H - 2                 //弹窗竖直方向出场起始位置
#define   WIN_Y_TRG           - WIN_H - 2                 //弹窗竖直方向退场终止位置

// 关于本机页变量
#define   ABOUT_FONT          u8g2_font_HelvetiPixel_tr   //关于本机字体
#define   ABOUT_INDI_S        4                           //关于本机页面列表指示左边距
#define   ABOUT_INDI_W        2                           //关于本机页面列表指示器宽度

// 按键变量
#define   BTN_PARAM_TIMES     2 

// 中断函数 (ESP32需要IRAM_ATTR)
static void IRAM_ATTR knob_inter() 
{
  WouoUI.btn.alv = digitalRead(KNOB_AIO);
  WouoUI.btn.blv = digitalRead(KNOB_BIO);
  if (!WouoUI.btn.flag && WouoUI.btn.alv == LOW) 
  {
    WouoUI.btn.CW_1 = WouoUI.btn.blv;
    WouoUI.btn.flag = true;
  }
  if (WouoUI.btn.flag && WouoUI.btn.alv) 
  {
    WouoUI.btn.CW_2 = !WouoUI.btn.blv;
    if (WouoUI.btn.CW_1 && WouoUI.btn.CW_2)
     {
      WouoUI.btn.id = WouoUI.ui.param[KNOB_DIR];
      WouoUI.btn.pressed = true;
    }
    if (WouoUI.btn.CW_1 == false && WouoUI.btn.CW_2 == false) 
    {
      WouoUI.btn.id = !WouoUI.ui.param[KNOB_DIR];
      WouoUI.btn.pressed = true;
    }
    WouoUI.btn.flag = false;
  }
}

// -------------------------------------------------------------------------
// 类实现
// -------------------------------------------------------------------------

void WouoUI_Class::btn_scan() 
{
  btn.val = digitalRead(KNOB_SW);
  if (btn.val != btn.val_last)
  {
    btn.val_last = btn.val;
    delay(ui.param[BTN_SPT] * BTN_PARAM_TIMES);
    btn.val = digitalRead(KNOB_SW);
    if (btn.val == LOW)
    {
      btn.pressed = true;
      btn.count = 0;
      while (!digitalRead(KNOB_SW))
      {
        btn.count++;
        delay(1);
      }
      if (btn.count < ui.param[BTN_LPT] * BTN_PARAM_TIMES)  btn.id = BTN_ID_SP;
      else  btn.id = BTN_ID_LP;
    }
  }
}

void WouoUI_Class::btn_init() 
{
  pinMode(KNOB_AIO, INPUT);
  pinMode(KNOB_BIO, INPUT);
  pinMode(KNOB_SW, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(KNOB_AIO), knob_inter, CHANGE);
}

void WouoUI_Class::eeprom_write_all_data()
{
  eeprom.address = 0;
  for (uint8_t i = 0; i < EEPROM_CHECK; ++i)    EEPROM.write(eeprom.address + i, eeprom.check_param[i]);  eeprom.address += EEPROM_CHECK;
  for (uint8_t i = 0; i < UI_PARAM; ++i)        EEPROM.write(eeprom.address + i, ui.param[i]);            eeprom.address += UI_PARAM;
  for (uint8_t i = 0; i < KNOB_PARAM; ++i)      EEPROM.write(eeprom.address + i, knob.param[i]);          eeprom.address += KNOB_PARAM;
  EEPROM.commit(); // ESP32需要手动commit
}

void WouoUI_Class::eeprom_read_all_data()
{
  eeprom.address = EEPROM_CHECK;   
  for (uint8_t i = 0; i < UI_PARAM; ++i)        ui.param[i]   = EEPROM.read(eeprom.address + i);          eeprom.address += UI_PARAM;
  for (uint8_t i = 0; i < KNOB_PARAM; ++i)      knob.param[i] = EEPROM.read(eeprom.address + i);          eeprom.address += KNOB_PARAM;
}

void WouoUI_Class::ui_param_init()
{
  ui.param[DISP_BRI]  = 255;      //屏幕亮度
  ui.param[TILE_ANI]  = 30;       //磁贴动画速度
  ui.param[LIST_ANI]  = 60;       //列表动画速度
  ui.param[WIN_ANI]   = 25;       //弹窗动画速度
  ui.param[SPOT_ANI]  = 50;       //聚光动画速度
  ui.param[TAG_ANI]   = 60;       //标签动画速度
  ui.param[FADE_ANI]  = 30;       //消失动画速度
  ui.param[BTN_SPT]   = 25;       //按键短按时长
  ui.param[BTN_LPT]   = 150;      //按键长按时长
  ui.param[TILE_UFD]  = 1;        //磁贴图标从头展开开关
  ui.param[LIST_UFD]  = 1;        //菜单列表从头展开开关
  ui.param[TILE_LOOP] = 0;        //磁贴图标循环模式开关
  ui.param[LIST_LOOP] = 0;        //菜单列表循环模式开关
  ui.param[WIN_BOK]   = 0;        //弹窗背景虚化开关
  ui.param[KNOB_DIR]  = 0;        //旋钮方向切换开关   
  ui.param[DARK_MODE] = 1;        //黑暗模式开关   
}

void WouoUI_Class::eeprom_init()
{
  EEPROM.begin(4096); // ESP32必须初始化大小
  eeprom.check = 0;
  eeprom.address = 0; 
  // 初始化检查字符串
  eeprom.check_param[0]='a'; eeprom.check_param[1]='b'; eeprom.check_param[2]='c';
  eeprom.check_param[3]='d'; eeprom.check_param[4]='e'; eeprom.check_param[5]='f';
  eeprom.check_param[6]='g'; eeprom.check_param[7]='h'; eeprom.check_param[8]='i';
  eeprom.check_param[9]='j'; eeprom.check_param[10]='k';

  for (uint8_t i = 0; i < EEPROM_CHECK; ++i)  if (EEPROM.read(eeprom.address + i) != eeprom.check_param[i])  eeprom.check ++;
  if (eeprom.check <= 1) eeprom_read_all_data();  //允许一位误码
  else ui_param_init();
}

void WouoUI_Class::check_box_v_init(uint8_t *param) { check_box.v = param; }
void WouoUI_Class::check_box_m_init(uint8_t *param) { check_box.m = param; }
void WouoUI_Class::check_box_s_init(uint8_t *param, uint8_t *param_p) { check_box.s = param; check_box.s_p = param_p; }

void WouoUI_Class::check_box_m_select(uint8_t param)
{
  check_box.m[param] = !check_box.m[param];
  eeprom.change = true;
}

void WouoUI_Class::check_box_s_select(uint8_t val, uint8_t pos)
{
  *check_box.s = val;
  *check_box.s_p = pos;
  eeprom.change = true;
}

void WouoUI_Class::window_value_init(const char *title, uint8_t select, uint8_t *value, uint8_t max, uint8_t min, uint8_t step, MENU *bg, uint8_t index)
{
  strcpy(win.title, title);
  win.select = select;
  win.value = value;
  win.max = max;
  win.min = min;
  win.step = step;
  win.bg = bg;
  win.index = index;  
  ui.index = M_WINDOW;
  ui.state = S_WINDOW;
}

void WouoUI_Class::ui_init()
{
  ui.num[M_MAIN]      = sizeof( main_menu     )   / sizeof(M_SELECT);
  ui.num[M_EDITOR]    = sizeof( editor_menu   )   / sizeof(M_SELECT);
  ui.num[M_KNOB]      = sizeof( knob_menu     )   / sizeof(M_SELECT);
  ui.num[M_KRF]       = sizeof( krf_menu      )   / sizeof(M_SELECT);
  ui.num[M_KPF]       = sizeof( kpf_menu      )   / sizeof(M_SELECT);
  ui.num[M_VOLT]      = sizeof( volt_menu     )   / sizeof(M_SELECT);
  ui.num[M_SETTING]   = sizeof( setting_menu  )   / sizeof(M_SELECT);
  ui.num[M_ABOUT]     = sizeof( about_menu    )   / sizeof(M_SELECT);   

  // 初始化变量
  ui.layer = 0;
  ui.index = M_SLEEP;
  ui.state = S_NONE;
  ui.sleep = true;
  ui.fade = 1;

  // 初始化旋钮参数默认值
  knob.param[0] = KNOB_DISABLE;
  knob.param[1] = KNOB_DISABLE;
  knob.param[2] = 2;
  knob.param[3] = 2;
  
  // 初始化模拟引脚 (ESP32-S3 示例，请根据实际连接修改)
  // 注意：ADC2 在使用 WiFi 时可能受限，S3 改进了这一点但仍需留意
  analog_pin[0] = 1; analog_pin[1] = 2; analog_pin[2] = 3; analog_pin[3] = 4;
  analog_pin[4] = 5; analog_pin[5] = 6; analog_pin[6] = 7; analog_pin[7] = 8;
  analog_pin[8] = 9; analog_pin[9] = 10;
}

void WouoUI_Class::tile_param_init()
{
  ui.init = false;
  // 重新计算常量，因为它们依赖类成员初始化，这里直接赋值
  tile.title_y_calc = TILE_INDI_S + (TILE_INDI_H - TILE_B_TITLE_H) / 2 + TILE_B_TITLE_H * 2;
  tile.title_y_trg_calc = TILE_INDI_S + (TILE_INDI_H - TILE_B_TITLE_H) / 2 + TILE_B_TITLE_H;
  
  tile.icon_x = 0;
  tile.icon_x_trg = TILE_ICON_S;
  tile.icon_y = -TILE_ICON_H;
  tile.icon_y_trg = 0;
  tile.indi_x = 0;
  tile.indi_x_trg = TILE_INDI_W;
  tile.title_y = tile.title_y_calc;
  tile.title_y_trg = tile.title_y_trg_calc;
}

void WouoUI_Class::sleep_param_init()
{
  u8g2.setDrawColor(0);
  u8g2.drawBox(0, 0, DISP_W, DISP_H);
  u8g2.setPowerSave(1);
  ui.state = S_NONE;  
  ui.sleep = true;
  if (eeprom.change)
  {
    eeprom_write_all_data();
    eeprom.change = false;
  }
}

void WouoUI_Class::knob_param_init() { check_box_v_init(knob.param); }
void WouoUI_Class::krf_param_init() { check_box_s_init(&knob.param[KNOB_ROT], &knob.param[KNOB_ROT_P]); }
void WouoUI_Class::kpf_param_init() { check_box_s_init(&knob.param[KNOB_COD], &knob.param[KNOB_COD_P]); }

void WouoUI_Class::volt_param_init()
{
  volt.text_bg_l = 0;
  volt.text_bg_l_trg = DISP_W; 
}

void WouoUI_Class::setting_param_init()
{
  check_box_v_init(ui.param);
  check_box_m_init(ui.param);
}

void WouoUI_Class::about_param_init()
{
  about.indi_x = 0;
  about.indi_x_trg = ABOUT_INDI_S;
}

void WouoUI_Class::window_param_init()
{
  win.bar = 0;
  win.l = (DISP_W - WIN_W) / 2;
  win.u = (DISP_H - WIN_H) / 2;
  win.y = WIN_Y;
  win.y_trg = win.u;
  ui.state = S_NONE;
}

void WouoUI_Class::layer_init_in()
{
  ui.layer ++;
  ui.init = 0;
  list.line_n = DISP_H / LIST_LINE_H;
  list.y = 0;
  list.y_trg = LIST_LINE_H;
  list.box_x = 0;
  list.box_y = 0;
  list.bar_y = 0;
  ui.state = S_FADE;
  switch (ui.index)
  {
    case M_MAIN:    tile_param_init();    break;  
    case M_KNOB:    knob_param_init();    break;  
    case M_KRF:     krf_param_init();     break;    
    case M_KPF:     kpf_param_init();     break;   
    case M_VOLT:    volt_param_init();    break;  
    case M_SETTING: setting_param_init(); break;  
    case M_ABOUT:   about_param_init();   break;  
  }
}

void WouoUI_Class::layer_init_out()
{
  ui.select[ui.layer] = 0;
  list.box_y_trg[ui.layer] = 0;
  ui.layer --;
  ui.init = 0;
  list.y = 0;
  list.y_trg = LIST_LINE_H;
  list.bar_y = 0;
  ui.state = S_FADE;
  switch (ui.index)
  {
    case M_SLEEP: sleep_param_init(); break;    
    case M_MAIN:  tile_param_init();  break;    
  }
}

void WouoUI_Class::animation(float *a, float *a_trg, uint8_t n)
{
  if (*a != *a_trg)
  {
    if (fabs(*a - *a_trg) < 0.15f) *a = *a_trg;
    else *a += (*a_trg - *a) / (ui.param[n] / 10.0f);
  }
}

void WouoUI_Class::fade()
{
  delay(ui.param[FADE_ANI]);
  // 注意：ESP32 buffer操作可能需要互斥或直接访问，这里直接操作u8g2 buffer
  if (ui.param[DARK_MODE])
  {
    switch (ui.fade)
    {
      case 1: for (uint16_t i = 0; i < buf_len; ++i)  if (i % 2 != 0) buf_ptr[i] = buf_ptr[i] & 0xAA; break;
      case 2: for (uint16_t i = 0; i < buf_len; ++i)  if (i % 2 != 0) buf_ptr[i] = buf_ptr[i] & 0x00; break;
      case 3: for (uint16_t i = 0; i < buf_len; ++i)  if (i % 2 == 0) buf_ptr[i] = buf_ptr[i] & 0x55; break;
      case 4: for (uint16_t i = 0; i < buf_len; ++i)  if (i % 2 == 0) buf_ptr[i] = buf_ptr[i] & 0x00; break;
      default: ui.state = S_NONE; ui.fade = 0; break;
    }
  }
  else
  {
    switch (ui.fade)
    {
      case 1: for (uint16_t i = 0; i < buf_len; ++i)  if (i % 2 != 0) buf_ptr[i] = buf_ptr[i] | 0xAA; break;
      case 2: for (uint16_t i = 0; i < buf_len; ++i)  if (i % 2 != 0) buf_ptr[i] = buf_ptr[i] | 0x00; break;
      case 3: for (uint16_t i = 0; i < buf_len; ++i)  if (i % 2 == 0) buf_ptr[i] = buf_ptr[i] | 0x55; break;
      case 4: for (uint16_t i = 0; i < buf_len; ++i)  if (i % 2 == 0) buf_ptr[i] = buf_ptr[i] | 0x00; break;
      default: ui.state = S_NONE; ui.fade = 0; break;
    }    
  }
  ui.fade++;
}

void WouoUI_Class::tile_show(struct MENU arr_1[], struct MENU arr_2[], const uint8_t icon_pic[][16*18])
{
  animation(&tile.icon_x, &tile.icon_x_trg, TILE_ANI);
  animation(&tile.icon_y, &tile.icon_y_trg, TILE_ANI);
  animation(&tile.indi_x, &tile.indi_x_trg, TILE_ANI);
  animation(&tile.title_y, &tile.title_y_trg, TILE_ANI);

  u8g2.setDrawColor(1);
  u8g2.setFontDirection(0);

  u8g2.setFont(TILE_B_FONT); 
  u8g2.drawStr(((DISP_W - TILE_INDI_W) - u8g2.getStrWidth(arr_1[ui.select[ui.layer]].m_select)) / 2 + TILE_INDI_W, tile.title_y, arr_1[ui.select[ui.layer]].m_select);

  u8g2.setFont(TILE_S_FONT);
  u8g2.drawStr(((DISP_W - u8g2.getStrWidth(arr_2[ui.select[ui.layer]].m_select)) / 2), 0.5 * (TILE_ICON_S + TILE_INDI_H + DISP_H + LIST_TEXT_H), arr_2[ui.select[ui.layer]].m_select);

  u8g2.drawBox(0, TILE_ICON_S, tile.indi_x, TILE_INDI_H);

  if (!ui.init)
  {
    for (uint8_t i = 0; i < ui.num[ui.index]; ++i)  
    {
      if (ui.param[TILE_UFD]) tile.temp = (DISP_W - TILE_ICON_W) / 2 + i * tile.icon_x - TILE_ICON_S * ui.select[ui.layer];
      else tile.temp = (DISP_W - TILE_ICON_W) / 2 + (i - ui.select[ui.layer]) * tile.icon_x;
      u8g2.drawXBMP(tile.temp, (int16_t)tile.icon_y, TILE_ICON_W, TILE_ICON_H, icon_pic[i]); 
    }
    if (tile.icon_x == tile.icon_x_trg) 
    {
      ui.init = true;
      tile.icon_x = tile.icon_x_trg = - ui.select[ui.layer] * TILE_ICON_S;
    }
  }
  else for (uint8_t i = 0; i < ui.num[ui.index]; ++i) u8g2.drawXBMP((DISP_W - TILE_ICON_W) / 2 + (int16_t)tile.icon_x + i * TILE_ICON_S, 0, TILE_ICON_W, TILE_ICON_H, icon_pic[i]);

  u8g2.setDrawColor(2);
  if (!ui.param[DARK_MODE]) u8g2.drawBox(0, 0, DISP_W, DISP_H);
}

void WouoUI_Class::list_draw_value(int n) { u8g2.print(check_box.v[n - 1]); }
void WouoUI_Class::list_draw_check_box_frame() { u8g2.drawRFrame(CHECK_BOX_L_S, list.temp + CHECK_BOX_U_S, CHECK_BOX_F_W, CHECK_BOX_F_H, 1); }
void WouoUI_Class::list_draw_check_box_dot() { u8g2.drawBox(CHECK_BOX_L_S + CHECK_BOX_D_S + 1, list.temp + CHECK_BOX_U_S + CHECK_BOX_D_S + 1, CHECK_BOX_F_W - (CHECK_BOX_D_S + 1) * 2, CHECK_BOX_F_H - (CHECK_BOX_D_S + 1) * 2); }

void WouoUI_Class::list_draw_krf(int n) 
{ 
  switch (check_box.v[n - 1])
  {
    case 0: u8g2.print("OFF"); break;
    case 1: u8g2.print("VOL"); break;
    case 2: u8g2.print("BRI"); break;
  }
}

void WouoUI_Class::list_draw_kpf(int n) 
{ 
  if (check_box.v[n - 1] == 0) u8g2.print("OFF");
  else if (check_box.v[n - 1] <= 90) u8g2.print((char)check_box.v[n - 1]);
  else u8g2.print("?");
}

void WouoUI_Class::list_draw_text_and_check_box(struct MENU arr[], int i)
{
  u8g2.drawStr(LIST_TEXT_S, list.temp + LIST_TEXT_H + LIST_TEXT_S, arr[i].m_select);
  u8g2.setCursor(CHECK_BOX_L_S, list.temp + LIST_TEXT_H + LIST_TEXT_S);
  switch (arr[i].m_select[0])
  {
    case '~': list_draw_value(i); break;
    case '+': list_draw_check_box_frame(); if (check_box.m[i - 1] == 1)  list_draw_check_box_dot(); break;
    case '=': list_draw_check_box_frame(); if (*check_box.s_p == i)      list_draw_check_box_dot(); break;
    case '#': list_draw_krf(i);   break;
    case '$': list_draw_kpf(i);   break;
  }
}

void WouoUI_Class::list_show(struct MENU arr[], uint8_t ui_index)
{
  u8g2.setFont(LIST_FONT);
  list.box_x_trg = u8g2.getStrWidth(arr[ui.select[ui.layer]].m_select) + LIST_TEXT_S * 2;
  list.bar_y_trg = ceil((ui.select[ui.layer]) * ((float)DISP_H / (ui.num[ui_index] - 1)));
  
  animation(&list.y, &list.y_trg, LIST_ANI);
  animation(&list.box_x, &list.box_x_trg, LIST_ANI);
  animation(&list.box_y, &list.box_y_trg[ui.layer], LIST_ANI);
  animation(&list.bar_y, &list.bar_y_trg, LIST_ANI);

  if (list.loop && list.box_y == list.box_y_trg[ui.layer]) list.loop = false;

  u8g2.setDrawColor(1);
  
  u8g2.drawHLine(DISP_W - LIST_BAR_W, 0, LIST_BAR_W);
  u8g2.drawHLine(DISP_W - LIST_BAR_W, DISP_H - 1, LIST_BAR_W);
  u8g2.drawVLine(DISP_W - ceil((float)LIST_BAR_W / 2), 0, DISP_H);
  u8g2.drawBox(DISP_W - LIST_BAR_W, 0, LIST_BAR_W, list.bar_y);

  if (!ui.init)
  {
    for (int i = 0; i < ui.num[ui_index]; ++ i)
    {
      if (ui.param[LIST_UFD]) list.temp = i * list.y - LIST_LINE_H * ui.select[ui.layer] + list.box_y_trg[ui.layer];
      else list.temp = (i - ui.select[ui.layer]) * list.y + list.box_y_trg[ui.layer];
      list_draw_text_and_check_box(arr, i);
    }
    if (list.y == list.y_trg) 
    {
      ui.init = true;
      list.y = list.y_trg = - LIST_LINE_H * ui.select[ui.layer] + list.box_y_trg[ui.layer];
    }
  }
  else for (int i = 0; i < ui.num[ui_index]; ++ i)
  {
    list.temp = LIST_LINE_H * i + list.y;
    list_draw_text_and_check_box(arr, i);
  }

  u8g2.setDrawColor(2);
  u8g2.drawRBox(0, list.box_y, list.box_x, LIST_LINE_H, LIST_BOX_R);

  if (!ui.param[DARK_MODE])
  {
    u8g2.drawBox(0, 0, DISP_W, DISP_H);
    switch(ui.index)
    {
      case M_WINDOW: 
      case M_VOLT:
      u8g2.drawBox(0, 0, DISP_W, DISP_H);  
    }
  }
}

void WouoUI_Class::volt_show()
{
  u8g2.setFont(LIST_FONT);
  list.box_x_trg = u8g2.getStrWidth(volt_menu[ui.select[ui.layer]].m_select) + LIST_TEXT_S * 2;

  animation(&list.y, &list.y_trg, LIST_ANI);
  animation(&list.box_x, &list.box_x_trg, LIST_ANI);
  animation(&list.box_y, &list.box_y_trg[ui.layer], LIST_ANI);
  animation(&volt.text_bg_l, &volt.text_bg_l_trg, TAG_ANI);

  if (list.loop && list.box_y == list.box_y_trg[ui.layer]) list.loop = false;

  u8g2.setDrawColor(1);  

  u8g2.setFontDirection(1);
  if (!ui.init)
  {
    for (uint8_t i = 0; i < ui.num[ui.index]; ++ i) u8g2.drawStr(LIST_TEXT_S + (i - ui.select[ui.layer]) * list.y + list.box_y_trg[ui.layer] - 1, VOLT_LIST_U_S , volt_menu[i].m_select);
    if (list.y == list.y_trg) 
    {
      ui.init = true;
      list.y = list.y_trg = - LIST_LINE_H * ui.select[ui.layer] + list.box_y_trg[ui.layer];
    }
  }
  else for (uint8_t i = 0; i < ui.num[ui.index]; ++ i) u8g2.drawStr(LIST_TEXT_S + LIST_LINE_H * i + (int16_t)list.y - 1, VOLT_LIST_U_S , volt_menu[i].m_select); 
  
  volt.val = 0;
  u8g2.drawFrame(0, 0, WAVE_BOX_W, WAVE_BOX_H);
  u8g2.drawFrame(1, 1, WAVE_BOX_W - 2, WAVE_BOX_H - 2);
  if (list.box_y == list.box_y_trg[ui.layer] && list.y == list.y_trg)
  {
    // ESP32 ADC 读取
    for (int i = 0; i < WAVE_SAMPLE * WAVE_W; i++) volt.ch0_adc[i] = volt.val = analogRead(analog_pin[ui.select[ui.layer]]);
    for (int i = 1; i < WAVE_W - 1; i++)
    { 
      volt.ch0_wave[i] = map(volt.ch0_adc[int(5 * i)], 0, 4095, WAVE_MAX, WAVE_MIN);   
      u8g2.drawLine(WAVE_L + i - 1, WAVE_U + volt.ch0_wave[i - 1], WAVE_L + i, WAVE_U + volt.ch0_wave[i]);
    }
  }

  u8g2.setFontDirection(0);
  u8g2.setFont(VOLT_FONT); 
  u8g2.setCursor(23, VOLT_LIST_U_S - 12);
  u8g2.print(volt.val / 4096.0f * 3.3f);
  u8g2.print("V");

  u8g2.setDrawColor(2);
  u8g2.drawRBox(list.box_y, VOLT_LIST_U_S - LIST_TEXT_S, LIST_LINE_H, list.box_x, LIST_BOX_R);
  u8g2.drawBox(DISP_W - volt.text_bg_l, VOLT_TEXT_BG_U_S, DISP_W, VOLT_TEXT_BG_H);

  if (!ui.param[DARK_MODE]) u8g2.drawBox(0, 0, DISP_W, DISP_H);
}

void WouoUI_Class::window_show()
{
  list_show(win.bg, win.index);
  if (ui.param[WIN_BOK]) for (uint16_t i = 0; i < buf_len; ++i)  buf_ptr[i] = buf_ptr[i] & (i % 2 == 0 ? 0x55 : 0xAA);

  u8g2.setFont(WIN_FONT);
  win.bar_trg = (float)(*win.value - win.min) / (float)(win.max - win.min) * (WIN_BAR_W - 4);

  animation(&win.bar, &win.bar_trg, WIN_ANI);
  animation(&win.y, &win.y_trg, WIN_ANI);

  u8g2.setDrawColor(0); u8g2.drawRBox(win.l, (int16_t)win.y, WIN_W, WIN_H, 2);    
  u8g2.setDrawColor(1); u8g2.drawRFrame(win.l, (int16_t)win.y, WIN_W, WIN_H, 2);  
  u8g2.drawRFrame(win.l + 5, (int16_t)win.y + 20, WIN_BAR_W, WIN_BAR_H, 1);       
  u8g2.drawBox(win.l + 7, (int16_t)win.y + 22, win.bar, WIN_BAR_H - 4);           
  u8g2.setCursor(win.l + 5, (int16_t)win.y + 14); u8g2.print(win.title);          
  u8g2.setCursor(win.l + 78, (int16_t)win.y + 14); u8g2.print(*win.value);        
  
  if (!strcmp(win.title, "Disp Bri")) u8g2.setContrast(ui.param[DISP_BRI]);

  u8g2.setDrawColor(2);
  if (!ui.param[DARK_MODE]) u8g2.drawBox(0, 0, DISP_W, DISP_H);
}

void WouoUI_Class::about_show()
{
  u8g2.setFont(ABOUT_FONT);
  list.box_x_trg = u8g2.getStrWidth(about_menu[0].m_select) + LIST_TEXT_S * 2;

  animation(&list.box_x, &list.box_x_trg, TAG_ANI);
  animation(&about.indi_x, &about.indi_x_trg, TAG_ANI);

  u8g2.setDrawColor(1);

  u8g2.drawStr(ABOUT_INDI_S + LIST_TEXT_S, ABOUT_INDI_S + LIST_TEXT_S + LIST_TEXT_H, about_menu[0].m_select);
  u8g2.drawStr(ABOUT_INDI_S + list.box_x_trg + ABOUT_INDI_S, ABOUT_INDI_S + LIST_TEXT_S + LIST_TEXT_H, about_menu[1].m_select);
  for (int i = 2 ; i < ui.num[M_ABOUT] ; i++) u8g2.drawStr(about.indi_x_trg + ABOUT_INDI_W + ABOUT_INDI_S * 2, ABOUT_INDI_S + LIST_LINE_H + LIST_TEXT_S / 2 + (i - 1) * LIST_LINE_H, about_menu[i].m_select);
  u8g2.drawBox(about.indi_x, ABOUT_INDI_S + LIST_LINE_H + ABOUT_INDI_S * 2, ABOUT_INDI_W, (ui.num[M_ABOUT] - 2) * LIST_LINE_H - LIST_TEXT_S);

  u8g2.setDrawColor(2);
  u8g2.drawRBox(ABOUT_INDI_S, ABOUT_INDI_S, list.box_x, LIST_LINE_H, LIST_BOX_R);

  if (!ui.param[DARK_MODE]) u8g2.drawBox(0, 0, DISP_W, DISP_H);
}

void WouoUI_Class::tile_rotate_switch()
{
  switch (btn.id)
  { 
    case BTN_ID_CC:
      if (ui.init)
      {
        if (ui.select[ui.layer] > 0)
        {
          ui.select[ui.layer] -= 1;
          tile.icon_x_trg += TILE_ICON_S;
          tile.select_flag = false;
        }
        else 
        {
          if (ui.param[TILE_LOOP])
          {
            ui.select[ui.layer] = ui.num[ui.index] - 1;
            tile.icon_x_trg = - TILE_ICON_S * (ui.num[ui.index] - 1);
            break;
          }
          else tile.select_flag = true;
        }
      }
      break;

    case BTN_ID_CW:
      if (ui.init)
      {
        if (ui.select[ui.layer] < (ui.num[ui.index] - 1)) 
        {
          ui.select[ui.layer] += 1;
          tile.icon_x_trg -= TILE_ICON_S;
          tile.select_flag = false;
        }
        else 
        {
          if (ui.param[TILE_LOOP])
          {
            ui.select[ui.layer] = 0;
            tile.icon_x_trg = 0;
            break;
          }
          else tile.select_flag = true;
        }
      }
      break;
  }
}

void WouoUI_Class::list_rotate_switch()
{
  if (!list.loop)
  {
    switch (btn.id)
    {
      case BTN_ID_CC:
        if (ui.select[ui.layer] == 0)
        {
          if (ui.param[LIST_LOOP] && ui.init)
          {
            list.loop = true;
            ui.select[ui.layer] = ui.num[ui.index] - 1;
            if (ui.num[ui.index] > list.line_n) 
            {
              list.box_y_trg[ui.layer] = DISP_H - LIST_LINE_H;
              list.y_trg = DISP_H - ui.num[ui.index] * LIST_LINE_H;
            }
            else list.box_y_trg[ui.layer] = (ui.num[ui.index] - 1) * LIST_LINE_H;
            break;
          }
          else break;
        }
        if (ui.init)
        {
          ui.select[ui.layer] -= 1;
          if (ui.select[ui.layer] < - (list.y_trg / LIST_LINE_H)) 
          {
            if (!(DISP_H % LIST_LINE_H)) list.y_trg += LIST_LINE_H;
            else
            {
              if (list.box_y_trg[ui.layer] == DISP_H - LIST_LINE_H * list.line_n)
              {
                list.y_trg += (list.line_n + 1) * LIST_LINE_H - DISP_H;
                list.box_y_trg[ui.layer] = 0;
              }
              else if (list.box_y_trg[ui.layer] == LIST_LINE_H)
              {
                list.box_y_trg[ui.layer] = 0;
              }
              else list.y_trg += LIST_LINE_H;
            }
          }
          else list.box_y_trg[ui.layer] -= LIST_LINE_H;
          break;
        }

      case BTN_ID_CW:
        if (ui.select[ui.layer] == (ui.num[ui.index] - 1))
        {
          if (ui.param[LIST_LOOP] && ui.init)
          {
            list.loop = true;
            ui.select[ui.layer] = 0;
            list.y_trg = 0;
            list.box_y_trg[ui.layer] = 0;
            break;
          }
          else break;
        }
        if (ui.init)
        {
          ui.select[ui.layer] += 1;
          if ((ui.select[ui.layer] + 1) > (list.line_n - list.y_trg / LIST_LINE_H))
          {
            if (!(DISP_H % LIST_LINE_H)) list.y_trg -= LIST_LINE_H;
            else
            {
              if (list.box_y_trg[ui.layer] == LIST_LINE_H * (list.line_n - 1))
              {
                list.y_trg -= (list.line_n + 1) * LIST_LINE_H - DISP_H;
                list.box_y_trg[ui.layer] = DISP_H - LIST_LINE_H;
              }
              else if (list.box_y_trg[ui.layer] == DISP_H - LIST_LINE_H * 2)
              {
                list.box_y_trg[ui.layer] = DISP_H - LIST_LINE_H;
              }
              else list.y_trg -= LIST_LINE_H;
            }
          }
          else list.box_y_trg[ui.layer] += LIST_LINE_H;
          break;
        }
        break;
    }
  }
}

void WouoUI_Class::window_proc()
{
  window_show();
  if (win.y == WIN_Y_TRG) ui.index = win.index;
  if (btn.pressed && win.y == win.y_trg && win.y != WIN_Y_TRG)
  {
    btn.pressed = false;
    switch (btn.id)
    {
      case BTN_ID_CW: if (*win.value < win.max)  *win.value += win.step;  eeprom.change = true;  break;
      case BTN_ID_CC: if (*win.value > win.min)  *win.value -= win.step;  eeprom.change = true;  break;  
      case BTN_ID_SP: case BTN_ID_LP: win.y_trg = WIN_Y_TRG; break;
    }
  }
}

void WouoUI_Class::sleep_proc()
{
  while (ui.sleep)
  {
    btn_scan();
    if (btn.pressed) { btn.pressed = false; switch (btn.id) {    
        case BTN_ID_CW:
          // ESP32 USB HID not implemented in this port, add USBHID library to support
          break;
        case BTN_ID_CC:
          // ESP32 USB HID not implemented in this port
          break;
        case BTN_ID_SP: 
          // Keyboard.press(knob.param[KNOB_COD]); ...
          break;   
        case BTN_ID_LP: ui.index = M_MAIN;  ui.state = S_LAYER_IN; u8g2.setPowerSave(0); ui.sleep = false; break;
      }
    }
  }
}

void WouoUI_Class::main_proc()
{
  tile_show(main_menu, main_menu_exp, main_icon_pic);
  if (btn.pressed) { btn.pressed = false; switch (btn.id) { case BTN_ID_CW: case BTN_ID_CC: tile_rotate_switch(); break; case BTN_ID_SP: switch (ui.select[ui.layer]) {
        case 0: ui.index = M_SLEEP;   ui.state = S_LAYER_OUT; break;
        case 1: ui.index = M_EDITOR;  ui.state = S_LAYER_IN;  break;
        case 2: ui.index = M_VOLT;    ui.state = S_LAYER_IN;  break;
        case 3: ui.index = M_SETTING; ui.state = S_LAYER_IN;  break;
      }
    }
    if (!tile.select_flag && ui.init) { tile.indi_x = 0; tile.title_y = tile.title_y_calc; }
  }
}

void WouoUI_Class::editor_proc()
{
  list_show(editor_menu, M_EDITOR); 
  if (btn.pressed) { btn.pressed = false; switch (btn.id) { case BTN_ID_CW: case BTN_ID_CC: list_rotate_switch(); break; case BTN_ID_LP: ui.select[ui.layer] = 0; case BTN_ID_SP: switch (ui.select[ui.layer]) {
        case 0:   ui.index = M_MAIN;  ui.state = S_LAYER_OUT; break;
        case 11:  ui.index = M_KNOB;  ui.state = S_LAYER_IN;  break;
      }
    }
  }
}

void WouoUI_Class::knob_proc()
{
  list_show(knob_menu, M_KNOB);
  if (btn.pressed) { btn.pressed = false; switch (btn.id) { case BTN_ID_CW: case BTN_ID_CC: list_rotate_switch(); break; case BTN_ID_LP: ui.select[ui.layer] = 0; case BTN_ID_SP: switch (ui.select[ui.layer]) {
        case 0: ui.index = M_EDITOR;  ui.state = S_LAYER_OUT; break;
        case 1: ui.index = M_KRF;     ui.state = S_LAYER_IN;  check_box_s_init(&knob.param[KNOB_ROT], &knob.param[KNOB_ROT_P]); break;
        case 2: ui.index = M_KPF;     ui.state = S_LAYER_IN;  check_box_s_init(&knob.param[KNOB_COD], &knob.param[KNOB_COD_P]); break;
      }
    }
  }
}

void WouoUI_Class::krf_proc()
{
  list_show(krf_menu, M_KRF);
  if (btn.pressed) { btn.pressed = false; switch (btn.id) { case BTN_ID_CW: case BTN_ID_CC: list_rotate_switch(); break; case BTN_ID_LP: ui.select[ui.layer] = 0; case BTN_ID_SP: switch (ui.select[ui.layer]) {
        case 0: ui.index = M_KNOB;  ui.state = S_LAYER_OUT; break;
        case 1: break;
        case 2: check_box_s_select(KNOB_DISABLE, ui.select[ui.layer]); break;
        case 3: break;
        case 4: check_box_s_select(KNOB_ROT_VOL, ui.select[ui.layer]); break;
        case 5: check_box_s_select(KNOB_ROT_BRI, ui.select[ui.layer]); break;
        case 6: break;
      }
    }
  }
}

void WouoUI_Class::kpf_proc()
{
  list_show(kpf_menu, M_KPF);
  if (btn.pressed) { btn.pressed = false; switch (btn.id) { case BTN_ID_CW: case BTN_ID_CC: list_rotate_switch(); break;  case BTN_ID_LP: ui.select[ui.layer] = 0; case BTN_ID_SP: switch (ui.select[ui.layer]) {
        case 0:   ui.index = M_KNOB;  ui.state = S_LAYER_OUT; break;
        // 原代码中大量的Key Mapping，这里为了简洁和编译通过省略，如需使用请恢复并配合 USBHID 库
        default:  check_box_s_select('A' + ui.select[ui.layer], ui.select[ui.layer]); break; 
      }
    }
  }
}

void WouoUI_Class::volt_proc()
{
  volt_show();
  if (btn.pressed) { btn.pressed = false; switch (btn.id) { case BTN_ID_CW: case BTN_ID_CC: list_rotate_switch(); break;
      case BTN_ID_SP: case BTN_ID_LP: ui.index = M_MAIN;  ui.state = S_LAYER_OUT; break;
    }
  }
}

void WouoUI_Class::setting_proc()
{
  list_show(setting_menu, M_SETTING);
  if (btn.pressed) { btn.pressed = false; switch (btn.id) { case BTN_ID_CW: case BTN_ID_CC: list_rotate_switch(); break; case BTN_ID_LP: ui.select[ui.layer] = 0; case BTN_ID_SP: switch (ui.select[ui.layer]) {
        case 0:   ui.index = M_MAIN;  ui.state = S_LAYER_OUT; break;
        case 1:   window_value_init("Disp Bri", DISP_BRI, &ui.param[DISP_BRI],  255,  0,  5, setting_menu, M_SETTING);  break;
        case 2:   window_value_init("Tile Ani", TILE_ANI, &ui.param[TILE_ANI],  100, 10,  1, setting_menu, M_SETTING);  break;
        case 3:   window_value_init("List Ani", LIST_ANI, &ui.param[LIST_ANI],  100, 10,  1, setting_menu, M_SETTING);  break;
        case 4:   window_value_init("Win Ani",  WIN_ANI,  &ui.param[WIN_ANI],   100, 10,  1, setting_menu, M_SETTING);  break;
        case 5:   window_value_init("Spot Ani", SPOT_ANI, &ui.param[SPOT_ANI],  100, 10,  1, setting_menu, M_SETTING);  break;
        case 6:   window_value_init("Tag Ani",  TAG_ANI,  &ui.param[TAG_ANI],   100, 10,  1, setting_menu, M_SETTING);  break;
        case 7:   window_value_init("Fade Ani", FADE_ANI, &ui.param[FADE_ANI],  255,  0,  1, setting_menu, M_SETTING);  break;
        case 8:   window_value_init("Btn SPT",  BTN_SPT,  &ui.param[BTN_SPT],   255,  0,  1, setting_menu, M_SETTING);  break;
        case 9:   window_value_init("Btn LPT",  BTN_LPT,  &ui.param[BTN_LPT],   255,  0,  1, setting_menu, M_SETTING);  break;
        case 10:  check_box_m_select( TILE_UFD  );  break;
        case 11:  check_box_m_select( LIST_UFD  );  break;
        case 12:  check_box_m_select( TILE_LOOP );  break;
        case 13:  check_box_m_select( LIST_LOOP );  break;
        case 14:  check_box_m_select( WIN_BOK   );  break;
        case 15:  check_box_m_select( KNOB_DIR  );  break;
        case 16:  check_box_m_select( DARK_MODE );  break;
        case 17:  ui.index = M_ABOUT; ui.state = S_LAYER_IN; break;
      }
    }
  }
}

void WouoUI_Class::about_proc()
{
  about_show();
  if (btn.pressed) { btn.pressed = false; switch (btn.id) {
      case BTN_ID_SP: case BTN_ID_LP: ui.index = M_SETTING;  ui.state = S_LAYER_OUT;  break;
    }
  }
}

void WouoUI_Class::ui_proc()
{
  u8g2.sendBuffer();
  switch (ui.state)
  {
    case S_FADE:          fade();                   break;  //转场动画
    case S_WINDOW:        window_param_init();      break;  //弹窗初始化
    case S_LAYER_IN:      layer_init_in();          break;  //层级初始化
    case S_LAYER_OUT:     layer_init_out();         break;  //层级初始化
  
    case S_NONE: u8g2.clearBuffer(); switch (ui.index)      //直接选择页面
    {
      case M_WINDOW:      window_proc();            break;
      case M_SLEEP:       sleep_proc();             break;
      case M_MAIN:        main_proc();              break;
      case M_EDITOR:      editor_proc();            break;
      case M_KNOB:        knob_proc();              break;
      case M_KRF:         krf_proc();               break;
      case M_KPF:         kpf_proc();               break;
      case M_VOLT:        volt_proc();              break;
      case M_SETTING:     setting_proc();           break;
      case M_ABOUT:       about_proc();             break;
    }
  }
}

void WouoUI_Class::oled_init()
{
  // ------------------- 修改部分 START -------------------
  
  // 1. 初始化 ESP32-S3 的硬件 SPI 引脚映射
  // 参数: SPI.begin(SCK, MISO, MOSI, SS);
  // OLED_SCL -> SCK (时钟)
  // -1       -> MISO (OLED 不需要读数据，设为 -1)
  // OLED_SDA -> MOSI (数据发送)
  // OLED_CS  -> SS   (片选)
  SPI.begin(OLED_SCL, -1, OLED_SDA, OLED_CS);

  // 2. 设置 SPI 频率
  // SH1107 通常能跑很高，ESP32 硬件 SPI 可以轻松跑 20MHz+
  // 如果屏幕花屏或不显示，可以尝试降低到 10000000 (10MHz)
  u8g2.setBusClock(10000000); 

  u8g2.begin();

  u8g2.setContrast(ui.param[DISP_BRI]);
  buf_ptr = u8g2.getBufferPtr();
  buf_len = 8 * u8g2.getBufferTileHeight() * u8g2.getBufferTileWidth();
}

void WouoUI_Class::begin() 
{
  analogReadResolution(12); // ESP32 默认 12 位 ADC (0-4095)
  eeprom_init();
  ui_init();
  oled_init();
  btn_init();
}

void WouoUI_Class::loop() 
{
  btn_scan();
  ui_proc();
}