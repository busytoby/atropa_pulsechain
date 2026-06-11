#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <termios.h>
#include <poll.h>
#include <sys/ioctl.h>

// Terminal Screen Dimensions (Retro 4:3 Grid)
#define GRID_WIDTH 80
#define GRID_HEIGHT 24

typedef struct {
    float x, y, z;
} Point3D;

typedef struct {
    int x, y;
} Point2D;

// Projection from 3D to 2D screen coordinates
Point2D project(Point3D p) {
    Point2D out;
    // Vanishing point at center of the field
    float fov = 40.0f;
    float depth = p.z + 100.0f;
    if (depth <= 1.0f) depth = 1.0f;

    out.x = (int)((p.x * fov) / depth + (GRID_WIDTH / 2));
    out.y = (int)((-p.y * fov * 0.5f) / depth + (GRID_HEIGHT / 2) + 2); // Aspect ratio correction
    return out;
}

// Global Game State
float ballX = 0.0f, ballY = 40.0f, ballZ = 250.0f;
float ballVX = 0.0f, ballVY = 0.0f, ballVZ = 0.0f;
bool ballActive = false;

float batSwingAngle = 0.0f;
bool batSwinging = false;

int scoreHome = 2;
int scoreVisitor = 0;
int Inning = 5;
int strikes = 0;
int balls = 1;
int outs = 2;

bool showOutline = false;
char statusText[128] = "System Initialized. Awaiting Pitch (Press ENTER)...";

// Helper to set raw terminal mode
struct termios orig_termios;
void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    printf("\e[?25h\e[m"); // Show cursor, reset colors
}

void enableRawMode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode);

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    raw.c_iflag &= ~(IXON | ICRNL);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    printf("\e[?25l"); // Hide cursor
}

// Simple color definitions
#define COLOR_RESET   "\e[0m"
#define COLOR_FIELD   "\e[48;5;28m\e[38;5;15m" // Green
#define COLOR_DIRT    "\e[48;5;94m"            // Brown
#define COLOR_PITCHER "\e[38;5;15m\e[48;5;250m" // Gray / White
#define COLOR_BATTER  "\e[38;5;15m\e[48;5;21m"  // Blue
#define COLOR_BAT     "\e[38;5;214m"           // Wood Yellow
#define COLOR_CATCHER "\e[38;5;15m\e[48;5;160m" // Red
#define COLOR_BALL    "\e[38;5;15m\e[5m"        // White blinking
#define COLOR_HUD     "\e[48;5;235m\e[38;5;253m" // Scoreboard Dark slate

// Raster structures
char charGrid[GRID_HEIGHT][GRID_WIDTH];
uint8_t colorGrid[GRID_HEIGHT][GRID_WIDTH]; // 0: Field, 1: Dirt, 2: Pitcher, 3: Batter, 4: Bat, 5: Catcher, 6: Ball

void clearGrids() {
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            charGrid[y][x] = ' ';
            colorGrid[y][x] = 0; // Field
        }
    }
}

void drawLine3D(Point3D p1, Point3D p2, uint8_t colorCode, char c) {
    Point2D sc1 = project(p1);
    Point2D sc2 = project(p2);
    
    int dx = abs(sc2.x - sc1.x), sx = sc1.x < sc2.x ? 1 : -1;
    int dy = -abs(sc2.y - sc1.y), sy = sc1.y < sc2.y ? 1 : -1;
    int err = dx + dy, e2;

    int x = sc1.x, y = sc1.y;
    while (1) {
        if (x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT) {
            charGrid[y][x] = c;
            colorGrid[y][x] = colorCode;
        }
        if (x == sc2.x && y == sc2.y) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x += sx; }
        if (e2 <= dx) { err += dx; y += sy; }
    }
}

void drawCylinder3D(Point3D base, float radius, float height, uint8_t colorCode, char c) {
    for (float dy = -height/2; dy <= height/2; dy += 4.0f) {
        for (float dx = -radius; dx <= radius; dx += 2.0f) {
            Point3D pt = {base.x + dx, base.y + dy, base.z};
            Point2D sc = project(pt);
            if (sc.x >= 0 && sc.x < GRID_WIDTH && sc.y >= 0 && sc.y < GRID_HEIGHT) {
                charGrid[sc.y][sc.x] = c;
                colorGrid[sc.y][sc.x] = colorCode;
            }
        }
    }
}

