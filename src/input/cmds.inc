/*
 * Do nothing.
 */
COMMAND (nop)

/*
 * Exit the engine.
 */
COMMAND (exit)

/*
 * Output arguments to stdout
 */
COMMAND (echo)

/*
 * Bind a key to a command.
 * The key's name is the first argument
 * (In key names that contain spaces,
 *  they are replaced with underscores)
 * The command is the rest of the arguments
 */
COMMAND (bind)

/*
 * In-game interactive console
 */
COMMAND (console_open)
COMMAND (console_close)

/*
 * Run the commands from a file
 */
COMMAND (exec)

/*
 * Start/stop moving the camera in a direction
 * (f, l, r, b for forward, left, right, backward respectively)
 */
COMMAND (move_cam)

/*
 * Set app window size
 */
COMMAND (windowsize)

/*
 * Convert an obj model to rvd
 */
COMMAND (obj2rvd)
