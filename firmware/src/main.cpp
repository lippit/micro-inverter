/*
 *
 * Copyright (c) 2021-2024 LAAS-CNRS
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published by
 *   the Free Software Foundation, either version 2.1 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

/**
 * @brief  This file is the main entry point of the
 *         OwnTech Power API. Please check the OwnTech
 *         documentation for detailed information on
 *         how to use Power API: https://docs.owntech.org/
 *
 * @author Clément Foucher <clement.foucher@laas.fr>
 * @author Luiz Villa <luiz.villa@laas.fr>
 */

//-------------- OWNTECH APIs ---------------------------------
#include "TaskAPI.h"
#include "ShieldAPI.h"
#include "SpinAPI.h"
#include "auxiliary.h"

// Control library
#include "trigo.h"
#include "filters.h"
#include "ScopeMimicry.h"
#include "control_factory.h"
#include "singlePhaseInverter.h"
#include "sogi.h"
#include "user_data_api.h"
#include <zephyr/console/console.h>
#include <zephyr/sys/printk.h>


#define DUTY_MIN 0.1F
#define DUTY_MAX 0.9F
#define UDC_STARTUP 0.0F

//-------------- SETUP FUNCTIONS DECLARATION ------------------
/* Setups the hardware and software of the system */
void setup_routine();

//-------------- LOOP FUNCTIONS DECLARATION -------------------
// Code to be executed in the background task
void loop_application_task();
// Code to be executed in real time in the critical task
void loop_critical_task();

//-------------- USER VARIABLES DECLARATIONS ------------------
// [us] period of the control task
static const uint32_t control_task_period = 100;
// [bool] state of the PWM (ctrl task)
static bool pwm_enable = false;

// Measurement variables
float32_t Vlow_value;     // [V]
float32_t Vac_value;      // [V]
float32_t Ilow1_value;    // [A]
float32_t Ilow2_value;    // [A]
float32_t Vdc_bus;        // [V]
float32_t Iac_value;      // [A]
float32_t Vdc_bus_filt;   // [V]
float32_t meas_data;      // holds the measurement for validation

// [A] Current offset found experimentally 21/10/2025
static float32_t I1_current_offset = 0.25F;
static float32_t I2_current_offset = 0.25F; // [A]

float32_t Vgrid_meas; // [V]
float32_t VN_meas;    // [V]
float32_t Igrid_meas; // [A]

static singlePhaseInverter inverter;

static dqo_t power;


static dqo_t Vdq;        // Vdq measure (in)
static dqo_t Vdq_output; // Inverter output
dqo_t Vdq_ref;
dqo_t Vdq_ref_max;
dqo_t Vdq_ref_min;
static float32_t Valpha_in_out;

static dqo_t Idq;
dqo_t Idq_ref;
dqo_t Idq_ref_max;
dqo_t Idq_ref_min;
static dqo_t Idq_ref_delta;




static float32_t Id_ref_delta = 0.0;
static float32_t Iq_ref_delta = 0.0;


static float32_t Vd_ref_max = 20.0F;
static float32_t Vd_ref_min = 0.0F;


static clarke_t Vab;
static clarke_t Vab_output;
static clarke_t Iab;

static float32_t Vond;
static float32_t R_load = 10;

static float32_t Ialpha, Ibeta;
static bool is_net_synchronized;
static float32_t omega;

inverter_mode local_mode = FOLLOWING;

// Duty-cycle control
static float32_t delta_duty_cycle;   // [no unit]
static float32_t duty_cycle_1;       // [no unit]
static float32_t duty_cycle_2;       // [no unit]
static float32_t duty_cycle_offset;  // [no unit]
static float32_t boost_duty_cycle = 0.05F;
static uint16_t boost_pos_dt = 100;
static uint16_t boost_neg_dt = 100;
float32_t boost_voltage_reference = 33.0F;
bool boost_pwm_enable = false;
bool inverter_on = false;

static float32_t Udc = 63.0F;        // dc voltage supply assumed [V]
static const float f0 = 50.0F;       // fundamental frequency [Hz]
static const float32_t w0 = 2.0F * PI * f0;   // pulsation [rad/s]
static const float32_t sync_power_tolerance = 0.01F * w0;