// Keyboard input checks
int get_key() {
    struct pollfd pfd = { STDIN_FILENO, POLLIN, 0 };
    if (poll(&pfd, 1, 0) > 0) {
        char c;
        if (read(STDIN_FILENO, &c, 1) > 0) return c;
    }
    return -1;
}

// Canny Edge / Sobel contour simulation on character grids
void renderSobelGrid() {
    char tempGrid[GRID_HEIGHT][GRID_WIDTH];
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            tempGrid[y][x] = ' ';
        }
    }

    for (int y = 1; y < GRID_HEIGHT - 1; y++) {
        for (int x = 1; x < GRID_WIDTH - 1; x++) {
            // Find changes in colors or characters to determine contours
            if (colorGrid[y][x] != 0) {
                bool edge = false;
                for (int ky = -1; ky <= 1; ky++) {
                    for (int kx = -1; kx <= 1; kx++) {
                        if (colorGrid[y + ky][x + kx] != colorGrid[y][x]) {
                            edge = true;
                        }
                    }
                }
                if (edge) {
                    tempGrid[y][x] = '#'; // White outline border
                }
            }
        }
    }

    // Overlay Sobel output
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            charGrid[y][x] = tempGrid[y][x];
            // Highlight outlines in pure white on black background
            if (charGrid[y][x] == '#') {
                colorGrid[y][x] = 7; // Sobel White
            } else {
                colorGrid[y][x] = 8; // Sobel Black
            }
        }
    }
}

void updatePhysics() {
    // 1. Ball flight physics
    if (ballActive) {
        ballX += ballVX;
        ballY += ballVY;
        ballZ += ballVZ;

        // Curve ball gravity effect
        ballVY -= 0.5f;

        // Check if ball reaches catcher/batter zone
        if (ballZ <= -120.0f) {
            ballActive = false;
            // Determine result
            if (batSwinging && fabs(batSwingAngle - 1.2f) < 0.5f && fabs(ballX) < 20.0f && fabs(ballY - 30.0f) < 20.0f) {
                // IT'S A HIT!
                scoreHome++;
                strikes = 0;
                balls = 0;
                sprintf(statusText, "💥 CRACK! A SOLID HIT INTO DEEP LEFT FIELD! HOME RUN! (Press ENTER to Pitch)");
            } else {
                // Strike or ball
                if (fabs(ballX) < 15.0f && ballY > -10.0f && ballY < 50.0f) {
                    strikes++;
                    if (strikes >= 3) {
                        strikes = 0;
                        outs++;
                        if (outs >= 3) { outs = 0; Inning++; }
                        sprintf(statusText, "❌ STRIKE THREE! BATTER IS OUT! (Press ENTER to Pitch)");
                    } else {
                        sprintf(statusText, "⚾ STRIKE! Right down the middle! (Press ENTER to Pitch)");
                    }
                } else {
                    balls++;
                    if (balls >= 4) {
                        balls = 0;
                        sprintf(statusText, "🚶 WALK! Batter takes first base. (Press ENTER to Pitch)");
                    } else {
                        sprintf(statusText, "🟢 BALL! Outside the zone. (Press ENTER to Pitch)");
                    }
                }
            }
        }
    }

    // 2. Batter swing physics
    if (batSwinging) {
        batSwingAngle += 0.25f;
        if (batSwingAngle >= 3.14f) {
            batSwinging = false;
            batSwingAngle = 0.0f;
        }
    }
}

