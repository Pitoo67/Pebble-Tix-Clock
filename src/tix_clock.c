/*

  Just Another Bit watch - A "TIX" clock.

  See:
    <http://www.youtube.com/watch?v=S_ms9ctw5xM>

 */

#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "xprintf.h"



#define MY_UUID { 0xAE, 0x9D, 0x29, 0xE1, 0x55, 0x17, 0x47, 0x8D, 0xAB, 0x43, 0x79, 0xE7, 0xAA, 0x08, 0x9E, 0x8F }
PBL_APP_INFO(MY_UUID,
             "TIX clock", "pitoo.com",
             1, 2,
             RESOURCE_ID_IMAGE_MENU_ICON,
             APP_INFO_WATCH_FACE);

#define WINDOW_NAME "tix_clock"



#define CIRCLE_RADIUS 6
#define CIRCLE_LINE_THICKNESS 2

#define CIRCLE_PADDING 10 - CIRCLE_RADIUS // Number of padding pixels on each side
#define CELL_SIZE (2 * (CIRCLE_RADIUS + CIRCLE_PADDING)) // One "cell" is the square that contains the circle.
#define DIGIT_SIZE (CELL_SIZE * 4)
#define SIDE_PADDING ((144 - (2 * DIGIT_SIZE))/2) - CELL_SIZE

#define TIME_ZONE_OFFSET 1

Window window;
Layer display_layer;
TextLayer bottombar_layer;


unsigned long randSeed = 100;

int get_unix_time_from_current_time(PblTm *current_time)
{
  unsigned int unix_time;

  /* Convert time to seconds since epoch. */
      unix_time = ((0-TIME_ZONE_OFFSET)*3600) + /* time zone offset */
              + current_time->tm_sec /* start with seconds */
              + current_time->tm_min*60 /* add minutes */
              + current_time->tm_hour*3600 /* add hours */
              + current_time->tm_yday*86400 /* add days */
              + (current_time->tm_year-70)*31536000 /* add years since 1970 */
              + ((current_time->tm_year-69)/4)*86400 /* add a day after leap years, starting in 1973 */
              - ((current_time->tm_year-1)/100)*86400 /* remove a leap day every 100 years, starting in 2001 */
              + ((current_time->tm_year+299)/400)*86400; /* add a leap day back every 400 years, starting in 2001*/

  return unix_time;
}

int get_unix_time()
{
  PblTm current_time;
  get_time(&current_time);

  return get_unix_time_from_current_time(&current_time);
}

int random(int max)
{
  randSeed = (((randSeed * 214013L + 2531011L) >> 16) & 32767);

  return (randSeed % max);
}



void melange_tableau(int* tableau, int taille){

    int i=0;
    int tirage=0;
    int tmp=0;

    for(i = 0; i < taille; i++) tableau[i]=i; // On remplit le tableau trié
/**/
    for(i = 0; i < taille; i++){ // On melange
        tirage=random(taille);
        tmp = tableau[i]; tableau[i] = tableau[tirage]; tableau[tirage]=tmp;
    }

}

void draw_circle(GContext* ctx, GPoint center) {

  // Trace le cercle blanc
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_circle(ctx, center, CIRCLE_RADIUS);

}
GPoint get_center_point(unsigned short x, unsigned short y, unsigned short val) {

  unsigned short val_x = ((val / 3) * CELL_SIZE) + ((1-x) * CELL_SIZE);
  unsigned short val_y = ((val % 3) * CELL_SIZE);

  // emplacement (x, y) du chiffre ( heure ou minute / dizaine ou unité)
  // val est l'emplacement du point blanc dans le chiffre (0 à 5 ou 0 à 8)
  return GPoint(
            (SIDE_PADDING + (DIGIT_SIZE/2) + (DIGIT_SIZE * x)) + val_x - (CELL_SIZE / 2),
            (SIDE_PADDING + (DIGIT_SIZE/2) + (DIGIT_SIZE * y)) + val_y + (CELL_SIZE / 3)
        );

}

