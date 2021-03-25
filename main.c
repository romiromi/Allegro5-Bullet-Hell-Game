// [main.c]
// this template is provided for the 2D shooter game.

#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <math.h>

// If defined, logs will be shown on console and written to file.
// If commented out, logs will not be shown nor be saved.
#define LOG_ENABLED

/* Constants. */

// Frame rate (frame per second)
const int FPS = 60;
// Display (screen) width.
const int SCREEN_W = 800;
// Display (screen) height.
const int SCREEN_H = 600;
// At most 4 audios can be played at a time.
const int RESERVE_SAMPLES = 4;

const int PLANE_BLOOD = 10;
const int PLANE_LIVES = 3;
const int PLANE_LIVES_V[] = {1, 2, 3, 4};

const int ENEMY_BLOOD = 100;
const int ENEMY_LIVES = 1;
const int ENEMY_DIRECTION[] = {1, -1};

const float ITEM_DIRECTION_X[] = {0.98, 0.87, 0.5, 0, -0.5, -0.87, -0.98};
const float ITEM_DIRECTION_Y[] = {-0.17, -0.5, -0.87, -1, -0.87, -0.5, -0.17};

bool is_ult, is_item;
bool is_bgm, is_se, is_win;

int score, ult_times, character;

double now;

// Same as:
// const int SCENE_MENU = 1;
// const int SCENE_START = 2;
enum {
	SCENE_MENU = 1,
	SCENE_START = 2,
	// [HACKATHON 3-7]
	// TODO: Declare a new scene id.
	SCENE_SETTINGS = 3,
	SCENE_WIN = 4,
	SCENE_LOSE = 5,
	SCENE_INTRO = 6
};

enum {
	CH_PLANE = 1,
	CH_CAT = 2,
};

/* Input states */

// The active scene id.
int active_scene;
// Keyboard state, whether the key is down or not.
bool key_state[ALLEGRO_KEY_MAX];
// Mouse state, whether the key is down or not.
// 1 is for left, 2 is for right, 3 is for middle.
bool *mouse_state;
// Mouse position.
int mouse_x, mouse_y;
// TODO: More variables to store input states such as joysticks, ...

/* Variables for allegro basic routines. */

ALLEGRO_DISPLAY* game_display;
ALLEGRO_EVENT_QUEUE* game_event_queue;
ALLEGRO_TIMER* game_update_timer;

/* Shared resources*/

ALLEGRO_FONT* font_pirulen_32;
ALLEGRO_FONT* font_pirulen_24;
ALLEGRO_FONT* font_danger_60;
ALLEGRO_FONT* font_danger_24;
// TODO: More shared resources or data that needed to be accessed
// across different scenes.

/* Menu Scene resources*/
ALLEGRO_BITMAP* main_img_background;
// [HACKATHON 3-1]
// TODO: Declare 2 variables for storing settings images.
// Uncomment and fill in the code below.
ALLEGRO_BITMAP* img_settings;
ALLEGRO_BITMAP* img_settings2;
ALLEGRO_BITMAP* settings_img_background;
ALLEGRO_BITMAP* img_intro;
ALLEGRO_BITMAP* img_audio_on;
ALLEGRO_BITMAP* img_audio_off;
ALLEGRO_BITMAP* img_back;
ALLEGRO_SAMPLE* main_bgm;
ALLEGRO_SAMPLE_ID main_bgm_id;

ALLEGRO_BITMAP* img_line;

/* Start Scene resources*/
ALLEGRO_BITMAP* start_img_background;
ALLEGRO_BITMAP* start_img_dying;
ALLEGRO_BITMAP* start_img_plane;
ALLEGRO_BITMAP* start_img_plane_white;
ALLEGRO_BITMAP* start_img_cat;
ALLEGRO_BITMAP* start_img_cat_black;
ALLEGRO_BITMAP* start_img_cat_bullet;
ALLEGRO_BITMAP* start_img_cat_ult;
ALLEGRO_BITMAP* start_img_ult;
ALLEGRO_BITMAP* start_img_cat_background;
ALLEGRO_BITMAP* start_img_cat_background_dying;
ALLEGRO_BITMAP* start_img_enemy;
ALLEGRO_BITMAP* start_img_lives;
ALLEGRO_BITMAP* start_img_blood;
ALLEGRO_BITMAP* start_img_item;
ALLEGRO_SAMPLE* start_bgm;
ALLEGRO_SAMPLE_ID start_bgm_id;
ALLEGRO_SAMPLE* start_bgm_cat;
ALLEGRO_SAMPLE_ID start_bgm_cat_id;
ALLEGRO_SAMPLE* start_shot_sound;
ALLEGRO_SAMPLE* start_ult_sound;

/* Win Scene and Lose Scene resources*/
ALLEGRO_BITMAP* win_img_background;
ALLEGRO_BITMAP* lose_img_background;
ALLEGRO_SAMPLE* win_bgm;
ALLEGRO_SAMPLE* lose_bgm;
ALLEGRO_SAMPLE_ID win_bgm_id;
ALLEGRO_SAMPLE_ID lose_bgm_id;
ALLEGRO_BITMAP* img_try_again_win;
ALLEGRO_BITMAP* img_try_again_lose;

// [HACKATHON 2-1]
// TODO: Declare a variable to store your bullet's image.
// Uncomment and fill in the code below.
ALLEGRO_BITMAP* img_bullet;
ALLEGRO_BITMAP* img_e_bullet;

typedef struct {
	// The center coordinate of the image.
	float x, y;
	// The width and height of the object.
	float w, h;
	// The velocity in x, y axes.
	float vx, vy;
	// Should we draw this object on the screen.
	bool hidden;
	// The pointer to the object's image.
	ALLEGRO_BITMAP* img;
	int blood, lives;
} MovableObject;
void draw_movable_object(MovableObject obj);
#define MAX_ENEMY 3
// [HACKATHON 2-2]
// TODO: Declare the max bullet count that will show on screen.
// You can try max 4 bullets here and see if you needed more.
// Uncomment and fill in the code below.
#define MAX_BULLET 6
#define MAX_ITEM_BULLET 7 * 6
#define MAX_E_BULLET 6
MovableObject plane;
MovableObject enemies[MAX_ENEMY];
// [HACKATHON 2-3]
// TODO: Declare an array to store bullets with size of max bullet count.
// Uncomment and fill in the code below.
MovableObject bullets[MAX_BULLET];
MovableObject item_star; //blood: countdown of plane's effect, lives: timer
MovableObject item_bullets[MAX_ITEM_BULLET];
MovableObject ult_bullet;
MovableObject e_bullets[MAX_E_BULLET];

