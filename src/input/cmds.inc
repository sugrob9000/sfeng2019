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
COMMAND (cam_move)
COMMAND (cam_accelerate)
COMMAND (cam_decelerate)
COMMAND (cam_dump_pos)

/*
 * Set app window size
 */
COMMAND (windowsize)

/*
 * Convert an obj model to rvd
 */
COMMAND (obj2rvd)

/*
 * Fire a signal to an entity:
 * signal [delay] [target name] [signal name] [arguments...]
 */
COMMAND (signal)

/*
 * Load a map
 */
COMMAND (loadmap)

/*
 * Visibility debugging commands
 */
COMMAND (vis_worldwireframe)
COMMAND (vis_occluders)
COMMAND (vis_leaves)
COMMAND (vis_leaves_nodepth)
COMMAND (vis_disable)

/* Recreate lighting framebuffers to be of sufficient resolution */
COMMAND (light_refit_buffers)

/*
 * Single integer value
 * 0     => do not debug anything
 * -1    => draw the depth buffer of the last light processed
 * N > 0 => debug the screenspace lighting buffer
 *          as after processing the first N lights
 */
COMMAND (light_debug_level)

/*
 * (Demo) Tell the entity named "sun"
 * to have the same pos and ang as camera
 */
COMMAND (sun_at_view)
