#include "lvgl/lvgl.h"
#include <stdio.h>

static lv_obj_t *led;
static lv_obj_t *red_slider, *green_slider, *blue_slider, *switch_btn;
static lv_obj_t *arc;
static lv_color_t current_led_color;
static uint8_t current_led_brightness;
static bool led_is_on;
extern uint32_t mqtt_publish_data(int socket_fd, char * topic, char * message, uint8_t qos);
static int fd;
lv_obj_t *diban =NULL;

// 弧度控件的值改变时调用，用于调整LED的亮度
static void arc_value_changed_event_cb(lv_event_t * event) {
    if(lv_obj_has_state(switch_btn, LV_STATE_CHECKED)) {
        lv_obj_t *arc = lv_event_get_target(event);
        int val = lv_arc_get_value(arc); // 获取弧度控件的值
        lv_led_set_brightness(led, val * 2.55); // 将0-100的值映射到0-255的亮度范围
    }
}

// 滑动条的值改变时调用，用于更新LED的颜色
static void slider_value_changed_event_cb(lv_event_t * event) {
    if(lv_obj_has_state(switch_btn, LV_STATE_CHECKED)) {
        lv_color_t color = lv_color_make(
            (int)(2.25*(lv_slider_get_value(red_slider))),
            (int)(2.25*(lv_slider_get_value(green_slider))),
            (int)(2.25*(lv_slider_get_value(blue_slider)))
        );
        lv_led_set_color(led, color);
    }
}

