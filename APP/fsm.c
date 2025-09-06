#include "fsm.h"
#include <math.h>
#include <string.h>
#include "usr_config.h"
#include "foc_handle.h"
#include "drv8323.h"
#include "controller.h"
#include "util.h"
#include "foc_encoder.h"
#include "calibration.h"
#include "can.h"

float vdata;
typedef struct sFSM
{
	tFsmStat state;
	tFsmStat next_state;
	uint8_t ready;
	char cmd_buff[64];
	uint8_t bytecount;
} tFSM;

static tFSM Fsm = {
	.state = FS_STARTUP,
	.next_state = FS_STARTUP,
	.ready = 1,
	.bytecount = 0,
};

static volatile int mErrorCode = 0;
static volatile int mErrorCodeLast = 0;

static void enter_state(void);
static void exit_state(void);
static void reset_error(void);
static void print_menu(void);
static void print_setup(void);
static void process_user_input(void);
static void debug_input_q(void);
static void debug_input_w(void);

int FSM_input(char c)
{
	int ret = 0;

	if (c == CMD_MENU)
	{
		Fsm.next_state = FS_MENU_MODE;
		Fsm.ready = 0;
		return ret;
	}

	switch (Fsm.state)
	{
	case FS_STARTUP:
		ret = -1;
		break;

	case FS_MENU_MODE:
		switch (c)
		{
		case CMD_MOTOR:
			if (mErrorCode == 0 && UsrConfig.calib_valid)
			{
				Fsm.next_state = FS_MOTOR_MODE;
				Fsm.ready = 0;
			}
			else
			{
				ret = -1;
			}
			break;

		case CMD_CALIBRATION:
			if (mErrorCode == 0)
			{
				Fsm.next_state = FS_CALIBRATION_MODE;
				Fsm.ready = 0;
			}
			else
			{
				ret = -1;
			}
			break;

		case CMD_UPDATE_CONFIGS:
			ret = USR_CONFIG_save_config();
			if (ret == 0)
			{
				DEBUG("User Config Update OK!\n\r\n\r");
			}
			else
			{
				DEBUG("User Config Update FAIL!\n\r\n\r");
			}
			break;

		case CMD_RESET_ERROR:
			reset_error();
			break;

		case CMD_UART_SETUP:
			Fsm.next_state = FS_UART_SETUP;
			Fsm.ready = 0;
			break;

		default:
			ret = -1;
			break;
		}
		break;

	case FS_MOTOR_MODE:
		if (c == CMD_DEBUG_Q)
		{
			debug_input_q();
		}
		else if (c == CMD_DEBUG_W)
		{
			debug_input_w();
		}
		else if (c == CMD_MOTOR)
		{
			ret = 0;
		}
		else
		{
			ret = -1;
		}
		break;

	case FS_CALIBRATION_MODE:
		ret = -1;
		break;

	case FS_UART_SETUP:
		if (c == CMD_ENTER)
		{ // 'Enter'
			process_user_input();
			break;
		}
		else if (c == '\b')
		{
			if (Fsm.bytecount > 0)
			{
				Fsm.bytecount--;
				DEBUG("\b \b");
			}
		}
		else
		{
			Fsm.cmd_buff[Fsm.bytecount] = c;
			Fsm.bytecount++;
			if (Fsm.bytecount >= sizeof(Fsm.cmd_buff))
			{
				Fsm.bytecount = 0;
			}
			DEBUG("%c", c);
		}
		break;

	default:
		break;
	}

	if (ret == -1)
	{
		DEBUG("Invalid state!\n\r");
	}

	return ret;
}