MovableObject line_1, line_2;
// [HACKATHON 2-4]
// TODO: Set up bullet shooting cool-down variables.
// 1) Declare your shooting cool-down time as constant. (0.2f will be nice)
// 2) Declare your last shoot timestamp.
// Uncomment and fill in the code below.
const float MAX_COOLDOWN = 0.2f;
double last_shoot_timestamp;
double last_shoot_timestamp_e[MAX_ENEMY];

/* Declare function prototypes. */

// Initialize allegro5 library
void allegro5_init(void);
// Initialize variables and resources.
// Allows the game to perform any initialization it needs before
// starting to run.
void game_init(void);
// Process events inside the event queue using an infinity loop.
void game_start_event_loop(void);
// Run game logic such as updating the world, checking for collision,
// switching scenes and so on.
// This is called when the game should update its logic.
void game_update(void);
// Draw to display.
// This is called when the game should draw itself.
void game_draw(void);
// Release resources.
// Free the pointers we allocated.
void game_destroy(void);
// Function to change from one scene to another.
void game_change_scene(int next_scene);
// Load resized bitmap and check if failed.
ALLEGRO_BITMAP *load_bitmap_resized(const char *filename, int w, int h);
// [HACKATHON 3-2]
// TODO: Declare a function.
// Determines whether the point (px, py) is in rect (x, y, w, h).
// Uncomment the code below.
bool pnt_in_rect(int px, int py, int x, int y, int w, int h);

void fly_in(MovableObject *a, int h_up, int h_down);
void fly_out(MovableObject *a);

bool hit(MovableObject a, MovableObject b);
bool hurt(MovableObject *b);
void hurt_and_add_score(MovableObject *b);

void shoot_bullet(MovableObject *v, MovableObject *ed);

/* Event callbacks. */
void on_key_down(int keycode);
void on_mouse_down(int btn, int x, int y);

/* Declare function prototypes for debugging. */

// Display error message and exit the program, used like 'printf'.
// Write formatted output to stdout and file from the format string.
// If the program crashes unexpectedly, you can inspect "log.txt" for
// further information.
void game_abort(const char* format, ...);
// Log events for later debugging, used like 'printf'.
// Write formatted output to stdout and file from the format string.
// You can inspect "log.txt" for logs in the last run.
void game_log(const char* format, ...);
// Log using va_list.
void game_vlog(const char* format, va_list arg);

int main(int argc, char** argv) {
	// Set random seed for better random outcome.
	srand(time(NULL));
	allegro5_init();
	game_log("Allegro5 initialized");
	game_log("Game begin");
	// Initialize game variables.
	game_init();
	game_log("Game initialized");
	// Draw the first frame.
	game_draw();
	game_log("Game start event loop");
	// This call blocks until the game is finished.
	game_start_event_loop();
	game_log("Game end");
	game_destroy();
	return 0;
}

void allegro5_init(void) {
	if (!al_init())
		game_abort("failed to initialize allegro");

	// Initialize add-ons.
	if (!al_init_primitives_addon())
		game_abort("failed to initialize primitives add-on");
	if (!al_init_font_addon())
		game_abort("failed to initialize font add-on");
	if (!al_init_ttf_addon())
		game_abort("failed to initialize ttf add-on");
	if (!al_init_image_addon())
		game_abort("failed to initialize image add-on");
	if (!al_install_audio())
		game_abort("failed to initialize audio add-on");
	if (!al_init_acodec_addon())
		game_abort("failed to initialize audio codec add-on");
	if (!al_reserve_samples(RESERVE_SAMPLES))
		game_abort("failed to reserve samples");
	if (!al_install_keyboard())
		game_abort("failed to install keyboard");
	if (!al_install_mouse())
		game_abort("failed to install mouse");
	// TODO: Initialize other addons such as video, ...

	// Setup game display.
	game_display = al_create_display(SCREEN_W, SCREEN_H);
	if (!game_display)
		game_abort("failed to create display");
	al_set_window_title(game_display, "I2P(I)_2019 Final Project <108062111>");

	// Setup update timer.
	game_update_timer = al_create_timer(1.0f / FPS);
	if (!game_update_timer)
		game_abort("failed to create timer");

	// Setup event queue.
	game_event_queue = al_create_event_queue();
	if (!game_event_queue)
		game_abort("failed to create event queue");

	// Malloc mouse buttons state according to button counts.
	const unsigned m_buttons = al_get_mouse_num_buttons();
	game_log("There are total %u supported mouse buttons", m_buttons);
	// mouse_state[0] will not be used.
	mouse_state = malloc((m_buttons + 1) * sizeof(bool));
	memset(mouse_state, false, (m_buttons + 1) * sizeof(bool));

	// Register display, timer, keyboard, mouse events to the event queue.
	al_register_event_source(game_event_queue, al_get_display_event_source(game_display));
	al_register_event_source(game_event_queue, al_get_timer_event_source(game_update_timer));
	al_register_event_source(game_event_queue, al_get_keyboard_event_source());
	al_register_event_source(game_event_queue, al_get_mouse_event_source());
	// TODO: Register other event sources such as timer, video, ...

	// Start the timer to update and draw the game.
	al_start_timer(game_update_timer);
}