// Sinewave settings
static float32_t Vgrid_ref; // [V]
static float32_t Vgrid_amplitude_ref = 20.0F; // [V]
static float32_t Vgrid_amplitude = 20.0F; // [V]
static float angle = 0.0F; // [rad]
static float theta = 0.0F; // [rad]

//------------- PR RESONANT -----------------------------------
float32_t Ts = control_task_period * 1.0e-6F;

// kp is small due to the pure delay system (ref Viking)
static float32_t kp = 0.001F;
static float32_t Ti = 0.001F / 3000; // Ti is Kp/Ki
float32_t Td = 0.0F;
float32_t N = 1.0F;
float32_t upper_bound = Udc;
float32_t lower_bound = -Udc;

static Pid pi_current_d = controlLibFactory.pid(Ts, kp, Ti, Td, N,
                                                lower_bound, upper_bound);
static Pid pi_current_q = controlLibFactory.pid(Ts, kp, Ti, Td, N,
                                                lower_bound, upper_bound);

static Pid pi_voltage_d = controlLibFactory.pid(Ts, 0.01, 0.003, Td, N,
                                                lower_bound, upper_bound);
static Pid pi_voltage_q = controlLibFactory.pid(Ts, 0.01, 0.003, Td, N,
                                                lower_bound, upper_bound);

static float32_t boost_kp = 0.000215F;
static float32_t boost_Ti = 7.5175e-5F;
static float32_t boost_Td = 0.0F;
static float32_t boost_N = 0.0F;
static float32_t boost_upper_bound = 1.0F;
static float32_t boost_lower_bound = 0.0F;
static Pid boost_pid = controlLibFactory.pid(Ts, boost_kp, boost_Ti,
                                             boost_Td, boost_N,
                                             boost_lower_bound,
                                             boost_upper_bound);
Sogi sogi_i;
Sogi sogi_v;


// Comes from "filters.h"
LowPassFirstOrderFilter vHighFilter(Ts, 0.1F);
LowPassFirstOrderFilter VqFilter(Ts, 1.0F);
static uint32_t critical_task_counter;
static uint32_t decimation = 1;
static uint32_t sync_counter = 0;
static uint32_t desync_counter = 0;
static uint32_t power_counter = 0;
static float32_t desync_counter_scope;
static bool sync_start_flag = false;
static float32_t Vq_filtered;

// Scope records data during the critical task
// (library included via platformio.ini)
ScopeMimicry scope(1024, 19);
bool is_downloading;
bool trigger = false;
//---------------------------------------------------------------

static uint8_t mode = IDLEMODE;
uint8_t mode_asked = IDLEMODE;
static float32_t spying_mode = 0;
static const float32_t MAX_CURRENT = 8.0F;

//-------------- SETUP FUNCTIONS ------------------------------

/**
 * This is the setup routine.
 * It is used to call functions that will initialize your hardware and tasks.
 * In this example, we setup the version of the spin board and a 
 * background task. The critical task is defined but not started.
 * NOTE: It is important to follow the steps and initialize the hardware first 
 * and the tasks second. 
 */