void FSM_loop(void)
{
	/* state transition management */
	if (Fsm.next_state != Fsm.state)
	{
		exit_state();
		if (Fsm.ready)
		{
			Fsm.state = Fsm.next_state;
			enter_state();
		}
	}

	switch (Fsm.state)
	{
	case FS_STARTUP:
		return;

	case FS_CALIBRATION_MODE:
		CALIBRATION_loop(&Foc);
		break;

	case FS_MOTOR_MODE:
	{
		float current_setpoint = CONTROLLER_loop(&Controller, Encoder.velocity, Encoder.position);
		FOC_current(&Foc, 0, current_setpoint, Encoder.elec_angle, Encoder.elec_angle);

		// Over speed check
		if (fabs(Encoder.velocity) > UsrConfig.protect_over_speed)
		{
			mErrorCode |= ERR_OVER_SPEED;
		}
	}
	break;

	default:
		break;
	}

	// Over voltage check
	if (Foc.v_bus >= UsrConfig.protect_over_voltage)
	{
		vdata = Foc.v_bus;
		mErrorCode |= ERR_OVER_VOLTAGE;
	}

	// Under voltage check
	if (Foc.v_bus <= UsrConfig.protect_under_voltage)
	{
		mErrorCode |= ERR_UNDER_VOLTAGE;
	}

	// DRV8323 error
	if (DRV_IS_FAULT())
	{
		if (mErrorCode != mErrorCodeLast)
		{
			mErrorCode |= DRV8323_getFault();
		}
	}

	if (mErrorCode != mErrorCodeLast)
	{
		if (mErrorCode)
		{
			FOC_disarm();
			Fsm.next_state = FS_MENU_MODE;
			Fsm.ready = 0;
		}

		DEBUG("Error Code: %X\n\r", mErrorCode);
		CAN_report_error(mErrorCode);
		mErrorCodeLast = mErrorCode;
	}
}

int FSM_get_error(void)
{
	return mErrorCode;
}

tFsmStat FSM_get_stat(void)
{
	return Fsm.state;
}

static void enter_state(void)
{
	switch (Fsm.state)
	{
	case FS_MENU_MODE:
		print_menu();
		break;

	case FS_MOTOR_MODE:
		DEBUG("Enter Motor Mode\n\r");
		CONTROLLER_reset(&Controller);
		FOC_reset(&Foc);
		FOC_arm();
		break;

	case FS_CALIBRATION_MODE:
		DEBUG("Calibration Start\n\r");
		CALIBRATION_start();
		break;

	case FS_UART_SETUP:
		Fsm.bytecount = 0;
		memset(Fsm.cmd_buff, 0, sizeof(Fsm.cmd_buff));
		print_setup();
		break;

	default:
		break;
	}
}

static void exit_state(void)
{
	switch (Fsm.state)
	{
	case FS_STARTUP:
		CAN_reset_timeout();
		Fsm.ready = 1;
		break;

	case FS_MENU_MODE:
		Fsm.ready = 1;
		break;

	case FS_MOTOR_MODE:
		FOC_disarm();
		DEBUG("\n\r");
		Fsm.ready = 1;
		break;

	case FS_CALIBRATION_MODE:
		FOC_disarm();
		CALIBRATION_end();
		DEBUG("Calibration End\n\r\n\r");
		Fsm.ready = 1;
		break;

	case FS_UART_SETUP:
		DEBUG("\n\r");
		Fsm.ready = 1;
		break;

	default:
		break;
	}
}

static void reset_error(void)
{
	DRV8323_reset();

	uint32_t primask = cpu_enter_critical();
	mErrorCode = 0;
	mErrorCodeLast = 0;
	cpu_exit_critical(primask);
}

static void print_menu(void)
{
	DEBUG("Commands:\n\r");
	DEBUG(" m   - Motor Mode\n\r");
	DEBUG(" c   - Calibration\n\r");
	DEBUG(" z   - Clear Err\n\r");
	DEBUG(" s   - Setup\n\r");
	DEBUG(" u   - Update Configs\n\r");
	DEBUG(" Esc - Exit to Menu\n\r");
	DEBUG("\n\r");
}

