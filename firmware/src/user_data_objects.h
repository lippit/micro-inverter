#ifndef USER_DATA_OBJECTS_H
#define USER_DATA_OBJECTS_H

#include <thingset.h>
#include <thingset/sdk.h>

#include "user_data_api.h"

/* =========================================================================
 * Backing storage
 * ========================================================================= */

measurements_t user_meas = {0};
inverter_debug_t user_inv_dbg = {0};
boost_debug_t user_boost_dbg = {0};
command_t user_cmd = {
    .mode_request = 0,
    .inverter_on = false,
    .vd_ref = 0.0f,
    .id_ref = 0.0f,
    .scope_dump = false,
    .scope_trigger = false,
};
live_status_t user_live = {0};

/* =========================================================================
 * Callbacks
 * ========================================================================= */

void conf_command_cb(enum thingset_callback_reason reason);

/* =========================================================================
 * ID map
 * ========================================================================= */

#define ID_MEAS         0x10
#define ID_MEAS_VAL     0x11

#define ID_DBG          0x20
#define ID_DBG_INV      0x21
#define ID_DBG_BOOST    0x22

#define ID_CMD          0x30
#define ID_LIVE         0x40

/* =========================================================================
 * Measurements
 * ========================================================================= */

THINGSET_ADD_GROUP(TS_ID_ROOT, ID_MEAS, "Measurements", THINGSET_NO_CALLBACK);
THINGSET_ADD_GROUP(ID_MEAS, ID_MEAS_VAL, "rValues", THINGSET_NO_CALLBACK);

THINGSET_ADD_ITEM_FLOAT(ID_MEAS_VAL, 0x1101, "rVLow_V",     &user_meas.v_low,        3, THINGSET_ANY_R, TS_SUBSET_LIVE);
THINGSET_ADD_ITEM_FLOAT(ID_MEAS_VAL, 0x1102, "rVac_V",      &user_meas.v_ac,         3, THINGSET_ANY_R, 0);
THINGSET_ADD_ITEM_FLOAT(ID_MEAS_VAL, 0x1103, "rVdc_V",      &user_meas.v_dc_bus,     3, THINGSET_ANY_R, 0);
THINGSET_ADD_ITEM_FLOAT(ID_MEAS_VAL, 0x1104, "rILow1_A",    &user_meas.i_low1,       3, THINGSET_ANY_R, TS_SUBSET_LIVE);
THINGSET_ADD_ITEM_FLOAT(ID_MEAS_VAL, 0x1105, "rILow2_A",    &user_meas.i_low2,       3, THINGSET_ANY_R, TS_SUBSET_LIVE);
THINGSET_ADD_ITEM_FLOAT(ID_MEAS_VAL, 0x1106, "rIac_A",      &user_meas.i_ac,         3, THINGSET_ANY_R, 0);
THINGSET_ADD_ITEM_FLOAT(ID_MEAS_VAL, 0x1107, "rVdcFilt_V",  &user_meas.v_dc_bus_filt,3, THINGSET_ANY_R, TS_SUBSET_LIVE);
THINGSET_ADD_ITEM_FLOAT(ID_MEAS_VAL, 0x1108, "rVgrid_V",    &user_meas.v_grid,       3, THINGSET_ANY_R, 0);
THINGSET_ADD_ITEM_FLOAT(ID_MEAS_VAL, 0x1109, "rVn_V",       &user_meas.v_n,          3, THINGSET_ANY_R, TS_SUBSET_LIVE);
THINGSET_ADD_ITEM_FLOAT(ID_MEAS_VAL, 0x110A, "rIgrid_A",    &user_meas.i_grid,       3, THINGSET_ANY_R, 0);

/* =========================================================================
 * Debug
 * ========================================================================= */

THINGSET_ADD_GROUP(TS_ID_ROOT, ID_DBG, "Debug", THINGSET_NO_CALLBACK);
THINGSET_ADD_GROUP(ID_DBG, ID_DBG_INV, "Inverter", THINGSET_NO_CALLBACK);
THINGSET_ADD_GROUP(ID_DBG, ID_DBG_BOOST, "Boost", THINGSET_NO_CALLBACK);

THINGSET_ADD_ITEM_FLOAT(ID_DBG_INV, 0x2101, "rTheta_rad",     &user_inv_dbg.theta,         5, THINGSET_ANY_R, 0);
THINGSET_ADD_ITEM_FLOAT(ID_DBG_INV, 0x2102, "rVab_alpha",     &user_inv_dbg.vab_alpha,     5, THINGSET_ANY_R, 0);
THINGSET_ADD_ITEM_FLOAT(ID_DBG_INV, 0x2103, "rVab_beta",      &user_inv_dbg.vab_beta,      5, THINGSET_ANY_R, 0);
THINGSET_ADD_ITEM_FLOAT(ID_DBG_INV, 0x2104, "rVabOut_alpha",  &user_inv_dbg.vab_out_alpha, 5, THINGSET_ANY_R, 0);
THINGSET_ADD_ITEM_FLOAT(ID_DBG_INV, 0x2105, "rVabOut_beta",   &user_inv_dbg.vab_out_beta,  5, THINGSET_ANY_R, 0);
THINGSET_ADD_ITEM_FLOAT(ID_DBG_INV, 0x2106, "rIab_alpha",     &user_inv_dbg.iab_alpha,     5, THINGSET_ANY_R, 0);
THINGSET_ADD_ITEM_FLOAT(ID_DBG_INV, 0x2107, "rIab_beta",      &user_inv_dbg.iab_beta,      5, THINGSET_ANY_R, 0);
THINGSET_ADD_ITEM_FLOAT(ID_DBG_INV, 0x2108, "rVdq_d",         &user_inv_dbg.vdq_d,         5, THINGSET_ANY_R, TS_SUBSET_LIVE);
THINGSET_ADD_ITEM_FLOAT(ID_DBG_INV, 0x2109, "rVdq_q",         &user_inv_dbg.vdq_q,         5, THINGSET_ANY_R, TS_SUBSET_LIVE);
THINGSET_ADD_ITEM_FLOAT(ID_DBG_INV, 0x210A, "rVdqOut_d",      &user_inv_dbg.vdq_out_d,     5, THINGSET_ANY_R, TS_SUBSET_LIVE);
THINGSET_ADD_ITEM_FLOAT(ID_DBG_INV, 0x210B, "rVdqOut_q",      &user_inv_dbg.vdq_out_q,     5, THINGSET_ANY_R, TS_SUBSET_LIVE);
THINGSET_ADD_ITEM_FLOAT(ID_DBG_INV, 0x210C, "rIdq_d",         &user_inv_dbg.idq_d,         5, THINGSET_ANY_R, TS_SUBSET_LIVE);
THINGSET_ADD_ITEM_FLOAT(ID_DBG_INV, 0x210D, "rIdq_q",         &user_inv_dbg.idq_q,         5, THINGSET_ANY_R, 0);

