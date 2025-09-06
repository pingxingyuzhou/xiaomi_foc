#include "can.h"
#include "util.h"
#include "usr_config.h"
#include "foc_encoder.h"
#include "controller.h"
#include "fsm.h"
#include "foc_handle.h"
CanFrame frame;
static volatile uint32_t mRecTick = 0;
static float mCurrent, mVelocity;
static float mPosition;


static void sync_callback(void);
static void config_callback(uint32_t frameID, uint8_t* data, bool isSet);
void CAN_report_error(int32_t ecode)
{
    uint8_t data[4];
    uint32_t frameID = (CAN_CMD_ERROR_REPORT << 4) | (UsrConfig.can_id & 0xF);
    int_to_data(ecode, data);
    GD_CAN_Send_Message(frameID, data, 4);
}

void CAN_reset_timeout(void)
{
    mRecTick = SYSTICK_get_tick();
}
void CAN_timeout_check_loop(void)
{
    if (UsrConfig.can_timeout_ms == 0)
    {
        return;
    }

    if ((SYSTICK_get_ms_since(mRecTick) > UsrConfig.can_timeout_ms))
    {
        FSM_input(CMD_MENU);
    }
}
void CAN_report_calibration(int step, uint8_t *data)
{
       uint8_t send_data[8];

       uint32_t frameID = (CAN_CMD_CALIBRATION_REPORT << 4) | (UsrConfig.can_id & 0xF);

       int_to_data(step, send_data);
       send_data[4] = data[0];
       send_data[5] = data[1];
       send_data[6] = data[2];
       send_data[7] = data[3];
       GD_CAN_Send_Message(frameID, send_data, 8);
}