static void print_setup(void)
{
	DEBUG("Configuration Options\n\r");

	DEBUG(" Motor\n\r");
	DEBUG("  motor_pole_pairs = %d\n\r", UsrConfig.motor_pole_pairs);
	DEBUG("  motor_phase_resistance = %f\n\r", UsrConfig.motor_phase_resistance);
	DEBUG("  motor_phase_inductance = %f\n\r", UsrConfig.motor_phase_inductance);
	DEBUG("  inertia = %f\n\r", UsrConfig.inertia);

	DEBUG(" Calibration\n\r");
	DEBUG("  calib_current = %f\n\r", UsrConfig.calib_current);
	DEBUG("  calib_max_voltage = %f\n\r", UsrConfig.calib_max_voltage);

	DEBUG(" Control\n\r");
	DEBUG("  control_mode = %d\n\r", UsrConfig.control_mode);
	DEBUG("  current_ramp_rate = %f\n\r", UsrConfig.current_ramp_rate);
	DEBUG("  vel_ramp_rate = %f\n\r", UsrConfig.vel_ramp_rate);
	DEBUG("  traj_vel = %f\n\r", UsrConfig.traj_vel);
	DEBUG("  traj_accel = %f\n\r", UsrConfig.traj_accel);
	DEBUG("  traj_decel = %f\n\r", UsrConfig.traj_decel);
	DEBUG("  pos_gain = %f\n\r", UsrConfig.pos_gain);
	DEBUG("  vel_gain = %f\n\r", UsrConfig.vel_gain);
	DEBUG("  vel_integrator_gain = %f\n\r", UsrConfig.vel_integrator_gain);
	DEBUG("  vel_limit = %f\n\r", UsrConfig.vel_limit);
	DEBUG("  current_limit = %f\n\r", UsrConfig.current_limit);
	DEBUG("  current_ctrl_bandwidth = %d\n\r", UsrConfig.current_ctrl_bandwidth);

	DEBUG(" Protect\n\r");
	DEBUG("  protect_under_voltage = %f\n\r", UsrConfig.protect_under_voltage);
	DEBUG("  protect_over_voltage = %f\n\r", UsrConfig.protect_over_voltage);
	DEBUG("  protect_over_speed = %f\n\r", UsrConfig.protect_over_speed);

	DEBUG(" CAN\n\r");
	DEBUG("  can_id = %d\n\r", UsrConfig.can_id);
	DEBUG("  can_timeout_ms = %d\n\r", UsrConfig.can_timeout_ms);
	DEBUG("  can_sync_target_enable = %d\n\r", UsrConfig.can_sync_target_enable);
	DEBUG("  Initangle = %d\n\r", UsrConfig.Initangle_Hall);
	DEBUG("\n\rTo change a value, type 'prefix' = 'value''ENTER'\n\ri.e. 'motor_pole_pairs = 7''ENTER'\n\r\n\r");
}

