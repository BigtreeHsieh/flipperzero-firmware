#include <furi_hal_irda.h>
#include <irda.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Send message over IRDA.
 *
 * \param[in]   message     - message to send.
 * \param[in]   times       - number of times message should be sent.
 */
void irda_send(const IrdaMessage* message, int times);

/**
 * Send raw data through infrared port.
 *
 * \param[in]   timings - array of timings to send.
 * \param[in]   timings_cnt - timings array size.
 * \param[in]   start_from_mark - true if timings starts from mark,
 *              otherwise from space
 */
void irda_send_raw(const uint32_t timings[], uint32_t timings_cnt, bool start_from_mark);

/**
 * Send raw data through infrared port, with additional settings.
 *
 * \param[in]   timings - array of timings to send.
 * \param[in]   timings_cnt - timings array size.
 * \param[in]   start_from_mark - true if timings starts from mark,
 *              otherwise from space
 * \param[in]   duty_cycle - duty cycle to generate on PWM
 * \param[in]   frequency - frequency to generate on PWM
 */
void irda_send_raw_ext(
    const uint32_t timings[],
    uint32_t timings_cnt,
    bool start_from_mark,
    uint32_t frequency,
    float duty_cycle);

#ifdef __cplusplus
}
#endif
