int Init_joypad(void);	// Must call at the first to open the joypad device
int End_joypad(void);	// Must call at the end to close the joypad device
int Read_joypad(void);	// Read input buffer the check joypad press/release status