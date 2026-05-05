#include <Ethernet.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include "lvgl86.h"
 
byte mac_addr[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

IPAddress server_addr(192,168,2,130);  // IP of the MySQL *server* here
char user[] = "root";              // MySQL user login username
char password[] = "roboard";        // MySQL user login password

// Sample query
char INSERT_SQL[] = "INSERT INTO test.acen (name, age, class_name) values ('acen', 34, '405')";

EthernetClient client;
unsigned long t = 0;
MySQL_Connection conn((Client *)&client);

static void event_handler1(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) {
        Serial.println("Recording data.");
        MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
        cur_mem->execute(INSERT_SQL);
        delete cur_mem;
    }
}

lv_obj_t *sw2;
int sw_state = 0; // 0: OFF, 1: ON
static void event_handler2(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) {
        if (lv_switch_get_state(obj)) {
          if (conn.connect(server_addr, 3306, user, password)) {
            Serial.println("SQL Connection Success.");
            delay(1000);
            sw_state = 1;
          } else
            Serial.println("Connection failed.");
        } else {
          Serial.println("SQL Close.");
          conn.close();
          sw_state = 0;
        }
    }
}

void setup() {
  Serial.begin(115200);
  while (!Serial); // wait for serial port to connect
  lv86_init();
  
  Ethernet.begin();
  Serial.println("Connecting...");
  if (conn.connect(server_addr, 3306, user, password)) {
    delay(1000);
  }
  else
    Serial.println("Connection failed.");
  t = millis();

  lv_obj_t *sw1 = lv_switch_create(lv_scr_act(), NULL);
  lv_obj_align(sw1, NULL, LV_ALIGN_CENTER, 0, -30);
  lv_obj_set_event_cb(sw1, event_handler1);

  sw2 = lv_switch_create(lv_scr_act(), NULL);
  lv_obj_align(sw2, NULL, LV_ALIGN_CENTER, 0, 30);
  lv_obj_set_event_cb(sw2, event_handler2);
}


void loop() {
  conn.handleData();
  lv_task_handler();
  if (sw_state == 1 && !conn.connected()) {lv_switch_off(sw2, LV_ANIM_OFF); sw_state = 0;}
  if (sw_state == 0 && conn.connected())  {lv_switch_on(sw2, LV_ANIM_OFF);  sw_state = 1;}
}