static void process_user_input(void)
{
	// Parse CMD
	if (0 == memcmp(Fsm.cmd_buff, "motor_pole_pairs = ", sizeof("motor_pole_pairs = ") - 1))
	{
		UsrConfig.motor_pole_pairs = atoi(Fsm.cmd_buff + sizeof("motor_pole_pairs = ") - 1);
		DEBUG("\n\r  motor_pole_pairs = %d\n\r", UsrConfig.motor_pole_pairs);
	}
	else if (0 == memcmp(Fsm.cmd_buff, "motor_phase_resistance = ", sizeof("motor_phase_resistance = ") - 1))
	{
		UsrConfig.motor_phase_resistance = atof(Fsm.cmd_buff + sizeof("motor_phase_resistance = ") - 1);
		DEBUG("\n\r  motor_phase_resistance = %f\n\r", UsrConfig.motor_phase_resistance);
	}
	else if (0 == memcmp(Fsm.cmd_buff, "motor_phase_inductance = ", sizeof("motor_phase_inductance = ") - 1))
	{
		UsrConfig.motor_phase_inductance = atof(Fsm.cmd_buff + sizeof("motor_phase_inductance = ") - 1);
		DEBUG("\n\r  motor_phase_inductance = %f\n\r", UsrConfig.motor_phase_inductance);
	}
	else if (0 == memcmp(Fsm.cmd_buff, "inertia = ", sizeof("inertia = ") - 1))
	{
		UsrConfig.inertia = atof(Fsm.cmd_buff + sizeof("inertia = ") - 1);
		DEBUG("\n\r  inertia = %f\n\r", UsrConfig.inertia);
	}
	else if (0 == memcmp(Fsm.cmd_buff, "calib_current = ", sizeof("calib_current = ") - 1))
	{
		UsrConfig.calib_current = atof(Fsm.cmd_buff + sizeof("calib_current = ") - 1);
		DEBUG("\n\r  calib_current = %f\n\r", UsrConfig.calib_current);
	}
	else if (0 == memcmp(Fsm.cmd_buff, "calib_max_voltage = ", sizeof("calib_max_voltage = ") - 1))
	{
		UsrConfig.calib_max_voltage = atof(Fsm.cmd_buff + sizeof("calib_max_voltage = ") - 1);
		DEBUG("\n\r  calib_max_voltage = %f\n\r", UsrConfig.calib_max_voltage);
	}
	else if (0 == memcmp(Fsm.cmd_buff, "control_mode = ", sizeof("control_mode = ") - 1))
	{
		UsrConfig.control_mode = atoi(Fsm.cmd_buff + sizeof("control_mode = ") - 1);
		DEBUG("\n\r  control_mode = %d\n\r", UsrConfig.control_mode);
	}
	else if (0 == memcmp(Fsm.cmd_buff, "current_ramp_rate = ", sizeof("current_ramp_rate = ") - 1))
	{
		UsrConfig.current_ramp_rate = atof(Fsm.cmd_buff + sizeof("current_ramp_rate = ") - 1);
		DEBUG("\n\r  current_ramp_rate = %f\n\r", UsrConfig.current_ramp_rate);
	}
	else if (0 == memcmp(Fsm.cmd_buff, "vel_ramp_rate = ", sizeof("vel_ramp_rate = ") - 1))
	{
		UsrConfig.vel_ramp_rate = atof(Fsm.cmd_buff + sizeof("vel_ramp_rate = ") - 1);
		DEBUG("\n\r  vel_ramp_rate = %f\n\r", UsrConfig.vel_ramp_rate);
	}
	else if (0 == memcmp(Fsm.cmd_buff, "traj_vel = ", sizeof("traj_vel = ") - 1))
	{
		UsrConfig.traj_vel = atoi(Fsm.cmd_buff + sizeof("traj_vel = ") - 1);
		DEBUG("\n\r  traj_vel = %f\n\r", UsrConfig.traj_vel);
	}
	else if (0 == memcmp(Fsm.cmd_buff, "traj_accel = ", sizeof("traj_accel = ") - 1))
	{
		UsrConfig.traj_accel = atoi(Fsm.cmd_buff + sizeof("traj_accel = ") - 1);
		DEBUG("\n\r  traj_accel = %f\n\r", UsrConfig.traj_accel);
	}
	else if (0 == memcmp(Fsm.cmd_buff, "traj_decel = ", sizeof("traj_decel = ") - 1))
	{
		UsrConfig.traj_decel = atoi(Fsm.cmd_buff + sizeof("traj_decel = ") - 1);
		DEBUG("\n\r  traj_decel = %f\n\r", UsrConfig.traj_decel);
	}
	else if (0 == memcmp(Fsm.cmd_buff, "pos_gain = ", sizeof("pos_gain = ") - 1))
	{
		UsrConfig.pos_gain = atof(Fsm.cmd_buff + sizeof("pos_gain = ") - 1);
		DEBUG("\n\r  pos_gain = %f\n\r", UsrConfig.pos_gain);
	}
	else if (0 == memcmp(Fsm.cmd_buff, "vel_gain = ", sizeof("vel_gain = ") - 1))
	{
		UsrConfig.vel_gain = atof(Fsm.cmd_buff + sizeof("vel_gain = ") - 1);
		DEBUG("\n\r  vel_gain = %f\n\r", UsrConfig.vel_gain);
	}
	else if (0 == memcmp(Fsm.cmd_buff, "vel_integrator_gain = ", sizeof("vel_integrator_gain = ") - 1))
	{
		UsrConfig.vel_integrator_gain = atof(Fsm.cmd_buff + sizeof("vel_integrator_gain = ") - 1);
		DEBUG("\n\r  vel_integrator_gain = %f\n\r", UsrConfig.vel_integrator_gain);
	}
	else if (0 == memcmp(Fsm.cmd_buff, "vel_limit = ", sizeof("vel_limit = ") - 1))
	{
		UsrConfig.vel_limit = atof(Fsm.cmd_buff + sizeof("vel_limit = ") - 1);
		DEBUG("\n\r  vel_limit = %f\n\r", UsrConfig.vel_limit);
	}
	else if (0 == memcmp(Fsm.cmd_buff, "current_limit = ", sizeof("current_limit = ") - 1))
	{
		UsrConfig.current_limit = atof(Fsm.cmd_buff + sizeof("current_limit = ") - 1);
		DEBUG("\n\r  current_limit = %f\n\r", UsrConfig.current_limit);
	}
	else if (0 == memcmp(Fsm.cmd_buff, "current_ctrl_bandwidth = ", sizeof("current_ctrl_bandwidth = ") - 1))
	{
		UsrConfig.current_ctrl_bandwidth = atoi(Fsm.cmd_buff + sizeof("current_ctrl_bandwidth = ") - 1);
		DEBUG("\n\r  current_ctrl_bandwidth = %d\n\r", UsrConfig.current_ctrl_bandwidth);
	}
	else if (0 == memcmp(Fsm.cmd_buff, "protect_under_voltage = ", sizeof("protect_under_voltage = ") - 1))
	{
		UsrConfig.protect_under_voltage = atof(Fsm.cmd_buff + sizeof("protect_under_voltage = ") - 1);
		DEBUG("\n\r  protect_under_voltage = %f\n\r", UsrConfig.protect_under_voltage);
	}
	else if (0 == memcmp(Fsm.cmd_buff, "protect_over_voltage = ", sizeof("protect_over_voltage = ") - 1))
	{
		UsrConfig.protect_over_voltage = atof(Fsm.cmd_buff + sizeof("protect_over_voltage = ") - 1);
		DEBUG("\n\r  protect_over_voltage = %f\n\r", UsrConfig.protect_over_voltage);
	}
	else if (0 == memcmp(Fsm.cmd_buff, "protect_over_speed = ", sizeof("protect_over_speed = ") - 1))
	{
		UsrConfig.protect_over_speed = atof(Fsm.cmd_buff + sizeof("protect_over_speed = ") - 1);
		DEBUG("\n\r  protect_over_speed = %f\n\r", UsrConfig.protect_over_speed);
	}
	else if (0 == memcmp(Fsm.cmd_buff, "can_id = ", sizeof("can_id = ") - 1))
	{
		UsrConfig.can_id = atoi(Fsm.cmd_buff + sizeof("can_id = ") - 1);
		DEBUG("\n\r  can_id = %d\n\r", UsrConfig.can_id);
	}
	else if (0 == memcmp(Fsm.cmd_buff, "can_timeout_ms = ", sizeof("can_timeout_ms = ") - 1))
	{
		UsrConfig.can_timeout_ms = atoi(Fsm.cmd_buff + sizeof("can_timeout_ms = ") - 1);
		DEBUG("\n\r  can_timeout_ms = %d\n\r", UsrConfig.can_timeout_ms);
	}
	else if (0 == memcmp(Fsm.cmd_buff, "can_sync_target_enable = ", sizeof("can_sync_target_enable = ") - 1))
	{
		UsrConfig.can_sync_target_enable = atoi(Fsm.cmd_buff + sizeof("can_sync_target_enable = ") - 1);
		DEBUG("\n\r  can_sync_target_enable = %d\n\r", UsrConfig.can_sync_target_enable);
	}
	else
	{
		DEBUG("\n\rInvalid input\n\r");
	}

	Fsm.bytecount = 0;
	memset(Fsm.cmd_buff, 0, sizeof(Fsm.cmd_buff));
}

