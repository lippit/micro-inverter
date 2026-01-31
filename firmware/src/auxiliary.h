#ifndef AUXILIARY_H
#define AUXILIARY_H

#include "ScopeMimicry.h"

/**
 * @brief List of possible modes for the OwnTech converter.
 */
enum serial_interface_menu_mode
{
    IDLEMODE = 0,
    POWERMODE = 1,
    ERRORMODE = 3,
    STARTUPMODE = 4
};

/**
 * @brief Scope trigger callback used by ScopeMimicry.
 *
 * @return true when the capture trigger is active.
 */
bool a_trigger();

/**
 * @brief Print recorded data of the ScopeMimicry instance to console.
 * We use this function in coordination with a miniterm python filter on the
 * host side. `filter_recorded_data.py` saves the data in a file and formats
 * them as floats.
 *
 * @param scope Scope instance to dump.
 */
void dump_scope_datas(ScopeMimicry &scope);

/**
 * @brief Apply and clamp user commands to internal references/state.
 */
void app_apply_command(void);

/**
 * @brief Clamp a value between a minimum and maximum.
 *
 * @param x Input value.
 * @param min Lower bound.
 * @param max Upper bound.
 * @return Clamped value.
 */
float32_t saturate(const float32_t x, float32_t min, float32_t max);

/**
 * @brief Return the sign of a value with a deadband.
 *
 * @param x Input value.
 * @param tol Deadband threshold around zero.
 * @return -1, 0, or 1 depending on the sign of x.
 */
float32_t sign(float32_t x, float32_t tol = 1e-3);

/**
 * @brief Apply a symmetric rate limit to a reference value.
 *
 * @param ref Desired reference.
 * @param value Current value (previous output).
 * @param rate Maximum rate of change (units per second).
 * @return Updated value after rate limiting.
 */
float32_t rate_limiter(const float32_t ref,
                       float32_t value,
                       const float32_t rate);

/**
 * @brief Configure the uSolarVerter sensor sampling and triggers.
 */
void enableUSolarVerterSensors();

#endif // AUXILIARY_H
