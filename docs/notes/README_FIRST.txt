CATCH THE ROCKET - BIT BY BIT
FINAL CODE SUBMISSION PACKAGE

IMPORTANT
Each numbered folder is a separate Arduino sketch. Open the .ino file inside that folder, not the parent folder. Do not put multiple .ino files in the same Arduino sketch folder.

01_Level0
- Level0_to_Level2_Combined.ino is the actual combined Level 0-2 development sketch retained from the project. It includes the official tracker and contains the Level 1/2 calculations as well.

02_Level1
- Level1_Descent_Landing_HistoricalBase.ino is a runnable copy of the retained combined Level 0-2 sketch, kept as the Level 1 historical base.

03_Level2
- Level2_Gravity_Apogee_HistoricalBase.ino is a runnable copy of the retained combined Level 0-2 sketch, kept as the Level 2 historical base.

04_Level3
- Level3_Deployment_ALT_Runnable.ino is a runnable Level 3-compatible copy of the recorded deployment/ALT implementation, configured with the supplied Official Tracker and glitches disabled. The standalone historical Level 3 source file was not retained as a separate file in the conversation, so this is explicitly labeled as a reconstructed runnable copy rather than claimed as the exact historical file.

05_Level4
- Level4_Glitches_Runnable.ino is a runnable Level 4 copy of the recorded glitch implementation, configured with the supplied Official Tracker and glitches enabled.
- Exact historical adversarial Level 4 source files are stored in Archive_Historical_Sources.

06_Level5_Final
- CatchTheRocket_Level5_Final.ino is the final submission candidate with the final state-struct cleanup and command-latency measurement. It is configured with the supplied Official Tracker and glitches enabled so it can be compiled/run directly from this folder.
- Exact pre-final historical Level 5 source is in Archive_Historical_Sources.

Trackers
- official_tracker.h and practice_tracker_b.h are the supplied tracker headers. They are kept unchanged.

Arduino setup
- Board: ESP32 Dev Module
- Serial Monitor: 115200 baud
- USB connection to ESP32

FINAL MEASURED HARDWARE RESULT
- Total readings: 100000
- Valid readings: 99998
- Glitches: 2
- State size: 248 bytes
- Worst reading: 10394 cycles
- Reading budget: 12000 cycles
- Performance margin: 1606 cycles
- Last command cycles: 368
- Worst command cycles: 8051

The 10394-cycle and command-latency numbers above are the actual final ESP32 evidence from the final Level 5 run. The runnable default Level 5 folder is configured for the Official Tracker, so its numerical output will not necessarily reproduce the adversarial-1 screenshot values.

Do not modify any supplied tracker header.
