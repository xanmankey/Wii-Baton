# Records the acceleration and rotational data of the gamepad
# Each half rotation is marked when acceleration is near 0 (during the turnaround)

import pygame
import os
import sys
import csv
import time

# Add the parent directory to the system path
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))


def main():
    pygame.init()
    pygame.joystick.init()

    # Check if a joystick is connected
    if pygame.joystick.get_count() == 0:
        print("No joystick connected.")
        return

    # Initialize the first joystick
    joystick = pygame.joystick.Joystick(0)
    joystick.init()
    print(f"Joystick initialized: {joystick.get_name()}")

    # Open a CSV file to save the data
    output_file = os.path.join(
        os.path.dirname(os.path.abspath(__file__)), "gamepad_data.csv"
    )
    with open(output_file, mode="w", newline="") as file:
        writer = csv.writer(file)
        writer.writerow(
            [
                "Acceleration_X",
                "Acceleration_Y",
                "Acceleration_Z",
                "Rotation_X",
                "Rotation_Y",
                "Rotation_Z",
            ]
        )

        try:
            time.sleep(9)
            print("1 second to start recording...")
            time.sleep(1)
            while True:
                pygame.event.pump()  # Process events

                # Get acceleration and rotational data
                # Assuming axes 0, 1, 2 are for acceleration and 3, 4, 5 are for rotation
                acceleration = [joystick.get_axis(i) for i in range(3)]
                rotation = [joystick.get_axis(i) for i in range(3, 6)]

                print(f"Acceleration: {acceleration}, Rotation: {rotation}")

                # Write the data to the CSV file
                writer.writerow(
                    [
                        acceleration[0],
                        acceleration[1],
                        acceleration[2],
                        rotation[0],
                        rotation[1],
                        rotation[2],
                    ]
                )

                # Add logic to detect half rotations based on acceleration near 0
                # Example: if abs(acceleration[0]) < threshold: ...

                pygame.time.wait(100)  # Delay to reduce CPU usage
        except KeyboardInterrupt:
            print("Exiting...")
        finally:
            joystick.quit()
            pygame.quit()


if __name__ == "__main__":
    main()
