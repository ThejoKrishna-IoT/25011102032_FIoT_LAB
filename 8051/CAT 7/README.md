# CAT7_LCD_Keypad_Activity

# CAT2–CAT7 AT89S52 starter project
Board assumptions: AT89S52 at 11.0592 MHz, EA tied to +5 V. Active-low LEDs on P1. P0 requires external pull-ups. Verify all pins against your circuit before wiring.
CAT3: button P3.2 active-low; LEDs P1.0–P1.3 active-low.
CAT4: keypad P2, LEDs P1.0–P1.3, common-anode 7-segment on P0. Keypad positions are returned as 0–15; adjust the mapping to match your printed keypad legend.
CAT5: P1.0 is timer output; P3.2 selects one of five approximate timer periods. This is not calibrated hardware PWM; verify with an oscilloscope and calculate timer reloads for your clock. Optional stepper motor must use ULN2003/driver, never direct MCU pins.
CAT6: UART P3.1 TXD/P3.0 RXD, 9600 8-N-1 at 11.0592 MHz; share GND with USB-TTL and check voltage compatibility.
CAT7: LCD RS=P1.0, E=P1.1, D4–D7=P1.4–P1.7; keypad on P2. Sample scan recognizes 1/2/3/A; adapt it to your physical key map. Movement logic expects 2/8/4/6.
The PDFs are diagram-heavy and do not expose every pin/polarity in extracted text, so these are documented starter assumptions, not confirmed reproductions of every diagram. The `.uvproj` files are templates. No `.hex` files are included because these projects have not been compiled or tested. If STARTUP.A51 conflicts with Keil's C51 runtime startup, remove it and use the installed runtime startup.
