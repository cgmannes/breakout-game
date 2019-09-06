/*
Running the command "make" in the command-line in the working directory
will build an executable file that can be run with the default game
parameters.

Command-line instructions to compile and run:

    g++ -o breakoutGame breakoutGame.cpp -L/usr/X11R6/lib -lX11 -lstdc++
	./snake

Note: the -L option and -lstdc++ may not be needed on some machines.
Execution of the above command-line inputs will build an executable
with the default game parameters.

Change difficulty by adding two space separated arguments at the end
of line 1. Specifying integers [0-9] will specify the desired ball
speed and paddle speed, respectively. Adding a third arugment with
integer [0-4] will specify the desired paddle length. An error is 
displayed if any other argument format is given.
*/

// Import header files.
#include <iostream>
#include <unistd.h> // sleep() etc.
#include <stdlib.h> // getenv() etc.
#include <sys/time.h>
#include <string>
#include <math.h>

// Header files for X functions.
#include <X11/Xlib.h>
#include <X11/Xutil.h> 

// Screen parameters.
const int SCREEN_WIDTH = 1300;
const int SCREEN_HEIGHT = 800;
const int WINDOW_CORNER_X = 10;
const int WINDOW_CORNER_Y = 10;
const int BORDER_WIDTH = 5;
const int STATS_OFFSET = 200;
const int WINDOW_HEIGHT = SCREEN_HEIGHT + STATS_OFFSET;

// Brick parameters.
int bricksRemaining;
const int NUM_OF_ROWS = 6;
const int NUM_OF_COLS = 13;
const int BRICK_WIDTH = 100;
const int BRICK_HEIGHT = 25;

// Ball parameters.
const double BALL_DIAMETER = 25.0;
const double INITIAL_BALL_X = 50.0; 
const double INITIAL_BALL_Y = 50.0;

// Paddle parameters.
int paddleLength = 50;
const int PADDLE_HEIGHT = 20;
const double INITIAL_PADDLE_X = (SCREEN_WIDTH / 2) - (paddleLength / 2);
const double INITIAL_PADDLE_Y = (SCREEN_HEIGHT - 100);

/*
 * Other parameters.
 */
// Fixed frames per second.
const double FPS = 60.0;

// Buffersize.
const int BUFFER_SIZE = 10;

// Array of speed values for game.
double speedArray[10] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};

// Array of length values for paddle length.
int paddleLengthValues[5] = {70, 80, 90, 100, 110};

// Default ball and paddle speed.
double ballSpeed = speedArray[5];
double paddleSpeed = speedArray[5];

// Scoring values.
int score = 0;
int destroyBrickPoints = 50.0;
int paddleBouncePoints = 20.0;

// Boolean game parameters.
bool showSplash = true;
bool alive = true;
bool gameWon = false;
bool gamePaused = false;
bool paddleLeft = false;
bool paddleRight = false;

enum Color {DEAD, RED, GREEN, BLUE, YELLOW, PURPLE, ORANGE};

Color brickArray[NUM_OF_ROWS][NUM_OF_COLS] = { {DEAD} };

void setBrickArray() {
    bricksRemaining = 0;
    for (int i = 2; i < 11; i++){
        brickArray[0][i] = RED;
        brickArray[1][i] = GREEN;
        brickArray[2][i] = BLUE;
        brickArray[3][i] = YELLOW;
        brickArray[4][i] = PURPLE;
        brickArray[5][i] = ORANGE;
        bricksRemaining += 6;
    }
}

/*
 * Function to extract the time in microseconds.
 */
unsigned long now() {
    timeval tv;
    gettimeofday(&tv, NULL);
    unsigned long us = tv.tv_sec * 1000000 + tv.tv_usec;
    return us;
}

/*
 * Function to output message on error exit.
 */
void error(std::string str) {

    std::cerr << str << std::endl;

    exit(0);
}

/*
 * function: Create_simple_window. Creates a window with a black background
 *           in the given size.
 * input:    Display, size of the window (in pixels), and location of the window's
 *           left corner (in pixels).
 * output:   Window's ID.
 * notes:    Window is created with a white border, 5 pixels wide.
 *           the window is automatically mapped after its creation.
 */
