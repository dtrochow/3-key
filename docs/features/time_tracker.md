# TimeTracker Documentation

The `TimeTracker` feature enables users to track time spent on various tasks, such as work and meetings, with the press of a button. It also provides visual feedback using LED lights and allows users to log and review their time data.

---

## Key Features

1. **Track Work and Meeting Time**  
   Log time spent on work or meetings using dedicated buttons.

2. **Session Management**  
   Easily switch between sessions to track separate periods of activity.

3. **LED Feedback**  
   LED lights provide real-time feedback:
    - **Red**: Work tracking is active.
    - **Green**: Meeting tracking is active.

4. **View and Log Data**  
   Generate detailed time logs to review your tracked activities.

---

## How to Use

### Buttons and Their Functions

- **Button 1 (Red)**:  
   Toggles **Work Tracking**
    - Press to start tracking work time
    - Press again to stop tracking work time
    - The red LED will light up when work tracking is active

- **Button 2 (Green)**:  
   Toggles **Meeting Tracking**
    - Press to start tracking meeting time
    - Press again to stop tracking meeting time
    - The green LED will light up when meeting tracking is active

- **Button 3 (Blue)**:
   Ends the current session and starts a new one
    - Stops any active tracking (work or meetings)
    - Clears the LED indicators
    - Moves to the next tracking entry, allowing you to start a new session

---

## Retrieving Data

To review your tracked time, the `TimeTracker` provides logs that can be generated and displayed in serial terminal:

1. **Work Time Log**:  
   Displays the total time spent on work in the current session.  
   Example Output:  
   `2025-01-26 Work: 2h 15min 30s`

2. **Meeting Time Log**:  
   Displays the total time spent on meetings in the current session.  
   Example Output:  
   `2025-01-26 Meetings: 1h 45min 15s`

---

## Additional Notes

- **Session Limit**: The TimeTracker supports up to a predefined maximum number of sessions. Once the limit is reached, it cycles back to the first session.
- **Data Persistence**: All tracked time data is saved, ensuring that it remains available even after powering off the device.
- **Factory Reset**: If needed, the feature can be reset to its factory settings, clearing all stored data.

---

With the `TimeTracker`, you can easily manage and review your time spent on work and meetings, ensuring better productivity and accountability.