void game_init(void) {
	/* Shared resources*/
	font_pirulen_32 = al_load_font("font/pirulen.ttf", 32, 0);
	if (!font_pirulen_32)
		game_abort("failed to load font: font/pirulen.ttf with size 32");
	font_pirulen_24 = al_load_font("font/pirulen.ttf", 24, 0);
	if (!font_pirulen_24)
		game_abort("failed to load font: font/pirulen.ttf with size 24");
    font_danger_60 = al_load_font("font/Danger_Zone_Warning.ttf", 60, 0);
	if (!font_danger_60)
		game_abort("failed to load font: font/danger.ttf with size 60");
    font_danger_24 = al_load_font("font/Danger_Zone_Warning.ttf", 24, 0);
    if (!font_danger_24)
		game_abort("failed to load font: font/danger.ttf with size 24");

	/* Menu Scene resources*/
	main_img_background = load_bitmap_resized("img/home.png", SCREEN_W, SCREEN_H);

	main_bgm = al_load_sample("SE/S31_Night Prowler.ogg");
	if (!main_bgm)
		game_abort("failed to load audio: SE/S31_Night Prowler.ogg");

	// [HACKATHON 3-4]
	// TODO: Load settings images.
	// Don't forget to check their return values.
	// Uncomment and fill in the code below.
	img_settings = al_load_bitmap("img/settings.png");
	if (!img_settings)
		game_abort("failed to load image: img/settings.png");
	img_settings2 = al_load_bitmap("img/settings2.png");
	if (!img_settings2)
		game_abort("failed to load image: img/settings2.png");
    img_audio_on = load_bitmap_resized("img/audio_on.png", 40, 40);
    img_audio_off = load_bitmap_resized("img/audio_off.png", 40, 40);
    img_back = load_bitmap_resized("img/back.png", 38, 38);

    img_line = al_load_bitmap("img/line_bg.png");
	if (!img_line)
		game_abort("failed to load image: img/line_bg.png");


	/* Start Scene resources*/
	start_img_background = load_bitmap_resized("img/start_bg.jpg", SCREEN_W, SCREEN_H);
	start_img_dying = load_bitmap_resized("img/dying.png", SCREEN_W, SCREEN_H);
	start_img_lives = load_bitmap_resized("img/lives.png", 40, 40);
    start_img_blood = load_bitmap_resized("img/hp.png", 10, 40);
    start_img_item = load_bitmap_resized("img/item.png", 40, 40);
    settings_img_background = load_bitmap_resized("img/settings_bg.jpg", SCREEN_W, SCREEN_H);
    img_intro = load_bitmap_resized("img/intro.png", SCREEN_W, SCREEN_H);


	/* Win Scene and Lose Scene resources*/
	win_img_background = load_bitmap_resized("img/win_bg.jpg", SCREEN_W, SCREEN_H);
	lose_img_background = load_bitmap_resized("img/lose_bg.jpg", SCREEN_W, SCREEN_H);
	img_try_again_win = al_load_bitmap("img/try_again_win.png");
    img_try_again_lose = al_load_bitmap("img/try_again_lose.png");

	start_img_plane = al_load_bitmap("img/tegami_gray.png");
	if (!start_img_plane)
		game_abort("failed to load image: img/tegami_gray.png");
    start_img_plane_white = al_load_bitmap("img/tegami.png");
	if (!start_img_plane_white)
		game_abort("failed to load image: img/tegami.png");

    start_img_cat = al_load_bitmap("img/cat.png");
	if (!start_img_cat)
		game_abort("failed to load image: img/cat.png");
    start_img_cat_black = al_load_bitmap("img/cat_black.png");
	if (!start_img_cat_black)
		game_abort("failed to load image: img/cat_black.png");

    start_img_cat_background = al_load_bitmap("img/cat_bg.jpg");
	if (!start_img_cat_background)
		game_abort("failed to load image: img/cat_bg.jpg");

    start_img_cat_background_dying = al_load_bitmap("img/cat_bg_dying.jpg");
	if (!start_img_cat_background_dying)
		game_abort("failed to load image: img/cat_bg_dying.jpg");

    start_img_cat_bullet = al_load_bitmap("img/cat_bullet.png");
	if (!start_img_cat_bullet)
		game_abort("failed to load image: img/cat_bullet.png");

    start_img_cat_ult = load_bitmap_resized("img/cat_ult.png", 250, 500);
    start_img_ult = load_bitmap_resized("img/ult.png", 250, 500);

	start_img_enemy = al_load_bitmap("img/unnamed.png");
	if (!start_img_enemy)
		game_abort("failed to load image: img/unnamed.png");

	start_bgm = al_load_sample("SE/mythica.ogg");
	if (!start_bgm)
		game_abort("failed to load audio: SE/mythica.ogg");

    start_bgm_cat = al_load_sample("SE/cat_bgm.ogg");
	if (!start_bgm_cat)
		game_abort("failed to load audio: SE/cat_bgm.ogg");

    start_shot_sound = al_load_sample("SE/shot_sound.ogg");
	if (!start_shot_sound)
		game_abort("failed to load audio: SE/shot_sound.ogg");

    start_ult_sound = al_load_sample("SE/ult_sound.ogg");
	if (!start_ult_sound)
		game_abort("failed to load audio: SE/ult_sound.ogg");

    win_bgm = al_load_sample("SE/win_sound_effect.ogg");
    if (!win_bgm)
        game_abort("failed to load audio: SE/win_sound_effect.ogg");

    lose_bgm = al_load_sample("SE/lose_sound_effect.ogg");
    if (!lose_bgm)
        game_abort("failed to load audio: SE/lose_sound_effect.ogg");

	// [HACKATHON 2-5]
	// TODO: Initialize bullets.
	// 1) Search for a bullet image online and put it in your project.
	//    You can use the image we provided.
	// 2) Load it in by 'al_load_bitmap' or 'load_bitmap_resized'.
	// 3) If you use 'al_load_bitmap', don't forget to check its return value.
	// Uncomment and fill in the code below.
	img_bullet = al_load_bitmap("img/image12.png");
	if (!img_bullet)
		game_abort("failed to load image: img/image12.png");
    img_e_bullet = al_load_bitmap("img/image13.png");
	if (!img_e_bullet)
		game_abort("failed to load image: img/image13.png");

    is_bgm = is_se = true;
    is_win = false;
    character = CH_PLANE;

	// Change to first scene.
	game_change_scene(SCENE_MENU);
}

void game_start_event_loop(void) {
	bool done = false;
	ALLEGRO_EVENT event;
	int redraws = 0;
	while (!done) {
		al_wait_for_event(game_event_queue, &event);
		if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			// Event for clicking the window close button.
			game_log("Window close button clicked");
			done = true;
		} else if (event.type == ALLEGRO_EVENT_TIMER) {
			// Event for redrawing the display.
			if (event.timer.source == game_update_timer)
				// The redraw timer has ticked.
				redraws++;
		} else if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
			// Event for keyboard key down.
			game_log("Key with keycode %d down", event.keyboard.keycode);
			key_state[event.keyboard.keycode] = true;
			on_key_down(event.keyboard.keycode);
        } else if (event.type == ALLEGRO_EVENT_KEY_UP) {
			// Event for keyboard key up.
			game_log("Key with keycode %d up", event.keyboard.keycode);
			key_state[event.keyboard.keycode] = false;
		} else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
			// Event for mouse key down.
			game_log("Mouse button %d down at (%d, %d)", event.mouse.button, event.mouse.x, event.mouse.y);
			mouse_state[event.mouse.button] = true;
			on_mouse_down(event.mouse.button, event.mouse.x, event.mouse.y);
		} else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
			// Event for mouse key up.
			game_log("Mouse button %d up at (%d, %d)", event.mouse.button, event.mouse.x, event.mouse.y);
			mouse_state[event.mouse.button] = false;
		} else if (event.type == ALLEGRO_EVENT_MOUSE_AXES) {
			if (event.mouse.dx != 0 || event.mouse.dy != 0) {
				// Event for mouse move.
				//game_log("Mouse move to (%d, %d)", event.mouse.x, event.mouse.y);
				mouse_x = event.mouse.x;
				mouse_y = event.mouse.y;
			} else if (event.mouse.dz != 0) {
				// Event for mouse scroll.
				game_log("Mouse scroll at (%d, %d) with delta %d", event.mouse.x, event.mouse.y, event.mouse.dz);
			}
		}
		// TODO: Process more events and call callbacks by adding more
		// entries inside Scene.

		// Redraw
		if (redraws > 0 && al_is_event_queue_empty(game_event_queue)) {
			// if (redraws > 1)
			// 	game_log("%d frame(s) dropped", redraws - 1);
			// Update and draw the next frame.
			game_update();
			game_draw();
			redraws = 0;
		}
	}
}