void drawScreen() {
    // Escape to move cursor to top-left
    printf("\e[H");

    // 1. Draw Scoreboard
    printf(COLOR_HUD "================================================================================" COLOR_RESET "\n");
    printf(COLOR_HUD "  C64 HARDBALL  |  VISITORS: %-2d  |  HOME: %-2d  |  INNING: %-2d  |  S:%d  B:%d  O:%d  " COLOR_RESET "\n", 
           scoreVisitor, scoreHome, Inning, strikes, balls, outs);
    printf(COLOR_HUD "================================================================================" COLOR_RESET "\n");

    // 2. Clear grids & draw volumetric 3D components
    clearGrids();

    // Perspective Foul Lines
    drawLine3D((Point3D){-120.0f, -50.0f, -150.0f}, (Point3D){-5.0f, -50.0f, 600.0f}, 1, '.');
    drawLine3D((Point3D){120.0f, -50.0f, -150.0f}, (Point3D){5.0f, -50.0f, 600.0f}, 1, '.');

    // Volumetric Pitcher
    drawCylinder3D((Point3D){0.0f, 20.0f, 220.0f}, 10.0f, 40.0f, 2, 'P');

    // Volumetric Batter & bat
    drawCylinder3D((Point3D){-45.0f, 15.0f, -60.0f}, 14.0f, 60.0f, 3, 'B');
    if (batSwinging) {
        float batX = -45.0f + cosf(batSwingAngle) * 45.0f;
        float batY = 25.0f + sinf(batSwingAngle) * 15.0f;
        drawLine3D((Point3D){-45.0f, 25.0f, -60.0f}, (Point3D){batX, batY, -60.0f}, 4, '/');
    } else {
        drawLine3D((Point3D){-45.0f, 25.0f, -60.0f}, (Point3D){-25.0f, 65.0f, -60.0f}, 4, '/');
    }

    // Volumetric Catcher
    drawCylinder3D((Point3D){0.0f, -20.0f, -100.0f}, 16.0f, 40.0f, 5, 'C');

    // Ball
    if (ballActive) {
        Point2D sc = project((Point3D){ballX, ballY, ballZ});
        if (sc.x >= 0 && sc.x < GRID_WIDTH && sc.y >= 0 && sc.y < GRID_HEIGHT) {
            charGrid[sc.y][sc.x] = 'o';
            colorGrid[sc.y][sc.x] = 6;
        }
    }

    // Apply Sobel outlines if toggled
    if (showOutline) {
        renderSobelGrid();
    }

    // Print Grid to Terminal
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            uint8_t cCode = colorGrid[y][x];
            char c = charGrid[y][x];

            if (cCode == 0) printf(COLOR_FIELD "%c" COLOR_RESET, c);
            else if (cCode == 1) printf(COLOR_DIRT "%c" COLOR_RESET, c);
            else if (cCode == 2) printf(COLOR_PITCHER "%c" COLOR_RESET, c);
            else if (cCode == 3) printf(COLOR_BATTER "%c" COLOR_RESET, c);
            else if (cCode == 4) printf(COLOR_BAT "%c" COLOR_RESET, c);
            else if (cCode == 5) printf(COLOR_CATCHER "%c" COLOR_RESET, c);
            else if (cCode == 6) printf(COLOR_BALL "%c" COLOR_RESET, c);
            else if (cCode == 7) printf("\e[48;5;16m\e[38;5;15m%c\e[0m", c); // Sobel white outline
            else if (cCode == 8) printf("\e[48;5;16m \e[0m");               // Sobel black background
        }
        printf("\n");
    }

    // 3. Print HUD Status info
    printf("\e[48;5;236m\e[38;5;253m MESSAGE: %-70s \e[0m\n", statusText);
    printf("\e[2m [Controls: ENTER=Pitch | SPACE=Swing Bat | O=Toggle Sobel outline | ESC=Exit] \e[0m\n");
}

int main() {
    enableRawMode();
    printf("\e[2J\e[H"); // Clear screen

    bool running = true;
    while (running) {
        int key = get_key();
        if (key == 27) { // ESC
            running = false;
        } else if (key == 13 || key == 'p' || key == 'P') { // ENTER
            if (!ballActive) {
                ballActive = true;
                ballX = 0.0f;
                ballY = 35.0f;
                ballZ = 220.0f;
                
                // Randomize pitch trajectory slightly
                ballVX = (float)(rand() % 20 - 10) / 10.0f;
                ballVY = 5.0f;
                ballVZ = -8.5f; // pitch speed
                sprintf(statusText, "⚾ PITCH IS IN! Watch the ball trajectory...");
            }
        } else if (key == ' ' || key == 's' || key == 'S') { // SPACE
            if (!batSwinging) {
                batSwinging = true;
                batSwingAngle = 0.0f;
            }
        } else if (key == 'o' || key == 'O') { // O key
            showOutline = !showOutline;
            sprintf(statusText, "System: Sobel Canny-edge outline mode set to %s.", showOutline ? "ACTIVE" : "INACTIVE");
        }

        updatePhysics();
        drawScreen();

        usleep(33000); // ~30 FPS terminal tick rate
    }

    disableRawMode();
    printf("\n[SUCCESS] Interactive Hardball Terminal Simulator closed safely.\n");
    return 0;
}
