#include "auxiliary.h"

#include "TaskAPI.h"
#include "ShieldAPI.h"
#include "SpinAPI.h"
#include "singlePhaseInverter.h"
#include "user_data_api.h"
#include <zephyr/sys/printk.h>

extern ScopeMimicry scope;
extern bool is_downloading;
extern bool trigger;
extern bool inverter_on;
extern inverter_mode local_mode;
extern dqo_t Vdq_ref_max;
extern dqo_t Vdq_ref_min;
extern dqo_t Vdq_ref;
extern dqo_t Idq_ref_max;
extern dqo_t Idq_ref_min;
extern dqo_t Idq_ref;
extern float32_t Ts;
extern uint8_t mode_asked;

bool a_trigger()
{
    return trigger;
}

void dump_scope_datas(ScopeMimicry &scope)
{
    scope.reset_dump();
    printk("begin record\n");
    while (scope.get_dump_state() != finished) {
        printk("%s", scope.dump_datas());
        task.suspendBackgroundUs(100);
    }
    printk("end record\n");
}

void app_apply_command(void)
{
    switch (user_cmd.mode_request) {
        case IDLEMODE:
            mode_asked = IDLEMODE;
            break;
        case POWERMODE:
            if (!is_downloading) {
                scope.start();
            }
            mode_asked = POWERMODE;
            break;
        default:
            break;
    }

    inverter_on = user_cmd.inverter_on;

    if (local_mode == FORMING) {
        float32_t vd = user_cmd.vd_ref;
        if (vd > Vdq_ref_max.d) {
            vd = Vdq_ref_max.d;
        }
        if (vd < Vdq_ref_min.d) {
            vd = Vdq_ref_min.d;
        }
        Vdq_ref.d = vd;
        user_cmd.vd_ref = vd;
    } else {
        float32_t id = user_cmd.id_ref;
        if (id > Idq_ref_max.d) {
            id = Idq_ref_max.d;
        }
        if (id < Idq_ref_min.d) {
            id = Idq_ref_min.d;
        }
        Idq_ref.d = id;
        user_cmd.id_ref = id;
    }

    if (user_cmd.scope_dump) {
        is_downloading = true;
        trigger = false;
        user_cmd.scope_dump = false;
    }
    if (user_cmd.scope_trigger) {
        trigger = true;
        user_cmd.scope_trigger = false;
    }
}

float32_t saturate(const float32_t x, float32_t min, float32_t max)
{
    if (x > max) {
        return max;
    }
    if (x < min) {
        return min;
    }
    return x;
}

float32_t sign(float32_t x, float32_t tol)
{
    if (x > tol) {
        return 1.0F;
    }
    if (x < -tol) {
        return -1.0F;
    }
    return 0.0F;
}

float32_t rate_limiter(const float32_t ref,
                       float32_t value,
                       const float32_t rate)
{
    value += Ts * rate * sign(ref - value);
    return value;
}

void enableUSolarVerterSensors()
{
    spin.data.configureTriggerSource(ADC_1, TRIG_PWM);
    spin.data.configureTriggerSource(ADC_2, TRIG_PWM);
    spin.data.configureTriggerSource(ADC_3, TRIG_SOFTWARE);
    spin.data.configureTriggerSource(ADC_4, TRIG_SOFTWARE);
    spin.data.configureTriggerSource(ADC_5, TRIG_SOFTWARE);

    spin.data.configureDiscontinuousMode(ADC_1, 1);
    spin.data.configureDiscontinuousMode(ADC_2, 1);

    shield.sensors.enableSensor(ILow1, ADC_1);
    shield.sensors.enableSensor(VLow, ADC_1);
    shield.sensors.enableSensor(VDCBus, ADC_1);

    shield.sensors.enableSensor(ILow2, ADC_2);
    shield.sensors.enableSensor(VAC, ADC_2);
    shield.sensors.enableSensor(IAC, ADC_2);
}
