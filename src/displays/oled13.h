//general display setup
#include "U8g2lib.h"
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);
//weiterer setupcode

void init_display()
{
  // code für Displayinitialisierung
  oled.begin();
  oled.clearBuffer();
  oled.setFont(u8g2_font_6x13_tr);
}
void display_splash()
{
  //code für splash screen, wenn gewüncht
}
void display_boxdata()
{
  //Anzeige der Daten der Box (Rufzeichen, Locator, MacAdresse)
  oled.drawStr(2,15, "Newsbox-Projekt");
  oled.drawStr(2,36, Rufzeichen);
  oled.drawStr(70,36, Locator);
  oled.drawStr(2,50, "Mac: ");
  oled.drawStr(27,50, MacAddr.c_str());
  oled.sendBuffer();
}
void display_wifi_connecting()
{
   //code für connecting, wenn gewünscht
}
void display_wifi_connected()
{
   //code für die Anzeige der IP-Adresse nach Verbindung zum WLAN
   
}
void display_fetch_flag()
{
   //code für Abrufsignal
  oled.drawStr(60,15, "*");
  oled.sendBuffer();
}
void remove_fetch_flag()
{
   //code für Entfernung des Abrufsignals
  oled.drawStr(60,15, " ");
  oled.sendBuffer();
}
void display_message()
{
   //code für das Anzeigen einer Nachricht
  oled.clear();  // Display löschen für neue Nachrichte 
  oled.drawStr(2,15, news_topic);
  oled.drawStr(65,15, news_date);
  oled.drawStr(2,36, news_line1);
  oled.drawStr(2,50, news_line2);
  oled.drawStr(2,63, news_line3);
  oled.sendBuffer();
}