Window create_simple_window(Display* display, 
                            const int SCREEN_WIDTH, 
                            const int WINDOW_HEIGHT, 
                            const int WINDOW_CORNER_X, 
                            const int WINDOW_CORNER_Y) {
    
    // Returns the default screen number referenced by the XOpenDisplay() function.
    int screen_number = DefaultScreen(display);

    Window window;
    /* create a simple window, as a direct child of the screen's */
    /* root window. Use the screen's black and white colors as   */
    /* the foreground and background colors of the window,       */
    /* respectively. Place the new window's top-left corner at   */
    /* the given 'x,y' coordinates.                              */
    window = XCreateSimpleWindow(display,
                                RootWindow(display, screen_number),
                                WINDOW_CORNER_X,
                                WINDOW_CORNER_Y, 
                                SCREEN_WIDTH,
                                WINDOW_HEIGHT,
                                BORDER_WIDTH,
                                WhitePixel(display, screen_number),
                                BlackPixel(display, screen_number));
    
    // Make the window actually appear on the screen.
    XMapWindow(display, window);

    // Flush all pending requests to the X server. 
    XFlush(display);

    // Set event to monitor window.
    XSelectInput(display, window, KeyPressMask | KeyReleaseMask);

    // Window name.
    XStoreName(display, window, "BREAKOUT!");

    return window;
}