void CAN_rx_callback(CanFrame *rx_frame)
{
	// Frame
    // CMD    | nodeID
    // 7 bits | 4 bits
	uint32_t frameID = rx_frame->can_id&0xff;
	uint32_t cmd = (frameID >> 4);
    uint32_t nodeID = (frameID & 0xF);

	if(nodeID != UsrConfig.can_id && nodeID != 0){
		return;
	}
	
	if(UsrConfig.can_timeout_ms != 0){
		mRecTick = SYSTICK_get_tick();
	}
	
	switch(cmd){
		case CAN_CMD_MOTOR_DISABLE:
			int_to_data(FSM_input(CMD_MENU), rx_frame->data);
			GD_CAN_Send_Message(frameID, rx_frame->data, 4);
			break;
		
		case CAN_CMD_MOTOR_ENABLE:
			int_to_data(FSM_input(CMD_MOTOR), rx_frame->data);
			float_to_data(Encoder.position, &rx_frame->data[4]);
			GD_CAN_Send_Message(frameID, rx_frame->data, 8);
			break;
		
		case CAN_CMD_ERROR_RESET:
			int_to_data(FSM_input(CMD_RESET_ERROR), rx_frame->data);
			GD_CAN_Send_Message(frameID, rx_frame->data, 4);
			break;
		
		case CAN_CMD_GET_STAT:
			int_to_data(FSM_get_stat(), rx_frame->data);
			int_to_data(FSM_get_error(), &rx_frame->data[4]);
			GD_CAN_Send_Message(frameID, rx_frame->data, 8);
			break;
		
		case CAN_CMD_CALIBRATION_START:
			int_to_data(FSM_input(CMD_CALIBRATION), rx_frame->data);
			GD_CAN_Send_Message(frameID, rx_frame->data, 4);
			break;
		
		case CAN_CMD_CALIBRATION_ABORT:
			int_to_data(FSM_input(CMD_MENU), rx_frame->data);
			GD_CAN_Send_Message(frameID, rx_frame->data, 4);
			break;

		case CAN_CMD_SYNC:
			if(UsrConfig.can_sync_target_enable){
				sync_callback();
			}
			break;
			
		case CAN_CMD_SET_TARGET_POSITION:
			mPosition = data_to_float(&rx_frame->data[0]);
			if(!UsrConfig.can_sync_target_enable){
				sync_callback();
			}
			break;
		
		case CAN_CMD_SET_TARGET_VELOCITY:
			mVelocity = data_to_float(&rx_frame->data[0]);
			if(!UsrConfig.can_sync_target_enable){
				sync_callback();
			}
			break;
		
		case CAN_CMD_SET_TARGET_CURRENT:
			mCurrent = data_to_float(&rx_frame->data[0]);
			if(!UsrConfig.can_sync_target_enable){
				sync_callback();
			}
			break;
		
		case CAN_CMD_GET_POSITION:
			float_to_data(Encoder.position, rx_frame->data);
			GD_CAN_Send_Message(frameID, rx_frame->data, 4);
			break;
		
		case CAN_CMD_GET_VELOCITY:
			float_to_data(Encoder.velocity, rx_frame->data);
			GD_CAN_Send_Message(frameID, rx_frame->data, 4);
			break;
		
		case CAN_CMD_GET_CURRENT:
			float_to_data(Foc.i_q_filt, rx_frame->data);
			GD_CAN_Send_Message(frameID, rx_frame->data, 4);
			break;
		
		case CAN_CMD_GET_VBUS:
			float_to_data(Foc.v_bus, rx_frame->data);
			GD_CAN_Send_Message(frameID, rx_frame->data, 4);
			break;
		
		case CAN_CMD_GET_IBUS:
			float_to_data(Foc.i_bus_filt, rx_frame->data);
			GD_CAN_Send_Message(frameID, rx_frame->data, 4);
			break;
		
		case CAN_CMD_SET_CONFIG:
			config_callback(frameID, rx_frame->data, true);
			break;
		
		case CAN_CMD_GET_CONFIG:
			config_callback(frameID, rx_frame->data, false);
			break;
		
		case CAN_CMD_UPDATE_CONFIGS:
			int_to_data(FSM_input(CMD_UPDATE_CONFIGS), rx_frame->data);
			GD_CAN_Send_Message(frameID, rx_frame->data, 4);
			break;
		
		case CAN_CMD_RESET_ALL_CONFIGS:
			USR_CONFIG_set_default_config();
			int_to_data(0, rx_frame->data);
			GD_CAN_Send_Message(frameID, rx_frame->data, 4);
		
		case CAN_CMD_GET_FW_VERSION:
			int_to_data(FW_VERSION_MAJOR, &rx_frame->data[0]);
			int_to_data(FW_VERSION_MINOR, &rx_frame->data[4]);
			GD_CAN_Send_Message(frameID, rx_frame->data, 8);
			break;
		
		default:
			break;
	}
}
static void sync_callback(void)
{
	if(FSM_get_stat() != FS_MOTOR_MODE){
		return;
	}
	
	switch(UsrConfig.control_mode){
		case CONTROL_MODE_CURRENT:
		case CONTROL_MODE_CURRENT_RAMP:
			Controller.input_current = mCurrent;
			break;
		
		case CONTROL_MODE_VELOCITY:
		case CONTROL_MODE_VELOCITY_RAMP:
			Controller.input_velocity = mVelocity;
			break;
		
		case CONTROL_MODE_POSITION:
			Controller.input_position = mPosition;
			break;
		
		case CONTROL_MODE_POSITION_TRAP:
			CONTROLLER_move_to_pos(mPosition);
			break;
		
		default:
			break;
	}
}