void draw_square(GContext* ctx, GRect rect) {

  // Trace le cercle blanc
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, rect, 4, GCornersAll);

}
GRect get_square_size(unsigned short x, unsigned short y, unsigned short val) {

  unsigned short val_x = ((val / 3) * CELL_SIZE) + ((1-x) * CELL_SIZE);
  unsigned short val_y = ((val % 3) * CELL_SIZE);

  // emplacement (x, y) du chiffre ( heure ou minute / dizaine ou unité)
  // val est l'emplacement du point blanc dans le chiffre (0 à 5 ou 0 à 8)
  return GRect(
            (SIDE_PADDING + (DIGIT_SIZE/2) + (DIGIT_SIZE * x)) + val_x - (CELL_SIZE),
            (SIDE_PADDING + (DIGIT_SIZE/2) + (DIGIT_SIZE * y)) + val_y - (CELL_SIZE / 3),
            CELL_SIZE - 4,
            CELL_SIZE - 4
        );

}

void draw_cells_for_digit(GContext* ctx,
                          unsigned short digit,
                          unsigned short cell_col,
                          unsigned short cell_row,
                          unsigned short max_value) {

  if (digit > 0)
  {
    int i =0;
    int t[10]; // le tableau de nombres

    // Tirage de digit cases parmi max_value
    melange_tableau(t, max_value);

    // La suite aléatoire est :
    for(i = 0; i < digit; i++)
    {
//    draw_circle(ctx, get_center_point(cell_col, cell_row, t[i]));
      draw_square(ctx, get_square_size(cell_col, cell_row, t[i]));
    }
  }

}

unsigned short get_display_hour(unsigned short hour) {

  if (clock_is_24h_style()) return hour;

  unsigned short display_hour = hour % 12; // Converts "0" to "12"
  return display_hour ? display_hour : 12;

}

void display_layer_update_callback(Layer *me, GContext* ctx) {

  (void)me;
  PblTm t;

  char* infotxt = "TIX PEBBLE • DD-MM-YYYY";

  get_time(&t);
  unsigned short display_hour = get_display_hour(t.tm_hour);

  draw_cells_for_digit(ctx, (display_hour / 10), 0, 0, 6);
  draw_cells_for_digit(ctx, (display_hour % 10), 1, 0, 9);

//  draw_cells_for_digit(ctx, (t.tm_sec / 10), 0, 0, 6);
//  draw_cells_for_digit(ctx, (t.tm_sec % 10), 1, 0, 9);

  draw_cells_for_digit(ctx, (t.tm_min / 10), 0, 1, 6);
  draw_cells_for_digit(ctx, (t.tm_min % 10), 1, 1, 9);

  xsprintf(infotxt, "TIX PEBBLE • %d %d", t.tm_mday, (t.tm_mon + 1));
//  xsprintf(infotxt, "TIX PEBBLE • %d %d", t.tm_hour, t.tm_min);
  text_layer_set_text(&bottombar_layer, infotxt);
}

void handle_second_tick(AppContextRef ctx, PebbleTickEvent *t) {

  (void)ctx; // TODO: Pass tick event/PblTime rather than make layer use `get_time()`?
  (void)t;
  static int waitaminute=0;

  waitaminute++;
  waitaminute = (waitaminute % 5);

  if (waitaminute == 0) layer_mark_dirty(&display_layer);

}

void handle_init(AppContextRef ctx) {

  (void)ctx;
  PblTm t;

  get_time(&t);

  window_init(&window, WINDOW_NAME);
  window_stack_push(&window, true);

  window_set_background_color(&window, GColorBlack);

  // Init the layer for the display
  layer_init(&display_layer, GRect(0, 0, 144, 150));
  display_layer.update_proc = &display_layer_update_callback;
  layer_add_child(&window.layer, &display_layer);

/*
*/
  text_layer_init(&bottombar_layer, GRect(0, 150, 144, 18));
  text_layer_set_text_color(&bottombar_layer, GColorWhite);
  text_layer_set_background_color(&bottombar_layer, GColorBlack);
  text_layer_set_font(&bottombar_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(&bottombar_layer, GTextAlignmentCenter);
  layer_add_child(&window.layer, &bottombar_layer.layer);

  //Only set the seed once
  randSeed = get_unix_time();

}

void pbl_main(void *params) {

  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,

    .tick_info = {
      .tick_handler = &handle_second_tick,
      .tick_units = SECOND_UNIT
    }
  };
  app_event_loop(params, &handlers);

}
