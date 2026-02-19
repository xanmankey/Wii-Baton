# C-stick X is used as a placeholder for speed and is set by the controller code
# Read the current value of C-Stick X (0x8034C202) into f31 and zero
lis     r12, 0x8034        # Load upper 16 bits into r12 (0x80340000)
ori     r12, r12, 0xC202   # Add lower 16 bits (now r12 = 0x8034C202)
li      r11, 0             # Clear r11
lbz     r11, 0(r12)        # Load 8-bit value into r11 (zero-extended)

# Convert the C-stick value (0-255) in r11 to a valid speed:
# 0 km/h = 0000, 5 km/h = 41C8, 50 km/h = 4248, 75 km/h = 4296, 100 km/h = 42C8...
# Output speed value will be stored back in r11

cmpwi   r11, 0          
beq     set_base_speed_0      # if r11 = 0, set to 0 km/h

cmpwi   r11, 51
blt     set_base_speed_10     # 1-50 -> 10 km/h

cmpwi   r11, 77
blt     set_base_speed_25     # 51-76 -> 25 km/h

...

set_base_speed_0:
    li  r11, 0  
    lis  r11, 0x0000      # 0 km/h
    b   set_base_speed

set_base_speed_10:
    li  r11, 0
    lis  r11, 0x4120      # 10 km/h
    b   set_base_speed

set_base_speed_25:
    li  r11, 0
    lis  r11, 0x41C8      # 25 km/h
    b   set_base_speed

...

# Use this value to set speeds (technically of the player and CPUs, but CPUs have a speed cap)
set_base_speed:
    # Inject > 80571CA4#

    # Set base speed
    mr r8, r11
    stw r8, 0x0018 (r3)
    lfs f1, 0x0018 (r3)

# Then follow the same process for max speed and inject at 80571CF4#