static void config_callback(uint32_t frameID, uint8_t* data, bool isSet)
{
	switch(data_to_int(data)){
		case CAN_CONFIG_MOTOR_POLE_PAIRS:
			if(isSet){
				UsrConfig.motor_pole_pairs = data_to_int(&data[4]);
			}
			int_to_data(UsrConfig.motor_pole_pairs, &data[4]);
			GD_CAN_Send_Message(frameID, data, 8);
			break;
			
		case CAN_CONFIG_MOTOR_PHASE_RESISTANCE:
			if(isSet){
				UsrConfig.motor_phase_resistance = data_to_float(&data[4]);
			}
			float_to_data(UsrConfig.motor_phase_resistance, &data[4]);
			GD_CAN_Send_Message(frameID, data, 8);
			break;
			
		case CAN_CONFIG_MOTOR_PHASE_INDUCTANCE:
			if(isSet){
				UsrConfig.motor_phase_inductance = data_to_float(&data[4]);
			}
			float_to_data(UsrConfig.motor_phase_inductance, &data[4]);
			GD_CAN_Send_Message(frameID, data, 8);
			break;
			
		case CAN_CONFIG_INERTIA:
			if(isSet){
				UsrConfig.inertia = data_to_float(&data[4]);
			}
			float_to_data(UsrConfig.inertia, &data[4]);
			GD_CAN_Send_Message(frameID, data, 8);
			break;
			
		case CAN_CONFIG_ENCODER_DIR_REV:
			if(isSet){
				UsrConfig.encoder_dir_rev = data_to_int(&data[4]);
			}
			int_to_data(UsrConfig.encoder_dir_rev, &data[4]);
			GD_CAN_Send_Message(frameID, data, 8);
			break;
			
		case CAN_CONFIG_ENCODER_OFFSET:
			if(isSet){
				UsrConfig.encoder_offset = data_to_int(&data[4]);
			}
			int_to_data(UsrConfig.encoder_offset, &data[4]);
			GD_CAN_Send_Message(frameID, data, 8);
			break;
			
		case CAN_CONFIG_CALIB_VALID:
			if(isSet){
				UsrConfig.calib_valid = data_to_int(&data[4]);
			}
			int_to_data(UsrConfig.calib_valid, &data[4]);
			GD_CAN_Send_Message(frameID, data, 8);
			break;
			
		case CAN_CONFIG_CALIB_CURRENT:
			if(isSet){
				UsrConfig.calib_current = data_to_float(&data[4]);
			}
			float_to_data(UsrConfig.calib_current, &data[4]);
			GD_CAN_Send_Message(frameID, data, 8);
			break;
			
		case CAN_CONFIG_CALIB_MAX_VOLTAGE:
			if(isSet){
				UsrConfig.calib_max_voltage = data_to_float(&data[4]);
			}
			float_to_data(UsrConfig.calib_max_voltage, &data[4]);
			GD_CAN_Send_Message(frameID, data, 8);
			break;
			
		case CAN_CONFIG_CONTROL_MODE:
			if(isSet){
				UsrConfig.control_mode = data_to_int(&data[4]);
			}
			int_to_data(UsrConfig.control_mode, &data[4]);
			GD_CAN_Send_Message(frameID, data, 8);
			break;
			
		case CAN_CONFIG_CURRENT_RAMP_RATE:
			if(isSet){
				UsrConfig.current_ramp_rate = data_to_float(&data[4]);
			}
			float_to_data(UsrConfig.current_ramp_rate, &data[4]);
			GD_CAN_Send_Message(frameID, data, 8);
			break;
			
		case CAN_CONFIG_VEL_RAMP_RATE:
			if(isSet){
				UsrConfig.vel_ramp_rate = data_to_float(&data[4]);
			}
			float_to_data(UsrConfig.vel_ramp_rate, &data[4]);
			GD_CAN_Send_Message(frameID, data, 8);
			break;
			
		case CAN_CONFIG_TRAJ_VEL:
			if(isSet){
				UsrConfig.traj_vel = data_to_float(&data[4]);
			}
			float_to_data(UsrConfig.traj_vel, &data[4]);
			GD_CAN_Send_Message(frameID, data, 8);
			break;
			
		case CAN_CONFIG_TRAJ_ACCEL:
			if(isSet){
				UsrConfig.traj_accel = data_to_float(&data[4]);
			}
			float_to_data(UsrConfig.traj_accel, &data[4]);
			GD_CAN_Send_Message(frameID, data, 8);
			break;
			
		case CAN_CONFIG_TRAJ_DECEL:
			if(isSet){
				UsrConfig.traj_decel = data_to_float(&data[4]);
			}
			float_to_data(UsrConfig.traj_decel, &data[4]);
			GD_CAN_Send_Message(frameID, data, 8);
			break;
			
		case CAN_CONFIG_POS_GAIN:
			if(isSet){
				UsrConfig.pos_gain = data_to_float(&data[4]);
			}
			float_to_data(UsrConfig.pos_gain, &data[4]);
			GD_CAN_Send_Message(frameID, data, 8);
			break;
			
		case CAN_CONFIG_VEL_GAIN:
			if(isSet){
				UsrConfig.vel_gain = data_to_float(&data[4]);
			}
			float_to_data(UsrConfig.vel_gain, &data[4]);
			GD_CAN_Send_Message(frameID, data, 8);
			break;
			
		case CAN_CONFIG_VEL_INTEGRATOR_GAIN:
			if(isSet){
				UsrConfig.vel_integrator_gain = data_to_float(&data[4]);
			}
			float_to_data(UsrConfig.vel_integrator_gain, &data[4]);
			GD_CAN_Send_Message(frameID, data, 8);
			break;
			
		case CAN_CONFIG_VEL_LIMIT:
			if(isSet){
				UsrConfig.vel_limit = data_to_float(&data[4]);
			}
			float_to_data(UsrConfig.vel_limit, &data[4]);
			GD_CAN_Send_Message(frameID, data, 8);
			break;
			
		case CAN_CONFIG_CURRENT_LIMIT:
			if(isSet){
				UsrConfig.current_limit = data_to_float(&data[4]);
			}
			float_to_data(UsrConfig.current_limit, &data[4]);
			GD_CAN_Send_Message(frameID, data, 8);
			break;
			
		case CAN_CONFIG_CURRENT_CTRL_P_GAIN:
			if(isSet){
				UsrConfig.current_ctrl_p_gain = data_to_float(&data[4]);
			}
			float_to_data(UsrConfig.current_ctrl_p_gain, &data[4]);
			GD_CAN_Send_Message(frameID, data, 8);
			break;
			
		case CAN_CONFIG_CURRENT_CTRL_I_GAIN:
			if(isSet){
				UsrConfig.current_ctrl_i_gain = data_to_float(&data[4]);
			}
			float_to_data(UsrConfig.current_ctrl_i_gain, &data[4]);
			GD_CAN_Send_Message(frameID, data, 8);
			break;
			
		case CAN_CONFIG_CURRENT_CTRL_BW:
			if(isSet){
				UsrConfig.current_ctrl_bandwidth = data_to_int(&data[4]);
			}
			int_to_data(UsrConfig.current_ctrl_bandwidth, &data[4]);
			GD_CAN_Send_Message(frameID, data, 8);
			break;
			
		case CAN_CONFIG_PROTECT_UNDER_VOLTAGE:
			if(isSet){
				UsrConfig.protect_under_voltage = data_to_float(&data[4]);
			}
			float_to_data(UsrConfig.protect_under_voltage, &data[4]);
			GD_CAN_Send_Message(frameID, data, 8);
			break;
			
		case CAN_CONFIG_PROTECT_OVER_VOLTAGE:
			if(isSet){
				UsrConfig.protect_over_voltage = data_to_float(&data[4]);
			}
			float_to_data(UsrConfig.protect_over_voltage, &data[4]);
			GD_CAN_Send_Message(frameID, data, 8);
			break;
			
		case CAN_CONFIG_PROTECT_OVER_SPEED:
			if(isSet){
				UsrConfig.protect_over_speed = data_to_float(&data[4]);
			}
			float_to_data(UsrConfig.protect_over_speed, &data[4]);
			GD_CAN_Send_Message(frameID, data, 8);
			break;
			
		case CAN_CONFIG_CAN_ID:
			if(isSet){
				UsrConfig.can_id = data_to_int(&data[4]);
			}
			int_to_data(UsrConfig.can_id, &data[4]);
			GD_CAN_Send_Message(frameID, data, 8);
			break;
			
		case CAN_CONFIG_CAN_TIMEOUT_MS:
			if(isSet){
				UsrConfig.can_timeout_ms = data_to_int(&data[4]);
			}
			int_to_data(UsrConfig.can_timeout_ms, &data[4]);
			GD_CAN_Send_Message(frameID, data, 8);
			break;
			
		case CAN_CONFIG_CAN_SYNC_TARGET_ENABLE:
			if(isSet){
				UsrConfig.can_sync_target_enable = data_to_int(&data[4]);
			}
			int_to_data(UsrConfig.can_sync_target_enable, &data[4]);
			GD_CAN_Send_Message(frameID, data, 8);
			break;
		
		default:
			break;
	}
}