void setup_routine()
{
    // Setup the hardware first
    enableUSolarVerterSensors();

    // Boost control on low legs (parallel boost)
    shield.power.initBoost(LEG1_LOW);
    shield.power.initBoost(LEG2_LOW);

    scope.connectChannel(Ilow1_value, "Ilow1_value");
    scope.connectChannel(Iac_value, "Iac_value");
    scope.connectChannel(Vgrid_meas, "Vgrid");
    scope.connectChannel(Vdc_bus, "Vdc_bus");
    scope.connectChannel(duty_cycle_1, "duty_cycle_1");
    scope.connectChannel(duty_cycle_2, "duty_cycle_2");
    scope.connectChannel(Idq.d, "Id");
    scope.connectChannel(Idq.q, "Iq");
    scope.connectChannel(Idq_ref.d, "Id_ref");
    scope.connectChannel(Iab.alpha, "Ialpha");
    scope.connectChannel(Iab.beta, "Ibeta");
    scope.connectChannel(Vdq.q, "Vq_in");
    scope.connectChannel(Vdq.d, "Vd_in");
    scope.connectChannel(Vdq_output.q, "Vq_out");
    scope.connectChannel(Vdq_output.d, "Vd_out");
    scope.connectChannel(Vab.alpha, "Valpha");
    scope.connectChannel(Vab.beta, "Vbeta");
    scope.connectChannel(Valpha_in_out, "Valpha(out-in)");
    scope.connectChannel(Vab_output.alpha, "ValphaOut");
    scope.connectChannel(Vab_output.beta, "VbetaOut");
    scope.set_delay(0.5F);
    scope.set_trigger(a_trigger);
    scope.start();

    // PR initialization
    inverter.init(local_mode, Udc, Vgrid_amplitude_ref, w0, Ts);

    sogi_v.init(500.0, Ts);
    sogi_i.init(500.0, Ts);

    Idq_ref.d = 0.0;
    Idq_ref.q = 0.0;
    Vdq_ref.d = 0.0;
    Vdq_ref.q = 0.0;

    Idq_ref_max.d = 8.0;
    Idq_ref_max.q = 1.0;
    Idq_ref_min.d = -0.1;
    Idq_ref_min.q = -0.1;

    Vdq_ref_max.d = 30.0;
    Vdq_ref_max.q = 30.0;
    Vdq_ref_min.d = -0.1;
    Vdq_ref_min.q = -0.1;



    Idq_ref_delta.d = 0.0;
    Idq_ref_delta.q = 0.0;



    pi_current_d.reset();
    pi_current_q.reset();
    pi_voltage_d.reset();
    pi_voltage_q.reset();
    is_net_synchronized = false;

    /* Buck voltage mode */
    shield.power.initBuck(LEG1_HIGH);
    shield.power.initBuck(LEG2_HIGH);

    // Then declare tasks
    uint32_t app_task_number = task.createBackground(loop_application_task);
    task.createCritical(loop_critical_task, control_task_period);

    // Finally, start tasks
    task.startBackground(app_task_number);
    task.startCritical();
}

//-------------- LOOP FUNCTIONS -------------------------------

/**
 * This is the code loop of the background task
 * It is executed second as defined by it suspend task in its last line.
 * You can use it to execute slow code such as state-machines.
 */
void loop_application_task()
{
/* --- STATE MACHINE --------------------------------------------------------*/
// mode is the STATE variable
// in each state we compute the transitions
switch (mode) {
        case IDLEMODE:

            if (mode_asked == POWERMODE) {
                mode = POWERMODE;
            }
            spin.led.turnOn();
        break;
        case STARTUPMODE:
            if (!inverter_on) {
                mode = POWERMODE;
            }
            if (local_mode == FORMING && delta_duty_cycle > 0.49F)
            {
                mode = POWERMODE;
            } 
            else if (local_mode == FOLLOWING && is_net_synchronized) 
            {
                mode = POWERMODE;
                if (is_net_synchronized) spin.led.toggle();
            }

        break;
        case POWERMODE:
            if (mode_asked == IDLEMODE) {
                mode = IDLEMODE;
            }
            if (inverter_on) {
                if (local_mode == FORMING) {
                    if (Vdc_bus_filt >= UDC_STARTUP) {
                        mode = STARTUPMODE;
                    } 
                } else {
                    if (Vgrid_meas >= 10 && Vdc_bus_filt >= UDC_STARTUP) {
                        mode = STARTUPMODE;
                    }
                }
                if (is_net_synchronized) spin.led.toggle();
            }
        break;
        case ERRORMODE:
        break;
    }
    if (mode_asked == IDLEMODE) {
        mode = IDLEMODE; // global return to idle possible
    }
/* --- END OF STATE MACHINE -------------------------------------------------*/

    if (mode == IDLEMODE)
    {
        if (is_downloading) {
            dump_scope_datas(scope);
            is_downloading = false;
        }
    }

    user_live.mode = mode;
    user_live.omega = omega;
    user_live.vgrid_amp_ref = Vgrid_amplitude_ref;
    user_live.power_d = power.d;
    user_live.power_q = power.q;
    user_live.idq_ref_d = Idq_ref.d;
    user_live.idq_ref_delta_d = Idq_ref_delta.d;
    user_live.vdq_ref_d = Vdq_ref.d;
    user_live.vdq_ref_q = Vdq_ref.q;
    task.suspendBackgroundMs(100);
}

/**
 * This is the code loop of the critical task
 * It is executed every 100 micro-seconds defined in the setup_software
 * function. You can use it to execute an ultra-fast code with the highest
 * priority which cannot be interruped.
 * It is from it that you will control your power flow.
 */
