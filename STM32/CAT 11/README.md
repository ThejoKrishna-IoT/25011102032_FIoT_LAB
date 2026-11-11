# CAT11 — NUCLEO-F401RE onboard LED and button

PA5 is LD2 (onboard green LED). PC13 is B1 (user button, active low).
Task 1: LD2 GPIO bring-up; Task 2: button press toggles and remembers LED state; Task 3: short/long classification with 20 ms debounce and HAL_GetTick timing.
Short press toggles LED. Long press toggles a continuous blink mode. No blocking delay is used for press classification.
