# ASM

# Convert the value in r11 to a valid speed
# Normalize speed to be one of the following values for gameplay:
# 0 km/h = 0000
# 2 km/h = 4000
# 5 km/h = 41C8
# 50 km/h = 4248
# 75 km/h = 4296
# 100 km/h = 42C8
# 150 km/h = 4316
# 200 km/h = 4348
# 250 km/h = 437A
# 300 km/h = 4396
# 400 km/h = 43C8
# 500 km/h = 43FA
# 700 km/h = 442F
# 1000 km/h = 447A
# r11 contains the input value (0-255)
# Output speed value will be stored back in r11

li      r11, 0x4396      # 300 km/h
b       set_base_speed

# Use this value to set speeds (technically of the player and CPUs, but CPUs have a speed cap)
set_base_speed:
    # ~~~~~~~~~~~~#
    # Inject > 80571CA4#
    # ~~~~~~~~~~~#

    # Set base speed
    mr r8, r11
    stw r8, 0x0018 (r3)
    lfs f1, 0x0018 (r3)

li      r11, 0x4396      # 300 km/h
b       set_max_speed

set_max_speed:
    # ~~~~~~~~~~~~#
    # Inject > 80571CF4#
    # ~~~~~~~~~~~#

    # Set max speed (same as base speed)
    mr r12, r11
    stw r12, 0x1134 (r9)
    lfs f0, 0x1134 (r9)