THINGSET_ADD_ITEM_FLOAT(ID_DBG_BOOST, 0x2201, "rDutyLeg1",    &user_boost_dbg.duty_leg1,   5, THINGSET_ANY_R, 0);
THINGSET_ADD_ITEM_FLOAT(ID_DBG_BOOST, 0x2202, "rDutyLeg2",    &user_boost_dbg.duty_leg2,   5, THINGSET_ANY_R, 0);
THINGSET_ADD_ITEM_UINT16(ID_DBG_BOOST,0x2203, "rDTRise_ns",   &user_boost_dbg.dt_rise_ns,     THINGSET_ANY_R, 0);
THINGSET_ADD_ITEM_UINT16(ID_DBG_BOOST,0x2204, "rDTFall_ns",   &user_boost_dbg.dt_fall_ns,     THINGSET_ANY_R, 0);

/* =========================================================================
 * Commands
 * ========================================================================= */

THINGSET_ADD_GROUP(TS_ID_ROOT, ID_CMD, "Command", &conf_command_cb);
THINGSET_ADD_ITEM_UINT8(ID_CMD, 0x3001, "wMode",       &user_cmd.mode_request, THINGSET_ANY_RW, 0);
THINGSET_ADD_ITEM_BOOL(ID_CMD,  0x3002, "wInverterOn", &user_cmd.inverter_on,  THINGSET_ANY_RW, 0);
THINGSET_ADD_ITEM_FLOAT(ID_CMD, 0x3003, "wVdRef",      &user_cmd.vd_ref,       3, THINGSET_ANY_RW, 0);
THINGSET_ADD_ITEM_FLOAT(ID_CMD, 0x3004, "wIdRef",      &user_cmd.id_ref,       3, THINGSET_ANY_RW, 0);
THINGSET_ADD_ITEM_BOOL(ID_CMD,  0x3005, "wDump",       &user_cmd.scope_dump,   THINGSET_ANY_RW, 0);
THINGSET_ADD_ITEM_BOOL(ID_CMD,  0x3006, "wTrig",       &user_cmd.scope_trigger,THINGSET_ANY_RW, 0);

/* =========================================================================
 * Live status (mirrors the previously printed loop values)
 * ========================================================================= */

THINGSET_ADD_GROUP(TS_ID_ROOT, ID_LIVE, "Live", THINGSET_NO_CALLBACK);
THINGSET_ADD_ITEM_UINT8(ID_LIVE, 0x4001, "rMode",        &user_live.mode,          THINGSET_ANY_R, TS_SUBSET_LIVE);
THINGSET_ADD_ITEM_FLOAT(ID_LIVE, 0x4002, "rOmega_rps",   &user_live.omega,         3, THINGSET_ANY_R, TS_SUBSET_LIVE);
THINGSET_ADD_ITEM_FLOAT(ID_LIVE, 0x4003, "rVgridRef_V",  &user_live.vgrid_amp_ref, 3, THINGSET_ANY_R, TS_SUBSET_LIVE);
THINGSET_ADD_ITEM_FLOAT(ID_LIVE, 0x4004, "rP_d",         &user_live.power_d,       3, THINGSET_ANY_R, TS_SUBSET_LIVE);
THINGSET_ADD_ITEM_FLOAT(ID_LIVE, 0x4005, "rP_q",         &user_live.power_q,       3, THINGSET_ANY_R, TS_SUBSET_LIVE);
THINGSET_ADD_ITEM_FLOAT(ID_LIVE, 0x4006, "rIdRef",       &user_live.idq_ref_d,     3, THINGSET_ANY_R, TS_SUBSET_LIVE);
THINGSET_ADD_ITEM_FLOAT(ID_LIVE, 0x4007, "rIdDelta",     &user_live.idq_ref_delta_d, 3, THINGSET_ANY_R, TS_SUBSET_LIVE);
THINGSET_ADD_ITEM_FLOAT(ID_LIVE, 0x4008, "rVdRef",       &user_live.vdq_ref_d,     3, THINGSET_ANY_R, TS_SUBSET_LIVE);
THINGSET_ADD_ITEM_FLOAT(ID_LIVE, 0x4009, "rVqRef",       &user_live.vdq_ref_q,     3, THINGSET_ANY_R, TS_SUBSET_LIVE);

#endif /* USER_DATA_OBJECTS_H */