// Enter main program.
int main(int argc, char * argv[]) {

    // Read command-line arguments and procees game parameters.
    if (argc == 1) 
    {
        ballSpeed = 25*speedArray[5];
        paddleSpeed = 25*speedArray[7];
        paddleLength = 80;
    }
    else if (argc == 3)
    {
        ballSpeed = 25*speedArray[std::stoi(argv[1])];
        paddleSpeed = 25*speedArray[std::stoi(argv[2])];
    }
    else if (argc == 4)
    {
        ballSpeed = 25*speedArray[std::stoi(argv[1])];
        paddleSpeed = 25*speedArray[std::stoi(argv[2])];
        paddleLength = paddleLengthValues[std::stoi(argv[3])];
    }
    else
    {
        error("Invalid inputs");
    }
    
    // Pointer to X Display structure.
    Display * display;		
    
    // Number of screen to place the window on.
    int screen_number;		

    // Pointer to the newly created window.
    Window window;

    // Address of the X display.
    char * display_name = getenv("Breakout!");

    // Open connection with the X server. 
    display = XOpenDisplay(display_name);
    if (display == NULL) {
        error("Cannot connect to X server and open display.");
        exit(1);
    }

    /* Create a simple window, as a direct child of the screen's   */
    /* root window. Use the screen's black color as the background */
    /* color of the window. Place the new window's top-left corner */
    /* at the given (x,y) coordinates.                             */
    window = create_simple_window(display, SCREEN_WIDTH, WINDOW_HEIGHT,
                                WINDOW_CORNER_X, WINDOW_CORNER_Y);


    // Allocate a new GC graphic context object for drawing in the window.

    // Which values in "values" to check when creating GC
    unsigned long valueMask = 0;		

    // Initial values for the GC.
    XGCValues values;		           	
	GC gc = XCreateGC(display, window, valueMask, &values);
	XWindowAttributes w;
	XGetWindowAttributes(display, window, &w);

    // Color logic.
    Colormap colormap;
    XColor red, green, blue, yellow, purple, orange;
    //XColor red, brown, blue, yellow, green, orange, purple, white, black;

    // Define the colors to use.
    // Returns the colormap ID.
    colormap = DefaultColormap(display, DefaultScreen(display));

    // XAllocColor() returns the pixel value of the color closest to the specified 
    // RGB elements supported by the hardware and returns the RGB value actually used.
    XAllocNamedColor(display, colormap, "red", &red, &red);
    XAllocNamedColor(display, colormap, "green", &green, &green);
    XAllocNamedColor(display, colormap, "blue", &blue, &blue);
    XAllocNamedColor(display, colormap, "yellow", &yellow, &yellow);
    XAllocNamedColor(display, colormap, "purple", &purple, &purple);
    XAllocNamedColor(display, colormap, "orange", &orange, &orange);

	// The XCreatePixmap() function creates a pixmap of the width, height, 
    // and depth you specified and returns a pixmap ID that identifies it. 
    // Windows and pixmaps are drawables. Pixmap buffer is generated here 
    // implement a double buffer for drawing.
	int depth = DefaultDepth(display, DefaultScreen(display));
	Pixmap buffer = XCreatePixmap(display, window, SCREEN_WIDTH, WINDOW_HEIGHT, depth);

    // Initialize bricks by setBrickArray.
    setBrickArray();

    // Ball position and velocity,
    double ballX = INITIAL_BALL_X;
    double ballY = INITIAL_BALL_Y;

    XPoint ballDir;
    ballDir.x = ballSpeed;
    ballDir.y = ballSpeed;

    // Paddle position and velocity.
    double paddleX = INITIAL_PADDLE_X;
    const double paddleY = INITIAL_PADDLE_Y; 
	bool paddleLeft = false;
	bool paddleRight = false;

    // Save time of last logic update.
    unsigned long lastUpdate = now();

    // Save time of last window update.
    unsigned long lastRepaint = 0;

    // Event handle for current event.
    XEvent event;

    while (true) 
    {
        if (XPending(display) > 0)
        {
            XNextEvent(display, &event);

            switch (event.type)
            {
                case KeyPress:
                {                   
                    KeySym key;
                    char text[BUFFER_SIZE];
                    /*
                    * Exit when 'q' is typed.
                    * Arguments for XLookupString :
                    *                 event - the keyboard event
                    *                 text - buffer into which text will be written
                    *                 BufferSize - size of text buffer
                    *                 key - workstation independent key symbol
                    *                 0 - pointer to a composeStatus structure
                    */
                    int i = XLookupString((XKeyEvent*)&event, text, 10, &key, 0);

                    // Start game.
                    if (i == 1 && text[0] == ' ' && showSplash == true)
                    {
                        showSplash = false;
                    }
                    // Re-start game after losing.
                    else if (i == 1 && text[0] == ' ' && alive == false)
                    {
                        paddleX = INITIAL_PADDLE_X;
                        ballX = INITIAL_BALL_X;
                        ballY = INITIAL_BALL_Y;
                        score = 0;
                        setBrickArray();

                        // Reset alive.
                        alive = true;
                    }
                    // Re-start game after winning.
                    else if (i == 1 && text[0] == ' ' && gameWon == true)
                    {
                        paddleX = INITIAL_PADDLE_X;
                        ballX = INITIAL_BALL_X;
                        ballY = INITIAL_BALL_Y;
                        score = 0;
                        setBrickArray();

                        // Reset gameWon.
                        gameWon = false;
                    }

                    // Pause game.
                    if (i == 1 && text[0] == 'p' && !gamePaused)
                    {
                        gamePaused = true;
                    }
                    // Unpause game.
                    if (i == 1 && text[0] == ' ' && gamePaused)
                    {
                        gamePaused = false;
                    }
                    // Quit game.
                    if (i == 1 && text[0] == 'q')
                    {
                        XCloseDisplay(display);
                        exit(0);
                    }
                    // Arrow keys.
                    switch(key)
                    {
                        // Move left.
                        case XK_Left:
                        {
                            paddleLeft = true;
                            break;
                        }
                        // Move right.
                        case XK_Right:
                        {
                            paddleRight = true;
                            break;
                        }
                    }
                    break;
                }
                case KeyRelease:
                {
                    KeySym key;
                    char text[BUFFER_SIZE];
                    int i = XLookupString((XKeyEvent*)&event, text, 10, &key, 0);
                    switch(key)
                    {
                        // Stop moving left.
                        case XK_Left:
                        {
                            paddleLeft = false;
                            break;
                        }
                        case XK_Right:
                        {
                            paddleRight = false;
                            break;
                        }
                    }
                    break;
                }
            }
        }
        // Get current time in microseconds.
        unsigned long end = now();

        // Get time increment for determining the distance increment.
        float deltaTime = (end - lastUpdate) / 1000000.0;

        // Deterimine if the game is won.
        if (alive && bricksRemaining <= 0 && !gameWon)
        {
            gameWon = true;
        }

        // Determine if the game logic should be executed.
        if (alive && bricksRemaining > 0 && !gameWon && !gamePaused && !showSplash)
        {
            // Determine if ball is in contact with vertical wall.
            if ( (ballX + BALL_DIAMETER / 2 >= SCREEN_WIDTH && ballDir.x > 0) 
                || (ballX - BALL_DIAMETER / 2 <= 0 && ballDir.x < 0) )
            {
                ballDir.x = -1*ballDir.x;
            }

            // Determine if ball is in contact if top wall.
            if ((ballY - BALL_DIAMETER / 2 <= 0) && (ballDir.y < 0))
            {
                ballDir.y = -1*ballDir.y;
            }

            // Determine if ball is in contact with the paddle.
            if ((ballY + BALL_DIAMETER/2 >= paddleY)
                && (ballY + BALL_DIAMETER / 2 <= paddleY + PADDLE_HEIGHT) 
                && (ballX + BALL_DIAMETER / 2 >= paddleX)
                && (ballX <= paddleX + paddleLength)
                && (ballDir.y > 0)) 
            {
                ballDir.y = -1*ballDir.y;
                score += paddleBouncePoints;
            }

            // Vertical brick break.
            for (int row = 0; row < NUM_OF_ROWS; row++)
            {
                for (int col = 0; col < NUM_OF_COLS; col++)
                {
                    if (brickArray[row][col] != DEAD)
                    {
                        if ((ballX >= col*BRICK_WIDTH)
                            && (ballX <= (col + 1)*BRICK_WIDTH)
                            && (ballY + BALL_DIAMETER / 2 >= row*BRICK_HEIGHT)
                            && (ballY < (row + 1)*BRICK_HEIGHT))
                        {
                            brickArray[row][col] = DEAD;
                            bricksRemaining--;
                            score += destroyBrickPoints;

                            ballDir.y = -1*ballDir.y;
                        }
                        else if ((ballX >= col*BRICK_WIDTH)
                                && (ballX <= (col + 1)*BRICK_WIDTH)
                                && (ballY - BALL_DIAMETER / 2 <= (row + 1)*BRICK_HEIGHT)
                                && (ballY > row*BRICK_HEIGHT))
                        {
                            brickArray[row][col] = DEAD;
                            bricksRemaining--;
                            score += destroyBrickPoints;

                            ballDir.y = -1*ballDir.y;
                        }
                    }
                }
            }

            // Horizontal brick break.
            for (int row = 0; row < NUM_OF_ROWS; row++)
            {
                for (int col = 0; col < NUM_OF_COLS; col++)
                {
                    if (brickArray[row][col] != DEAD)
                    {
                        if ((ballY >= row*BRICK_HEIGHT)
                            && (ballY <= (row + 1)*BRICK_HEIGHT)
                            && (ballX + BALL_DIAMETER / 2 >= col*BRICK_WIDTH)
                            && (ballX < (col + 1)*BRICK_WIDTH))
                        {
                            brickArray[row][col] = DEAD;
                            bricksRemaining--;
                            score += destroyBrickPoints;

                            ballDir.x = -1*ballDir.x;
                        }
                        else if ((ballY >= row*BRICK_HEIGHT)
                                && (ballY <= (row + 1)*BRICK_HEIGHT)
                                && (ballX - BALL_DIAMETER / 2 <= (col + 1)*BRICK_WIDTH)
                                && (ballX > col*BRICK_WIDTH))
                        {
                            brickArray[row][col] = DEAD;
                            bricksRemaining--;
                            score += destroyBrickPoints;

                            ballDir.x = -1*ballDir.x;
                        }
                    }
                }
            }

            // Update paddle position.
            if ( paddleLeft && paddleX >= 0)
            {
                paddleX -= paddleSpeed*deltaTime;
            }
            if (paddleRight && paddleX + paddleLength <= SCREEN_WIDTH)
            {
                paddleX += paddleSpeed*deltaTime;
            }

            // Update ball position.
            float ballXIncrement = ballDir.x*deltaTime;
            float ballYIncrement = ballDir.y*deltaTime;

            ballX += ballXIncrement;
            ballY += ballYIncrement;

            // Determine if the incremental ball movement ends
            // the game by touching the lower edge.
            if (ballY >= SCREEN_HEIGHT && !gameWon)
            {
                alive = false;
            }
        }

        lastUpdate = now();
        if (end - lastRepaint > 1000000/FPS )
        {
            Pixmap pixmap;
            pixmap = buffer;

		    XFontStruct * font;
		    font = XLoadQueryFont (display, "12x24");
			XSetFont (display, gc, font->fid);
			const int FONT_CHAR_LENGTH = 12;
			const int FONT_CHAR_HEIGHT = 24;

            XSetForeground( display, gc, BlackPixel( display, DefaultScreen(display) ) );
            XSetBackground( display, gc, BlackPixel( display, DefaultScreen(display) ) );

            XFillRectangle(display, pixmap, gc, 0, 0, SCREEN_WIDTH, WINDOW_HEIGHT);

            if (!showSplash)
            {
                XSetForeground( display, gc, WhitePixel( display, DefaultScreen(display) ) );
                XSetBackground( display, gc, BlackPixel( display, DefaultScreen(display) ) );

                // Draw game text.
                std::string scoreText("Score: " + std::to_string(score));
                std::string ballSpeedText("Ball Speed: " + std::to_string( (short) (ceil(ballSpeed*100)/100)));
                std::string paddleSpeedText("Paddle speed: " + std::to_string( (short) (ceil(paddleSpeed*100)/100)));
                std::string paddleLengthText("Paddle length: " + std::to_string(paddleLength));

                XDrawImageString(display, pixmap, gc,
                                SCREEN_WIDTH  / 6 + 75,
                                WINDOW_HEIGHT - STATS_OFFSET,
                                scoreText.c_str(),
                                scoreText.length());

                XDrawImageString(display, pixmap, gc,
                                2*SCREEN_WIDTH / 6 + 10,
                                WINDOW_HEIGHT - STATS_OFFSET,
                                ballSpeedText.c_str(),
                                ballSpeedText.length());

                XDrawImageString(display, pixmap, gc,
                                3*SCREEN_WIDTH / 6 - 15,
                                WINDOW_HEIGHT - STATS_OFFSET,
                                paddleSpeedText.c_str(),
                                paddleSpeedText.length());

                XDrawImageString(display, pixmap, gc,
                                4*SCREEN_WIDTH / 6 - 15,
                                WINDOW_HEIGHT - STATS_OFFSET,
                                paddleLengthText.c_str(),
                                paddleLengthText.length());

                // Draw paddle.
                XFillRectangle(display, pixmap, gc, 
                                paddleX, paddleY, paddleLength, PADDLE_HEIGHT);

                // Draw ball
                XFillArc(display, pixmap, gc,
                        ballX - BALL_DIAMETER / 2, ballY - BALL_DIAMETER / 2, 
                        BALL_DIAMETER, BALL_DIAMETER, 0*64, 360*64);

                // Draw bricks.GREEN, BLUE, YELLOW, PURPLE, ORANGE
                for (int row = 0; row < NUM_OF_ROWS; row++)
                {
                    for (int col = 0; col < NUM_OF_COLS; col++)
                    {
                        if (brickArray[row][col] != DEAD)
                        {
                            switch(brickArray[row][col])
                            {
                                case DEAD:
                                {
                                    XSetForeground(display, gc, BlackPixel( display, DefaultScreen(display) ) );
                                    break; 
                                }
                                case RED:
                                {
                                    XSetForeground(display, gc, red.pixel);
                                    break; 
                                }
                                case GREEN:
                                {
                                    XSetForeground(display, gc, green.pixel);
                                    break; 
                                }
                                case BLUE:
                                {
                                    XSetForeground(display, gc, blue.pixel);
                                    break; 
                                }
                                case YELLOW:
                                {
                                    XSetForeground(display, gc, yellow.pixel);
                                    break; 
                                }
                                case PURPLE:
                                {
                                    XSetForeground(display, gc, purple.pixel);
                                    break; 
                                }
                                case ORANGE:
                                {
                                    XSetForeground(display, gc, orange.pixel);
                                    break; 
                                }
                            }
                            XFillRectangle(display, pixmap, gc,
                                    col * BRICK_WIDTH, row * BRICK_HEIGHT,
                                    BRICK_WIDTH - 5, BRICK_HEIGHT - 5);
                        }
                    }
                }

            }

            if (alive == true && gameWon == true)
            {
                XSetForeground( display, gc, WhitePixel( display, DefaultScreen(display) ) );
                XSetBackground( display, gc, BlackPixel( display, DefaultScreen(display) ) );

                std::string winText1("Congratulations! Game complete.");
                std::string winText2("Press spacebar to play again.");

                XDrawImageString(display, pixmap, gc,
                                SCREEN_WIDTH / 2 - winText1.length()/2 * FONT_CHAR_LENGTH,
                                SCREEN_HEIGHT / 2 - FONT_CHAR_HEIGHT - 5,
                                winText1.c_str(),
                                winText1.length());

                XDrawImageString(display, pixmap, gc,
                                SCREEN_WIDTH / 2 - winText2.length()/2 * FONT_CHAR_LENGTH,
                                SCREEN_HEIGHT/2,
                                winText2.c_str(),
                                winText2.length());
            }

            if (alive == false && gameWon == false)
            {
                XSetForeground( display, gc, WhitePixel( display, DefaultScreen(display) ) );
                XSetBackground( display, gc, BlackPixel( display, DefaultScreen(display) ) );

                std::string loseText1("Game Over! You lose.");
                std::string loseText2("Press spacebar to play again.");

                XDrawImageString(display, pixmap, gc,
                                SCREEN_WIDTH / 2 - loseText1.length()/2 * FONT_CHAR_LENGTH,
                                SCREEN_HEIGHT / 2 - FONT_CHAR_HEIGHT - 5,
                                loseText1.c_str(),
                                loseText1.length());

                XDrawImageString(display, pixmap, gc,
                                SCREEN_WIDTH / 2 - loseText2.length()/2 * FONT_CHAR_LENGTH,
                                SCREEN_HEIGHT / 2,
                                loseText2.c_str(),
                                loseText2.length());
            }

            if (gamePaused == true && alive == true && !showSplash)
            {
                XSetForeground( display, gc, WhitePixel( display, DefaultScreen(display) ) );
                XSetBackground( display, gc, BlackPixel( display, DefaultScreen(display) ) );

                std::string pauseText("Game paused. Press spacebar to continue.");

                XDrawImageString(display, pixmap, gc,
                                SCREEN_WIDTH / 2 - pauseText.length()/2 * FONT_CHAR_LENGTH,
                                SCREEN_HEIGHT / 2,
                                pauseText.c_str(),
                                pauseText.length());
            }

            if (showSplash)
            {
                XSetForeground( display, gc, WhitePixel( display, DefaultScreen(display) ) );
                XSetBackground( display, gc, BlackPixel( display, DefaultScreen(display) ) );

                std::string titleText("Breakout!");
                std::string creatorText("Created by: Christopher Mannes");
                std::string instructionText1("Press left and right arrow keys to move the paddle.");
                std::string instructionText2("Press p to pause, q to quit, and spacebar to start.");

                XDrawImageString(display, pixmap, gc,
                                SCREEN_WIDTH / 2 - (titleText.length()/2 * FONT_CHAR_LENGTH ),
                                SCREEN_HEIGHT/2 - FONT_CHAR_HEIGHT - 5,
                                titleText.c_str(), titleText.length());

                XDrawImageString(display, pixmap, gc,
                                SCREEN_WIDTH / 2 - (creatorText.length()/2 * FONT_CHAR_LENGTH ),
                                SCREEN_HEIGHT / 2,
                                creatorText.c_str(), creatorText.length());

                XDrawImageString(display, pixmap, gc,
                                SCREEN_WIDTH / 2 - (instructionText1.length()/2 * FONT_CHAR_LENGTH ),
                                SCREEN_HEIGHT / 2 + FONT_CHAR_HEIGHT + 5,
                                instructionText1.c_str(), instructionText1.length());

                XDrawImageString(display, pixmap, gc,
                                SCREEN_WIDTH / 2 - (instructionText2.length()/2 * FONT_CHAR_LENGTH ),
                                SCREEN_HEIGHT / 2 + 2*(FONT_CHAR_HEIGHT + 5),
                                instructionText2.c_str(), instructionText2.length());	
            }
            // copy buffer to window
            XCopyArea(display, pixmap, window, gc, 
                    0, 0, SCREEN_WIDTH, WINDOW_HEIGHT, 0, 0);

            XFlush( display );

            lastRepaint = now(); // remember when the paint happened   
        }
		// IMPORTANT: sleep for a bit to let other processes work.
        if (XPending(display) == 0)
        {
            usleep(1000000 / FPS - (now() - lastRepaint));
        }
    }
    XCloseDisplay(display);

    return(0);
}


