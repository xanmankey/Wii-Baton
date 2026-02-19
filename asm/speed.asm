# ASM

# 0x8034C202 is the address of C-Stick X which isn't used in mario kart wii gameplay
# You may want to verify that this is the case for your version of Dolphin and your copy of the game though
# We can load this value into memory and then adapt it to whatever game/range we want

# Read the current value of accelerometer X into f31 and zero
# To load from an address, first store the address in another register
lis     r12, 0x8034        # Load upper 16 bits into r12 (0x80340000)
ori     r12, r12, 0xC202   # Add lower 16 bits (now r12 = 0x8034C202)
li      r11, 0             # Clear r11
lbz     r11, 0(r12)        # Load 8-bit value into r11 (zero-extended)

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

cmpwi   r11, 0          
beq     set_base_speed_0      # if r11 = 0, set to 0 km/h

cmpwi   r11, 51
blt     set_base_speed_10     # 1-50 -> 10 km/h

cmpwi   r11, 77
blt     set_base_speed_25     # 51-76 -> 25 km/h

cmpwi   r11, 128
blt     set_base_speed_50    # 77-127 -> 50 km/h

cmpwi   r11, 192
blt     set_base_speed_75    # 128-191 -> 75 km/h

cmpwi   r11, 205
blt     set_base_speed_100   # 192-204 -> 100 km/h

cmpwi   r11, 230
blt     set_base_speed_150    # 205-229 -> 150 km/h

cmpwi   r11, 245
blt     set_base_speed_200    # 230-244 -> 200 km/h

cmpwi   r11, 256
blt     set_base_speed_250    # 245-255 -> 250 km/h

# else
li      r11, 0x4396      # 300 km/h
b       set_base_speed

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

set_base_speed_50:
    li  r11, 0
    lis  r11, 0x4248      # 50 km/h
    b   set_base_speed

set_base_speed_75:
    li  r11, 0
    lis  r11, 0x4296      # 75 km/h
    b   set_base_speed

set_base_speed_100:
    li  r11, 0
    lis  r11, 0x42C8      # 100 km/h
    b   set_base_speed

set_base_speed_150:
    li  r11, 0
    lis  r11, 0x4316      # 150 km/h
    b   set_base_speed

set_base_speed_200:
    li  r11, 0
    lis  r11, 0x4348      # 200 km/h
    b   set_base_speed

set_base_speed_250:
    li  r11, 0
    lis  r11, 0x457A      # 250 km/h
    b   set_base_speed


# Use this value to set speeds (technically of the player and CPUs, but CPUs have a speed cap)
set_base_speed:
    # ~~~~~~~~~~~~#
    # Inject > 80571CA4#
    # ~~~~~~~~~~~#

    # Set base speed
    mr r8, r11
    stw r8, 0x0018 (r3)
    lfs f1, 0x0018 (r3)

lis     r12, 0x8034        # Load upper 16 bits into r12 (0x80340000)
ori     r12, r12, 0xC202   # Add lower 16 bits (now r12 = 0x8034C202)
li      r11, 0             # Clear r11
lbz     r11, 0(r12)        # Load 8-bit value into r11 (zero-extended)

cmpwi   r11, 0          
beq     set_max_speed_0      # if r11 = 0, set to 0 km/h

cmpwi   r11, 51
blt     set_max_speed_10     # 1-50 -> 10 km/h

cmpwi   r11, 77
blt     set_max_speed_25     # 51-76 -> 25 km/h

cmpwi   r11, 128
blt     set_max_speed_50    # 77-127 -> 50 km/h

cmpwi   r11, 192
blt     set_max_speed_75    # 128-191 -> 75 km/h

cmpwi   r11, 205
blt     set_max_speed_100   # 192-204 -> 100 km/h

cmpwi   r11, 230
blt     set_max_speed_150    # 205-229 -> 150 km/h

cmpwi   r11, 245
blt     set_max_speed_200    # 230-244 -> 200 km/h

cmpwi   r11, 256
blt     set_max_speed_250    # 245-255 -> 250 km/h

# else
li      r11, 0x4396      # 300 km/h
b       set_max_speed

set_max_speed_0:
    li  r11, 0  
    lis  r11, 0x0000      # 0 km/h
    b   set_max_speed

set_max_speed_10:
    li  r11, 0
    lis  r11, 0x4120      # 10 km/h
    b   set_max_speed

set_max_speed_25:
    li  r11, 0
    lis  r11, 0x41C8      # 25 km/h
    b   set_max_speed

set_max_speed_50:
    li  r11, 0
    lis  r11, 0x4248      # 50 km/h
    b   set_max_speed

set_max_speed_75:
    li  r11, 0
    lis  r11, 0x4296      # 75 km/h
    b   set_max_speed

set_max_speed_100:
    li  r11, 0
    lis  r11, 0x42C8      # 100 km/h
    b   set_max_speed

set_max_speed_150:
    li  r11, 0
    lis  r11, 0x4316      # 150 km/h
    b   set_max_speed

set_max_speed_200:
    li  r11, 0
    lis  r11, 0x4348      # 200 km/h
    b   set_max_speed

set_max_speed_250:
    li  r11, 0
    # lis  r11, 0x437A      # 250 km/h
    lis r11, 0x457A      # 250 km/h
    b   set_max_speed


set_max_speed:
    # ~~~~~~~~~~~~#
    # Inject > 80571CF4#
    # ~~~~~~~~~~~#

    # Set max speed (same as base speed)
    mr r12, r11
    stw r12, 0x1134 (r9)
    lfs f0, 0x1134 (r9)