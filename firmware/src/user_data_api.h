#ifndef USER_DATA_API_H
#define USER_DATA_API_H

#include <stdint.h>
#include <stdbool.h>

#include "ShieldAPI.h"

typedef struct {
    float32_t v_low;
    float32_t v_ac;
    float32_t v_dc_bus;
    float32_t i_low1;
    float32_t i_low2;
    float32_t i_ac;
    float32_t v_dc_bus_filt;
    float32_t v_grid;
    float32_t v_n;
    float32_t i_grid;
} measurements_t;

typedef struct {
    float32_t theta;
    float32_t vab_alpha;
    float32_t vab_beta;
    float32_t vab_out_alpha;
    float32_t vab_out_beta;
    float32_t iab_alpha;
    float32_t iab_beta;
    float32_t vdq_d;
    float32_t vdq_q;
    float32_t vdq_out_d;
    float32_t vdq_out_q;
    float32_t idq_d;
    float32_t idq_q;
} inverter_debug_t;

typedef struct {
    float32_t duty_leg1;
    float32_t duty_leg2;
    uint16_t dt_rise_ns;
    uint16_t dt_fall_ns;
} boost_debug_t;

typedef struct {
    uint8_t mode_request;
    bool inverter_on;
    float32_t vd_ref;
    float32_t id_ref;
    bool scope_dump;
    bool scope_trigger;
} command_t;

typedef struct {
    uint8_t mode;
    float32_t omega;
    float32_t vgrid_amp_ref;
    float32_t power_d;
    float32_t power_q;
    float32_t idq_ref_d;
    float32_t idq_ref_delta_d;
    float32_t vdq_ref_d;
    float32_t vdq_ref_q;
} live_status_t;

extern measurements_t user_meas;
extern inverter_debug_t user_inv_dbg;
extern boost_debug_t user_boost_dbg;
extern command_t user_cmd;
extern live_status_t user_live;

void app_apply_command(void);

#endif // USER_DATA_API_H
