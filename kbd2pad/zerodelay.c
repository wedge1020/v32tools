//
// zerodelay.c - custom keyboard input and processing tool, based heavily on
//               the showkey(1) program as found in the kbd package.
//
//               modifications to better suit the needs of kbd2pad, when the
//               USB ZERODELAY encoder board is being used.
//
//               This program will interface with the GPIO pins on the pi to
//               handle button "presses".  It will take keystrokes from  the
//               keyboard and "digitize" it, each bit position (of those low
//               7 bits) corresponding with a button.
//
// getkey.c, like the showkey.c it is based on, is licensed under the GNU
// General Public License  (GPL), version 2, or at your  option any later
// version.
//
//////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <poll.h>
#include <signal.h>
#include <fcntl.h>
#include <termios.h>
#include <linux/kd.h>
#include <linux/keyboard.h>
#include <sys/ioctl.h>
#include <wiringPi.h>

typedef struct termios TermIOS;
typedef struct pollfd  PollFD;

//////////////////////////////////////////////////////////////////////////////
//
// Global variables
//
int32_t  tmp;        // for debugging
int32_t  fd;
int32_t  oldkbmode;
TermIOS  old;

//////////////////////////////////////////////////////////////////////////////
//
// function prototypes
//
int32_t  is_a_console   (int32_t);
int32_t  open_a_console (int8_t *);
int32_t  getfd          (int8_t *);

//////////////////////////////////////////////////////////////////////////////
//
// get_mode(): determine the kbmode
//
// version 0.81 of showkey would restore kbmode unconditionally to XLATE,
// thus making the console unusable when it was called under X.
//
static void get_mode (void)
{
    int8_t *mode   = NULL;
    if (ioctl (fd, KDGKBMODE, &oldkbmode))
    {
        perror ("KDGKBMODE");
        exit (1);
    }

    switch (oldkbmode)
    {
      case K_RAW:
          mode     = (int8_t *) "RAW";
          break;

      case K_XLATE:
          mode     = (int8_t *) "XLATE";
          break;

      case K_MEDIUMRAW:
          mode     = (int8_t *) "MEDIUMRAW";
          break;

      case K_UNICODE:
          mode     = (int8_t *) "UNICODE";
          break;

      default:
          mode     = (int8_t *) "?UNKNOWN?";
          break;
    }

    fprintf(stdout, "kb mode was %s\n", mode);

    if (oldkbmode != K_XLATE)
    {
        fprintf (stdout, "[ if you are trying this under X, it might not work\n"
                         "since the X server is also reading /dev/console ]\n");
    }

    fprintf (stdout, "\n");
}

//////////////////////////////////////////////////////////////////////////////
//
// clean_up(): restore the session
//
static void clean_up (void)
{
    if (ioctl (fd, KDSKBMODE, oldkbmode))
    {
        perror ("KDSKBMODE");
        exit (1);
    }

    if (tcsetattr (fd, 0, &old) == -1)
    {
        perror("tcsetattr");
    }

    close (fd);
}

//////////////////////////////////////////////////////////////////////////////
//
// die(): process signal (clean up and terminate)
//
static void die (int32_t  signum)
{
    fprintf (stdout, "caught signal %d, cleaning up...\n", signum);
    clean_up ();
    exit (1);
}

static void watch_dog (int32_t  x)
{
    clean_up ();
    exit (0);
}

//////////////////////////////////////////////////////////////////////////////
//
// usage(): display usage information
//
static void display_usage (void)
{
    fprintf (stderr, "showkey\n\n");
    fprintf (stderr, "usage: showkey [options...]\n\n");
    fprintf (stderr, "valid options are:\n\n");
    fprintf (stderr, "  -h, --help       display this help text\n");
    fprintf (stderr, "  -a, --ascii      display the decimal/octal/hex values of the keys\n");
    fprintf (stderr, "  -s, --scancodes  display raw scan-codes\n");
    fprintf (stderr, "  -k, --keycodes   display interpreted keycodes\n\n");
    fprintf (stderr, "NOTE: default is to display interpreted keycodes\n\n");

    exit (1);
}