static void debug_input_q(void)
{
	switch (UsrConfig.control_mode)
	{
	case CONTROL_MODE_CURRENT:
		Controller.input_current += 0.1f;
		DEBUG("input_current = %f\n\r", Controller.input_current);
		break;

	case CONTROL_MODE_CURRENT_RAMP:
		Controller.input_current += 0.1f;
		DEBUG("[Ramp] input_current = %f\n\r", Controller.input_current);
		break;

	case CONTROL_MODE_VELOCITY:
		Controller.input_velocity += 0.5f;
		DEBUG("input_velocity = %f\n\r", Controller.input_velocity);
		break;

	case CONTROL_MODE_VELOCITY_RAMP:
		Controller.input_velocity += 0.5f;
		DEBUG("[Ramp] input_velocity = %f\n\r", Controller.input_velocity);
		break;

	case CONTROL_MODE_POSITION:
		Controller.input_position += 0.01f;
		DEBUG("input_position = %f\n\r", Controller.input_position);
		break;

	case CONTROL_MODE_POSITION_TRAP:
		CONTROLLER_move_to_pos(5);
		DEBUG("move_to_pos = 5\n\r");
		break;

	default:
		break;
	}
}

static void debug_input_w(void)
{
	switch (UsrConfig.control_mode)
	{
	case CONTROL_MODE_CURRENT:
		Controller.input_current -= 0.1f;
		DEBUG("input_current = %f\n\r", Controller.input_current);
		break;

	case CONTROL_MODE_CURRENT_RAMP:
		Controller.input_current -= 0.1f;
		DEBUG("[Ramp] input_current = %f\n\r", Controller.input_current);
		break;

	case CONTROL_MODE_VELOCITY:
		Controller.input_velocity -= 0.5f;
		DEBUG("input_velocity = %f\n\r", Controller.input_velocity);
		break;

	case CONTROL_MODE_VELOCITY_RAMP:
		Controller.input_velocity -= 0.5f;
		DEBUG("[Ramp] input_velocity = %f\n\r", Controller.input_velocity);
		break;

	case CONTROL_MODE_POSITION:
		Controller.input_position -= 0.01f;
		DEBUG("input_position = %f\n\r", Controller.input_position);
		break;

	case CONTROL_MODE_POSITION_TRAP:
		CONTROLLER_move_to_pos(0);
		DEBUG("move_to_pos = 0\n\r");
		break;

	default:
		break;
	}
}
