#pragma once

class TimeTracker {};

/*

The Storage must have the feature for reserving the config slots

The reservation must be done in the nvm memory, so the Storage
must have its own configuration at highest ID and it will be reserved for its purpose

The slots must be the std::variant type, because I would like to define different
kinds of slots for different modules - e.g. TimeTrackerCfg_t

Other modules will be able to reserve the slots and use them for their own configurations
We can call it blobs

There will be N blobs available

Each blob/config will have 1024 bytes

Time-Tracker Config


The values needed for beginning:

Each time the button is pressed it must trigger saving the config to nvm, because it should handle
the power loss So, if the time-tracker will be disconnected while measuring work time, it should
continue counting after re-connection. It can be handled by saving start time and end time (when
button pressed again) and then substracting start time from end.

- work_time
- start
- end
- is_working

To simplify the time-tracker it will be able to measure the time without handling the reports,
automatically moving to the next day, etc.

It should be as simple as possible in the first version. Just tracking start and end and calculating
the work time without handling the reports, automatically moving to the next day, etc.

It should be as simple as possible in the first version. Just tracking start and end and calculating
the work time. And handling device disconnection. Light green when the work is in progress and red
when not working.

Additionally, the feature for getting the current work time from the host side should be introduced.
It can be done using a simple "work_time" command for the beginning.

*/