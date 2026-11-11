#ifndef ER_OTA_H
#define ER_OTA_H
#include <stdint.h>
#include <stdbool.h>

#define ER_OTA_SLOTS 2
#define ER_OTA_HASH_BYTES 32
#define ER_OTA_SIGNATURE_BYTES 64

typedef enum { ER_OTA_EMPTY=0, ER_OTA_READY, ER_OTA_BOOTING, ER_OTA_CONFIRMED, ER_OTA_INVALID } er_ota_slot_state_t;
typedef enum { ER_OTA_OK=0, ER_OTA_EINVAL=-1, ER_OTA_ESTATE=-2, ER_OTA_EVERIFY=-3, ER_OTA_ERANGE=-4 } er_ota_status_t;
typedef struct {
    uint32_t version; uint32_t image_size; uint32_t hardware_id;
    uint8_t hash[ER_OTA_HASH_BYTES]; uint8_t signature[ER_OTA_SIGNATURE_BYTES];
} er_ota_manifest_t;
typedef struct { er_ota_slot_state_t state; er_ota_manifest_t manifest; uint32_t received; } er_ota_slot_t;
typedef bool (*er_ota_verify_fn)(const er_ota_manifest_t *manifest, void *user);
typedef struct { er_ota_slot_t slots[ER_OTA_SLOTS]; int active_slot; int pending_slot; uint32_t running_version; bool download_active; er_ota_verify_fn verify; void *verify_user; } er_ota_t;

void er_ota_init(er_ota_t *ota, uint32_t running_version, er_ota_verify_fn verify, void *user);
er_ota_status_t er_ota_begin(er_ota_t *ota, int slot, const er_ota_manifest_t *manifest);
er_ota_status_t er_ota_write(er_ota_t *ota, int slot, uint32_t offset, uint32_t length);
er_ota_status_t er_ota_finalize(er_ota_t *ota, int slot);
er_ota_status_t er_ota_select_boot(er_ota_t *ota, int slot);
er_ota_status_t er_ota_confirm_running(er_ota_t *ota, uint32_t version);
void er_ota_mark_boot_failed(er_ota_t *ota);
const er_ota_slot_t *er_ota_slot(const er_ota_t *ota, int slot);
#endif