// 开关的事件回调函数，用于控制LED的开关
static void switch_event_cb(lv_event_t * event) {
    lv_obj_t *sw = lv_event_get_target(event);
    bool state = lv_obj_has_state(sw, LV_STATE_CHECKED);
    if(state) {
        lv_led_on(led);
        // 启用滑动条和弧度控件
        lv_obj_add_flag(red_slider, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(green_slider, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(blue_slider, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(arc, LV_OBJ_FLAG_CLICKABLE);
    } else {
        lv_led_off(led);
        // 禁用滑动条和弧度控件
        lv_obj_clear_flag(red_slider, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(green_slider, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(blue_slider, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);
    }
    update_led_state();
}

//获取信息到MQTT
void update_led_state(void) {
    //printf("fd:%d",fd);
    char bf[512] = {0};
    if(lv_obj_has_state(switch_btn, LV_STATE_CHECKED)) {
        led_is_on = true;
        int current_led_red;
        int current_led_green;
        int current_led_blue;
        current_led_brightness = lv_arc_get_value(arc) * 2.55; // 映射到0-255的亮度范围
        current_led_red = lv_slider_get_value(red_slider);
        current_led_green = lv_slider_get_value(green_slider);
        current_led_blue = lv_slider_get_value(blue_slider);
        sprintf(bf,"{\"method\":\"report\",\"clientToken\":\"123\",\"params\":{\"power_switch\":%d,\"brightness\":%d,\"rgb_color\":{\"red\":%d,\"green\":%d,\"blue\":%d}}}",led_is_on,current_led_brightness,current_led_red,current_led_green,current_led_blue);
        printf("我写的:%s\n",bf);
        mqtt_publish_data( fd,"$thing/up/property/MRJO085OTL/RGB_1",bf,0);
    } else {
        led_is_on = false;
        sprintf(bf,"{\"method\":\"report\",\"clientToken\":\"123\",\"params\":{\"power_switch\":%d}}",led_is_on);
        printf("我写的:%s\n",bf);
        mqtt_publish_data( fd,"$thing/up/property/MRJO085OTL/RGB_1",bf,0);
    }
}

//接收处理接口
void xingxi(int nero,int zi){
    if (nero == 1)
    {
        lv_obj_t *kg = lv_obj_get_child(diban, 8);
        if (zi == 0)
        {
            lv_obj_clear_state(kg,LV_STATE_CHECKED);
        }else
        {
            lv_obj_add_state(kg,LV_STATE_CHECKED);
        }
        lv_event_send(kg,LV_EVENT_VALUE_CHANGED,NULL);
    }else if (nero == 2)
    {
        lv_obj_t *ldu = lv_obj_get_child(diban, 7);
        lv_arc_set_value(ldu,zi);
        lv_event_send(ldu,LV_EVENT_VALUE_CHANGED,NULL);
    }else if (nero == 3)
    {
        lv_obj_t *red = lv_obj_get_child(diban,1);
        lv_slider_set_value(red,zi,0);
        lv_event_send(red,LV_EVENT_VALUE_CHANGED,NULL);
    }else if (nero == 4)
    {
        lv_obj_t *green = lv_obj_get_child(diban,3);
        lv_slider_set_value(green,zi,0);
        lv_event_send(green,LV_EVENT_VALUE_CHANGED,NULL);
    }else if (nero == 5)
    {
        lv_obj_t *blue = lv_obj_get_child(diban,5);
        lv_slider_set_value(blue,zi,0);
        lv_event_send(blue,LV_EVENT_VALUE_CHANGED,NULL);
    }
    
    


    
}



void create_ui(int fde) {
    fd = fde;
    diban = lv_obj_create(lv_scr_act()); // 创建底版
    lv_obj_set_size(diban, 800, 480); // 底版大小
    lv_obj_set_style_bg_color(diban, lv_color_hex(0xADD8E6), 0); // 设置背景颜色为浅蓝色

    // 创建LED，并调整为长条形状
    led = lv_led_create(diban);
    lv_obj_set_size(led, 400, 40); // 设置LED为长条形状
    lv_obj_align(led, LV_ALIGN_TOP_MID, 0, 20);
    lv_led_off(led); // 默认关闭LED

    // 创建颜色滑动条和标签
    red_slider = lv_slider_create(diban);
    lv_slider_set_range(red_slider,1,100);
    lv_obj_set_size(red_slider, 200, 30);
    lv_obj_set_style_bg_color(red_slider, lv_color_hex(0xFF0000), LV_PART_INDICATOR); // 设置滑动条为红色
    
    lv_obj_align(red_slider, LV_ALIGN_LEFT_MID, 0, 0); // 调整位置
    lv_obj_t *red_label = lv_label_create(diban);
    lv_label_set_text(red_label, "Red");
    lv_obj_align_to(red_label, red_slider, LV_ALIGN_OUT_TOP_MID, 0, -10);

    green_slider = lv_slider_create(diban);
    lv_slider_set_range(green_slider,1,100);
    lv_obj_set_size(green_slider, 200, 30);
    lv_obj_set_style_bg_color(green_slider, lv_color_hex(0x00FF00), LV_PART_INDICATOR); // 设置滑动条为绿色
  
    lv_obj_align(green_slider, LV_ALIGN_CENTER, 0, 0); // 调整位置
    lv_obj_t *green_label = lv_label_create(diban);
    lv_label_set_text(green_label, "Green");
    lv_obj_align_to(green_label, green_slider, LV_ALIGN_OUT_TOP_MID, 0, -10);

    blue_slider = lv_slider_create(diban);
    lv_slider_set_range(blue_slider,1,100);
    lv_obj_set_size(blue_slider, 200, 30);
    lv_obj_set_style_bg_color(blue_slider, lv_color_hex(0x0000FF), LV_PART_INDICATOR); // 设置滑动条为蓝色
 
    lv_obj_align(blue_slider, LV_ALIGN_RIGHT_MID, 0, 0); // 调整位置
    lv_obj_t *blue_label = lv_label_create(diban);
    lv_label_set_text(blue_label, "Blue");
    lv_obj_align_to(blue_label, blue_slider, LV_ALIGN_OUT_TOP_MID, 0, -10);

    // 添加事件回调
    lv_obj_add_event_cb(red_slider, slider_value_changed_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(green_slider, slider_value_changed_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(blue_slider, slider_value_changed_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_add_event_cb(red_slider, update_led_state, LV_EVENT_RELEASED, NULL);
    lv_obj_add_event_cb(green_slider, update_led_state, LV_EVENT_RELEASED, NULL);
    lv_obj_add_event_cb(blue_slider, update_led_state, LV_EVENT_RELEASED, NULL);
    

    // 创建弧度控件
    arc = lv_arc_create(diban);
    lv_obj_set_size(arc, 170, 170);
    lv_arc_set_rotation(arc, 135);
    lv_arc_set_bg_angles(arc, 0, 270);
    lv_arc_set_range(arc, 0, 100); // 设置亮度调节范围
    lv_arc_set_value(arc, 50); // 默认亮度
    lv_obj_align(arc, LV_ALIGN_BOTTOM_MID, 0, 20); // 调整位置
    lv_obj_add_event_cb(arc, arc_value_changed_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(arc, update_led_state, LV_EVENT_RELEASED, NULL);
    

    // 创建开关控件
    switch_btn = lv_switch_create(diban);
    lv_obj_align(switch_btn, LV_ALIGN_BOTTOM_LEFT, 20, -20); // 将开关放置在左下角
    lv_obj_add_event_cb(switch_btn, switch_event_cb, LV_EVENT_VALUE_CHANGED,NULL);

    // 默认禁用滑动条和弧度控件
    lv_obj_clear_flag(red_slider, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(green_slider, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(blue_slider, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);
}