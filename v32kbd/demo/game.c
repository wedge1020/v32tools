#include "video.h"
#include "time.h"
#include "input.h"
#include "audio.h"
#include "string.h"
#include "keyboard.h"

//////////////////////////////////////////////////////////////////////////////
//
// v32kbd demo: "POTENTIAL FUN ADVENTURE GAME"
//
// 1) A title screen, with the title drawn as a blocky bitmap font made of
//    solid block BIOS characters (region 20 = solid 10x20 block, 19/18/17
//    are progressively lighter). Press START (or Enter on the v32kbd).
// 2) A player name entry screen, Zelda style: a grid of characters that
//    can be navigated with a regular gamepad in port 1, or you can just
//    type directly on the v32kbd (expected in gamepad port 2, id 1).
//    Typed characters are momentarily highlighted in the grid, and the
//    grid is encased in a frame of region 20 solid blocks. Simple, light
//    music plays while this screen is displayed.
// 3) A confirmation screen, then back to the title.
//
// NOTE ON MUSIC: the name entry screen plays sound 0 of the rom, which
// must be "music.wav" (any short, light tune; it is looped at low
// volume). To remove music, empty out start_music()/stop_music().
//
#define MUSIC_SOUND     0    // rom sound id of "music.wav"
#define MUSIC_CHANNEL   0    // audio channel used for the music
#define MUSIC_VOLUME    0.2  // volume is a float in range 0 to 1

#define STATE_TITLE     0
#define STATE_NAME      1
#define STATE_DONE      2

#define NAME_MAX        8    // player name length limit

#define GRID_COLS       8    // character grid is 8 x 5
#define GRID_ROWS       5
#define GRID_X          200  // top left corner of the grid (pixels)
#define GRID_Y          110
#define CELL_W          30   // spacing between grid cells
#define CELL_H          40

#define FLASH_TIME      20   // frames a typed character stays highlighted

//////////////////////////////////////////////////////////////////////////////
//
// The 3x5 blocky font for the big title. Each letter is 5 rows of 3 bits
// (bit 2 = leftmost column). Index 0 is the space, 1-26 are 'A' to 'Z'.
//
int [27*5] bigfont =
{
    // space
    0, 0, 0, 0, 0,
    // A        B        C        D        E        F
    2, 5, 7, 5, 5,    6, 5, 6, 5, 6,    3, 4, 4, 4, 3,    6, 5, 5, 5, 6,
    7, 4, 6, 4, 7,    7, 4, 6, 4, 4,
    // G        H        I        J        K        L
    3, 4, 5, 5, 3,    5, 5, 7, 5, 5,    7, 2, 2, 2, 7,    1, 1, 1, 5, 2,
    5, 5, 6, 5, 5,    4, 4, 4, 4, 7,
    // M        N        O        P        Q        R
    5, 7, 7, 5, 5,    6, 5, 5, 5, 5,    2, 5, 5, 5, 2,    6, 5, 6, 4, 4,
    2, 5, 5, 6, 1,    6, 5, 6, 5, 5,
    // S        T        U        V        W        X
    3, 4, 2, 1, 6,    7, 2, 2, 2, 2,    5, 5, 5, 5, 7,    5, 5, 5, 5, 2,
    5, 5, 7, 7, 5,    5, 5, 2, 5, 5,
    // Y        Z
    5, 5, 2, 2, 2,    7, 1, 2, 4, 7
};

//////////////////////////////////////////////////////////////////////////////
//
// Working strings (Vircon32 strings are int arrays)
//
int [14]    title_text1;   // "POTENTIAL FUN"
int [15]    title_text2;   // "ADVENTURE GAME"
int [2]     block_text;    // single block character string
int [2]     cell_text;     // single character string (for grid cells)
int [64]    bar_text;      // row of block characters (borders, strips)
int [NAME_MAX+1] name;     // the player name being entered
int [41]    grid_chars;    // the 40 characters of the name entry grid