void game_update(void) {
	if (active_scene == SCENE_START) {
        if (character == CH_PLANE) {
            if (!line_1.hidden) {
                line_1.y += line_1.vy;
                if (line_2.hidden && (line_1.y + line_1.h / 2 < SCREEN_H)) {
                    line_2.hidden = false;
                    line_2.y = SCREEN_H + line_2.h / 2;
                }
                if (line_1.y + line_1.h / 2 < 0) {
                    line_1.hidden = true;
                }
            }
            if (!line_2.hidden) {
                line_2.y += line_2.vy;
                if (line_1.hidden && line_2.y + line_2.h / 2 < SCREEN_H) {
                    line_1.hidden = false;
                    line_1.y = SCREEN_H + line_1.h / 2;
                }
                if (line_2.y + line_2.h / 2 < 0) {
                    line_2.hidden = true;
                }
            }
        }


        if(plane.blood < 0){
            plane.blood = PLANE_BLOOD;
            plane.lives--;
            game_log("lives: %d", plane.lives);
        }
        if(plane.lives <= 0){
            game_change_scene(SCENE_LOSE);
        }
		plane.vx = plane.vy = 0;
		//game_log("1 plane: x=%f y=%f", plane.x, plane.y);

		if (key_state[ALLEGRO_KEY_UP] || key_state[ALLEGRO_KEY_W])
			plane.vy -= 1;
		if (key_state[ALLEGRO_KEY_DOWN] || key_state[ALLEGRO_KEY_S])
			plane.vy += 1;
		if (key_state[ALLEGRO_KEY_LEFT] || key_state[ALLEGRO_KEY_A])
			plane.vx -= 1;
		if (key_state[ALLEGRO_KEY_RIGHT] || key_state[ALLEGRO_KEY_D])
			plane.vx += 1;
		// 0.71 is (1/sqrt(2)).
        plane.y += plane.vy * PLANE_LIVES_V[plane.lives] * (plane.vx ? 0.71f : 1);
        plane.x += plane.vx * PLANE_LIVES_V[plane.lives] * (plane.vy ? 0.71f : 1);
		// [HACKATHON 1-1]
		// TODO: Limit the plane's collision box inside the frame.
		//       (x, y axes can be separated.)
		// Uncomment and fill in the code below.
		//game_log("2 plane: x=%f y=%f", plane.x, plane.y);
		fly_in(&plane, SCREEN_H/3, SCREEN_H);

		//eat item
        if (!item_star.hidden && hit(plane, item_star)) {
            game_log("ate star");
            item_star.hidden = true;
            item_star.blood = 300;
            is_item = true;
        }

		// [HACKATHON 2-7]
		// TODO: Update bullet coordinates.
		// 1) For each bullets, if it's not hidden, update x, y
		// according to vx, vy.
		// 2) If the bullet is out of the screen, hide it.
		// Uncomment and fill in the code below.
		//game_log("plane: x=%f y=%f", plane.x, plane.y);
		int i, j, k;
		for (i = 0; i < MAX_BULLET; i++) {
			if (bullets[i].hidden)
				continue;
            fly_out(&bullets[i]);

            //If a bullet hits an enemy, the enemy's blood decrease.
            for (j = 0; j < MAX_ENEMY; j++){
                if (enemies[j].hidden)
                    continue;
                if (hit(bullets[i], enemies[j])) {
                    hurt_and_add_score(&enemies[j]);
                    bullets[i].hidden = true;
                }
            }
		}

		//update ult's x and y
		if (!ult_bullet.hidden) {
            fly_out(&ult_bullet);

            for (j = 0; j < MAX_ENEMY; j++){
                if (enemies[j].hidden)
                    continue;
                if (hit(ult_bullet, enemies[j]))
                    hurt_and_add_score(&enemies[j]);
            }
		}

		//update item's x and y
        for (i = 0; i < MAX_ITEM_BULLET; i++) {
            if (item_bullets[i].hidden)
                continue;
            fly_out(&item_bullets[i]);

            //If an ult bullet hit enemy
            for (j = 0; j < MAX_ENEMY; j++) {
                if (enemies[j].hidden)
                    continue;
                if (hit(item_bullets[i], enemies[j])) {
                    hurt_and_add_score(&enemies[j]);
                    item_bullets[i].hidden = true;
                }
            }
        }

		for (i = 0; i < MAX_E_BULLET; i++) {
			if (e_bullets[i].hidden)
				continue;
			fly_out(&e_bullets[i]);

            //If a bullet hits the plane, the plane's blood decrease.
            if (hit(e_bullets[i], plane)) {
                e_bullets[i].hidden = true;
                hurt(&plane);
            }
		}

		for (i = 0; i < MAX_ENEMY; i++){
            if (enemies[i].hidden)
                continue;
            enemies[i].y += enemies[i].vy * 2 * 0.71f;
            enemies[i].x += enemies[i].vx * 2 * 0.71f;
            fly_in(&enemies[i], 0, SCREEN_H/2);
		}

		//If all enemies are dead, the player wins.
		for (i = 0, j = 0; i < MAX_ENEMY; i++){
            if (enemies[i].hidden == true)
                j++;
		}
		if (j == MAX_ENEMY)
            game_change_scene(SCENE_WIN);

        if (score > 200 + 3000 * ult_times)
            is_ult = true;

		// [HACKATHON 2-8]
		// TODO: Shoot if key is down and cool-down is over.
		// 1) Get the time now using 'al_get_time'.
		// 2) If Space key is down in 'key_state' and the time
		//    between now and last shoot is not less that cool
		//    down time.
		// 3) Loop through the bullet array and find one that is hidden.
		//    (This part can be optimized.)
		// 4) The bullet will be found if your array is large enough.
		// 5) Set the last shoot time to now.
		// 6) Set hidden to false (recycle the bullet) and set its x, y to the
		//    front part of your plane.
		// Uncomment and fill in the code below.
		now = al_get_time();
		//game_log("now = %f", now);
		if (key_state[ALLEGRO_KEY_SPACE] && now - last_shoot_timestamp >= MAX_COOLDOWN) {
            if (is_item) {
                //i: iter item_bullet
                //j: count the number of unused bullet
                //k: which set of bullet
                for (i = 0, j = 0, k = 0; i < MAX_ITEM_BULLET; i++, j++){
                    if (j == 7)
                        break;
                    if (item_bullets[i].hidden == false) {
                        j = 0;
                        k++;
                        i = k * 7;
                    }
                }
                if (i < MAX_ITEM_BULLET) {
                    last_shoot_timestamp = now;
                    if (is_se)
                        al_play_sample(start_shot_sound, 1, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
                    for (j = i-6; j <= i; j++)
                        shoot_bullet(&plane, &item_bullets[j]);
                }
            } else {
                for (i = 0; i < MAX_BULLET; i++) {
                    if (bullets[i].hidden)
                        break;
                }
                if (i < MAX_BULLET) {
                    last_shoot_timestamp = now;
                    if (is_se)
                        al_play_sample(start_shot_sound, 1, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
                    shoot_bullet(&plane, &bullets[i]);
                }
            }
		}

		item_star.lives++;
        if (!(item_star.lives % 900) && item_star.hidden) {
            item_star.hidden = false;
            item_star.x = item_star.w / 2 + (float)rand() / RAND_MAX * (SCREEN_W - item_star.w);
            item_star.y = ((float)rand() / RAND_MAX) * (SCREEN_H / 3) + (SCREEN_H * 2 / 3) - item_star.h / 2;
		} else if (!(item_star.lives % 1080))
            item_star.hidden = true;

        if (item_star.blood) {
            item_star.blood--;
        } else {
            is_item = false;
        }

		if (is_ult && key_state[ALLEGRO_KEY_R] && now - last_shoot_timestamp >= MAX_COOLDOWN) {
            //shoot ult
            game_log("Shoot ult");
            is_ult = false;
            ult_times++;
            last_shoot_timestamp = now;
            if (is_se)
                al_play_sample(start_ult_sound, 1, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
            shoot_bullet(&plane, &ult_bullet);
		}

        for (i = 0; i < MAX_E_BULLET; i++) {
            if (e_bullets[i].hidden)
                break;
        }
        if (i < MAX_E_BULLET) {
            do{
                j = rand() % MAX_ENEMY;
            } while (enemies[j].hidden);
            if (now - last_shoot_timestamp_e[j] >= MAX_COOLDOWN){
                last_shoot_timestamp_e[j] = now;
                shoot_bullet(&enemies[j], &e_bullets[i]);
            }
        }
	}
}

void game_draw(void) {
	if (active_scene == SCENE_MENU) {
		al_draw_bitmap(main_img_background, 0, 0, 0);
		al_draw_text(font_pirulen_32, al_map_rgb(101, 119, 134), SCREEN_W / 2, 30, ALLEGRO_ALIGN_CENTER, "Social Network Shooter");
		al_draw_text(font_pirulen_24, al_map_rgb(101, 119, 134), 20, SCREEN_H - 50, 0, "Press enter key to start");
		// [HACKATHON 3-5]
		// TODO: Draw settings images.
		// The settings icon should be located at (x, y, w, h) =
		// (SCREEN_W - 48, 10, 38, 38).
		// Change its image according to your mouse position.
		// Uncomment and fill in the code below.
		if (pnt_in_rect(mouse_x, mouse_y, SCREEN_W - 48, 10, 38, 38))
			al_draw_bitmap(img_settings2, SCREEN_W - 48, 10, 0);
		else
			al_draw_bitmap(img_settings, SCREEN_W - 48, 10, 0);
	} else if (active_scene == SCENE_START) {
		int i;
		if (character == CH_PLANE){
            draw_movable_object(line_1);
            draw_movable_object(line_2);
		} else if (character == CH_CAT){
		    if (plane.lives == 1) {
                al_draw_bitmap(start_img_cat_background_dying, 0, 0, 0);
		    }
            else
                al_draw_bitmap(start_img_cat_background, 0, 0, 0);
		}
		// [HACKATHON 2-9]
		// TODO: Draw all bullets in your bullet array.
		// Uncomment and fill in the code below.
		for (i = 0; i < MAX_BULLET; i++)
			draw_movable_object(bullets[i]);
        draw_movable_object(ult_bullet);
        for (i = 0; i < MAX_ITEM_BULLET; i++)
			draw_movable_object(item_bullets[i]);
        for (i = 0; i < MAX_E_BULLET; i++)
			draw_movable_object(e_bullets[i]);

		draw_movable_object(plane);
		for (i = 0; i < MAX_ENEMY; i++)
			draw_movable_object(enemies[i]);

        draw_movable_object(item_star);

        for (i = 0; i < plane.lives; i++)
            al_draw_bitmap(start_img_lives, 10 + 50 * i, SCREEN_H - 50, 0);
        for (i = 0; i < plane.blood; i++)
            al_draw_bitmap(start_img_blood, 160 + 10 * i, SCREEN_H - 50, 0);
        al_draw_textf(font_pirulen_24, al_map_rgb(0, 0, 0), 10, 10, 0, "SCORE: %d", score);

        if (is_ult) {
            al_draw_text(font_danger_24, al_map_rgb(237, 28, 36), 10, 45, 0, "!!");
        }
	}
	// [HACKATHON 3-9]
	// TODO: If active_scene is SCENE_SETTINGS.
	// Draw anything you want, or simply clear the display.
	else if (active_scene == SCENE_SETTINGS) {
		al_draw_bitmap(settings_img_background, 0, 0, 0);
		al_draw_text(font_pirulen_24, al_map_rgb(255, 255, 255), 20, SCREEN_H - 90, 0, "bgm");
		al_draw_text(font_pirulen_24, al_map_rgb(255, 255, 255), 20, SCREEN_H - 50, 0, "sound effect");
        al_draw_bitmap(img_back, SCREEN_W - 48, 10, 0);

        if (is_bgm) {
            al_draw_bitmap(img_audio_on, 105, SCREEN_H - 95, 0);
        } else {
            al_draw_bitmap(img_audio_off, 105, SCREEN_H - 95, 0);
        }

        if (is_se) {
            al_draw_bitmap(img_audio_on, 288, SCREEN_H - 55, 0);
        } else {
            al_draw_bitmap(img_audio_off, 288, SCREEN_H - 55, 0);
        }

        if (character == CH_PLANE) {
            al_draw_bitmap(start_img_plane_white, 380, SCREEN_H - 75, 0);
            al_draw_bitmap(start_img_cat_black, 470, SCREEN_H - 80, 0);
        } if (character == CH_CAT) {
            al_draw_bitmap(start_img_plane, 380, SCREEN_H - 75, 0);
            al_draw_bitmap(start_img_cat, 470, SCREEN_H - 80, 0);
        }
	} else if (active_scene == SCENE_WIN) {
	    is_win = true;
		al_draw_bitmap(win_img_background, 0, 0, 0);
		al_draw_bitmap(img_try_again_win, SCREEN_W - 320, 240, 0);
		al_draw_text(font_danger_60, al_map_rgb(0, 162, 232), SCREEN_W - 200, 88, ALLEGRO_ALIGN_CENTER, "YOU WIN");
        al_draw_textf(font_danger_60, al_map_rgb(0, 162, 232), SCREEN_W - 200, 160, ALLEGRO_ALIGN_CENTER, "SCORE: %d", score);

		if (pnt_in_rect(mouse_x, mouse_y, SCREEN_W - 48, 10, 38, 38))
			al_draw_bitmap(img_settings2, SCREEN_W - 48, 10, 0);
		else
			al_draw_bitmap(img_settings, SCREEN_W - 48, 10, 0);
	} else if (active_scene == SCENE_LOSE) {
		al_draw_bitmap(lose_img_background, 0, 0, 0);
		al_draw_bitmap(img_try_again_lose, SCREEN_W - 273, SCREEN_H - 100, 0);
		al_draw_text(font_danger_60, al_map_rgb(237, 28, 36), SCREEN_W - 50, SCREEN_H - 242, ALLEGRO_ALIGN_RIGHT, "YOU LOSE");
		al_draw_textf(font_danger_60, al_map_rgb(237, 28, 36), SCREEN_W - 50, SCREEN_H - 170, ALLEGRO_ALIGN_RIGHT, "SCORE: %d", score);

		if (pnt_in_rect(mouse_x, mouse_y, SCREEN_W - 48, 10, 38, 38))
			al_draw_bitmap(img_settings2, SCREEN_W - 48, 10, 0);
		else
			al_draw_bitmap(img_settings, SCREEN_W - 48, 10, 0);
	} else if (active_scene == SCENE_INTRO) {
		al_draw_bitmap(img_intro, 0, 0, 0);
	}
	al_flip_display();
}

void game_destroy(void) {
	// Destroy everything you have created.
	// Free the memories allocated by malloc or allegro functions.
	// Destroy shared resources.
	al_destroy_font(font_pirulen_32);
	al_destroy_font(font_pirulen_24);

	/* Menu Scene resources*/
	al_destroy_bitmap(main_img_background);
	al_destroy_sample(main_bgm);
	// [HACKATHON 3-6]
	// TODO: Destroy the 2 settings images.
	// Uncomment and fill in the code below.
	al_destroy_bitmap(img_settings);
	al_destroy_bitmap(img_settings2);
	al_destroy_bitmap(settings_img_background);
	al_destroy_bitmap(img_audio_on);
	al_destroy_bitmap(img_audio_off);
	al_destroy_bitmap(img_back);

	al_destroy_bitmap(img_line);

	/* Start Scene resources*/
	al_destroy_bitmap(start_img_background);
	al_destroy_bitmap(start_img_dying);
	al_destroy_bitmap(start_img_plane);
	al_destroy_bitmap(start_img_plane_white);
	al_destroy_bitmap(start_img_cat);
	al_destroy_bitmap(start_img_cat_black);
	al_destroy_bitmap(start_img_cat_background);
	al_destroy_bitmap(start_img_cat_background_dying);
	al_destroy_bitmap(start_img_cat_bullet);
	al_destroy_bitmap(start_img_cat_ult);
	al_destroy_bitmap(start_img_ult);
	al_destroy_bitmap(start_img_enemy);
	al_destroy_bitmap(start_img_lives);
	al_destroy_bitmap(start_img_item);
	al_destroy_sample(start_bgm);
	al_destroy_sample(start_bgm_cat);
	al_destroy_sample(start_shot_sound);
	al_destroy_sample(start_ult_sound);

	/* Win Scene and Lose Scene resources*/
	al_destroy_bitmap(win_img_background);
	al_destroy_bitmap(lose_img_background);
	al_destroy_sample(win_bgm);
	al_destroy_sample(lose_bgm);
	al_destroy_bitmap(img_try_again_win);
	al_destroy_bitmap(img_try_again_lose);

	// [HACKATHON 2-10]
	// TODO: Destroy your bullet image.
	// Uncomment and fill in the code below.
	al_destroy_bitmap(img_bullet);
	al_destroy_bitmap(img_e_bullet);

	al_destroy_timer(game_update_timer);
	al_destroy_event_queue(game_event_queue);
	al_destroy_display(game_display);
	free(mouse_state);
}

void game_change_scene(int next_scene) {
	game_log("Change scene from %d to %d", active_scene, next_scene);
	// TODO: Destroy resources initialized when creating scene.
	if (active_scene == SCENE_MENU) {
		al_stop_sample(&main_bgm_id);
		game_log("stop audio (bgm)");
	} else if (active_scene == SCENE_START) {
		if (character == CH_PLANE)
            al_stop_sample(&start_bgm_id);
        else if(character == CH_CAT)
            al_stop_sample(&start_bgm_cat_id);
		game_log("stop audio (bgm)");
	} else if (active_scene == SCENE_WIN) {
		al_stop_sample(&win_bgm_id);
		game_log("stop audio (bgm)");
	} else if (active_scene == SCENE_LOSE) {
		al_stop_sample(&lose_bgm_id);
		game_log("stop audio (bgm)");
	}
	active_scene = next_scene;
	// TODO: Allocate resources before entering scene.
	if (active_scene == SCENE_MENU) {
		if (is_bgm && !al_play_sample(main_bgm, 1, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, &main_bgm_id))
			game_abort("failed to play audio (bgm)");
	} else if (active_scene == SCENE_START) {
		int i;
		if (character == CH_PLANE)
            plane.img = start_img_plane;
        else if (character == CH_CAT)
            plane.img = start_img_cat;

		plane.x = 400;
		plane.y = 500;
		plane.w = al_get_bitmap_width(plane.img);
        plane.h = al_get_bitmap_height(plane.img);
        plane.blood = PLANE_BLOOD;
        plane.lives = PLANE_LIVES;
        score = 0;
        ult_times = 0;
        is_ult = false;
        is_item = false;
        plane.hidden = false;

        line_1.img = line_2.img = img_line;
		line_1.w = line_2.w = al_get_bitmap_width(line_1.img);
        line_1.h = line_2.h = al_get_bitmap_height(line_1.img);
        line_1.x = line_2.x = line_1.w / 2;
        line_1.y = line_1.h / 2;
        line_1.vx = line_2.vx = 0;
		line_1.vy = line_2.vy = -1;
        line_1.hidden = false;
        line_2.hidden = true;

		for (i = 0; i < MAX_ENEMY; i++) {
			enemies[i].img = start_img_enemy;
			enemies[i].w = al_get_bitmap_width(start_img_enemy);
			enemies[i].h = al_get_bitmap_height(start_img_enemy);
			enemies[i].x = enemies[i].w / 2 + (float)rand() / RAND_MAX * (SCREEN_W - enemies[i].w);
			enemies[i].y = 80;
			enemies[i].vx = ENEMY_DIRECTION[rand()%2];
			enemies[i].vy = ENEMY_DIRECTION[rand()%2];
			enemies[i].blood = ENEMY_BLOOD;
			enemies[i].lives = ENEMY_LIVES;
			enemies[i].hidden = false;
		}
		// [HACKATHON 2-6]
		// TODO: Initialize bullets.
		// For each bullets in array, set their w and h to the size of
		// the image, and set their img to bullet image, hidden to true,
		// (vx, vy) to (0, -3).
		// Uncomment and fill in the code below.
		for (i = 0; i < MAX_BULLET; i++) {
            if (character == CH_PLANE) {
                bullets[i].w = al_get_bitmap_width(img_bullet);
                bullets[i].h = al_get_bitmap_height(img_bullet);
                bullets[i].img = img_bullet;
            } else if (character == CH_CAT) {
                bullets[i].w = al_get_bitmap_width(start_img_cat_bullet);
                bullets[i].h = al_get_bitmap_height(start_img_cat_bullet);
                bullets[i].img = start_img_cat_bullet;
            }
			bullets[i].vx = 0;
			bullets[i].vy = -3;
			bullets[i].hidden = true;
		}
		for (i = 0; i < MAX_E_BULLET; i++) {
			e_bullets[i].w = al_get_bitmap_width(img_e_bullet);
			e_bullets[i].h = al_get_bitmap_height(img_e_bullet);
			e_bullets[i].img = img_e_bullet;
			e_bullets[i].vx = 0;
			e_bullets[i].vy = 3;
			e_bullets[i].hidden = true;
		}

		item_star.w = al_get_bitmap_width(start_img_item);
        item_star.h = al_get_bitmap_height(start_img_item);
        item_star.img = start_img_item;
        item_star.vx = item_star.vy = 0;
        item_star.hidden = true;
        item_star.lives = 300;

		for (i = 0; i < MAX_ITEM_BULLET; i++) {
            if (character == CH_PLANE) {
                item_bullets[i].w = al_get_bitmap_width(img_bullet);
                item_bullets[i].h = al_get_bitmap_height(img_bullet);
                item_bullets[i].img = img_bullet;
            } else if (character == CH_CAT) {
                item_bullets[i].w = al_get_bitmap_width(start_img_cat_bullet);
                item_bullets[i].h = al_get_bitmap_height(start_img_cat_bullet);
                item_bullets[i].img = start_img_cat_bullet;
            }
			item_bullets[i].vx = 3 * ITEM_DIRECTION_X[i % 7];
			item_bullets[i].vy = 3 * ITEM_DIRECTION_Y[i % 7];
			item_bullets[i].hidden = true;
		}
		//ult_bullet
		if (character == CH_PLANE) {
            ult_bullet.w = al_get_bitmap_width(start_img_ult);
            ult_bullet.h = al_get_bitmap_height(start_img_ult);
            ult_bullet.img = start_img_ult;
        } else if (character == CH_CAT) {
            ult_bullet.w = al_get_bitmap_width(start_img_cat_ult);
            ult_bullet.h = al_get_bitmap_height(start_img_cat_ult);
            ult_bullet.img = start_img_cat_ult;
        }
        ult_bullet.vx = 0;
        ult_bullet.vy = -3;
        ult_bullet.hidden = true;

		if (is_bgm){
            if( character == CH_PLANE && !al_play_sample(start_bgm, 1, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, &start_bgm_id))
                game_abort("failed to play audio (bgm)");
            else if( character == CH_CAT && !al_play_sample(start_bgm_cat, 1, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, &start_bgm_cat_id))
                game_abort("failed to play audio (bgm)");
		}
	} else if (active_scene == SCENE_WIN) {
		if (is_bgm && !al_play_sample(win_bgm, 1, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, &win_bgm_id))
			game_abort("failed to play audio (bgm)");
	} else if (active_scene == SCENE_LOSE) {
		if (is_bgm && !al_play_sample(lose_bgm, 1, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, &lose_bgm_id))
			game_abort("failed to play audio (bgm)");
	}
}

void on_key_down(int keycode) {
	if (active_scene == SCENE_MENU) {
		if (keycode == ALLEGRO_KEY_ENTER)
			game_change_scene(SCENE_INTRO);
	} else if (active_scene == SCENE_INTRO) {
		if (keycode == ALLEGRO_KEY_ENTER)
			game_change_scene(SCENE_START);
	}
}

void on_mouse_down(int btn, int x, int y) {
	// [HACKATHON 3-8]
	// TODO: When settings clicked, switch to settings scene.
	// Uncomment and fill in the code below.
	if (btn == 1) {
		if (active_scene == SCENE_MENU) {
			if (pnt_in_rect(x, y, SCREEN_W - 48, 10, 38, 38))
				game_change_scene(SCENE_SETTINGS);
		} else if (active_scene == SCENE_WIN) {
		    //try again
            if (pnt_in_rect(x, y, SCREEN_W - 320, 240, 235, 62))
                game_change_scene(SCENE_INTRO);
            //settings
            if (pnt_in_rect(x, y, SCREEN_W - 48, 10, 38, 38))
				game_change_scene(SCENE_SETTINGS);
		} else if (active_scene == SCENE_LOSE) {
		    //try again
            if (pnt_in_rect(x, y, SCREEN_W - 273, SCREEN_H - 100, 235, 62))
                game_change_scene(SCENE_INTRO);
            //settings
            if (pnt_in_rect(x, y, SCREEN_W - 48, 10, 38, 38))
				game_change_scene(SCENE_SETTINGS);
		} else if (active_scene == SCENE_SETTINGS) {
		    //back to menu
		    if (pnt_in_rect(x, y, SCREEN_W - 48, 10, 38, 38))
				game_change_scene(SCENE_INTRO);
		    //click audio on/off
            else if (pnt_in_rect(x, y, 105, SCREEN_H - 95, 40, 40)) {
                is_bgm = !is_bgm;
                game_log("is_bgm = %d", is_bgm);
            } else if (pnt_in_rect(x, y, 288, SCREEN_H - 55, 40, 40)) {
                is_se = !is_se;
                game_log("is_se = %d", is_se);
            } //choose character
            else if (pnt_in_rect(x, y, 380, SCREEN_H - 75, 51, 51)) {
                character = CH_PLANE;
            } else if (is_win && pnt_in_rect(x, y, 470, SCREEN_H - 80, 70, 70)) {
                character = CH_CAT;
            }
		}
	}
}

void draw_movable_object(MovableObject obj) {
	if (obj.hidden)
		return;
	al_draw_bitmap(obj.img, round(obj.x - obj.w / 2), round(obj.y - obj.h / 2), 0);
}

ALLEGRO_BITMAP *load_bitmap_resized(const char *filename, int w, int h) {
	ALLEGRO_BITMAP* loaded_bmp = al_load_bitmap(filename);
	if (!loaded_bmp)
		game_abort("failed to load image: %s", filename);
	ALLEGRO_BITMAP *resized_bmp = al_create_bitmap(w, h);
	ALLEGRO_BITMAP *prev_target = al_get_target_bitmap();

	if (!resized_bmp)
		game_abort("failed to create bitmap when creating resized image: %s", filename);
	al_set_target_bitmap(resized_bmp);
	al_draw_scaled_bitmap(loaded_bmp, 0, 0,
		al_get_bitmap_width(loaded_bmp),
		al_get_bitmap_height(loaded_bmp),
		0, 0, w, h, 0);
	al_set_target_bitmap(prev_target);
	al_destroy_bitmap(loaded_bmp);

	game_log("resized image: %s", filename);

	return resized_bmp;
}

// [HACKATHON 3-3]
// TODO: Define bool pnt_in_rect(int px, int py, int x, int y, int w, int h)
// Uncomment and fill in the code below.
bool pnt_in_rect(int px, int py, int x, int y, int w, int h) {
	return (px < x+w) && (px > x) && (py < y+h) && (py > y);
}

void fly_in(MovableObject *a, int h_up, int h_down) {
    if (a->x - a->w/2 < 0) {
        a->x = a->w/2;
        a->vx *= -1;
    }
    else if (a->x + a->w/2 > SCREEN_W) {
        a->x = SCREEN_W - a->w/2;
        a->vx *= -1;
    }

    if (a->y - a->h/2 < h_up) {
        a->y = h_up + a->h/2;
        a->vy *= -1;
    }
    else if (a->y + a->h/2 > h_down) {
        a->y = h_down - a->h/2;
        a->vy *= -1;
    }
}
void fly_out(MovableObject *a) {
    a->x += a->vx;
    a->y += a->vy;
    if (a->y + a->h/2 < 0 || a->y - a->h/2 > SCREEN_H || \
        a->x + a->w/2 < 0 || a->x - a->w/2 > SCREEN_W)
        a->hidden = true;
}

bool hit(MovableObject a, MovableObject b) {
    return ( (a.x > b.x - b.w/2) && (a.x < b.x + b.w/2) && \
    (a.y > b.y - b.h/2) && (a.y < b.y + b.h/2) ) || \
    ( (b.x > a.x - a.w/2) && (b.x < a.x + a.w/2) && \
    (b.y > a.y - a.h/2) && (b.y < a.y + a.h/2) );
}
bool hurt(MovableObject *b) {
    if (b->hidden)
        return true;

    (b->blood)--;
    //game_log("Enemy's blood: %d", b->blood);
    if (b->blood <= 0) {
        (b->lives)--;
        (b->blood) = PLANE_BLOOD;
    }
    if (b->lives <= 0) {
        b->hidden = true;
        return true;
    }
    return false;
    //true means b is died, false means alive
}
void hurt_and_add_score(MovableObject *b) {
    score += 73;
    if (hurt(b))
        score += 877;
}

void shoot_bullet(MovableObject *v, MovableObject *ed) {
    ed->hidden = false;
    ed->x = v->x;
    if (ed->vy < 0)
        ed->y = v->y - (v->h)/2 - (ed->h)/2;
    else if (ed->vy > 0)
        ed->y = v->y + (v->h)/2;
}

// +=================================================================+
// | Code below is for debugging purpose, it's fine to remove it.    |
// | Deleting the code below and removing all calls to the functions |
// | doesn't affect the game.                                        |
// +=================================================================+

void game_abort(const char* format, ...) {
	va_list arg;
	va_start(arg, format);
	game_vlog(format, arg);
	va_end(arg);
	fprintf(stderr, "error occured, exiting after 2 secs");
	// Wait 2 secs before exiting.
	al_rest(2);
	// Force exit program.
	exit(1);
}

void game_log(const char* format, ...) {
#ifdef LOG_ENABLED
	va_list arg;
	va_start(arg, format);
	game_vlog(format, arg);
	va_end(arg);
#endif
}

void game_vlog(const char* format, va_list arg) {
#ifdef LOG_ENABLED
	static bool clear_file = true;
	vprintf(format, arg);
	printf("\n");
	// Write log to file for later debugging.
	FILE* pFile = fopen("log.txt", clear_file ? "w" : "a");
	if (pFile) {
		vfprintf(pFile, format, arg);
		fprintf(pFile, "\n");
		fclose(pFile);
	}
	clear_file = false;
#endif
}
