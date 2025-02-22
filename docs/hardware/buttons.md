# Buttons

## Button Press Detection Mechanism

The button press detection mechanism in the 3-key project involves several key components and processes to ensure accurate detection of both short and long presses. This document describes the overall mechanism, including debouncing and long press detection.

### Short Press Detection

When a button is pressed, an interrupt is triggered, and the `gpio_callback` function is called. This function checks if the button is already debouncing or pending handling. If not, it starts a debounce timer to filter out any noise or false triggers.

```plantuml
@startuml
participant User
participant Button
participant GPIO
participant Buttons
participant Timer

User -> Button: Press
Button -> GPIO: Trigger Interrupt
GPIO -> Buttons: gpio_callback()
Buttons -> Timer: Start debounce_timer
Timer -> Buttons: debounce_timer_callback()
Buttons -> GPIO: Check button state
alt Button still pressed
    Buttons -> Timer: Start long_press_timer
else Button released
    Buttons -> Buttons: Set is_pending_handle
end
@enduml
```

### Debouncing

Debouncing is the process of filtering out noise or false triggers that can occur when a button is pressed or released. The debounce timer ensures that the button state is stable before considering it as a valid press.

```plantuml
@startuml
participant Button
participant GPIO
participant Timer
participant Buttons

Button -> GPIO: Trigger Interrupt
GPIO -> Buttons: gpio_callback()
Buttons -> Timer: Start debounce_timer
Timer -> Buttons: debounce_timer_callback()
alt Button still pressed
    Buttons -> Timer: Start long_press_timer
else Button released
    Buttons -> Buttons: Set is_pending_handle
end
@enduml
```

### Long Press Detection

Long press detection involves starting a long press timer after the debounce timer confirms a valid press. The long press timer periodically checks if the button is still pressed. If the button remains pressed for a duration longer than the configured long press delay, it is considered a long press.

```plantuml
@startuml
participant Button
participant GPIO
participant Timer
participant Buttons

Button -> GPIO: Trigger Interrupt
GPIO -> Buttons: gpio_callback()
Buttons -> Timer: Start debounce_timer
Timer -> Buttons: debounce_timer_callback()
alt Button still pressed
    Buttons -> Timer: Start long_press_timer
    loop Check button state periodically
        Timer -> Buttons: long_press_timer_callback()
        Buttons -> GPIO: Check button state
        alt Long press duration met
            Buttons -> Buttons: Set is_long_press
        else Button released before duration
            Buttons -> Buttons: Set is_pending_handle
        end
    end
else Button released
    Buttons -> Buttons: Set is_pending_handle
end
@enduml
```

### Summary

- **Short Press Detection**: Triggered by an interrupt, followed by a debounce timer to confirm the press.
- **Debouncing**: Filters out noise and false triggers to ensure a stable button state.
- **Long Press Detection**: Starts a long press timer after debouncing, periodically checks the button state, and confirms a long press if the button remains pressed for the configured duration.

---