//////////////////////////////////////////////////////////////////////////////
//
// State
//
int state         = STATE_TITLE;
int prev_state    = STATE_TITLE;
int name_length   = 0;
int cursor        = 0;          // grid position of the gamepad cursor
int flash_cell    = -1;         // grid position of a typed character
int flash_timer   = 0;
bool prev_left    = false;      // for gamepad edge detection
bool prev_right   = false;
bool prev_up      = false;
bool prev_down    = false;
bool prev_a       = false;
bool prev_b       = false;
bool prev_start   = false;

v32kbd *keyboard  = NULL;

//////////////////////////////////////////////////////////////////////////////
//
// print_block(): print a single block character (a 10x20 BIOS region)
//
void print_block (int x, int y, int ch)
{
    block_text[0]  = ch;
    print_at (x, y, block_text);
}

//////////////////////////////////////////////////////////////////////////////
//
// print_block_bar(): print a horizontal row of block characters
//
void print_block_bar (int x, int y, int ch, int count)
{
    int i;

    for (i = 0; i < count; i = i + 1)
    {
        bar_text[i]  = ch;
    }
    bar_text[count] = 0;
    print_at (x, y, bar_text);
}

//////////////////////////////////////////////////////////////////////////////
//
// draw_big_text(): draw a string with the 3x5 block font. Each font "pixel"
// is a solid 10x20 block (BIOS region 20). A softer shadow made of the
// lighter region 19 is drawn first, offset one cell down and right
//
void draw_big_text (int *text, int x, int y)
{
    int  index    = 0;
    int  letter   = 0;
    int  row      = 0;
    int  col      = 0;
    int  bits     = 0;

    while (text[index] != 0)
    {
        // find the letter index in the font (space = 0, A-Z = 1-26)
        letter    = 0;
        if ((text[index] >= 'A') && (text[index] <= 'Z'))
        {
            letter = 1 + (text[index] - 'A');
        }

        // shadow pass: lighter block (region 19), offset one cell
        set_multiply_color (color_gray);
        for (row = 0; row < 5; row = row + 1)
        {
            bits = bigfont[(letter * 5) + row];
            for (col = 0; col < 3; col = col + 1)
            {
                if ((bits & (4 >> col)) != 0)
                {
                    print_block (x + (col * 10) + 10,
                                 y + (row * 20) + 20, 19);
                }
            }
        }

        // main pass: solid block (region 20)
        set_multiply_color (color_white);
        for (row = 0; row < 5; row = row + 1)
        {
            bits = bigfont[(letter * 5) + row];
            for (col = 0; col < 3; col = col + 1)
            {
                if ((bits & (4 >> col)) != 0)
                {
                    print_block (x + (col * 10), y + (row * 20), 20);
                }
            }
        }

        // advance: 3 cells for the letter + 1 cell of separation
        x = x + 40;
        index = index + 1;
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// draw_title_screen(): title, decorative block gradient, PRESS START
//
void draw_title_screen (void)
{
    int frame;

    frame        = get_frame_counter ();

    //////////////////////////////////////////////////////////////////////
    //
    // The big block title (13 letters = 510 pixels, 14 = 550)
    //
    draw_big_text (title_text1, 65, 60);
    draw_big_text (title_text2, 45, 180);

    //////////////////////////////////////////////////////////////////////
    //
    // A fading "floor" made of the progressively lighter block regions:
    // solid 20 first, then 19, 18 and 17
    //
    set_multiply_color (color_white);
    print_block_bar (0, 300, 20, 64);
    print_block_bar (0, 320, 19, 64);
    print_block_bar (0, 340, 18, 64);
    print_block_bar (0, 360, 17, 64);

    //////////////////////////////////////////////////////////////////////
    //
    // Small header and blinking PRESS START
    //
    set_multiply_color (color_yellow);
    print_at (260, 20, "V32KBD DEMO");

    if ((frame & 32) == 0)
    {
        set_multiply_color (color_white);
        print_at (215, 410, "PRESS START TO BEGIN");
    }

    set_multiply_color (color_gray);
    print_at (155, 440, "(OR PRESS ENTER ON THE V32KBD KEYBOARD)");
}

//////////////////////////////////////////////////////////////////////////////
//
// draw_grid_frame(): encase the character grid in solid region 20 blocks
//
void draw_grid_frame (void)
{
    int i;

    set_multiply_color (color_blue);

    // top and bottom edges (1 block taller than the grid)
    print_block_bar (GRID_X - 20, GRID_Y - 20, 20, 10);
    print_block_bar (GRID_X - 20, GRID_Y + (GRID_ROWS * CELL_H), 20, 10);

    // left and right edges
    for (i = 0; i <= GRID_ROWS; i = i + 1)
    {
        print_block (GRID_X - 20, GRID_Y - 20 + (i * CELL_H), 20);
        print_block (GRID_X + (GRID_COLS * CELL_W),
                     GRID_Y - 20 + (i * CELL_H), 20);
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// draw_name_screen(): name display, grid, cursor, highlights, help text
//
void draw_name_screen (void)
{
    int i;
    int c;
    int cell_x;
    int cell_y;
    int frame;

    frame   = get_frame_counter ();

    //////////////////////////////////////////////////////////////////////
    //
    // Header and the name being entered
    //
    set_multiply_color (color_yellow);
    print_at (245, 20, "ENTER YOUR NAME");

    set_multiply_color (color_white);
    print_at (180, 60, "NAME:");

    for (i = 0; i < NAME_MAX; i = i + 1)
    {
        if (i < name_length)
        {
            c = name[i];
        }
        else
        {
            c = '_';
        }
        cell_text[0] = c;
        print_at (250 + (i * 20), 60, cell_text);
    }

    //////////////////////////////////////////////////////////////////////
    //
    // The character grid, encased in solid blocks
    //
    draw_grid_frame ();

    for (i = 0; i < (GRID_COLS * GRID_ROWS); i = i + 1)
    {
        cell_x     = GRID_X + ((i % GRID_COLS) * CELL_W) + 10;
        cell_y     = GRID_Y + ((i / GRID_COLS) * CELL_H) + 10;
        cell_text[0] = grid_chars[i];

        // cell highlighted by a keyboard press: solid white + black char
        if ((i == flash_cell) && (flash_timer > 0))
        {
            set_multiply_color (color_white);
            print_block (cell_x - 10, cell_y - 10, 20);
            set_multiply_color (color_black);
            print_at (cell_x, cell_y, cell_text);
        }

        // cell under the gamepad cursor: blinking gray + black char
        else if (i == cursor)
        {
            if ((frame & 32) == 0)
            {
                set_multiply_color (color_gray);
                print_block (cell_x - 10, cell_y - 10, 20);
                set_multiply_color (color_black);
                print_at (cell_x, cell_y, cell_text);
            }
            else
            {
                set_multiply_color (color_white);
                print_at (cell_x, cell_y, cell_text);
            }
        }

        // plain cell
        else
        {
            set_multiply_color (color_white);
            print_at (cell_x, cell_y, cell_text);
        }
    }

    //////////////////////////////////////////////////////////////////////
    //
    // Controls help
    //
    set_multiply_color (color_gray);
    print_at (95, 330, "GAMEPAD: DPAD MOVE   A SELECT   B ERASE");
    print_at (215, 350, "START: DONE");

    print_at (70, 400, "V32KBD: TYPE DIRECTLY   ARROWS MOVE CURSOR");
    print_at (170, 420, "BACKSPACE ERASE   ENTER DONE");

    // live modifier indicators, in the spirit of the test program
    if (v32kbd_isdown (&keyboard, V32KEY_LSHIFT) ||
        v32kbd_isdown (&keyboard, V32KEY_RSHIFT))
    {
        set_multiply_color (color_yellow);
        print_at (310, 60, "SHIFT");
    }

    if (v32kbd_isdown (&keyboard, V32KEY_LCTRL) ||
        v32kbd_isdown (&keyboard, V32KEY_RCTRL))
    {
        set_multiply_color (color_cyan);
        print_at (370, 60, "CTRL");
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// draw_done_screen(): show the chosen name
//
void draw_done_screen (void)
{
    int i;

    set_multiply_color (color_white);
    print_at (270, 100, "WELCOME,");

    // show the name, centered, 20 pixels per character
    for (i = 0; i < name_length; i = i + 1)
    {
        cell_text[0] = name[i];
        print_at (320 - (name_length * 10) + (i * 20), 140, cell_text);
    }

    set_multiply_color (color_white);
    print_at (280, 200, "YOUR ADVENTURE");
    print_at (280, 220, "BEGINS... SOON.");

    set_multiply_color (color_gray);
    print_at (205, 300, "PRESS START TO RETURN TO TITLE");
}

//////////////////////////////////////////////////////////////////////////////
//
// append_name_char(): add a character to the name (if it fits)
//
void append_name_char (int c)
{
    if (name_length < NAME_MAX)
    {
        name[name_length] = c;
        name_length       = name_length + 1;
        name[name_length] = 0;
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// erase_name_char(): remove the last character of the name
//
void erase_name_char (void)
{
    if (name_length > 0)
    {
        name_length       = name_length - 1;
        name[name_length] = 0;
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// grid_find(): find a character in the grid; returns the cell index,
// or -1 if the character is not in the grid
//
int grid_find (int c)
{
    int i;

    for (i = 0; i < (GRID_COLS * GRID_ROWS); i = i + 1)
    {
        if (grid_chars[i] == c)
        {
            return (i);
        }
    }

    return (-1);
}

//////////////////////////////////////////////////////////////////////////////
//
// flash_grid_char(): highlight a grid cell for a moment (keyboard typing)
//
void flash_grid_char (int c)
{
    int cell;

    cell = grid_find (c);
    if (cell >= 0)
    {
        flash_cell  = cell;
        flash_timer = FLASH_TIME;
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// name_accept(): is the name valid to be confirmed?
//
bool name_accept (void)
{
    return (name_length > 0);
}

//////////////////////////////////////////////////////////////////////////////
//
// start_music(): begin playing the light music, looped, on entry to the
// name screen. Music is rom sound MUSIC_SOUND (i.e. "music.wav" as the
// first sound file of the rom); there is no load_sound() in Vircon32,
// sounds are played directly by their rom id
//
void start_music (void)
{
    select_channel (MUSIC_CHANNEL);
    set_channel_volume (MUSIC_VOLUME);
    set_channel_loop (true);
    play_sound_in_channel (MUSIC_SOUND, MUSIC_CHANNEL);
}

//////////////////////////////////////////////////////////////////////////////
//
// stop_music(): silence the music when leaving the name entry screen
// (set USE_MUSIC to 0 and make these two functions empty to remove music)
//
void stop_music (void)
{
    stop_channel (MUSIC_CHANNEL);
}

//////////////////////////////////////////////////////////////////////////////
//
// update_music(): start or stop the music on state transitions
//
void update_music (void)
{
    if (state != prev_state)
    {
        if (state == STATE_NAME)
        {
            start_music ();
        }
        else if (prev_state == STATE_NAME)
        {
            stop_music ();
        }

        prev_state = state;
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// update_gamepad(): handle player 1 gamepad input for the current state
//
void update_gamepad (void)
{
    bool left, right, up, down, a, b, start;

    select_gamepad (0);
    if (gamepad_is_connected () == false)
    {
        return;
    }

    left  = (gamepad_left ()  > 0);
    right = (gamepad_right () > 0);
    up    = (gamepad_up ()    > 0);
    down  = (gamepad_down ()  > 0);
    a     = (gamepad_button_a () > 0);
    b     = (gamepad_button_b () > 0);
    start = (gamepad_button_start () > 0);

    if (state == STATE_TITLE)
    {
        if ((start) && (prev_start == false))
        {
            state = STATE_NAME;
        }
    }

    else if (state == STATE_NAME)
    {
        // move the cursor (horizontal wrap, vertical clamp)
        if ((left) && (prev_left == false))
        {
            cursor = (cursor + (GRID_COLS * GRID_ROWS) - 1)
                     % (GRID_COLS * GRID_ROWS);
        }
        if ((right) && (prev_right == false))
        {
            cursor = (cursor + 1) % (GRID_COLS * GRID_ROWS);
        }
        if ((up) && (prev_up == false))
        {
            if (cursor >= GRID_COLS)
            {
                cursor = cursor - GRID_COLS;
            }
        }
        if ((down) && (prev_down == false))
        {
            if (cursor < (GRID_COLS * (GRID_ROWS - 1)))
            {
                cursor = cursor + GRID_COLS;
            }
        }

        // select, erase and confirm
        if ((a) && (prev_a == false))
        {
            append_name_char (grid_chars[cursor]);
            flash_cell  = cursor;
            flash_timer = FLASH_TIME;
        }
        if ((b) && (prev_b == false))
        {
            erase_name_char ();
        }
        if ((start) && (prev_start == false) && (name_accept ()))
        {
            state = STATE_DONE;
        }
    }

    else // STATE_DONE
    {
        if ((start) && (prev_start == false))
        {
            state = STATE_TITLE;
        }
    }

    prev_left  = left;
    prev_right = right;
    prev_up    = up;
    prev_down  = down;
    prev_a     = a;
    prev_b     = b;
    prev_start = start;
}

//////////////////////////////////////////////////////////////////////////////
//
// update_keyboard(): handle v32kbd input for the current state
//
void update_keyboard (void)
{
    int key;
    int c;

    key = v32kbd_read (&keyboard);
    while (key > 0)
    {
        //////////////////////////////////////////////////////////////////////
        //
        // Keys common to all screens
        //
        if (key == V32KEY_ENTER)
        {
            if (state == STATE_TITLE)
            {
                state = STATE_NAME;
            }
            else if ((state == STATE_NAME) && (name_accept ()))
            {
                state = STATE_DONE;
            }
            else if (state == STATE_DONE)
            {
                state = STATE_TITLE;
            }
        }

        else if (state == STATE_NAME)
        {
            // straight data entry: letters (uppercased), digits, symbols
            if ((key >= 32) && (key < 127))
            {
                c = key;
                if ((c >= 'a') && (c <= 'z'))
                {
                    c = c - 32;
                }

                if (grid_find (c) >= 0)
                {
                    append_name_char (c);
                    flash_grid_char (c);
                }
            }

            else if (key == V32KEY_BACKSPACE)
            {
                erase_name_char ();
            }

            // arrows also move the grid cursor, like the gamepad d-pad
            else if (key == V32KEY_LEFT)
            {
                cursor = (cursor + (GRID_COLS * GRID_ROWS) - 1)
                         % (GRID_COLS * GRID_ROWS);
            }
            else if (key == V32KEY_RIGHT)
            {
                cursor = (cursor + 1) % (GRID_COLS * GRID_ROWS);
            }
            else if (key == V32KEY_UP)
            {
                if (cursor >= GRID_COLS)
                {
                    cursor = cursor - GRID_COLS;
                }
            }
            else if (key == V32KEY_DOWN)
            {
                if (cursor < (GRID_COLS * (GRID_ROWS - 1)))
                {
                    cursor = cursor + GRID_COLS;
                }
            }
        }

        key = v32kbd_read (&keyboard);
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// main
//
void main (void)
{
    //////////////////////////////////////////////////////////////////////
    //
    // Initialize working strings and state
    //
    strcpy (title_text1, "POTENTIAL FUN");
    strcpy (title_text2, "ADVENTURE GAME");
    strcpy (grid_chars, "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 -.");

    block_text[1] = 0;
    cell_text[1]  = 0;

    name[0]       = 0;
    name_length   = 0;
    cursor        = 0;
    flash_cell    = -1;
    flash_timer   = 0;

    //////////////////////////////////////////////////////////////////////
    //
    // The v32kbd keyboard is expected in the SECOND gamepad port (id 1):
    // in the emulator, select "v32kbd" in menu Gamepads > Gamepad 2
    //
    keyboard      = v32kbd_init (1);

    while (true)
    {
        // check for keyboard activity (do this once on every frame)
        v32kbd_probe (&keyboard);

        // decay the typed-character highlight
        if (flash_timer > 0)
        {
            flash_timer = flash_timer - 1;
            if (flash_timer == 0)
            {
                flash_cell = -1;
            }
        }

        update_gamepad ();
        update_keyboard ();
        update_music ();

        // draw the current screen
        clear_screen (color_black);

        if (state == STATE_TITLE)
        {
            draw_title_screen ();
        }
        else if (state == STATE_NAME)
        {
            draw_name_screen ();
        }
        else
        {
            draw_done_screen ();
        }

        end_frame ();
    }
}