void loop_critical_task()
{
    critical_task_counter++;

    // Retrieve measurements
    meas_data = shield.sensors.getLatestValue(ILow1);
    if (meas_data != NO_VALUE) Ilow1_value = meas_data - I1_current_offset;

    meas_data = shield.sensors.getLatestValue(VLow);
    if (meas_data != NO_VALUE) Vlow_value = meas_data;

    meas_data = shield.sensors.getLatestValue(VAC);
    if (meas_data != NO_VALUE) Vac_value = meas_data;

    meas_data = shield.sensors.getLatestValue(ILow2);
    if (meas_data != NO_VALUE) Ilow2_value = meas_data - I2_current_offset;

    meas_data = shield.sensors.getLatestValue(VDCBus);
    if (meas_data != NO_VALUE) Vdc_bus = meas_data;

    meas_data = shield.sensors.getLatestValue(IAC);
    if (meas_data != NO_VALUE) Iac_value = meas_data;

    Vdc_bus_filt = vHighFilter.calculateWithReturn(Vdc_bus);

    Vgrid_meas = Vlow_value - Vac_value;
    VN_meas = (Vlow_value + Vac_value) / 2;
    Igrid_meas = Ilow1_value;

    user_meas.v_low = Vlow_value;
    user_meas.v_ac = Vac_value;
    user_meas.v_dc_bus = Vdc_bus;
    user_meas.i_low1 = Ilow1_value;
    user_meas.i_low2 = Ilow2_value;
    user_meas.i_ac = Iac_value;
    user_meas.v_dc_bus_filt = Vdc_bus_filt;
    user_meas.v_grid = Vgrid_meas;
    user_meas.v_n = VN_meas;
    user_meas.i_grid = Igrid_meas;

    // Overcurrent protection
    if (Ilow1_value > MAX_CURRENT
        || Ilow1_value < -MAX_CURRENT
        || Ilow2_value > MAX_CURRENT
        || Ilow2_value < -MAX_CURRENT)
    {
        mode = ERRORMODE;
    }


    if (mode == IDLEMODE || mode == ERRORMODE)
    {
        // FIRST WE STOP THE PWM
        if (pwm_enable == true)
        {
            shield.power.stop(ALL);
            spin.led.turnOff();
            pwm_enable = false;
        }
        if (boost_pwm_enable == true)
        {
            shield.power.stop(LEG1_LOW);
            shield.power.stop(LEG2_LOW);
            boost_pwm_enable = false;
        }
    }

    // Boost stage enabled in startup and power modes
    if (mode == STARTUPMODE || mode == POWERMODE) {
        boost_duty_cycle = boost_pid.calculateWithReturn(
            boost_voltage_reference,
            Vdc_bus_filt
        );
        shield.power.setDeadTime(LEG1_LOW, boost_pos_dt, boost_neg_dt);
        shield.power.setDeadTime(LEG2_LOW, boost_pos_dt, boost_neg_dt);
        shield.power.setDutyCycle(LEG1_LOW, boost_duty_cycle);
        shield.power.setDutyCycle(LEG2_LOW, boost_duty_cycle);
        if (!boost_pwm_enable)
        {
            shield.power.start(LEG1_LOW);
            shield.power.start(LEG2_LOW);
            boost_pwm_enable = true;
        }
    }

    // Stop inverter legs when not enabled
    if (!inverter_on) {
        if (pwm_enable == true)
        {
            shield.power.stop(LEG1_HIGH);
            shield.power.stop(LEG2_HIGH);
            pwm_enable = false;
        }
    }

    // Startup ramp and synchronization logic
    // Ramp up the common voltage to Udc/2
    if (mode == STARTUPMODE && inverter_on) {

        if (local_mode == FORMING) {

            // Ramp of 50/s
            delta_duty_cycle = rate_limiter(0.5F,
                                            delta_duty_cycle,
                                            50.0F);
            if (delta_duty_cycle > 0.5F) {
                delta_duty_cycle = 0.5F;
            }
            shield.power.setDutyCycle(LEG2_HIGH, 1 - delta_duty_cycle);
            shield.power.setDutyCycle(LEG1_HIGH, delta_duty_cycle);
            // WE START THE PWM
            if (!pwm_enable)
            {
                shield.power.start(ALL);
                pwm_enable = true;
            }

        } else {
            inverter.calculateDuty(Vgrid_meas, Igrid_meas);
            Vdq = inverter.getVdq();
            omega = inverter.getw();

            if (omega < w0 + sync_power_tolerance &&
                omega > w0 - sync_power_tolerance)
            {
                sync_counter++;
                if (sync_counter > 2000) {
                    is_net_synchronized = true;
                    sync_counter = 0;                                    
                }
            } else {
                sync_counter = 0;
                is_net_synchronized = false;                
            }

        }
    }

    // Closed-loop control in power mode
    if (mode == POWERMODE && inverter_on)
    {
        delta_duty_cycle = inverter.calculateDuty(Vgrid_meas, Igrid_meas);
        omega = inverter.getw();

        is_net_synchronized = omega <= w0 + sync_power_tolerance &&
                              omega >= w0 - sync_power_tolerance;

        if (!is_net_synchronized)
        {
            desync_counter++;
            desync_counter_scope = (float32_t)desync_counter;
            if (desync_counter > 200) {
                desync_counter = 0;
                sync_counter = 0;
                mode_asked = IDLEMODE;
                mode = IDLEMODE;
                printk("System no longer synchronized \n");
            }                
        }

        inverter.setVBus(Vdc_bus_filt);

        if (local_mode == FORMING) {
            inverter.setVdqRef(Vdq_ref);
        } else {
            inverter.setIdqRef(Idq_ref);
        }

        if (!pwm_enable)
        {        
            duty_cycle_offset = VN_meas/Vdc_bus_filt;        
        } 
        else
        {
            if (duty_cycle_offset < 0.5F) {
                // Ramp of 0.1 duty / 100 ms
                duty_cycle_offset = rate_limiter(0.5F,
                                                 duty_cycle_offset,
                                                 1.0F);
                
            } else {
                duty_cycle_offset = 0.5F;
            }

        }


        duty_cycle_1 = delta_duty_cycle + duty_cycle_offset;
        duty_cycle_2 = -delta_duty_cycle + duty_cycle_offset;
        
        if (local_mode == FOLLOWING && !pwm_enable)
        {
            power_counter++;
            if (power_counter > 2000) {
                shield.power.start(ALL);
                pwm_enable = true;
            }
        }



        shield.power.setDutyCycle(LEG1_HIGH, duty_cycle_1);
        shield.power.setDutyCycle(LEG2_HIGH, duty_cycle_2);

    }

    /* Retrieve multiple data for debugging */
    theta = inverter.getTheta();
    Vdq = inverter.getVdq();
    Vq_filtered = VqFilter.calculateWithReturn(Vdq.q);
    Vdq_output = inverter.getVdqOut();
    Vab = inverter.getVab();
    Vab_output = inverter.getVabOutput();
    Iab = inverter.getIab();
    Idq = inverter.getIdq();
    Idq_ref_delta = inverter.getIdqRefDelta();
    omega = inverter.getw();
    Valpha_in_out = Vab_output.alpha - Vab.alpha; 

    user_inv_dbg.theta = theta;
    user_inv_dbg.vab_alpha = Vab.alpha;
    user_inv_dbg.vab_beta = Vab.beta;
    user_inv_dbg.vab_out_alpha = Vab_output.alpha;
    user_inv_dbg.vab_out_beta = Vab_output.beta;
    user_inv_dbg.iab_alpha = Iab.alpha;
    user_inv_dbg.iab_beta = Iab.beta;
    user_inv_dbg.vdq_d = Vdq.d;
    user_inv_dbg.vdq_q = Vdq.q;
    user_inv_dbg.vdq_out_d = Vdq_output.d;
    user_inv_dbg.vdq_out_q = Vdq_output.q;
    user_inv_dbg.idq_d = Idq.d;
    user_inv_dbg.idq_q = Idq.q;

    user_boost_dbg.duty_leg1 = boost_duty_cycle;
    user_boost_dbg.duty_leg2 = boost_duty_cycle;
    user_boost_dbg.dt_rise_ns = boost_pos_dt;
    user_boost_dbg.dt_fall_ns = boost_neg_dt;


    if (critical_task_counter % decimation == 0) {
        spying_mode = (float32_t) mode;
        scope.acquire();
    }
}

/**
 * This is the main function of this example
 * This function is generic and does not need editing.
 */
int main(void)
{
    setup_routine();

    return 0;
}
