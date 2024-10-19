#include <iostream>
#include <cmath>
#include <string>
#include <thread>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

const int screen_width = 80;
const int screen_height = 24;
const float theta_spacing = 0.07;
const float phi_spacing = 0.02;

const float R1 = 1;
const float R2 = 2;
const float K2 = 5;

void get_terminal_size(int& width, int& height) {
#ifdef _WIN32
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
	height = csbi.srWindow.Bottom - csbi.swWindow.Top + 1;
#else
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	width = w.ws_col;
	height = w.ws_row;
#endif
}

void render_frame(float A, float B, int screen_width, int screen_height) {
	const float scale = 0.2f;
	const float K1 = scale * screen_width * K2 * 3 / (8 * (R1 + R2));

    // Initialize arrays
    std::vector<std::vector<char>> output(screen_height, std::vector<char>(screen_width, ' '));
    std::vector<std::vector<float>> zbuffer(screen_height, std::vector<float>(screen_width, 0));

    // Generate torus
    for (float theta = 0; theta < 2 * M_PI; theta += theta_spacing) {
        for (float phi = 0; phi < 2 * M_PI; phi += phi_spacing) {
            float cosA = cos(A), sinA = sin(A);
            float cosB = cos(B), sinB = sin(B);
            float costheta = cos(theta), sintheta = sin(theta);
            float cosphi = cos(phi), sinphi = sin(phi);

            float circlex = R2 + R1 * costheta;
            float circley = R1 * sintheta;

            float x = circlex * (cosB * cosphi + sinA * sinB * sinphi) - circley * cosA * sinB;
            float y = circlex * (sinB * cosphi - sinA * cosB * sinphi) + circley * cosA * cosB;
            float z = K2 + cosA * circlex * sinphi + circley * sinA;
            float ooz = 1 / z;

            int xp = static_cast<int>(screen_width / 2 + K1 * ooz * x);
            int yp = static_cast<int>(screen_height / 2 - K1 * ooz * y);

            float L = cosphi * costheta * sinB - cosA * costheta * sinphi -
                      sinA * sintheta + cosB * (cosA * sintheta - costheta * sinA * sinphi);

            if (xp >= 0 && xp < screen_width && yp >= 0 && yp < screen_height) {
	            if (L > 0) {
	                if (ooz > zbuffer[yp][xp]) {
	                    zbuffer[yp][xp] = ooz;
	                    int luminance_index = static_cast<int>(L * 8);
	                    luminance_index = std::max(0, std::min(luminance_index, 11));
	                    output[yp][xp] = ".,-~:;=!*#$@"[luminance_index];
	                }
	            }
	        }
        }
    }

    // Display the frame
    for (int i = 0; i < screen_height; i++) {
        for (int j = 0; j < screen_width; j++) {
            std::cout << output[i][j];
        }
        std::cout << std::endl;
    }
}

int main() {
    float A = 0, B = 0;
    int width, height;

    while (true) {
    	get_terminal_size(width, height);
        render_frame(A, B, width, height);
        A += 0.04;
        B += 0.02;

        // Clear the console (works on most Unix-like systems)
#ifdef _WIN32
        system("cls");
#else
        std::cout << "\x1B[2J\x1B[H";
#endif
        
        // Sleep for a short duration to control the frame rate
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    return 0;
}