int32_t  main (int32_t  argc, char **argv)
{
    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Declare and initialize local variables
    //
    int8_t  *short_opts              = (int8_t *) "hask";
    int32_t  index                   = 0;
    int32_t  value                   = 0;
    int32_t  mask                    = 0x40;
    int32_t  num_bytes               = 0;
    int32_t  option                  = 0;
    int32_t  print_ascii             = FALSE;
    int32_t  show_keycodes           = TRUE;
    int32_t  timeout_ms              = 32;
    PollFD   fds[1];
    TermIOS  new;
    uint8_t  buffer[16];

    fds[0].fd                        = STDIN_FILENO;
    fds[0].events                    = POLLIN;

    wiringPiSetup ();

    for (index           = 6;
         index          <= 12;
         index           = index + 1)
    {
        pinMode (index, OUTPUT);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Establish long option to short option mappings for getopt(3)
    //
    const struct option long_opts[]  = {
        { "help",      no_argument, NULL, 'h' },
        { "ascii",     no_argument, NULL, 'a' },
        { "scancodes", no_argument, NULL, 's' },
        { "keycodes",  no_argument, NULL, 'k' },
        { NULL,        0,           NULL,  0  }
    };

    option                           = getopt_long (argc, argv,
                                                    (const char *) short_opts,
                                                    long_opts,
                                                    NULL);
    while (option                   != -1)
    {
        switch (option)
        {
            case 's':
                show_keycodes        = FALSE;
                break;

            case 'k':
                show_keycodes        = TRUE;
                break;
            case 'a':
                print_ascii          = TRUE;
                break;

            case 'h':
            case '?':
                display_usage ();
                break;
        }
        option                       = getopt_long (argc, argv,
                                                    (const char *) short_opts,
                                                    long_opts,
                                                    NULL);
    }

    if (optind                      <  argc)
    {
        display_usage ();
    }

    if (print_ascii                 == TRUE)
    {
        /* no mode and signal and timer stuff - just read stdin */
        fd                           = 0;
        if (tcgetattr (fd, &old)    == -1)
        {
            perror ("tcgetattr");
        }

        if (tcgetattr (fd, &new)    == -1)
        {
            perror ("tcgetattr");
        }

        new.c_lflag                  = new.c_lflag & (~ (ICANON | ISIG));
        new.c_lflag                  = new.c_lflag | (ECHO | ECHOCTL);
        new.c_iflag                  = 0;
        new.c_cc[VMIN]               = 1;
        new.c_cc[VTIME]              = 0;
        if (tcsetattr (fd, TCSAFLUSH, &new) == -1)
        {
            perror ("tcgetattr");
        }

        fprintf (stdout, "\nPress any keys - "
                 "Ctrl-D will terminate this program\n\n");
        while (1)
        {
            //////////////////////////////////////////////////////////////////
            //
            // poll() results: greater than 0 if key is pressed, equal to
            //                 zero if we hit the timeout, and less  than
            //                 zero if some error occurred 
            //
            value                    = poll (fds, 1, timeout_ms);
			fprintf (stdout, "[value] %d\n", value);

            if (value               >  0) // key pressed
            {
                num_bytes            = read (fd, buffer, 1);
                if (num_bytes       == 1)
                {
                    printf(" \t%3d 0%03o 0x%02x\n",
                           buffer[0], buffer[0], buffer[0]);
                    mask             = 0x40;
                    for (index       = 6;
                         index      <= 12;
                         index       = index + 1)
                    {
                        value        = buffer[0] & mask;
                        if (value   >  0)
                        {
                            value    = HIGH;
                        }
                        digitalWrite (index, value);
                        mask         = mask >> 1;
                    }
                }
            }
            else if (value          == 0) // reached timeout
            {
                //////////////////////////////////////////////////////////////
                //
                // reset gamepad bits to 0. This is an attempt to simulate
                // a momentary button press
                //
                for (index           = 6;
                     index          <= 12;
                     index           = index + 1)
                {
                    digitalWrite (index, LOW);
                }
            }

            if ((num_bytes          != 1) ||
                (buffer[0]          == 04))
            {
                break;
            }
        }

        if (tcsetattr (fd, 0, &old) == -1)
        {
            perror ("tcsetattr");
        }
        exit (0);
    }

    fd                               = getfd (NULL);

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // watchdog: terminate program after 10 seconds of inactivity (no input)
    //
    signal (SIGALRM,   watch_dog);

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // any other signal of note: receiving a signal instigates a nice exit, where
    // the keyboard needs to be restored to a usable mode
    //
    signal (SIGHUP,    die);
    signal (SIGINT,    die);
    signal (SIGQUIT,   die);
    signal (SIGILL,    die);
    signal (SIGTRAP,   die);
    signal (SIGABRT,   die);
    signal (SIGIOT,    die);
    signal (SIGFPE,    die);
    signal (SIGKILL,   die);
    signal (SIGUSR1,   die);
    signal (SIGSEGV,   die);
    signal (SIGUSR2,   die);
    signal (SIGPIPE,   die);
    signal (SIGTERM,   die);
#ifdef SIGSTKFLT
    signal (SIGSTKFLT, die);
#endif
    signal (SIGCHLD,   die);
    signal (SIGCONT,   die);
    signal (SIGSTOP,   die);
    signal (SIGTSTP,   die);
    signal (SIGTTIN,   die);
    signal (SIGTTOU,   die);

    get_mode ();

    if (tcgetattr (fd, &old)        == -1)
    {
        perror ("tcgetattr");
    }

    if (tcgetattr (fd, &new)        == -1)
    {
        perror ("tcgetattr");
    }

    new.c_lflag                      = new.c_lflag & (~ (ICANON | ECHO | ISIG));
    new.c_iflag                      = 0;
    new.c_cc[VMIN]                   = sizeof (buffer);
    new.c_cc[VTIME]                  = 1;  // 0.1 sec intercharacter timeout

    if (tcsetattr (fd, TCSAFLUSH, &new) == -1)
    {
        perror ("tcsetattr");
    }

    if (ioctl (fd, KDSKBMODE, show_keycodes ? K_MEDIUMRAW : K_RAW))
    {
        perror ("KDSKBMODE");
        exit(1);
    }

    fprintf (stdout, "press any key (program terminates 10s after last keypress)...\n");

    while (1)
    {
        alarm (10);
        num_bytes                    = read (fd, buffer, sizeof (buffer));
        for (index                   = 0;
             index                  <  num_bytes;
             index                   = index + 1)
        {
            if (show_keycodes       == FALSE)
            {
                fprintf (stdout, "0x%02x ", buffer[index]);
            }
            else
            {
                fprintf (stdout, "keycode %3d %s\n",
                                  buffer[index] & 0x7f,
                                  buffer[index] & 0x80 ? "release"
                                                       : "press");
            }
        }

        if (show_keycodes           == FALSE)
        {
            fprintf (stdout, "\n");
        }
    }
    clean_up ();
    return (0);
}

int32_t  is_a_console (int32_t  fd)
{
    int8_t  arg   = 0;
    return ((ioctl (fd, KDGKBTYPE, &arg) == 0) &&
           ((arg == KB_101) ||
            (arg == KB_84)));
}

int32_t  open_a_console (int8_t *filename)
{
    int32_t  fd  = 0;

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // for ioctl purposes an fd is needed. Permissions do not matter.
    //
    // NOTE: setfont:activatemap() performs a write.
    //
    fd           = open ((const char *) filename, O_RDWR);
    if ((fd     <  0) &&
        (errno  == EACCES))
    {
        fd       = open ((const char *) filename, O_WRONLY);
    }

    if ((fd     <  0) &&
        (errno  == EACCES))
    {
        fd       = open ((const char *) filename, O_RDONLY);
    }

    if (fd      <  0)
    {
        return (-1);
    }

    if (!is_a_console (fd))
    {
        close (fd);
        return (-1);
    }

    return (fd);
}

////////////////////////////////////////////////////////////////////////////////////////
//
// getfd(): get a file descriptor  (fd) for use with kbd/console ioctls.
//          Different   methods   are   tried  because  the  opening  of
//          /dev/console will fail  if  an  X session  has  taken  place
//          (as  it does  a  chown  on /dev/console).
//
int32_t  getfd (int8_t *filename)
{
    int32_t  fd   = 0;
    if (filename == NULL)
    {
        fd        = open_a_console (filename);
        if (fd   >= 0)
        {
            return (fd);
        }

        fprintf (stderr, "Couldnt open %s\n", filename);
        exit (1);
    }

    fd            = open_a_console ((int8_t *) "/dev/tty");
    if (fd       >= 0)
    {
        return (fd);
    }

    fd            = open_a_console ((int8_t *) "/dev/tty0");
    if (fd       >= 0)
    {
        return (fd);
    }

    fd            = open_a_console ((int8_t *) "/dev/vc/0");
    if (fd       >= 0)
    {
        return (fd);
    }

    fd            = open_a_console ((int8_t *) "/dev/console");
    if (fd       >= 0)
    {
        return (fd);
    }

    for (fd       = 0;
         fd      <  3;
         fd       = fd + 1)
    {
        if (is_a_console (fd))
        {
            return (fd);
        }
    }

    fprintf (stderr, "Couldnt get a file descriptor referring to the console\n");

    exit (1);        /* total failure */
}
