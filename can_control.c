#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>

#define CAN_INTERFACE "can0"  // Adjust the interface name as needed

// Function to send a CAN message using SocketCAN
void send_can_message(int soc, int can_id, uint8_t data[8]) {
    struct can_frame frame;

    frame.can_id = can_id;
    frame.can_dlc = 8;

    for (int i = 0; i < 8; ++i) {
        frame.data[i] = data[i];
    }

    write(soc, &frame, sizeof(frame));
    usleep(1000000);  // Delay 1 second
}

int main() {
    int soc;
    struct sockaddr_can addr;

    // Create a socket
    soc = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (soc < 0) {
        perror("Socket creation failed");
        return -1;
    }

    // Set up the CAN interface
    strcpy(addr.ifr_name, CAN_INTERFACE);
    ioctl(soc, SIOCGIFINDEX, &addr);
    addr.can_family = AF_CAN;
    bind(soc, (struct sockaddr*)&addr, sizeof(addr));

    // Command 1: Initialize the driver
    send_can_message(soc, 0x1555AAB0, (uint8_t[8]){0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});

    // Command 2: Set absolute position control mode
    send_can_message(soc, 0x1555AAB1, (uint8_t[8]){0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00});

    // Command 3: Set PI GAINS
    send_can_message(soc, 0x1555AAB2, (uint8_t[8]){0x00, 0x00, 0x00, 0x0A, 0x32, 0x96, 0x00, 0x00});

    // Command 4: Set acceleration/deceleration values to 500rpm/sec (Maximum)
    send_can_message(soc, 0x1555AAB3, (uint8_t[8]){0x01, 0xF4, 0x01, 0xF4, 0x00, 0x00, 0x00, 0x00});

    // Counter for the loop
    int loop_counter = 0;

    // Loop commands 5a to 6b with DELAYS to sweep between the two targets until stopped by the user
    while (1) {
        // Loop between 5a and 6b

        // Command 5a: Set absolute position and speed set points -> target pos = 20deg(0.61revs), speed = 800rpm
        send_can_message(soc, 0x1555AAB5, (uint8_t[8]){0x00, 0x00, 0x00, 0x3D, 0x1F, 0x40, 0x00, 0x00});

        // Command 5b: To run motor after setting targets
        send_can_message(soc, 0x1555AAB6, (uint8_t[8]){0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});

        // Command 6a: Set absolute position and speed set points -> target pos = -20deg(-0.61revs), speed = 800rpm
        send_can_message(soc, 0x1555AAB5, (uint8_t[8]){0x80, 0x00, 0x00, 0x3D, 0x1F, 0x40, 0x00, 0x00});

        // Command 6b: To run motor after setting targets
        send_can_message(soc, 0x1555AAB6, (uint8_t[8]){0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});

        // Increment the loop counter
        loop_counter++;

        // Add your stopping condition here
        if (loop_counter >= 10) {
            // Break the loop if the counter reaches a certain value (adjust as needed)
            break;
        }
    }

    // Close the socket
    close(soc);

    return 0;
}
