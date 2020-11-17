/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#include "marshaller.h"

#include <stdlib.h>
#include <memory.h>
#include <errno.h>
#include <endian.h>
#include <byteswap.h>

static __always_inline void add_bytes(struct value *const value, const void *const bytes, const byte marker, const usize size) {
    usize pos = value->size;

    value->type = BYTES;
    value->size += size + 2;
    value->value = realloc(value->value, value->size);

    ((byte *) value->value)[pos] = marker;
    ((byte *) value->value)[pos + 1] = (uint_8) size;
    memcpy(&(((byte *) value->value)[pos + 2]), bytes, size);
}

static __always_inline void get_bytes(void *const value, void **const bytes, byte *const marker, uint_8 *const size) {
    *marker = ((byte *) value)[0];
    *size = ((byte *) value)[1];

    *bytes = realloc(*bytes, *size);
    memcpy(*bytes, &(((byte *) value)[2]), *size);
}

void marshall(const struct data *const data, const char *const service, const char *const method, struct value *const value) {
    uint_8 i;
    const struct value *d_value;

    if (service)
        add_bytes(value, service, 'S', strlen(service));

    if (method)
        add_bytes(value, method, 'M', strlen(method));

    for (i = 0; (d_value = data_get_value(data, i)) != NULL; ++i) {
        if (d_value->size > 255) {
            value->size = 0;
            errno = EINVAL;
            return;
        }

#if __BYTE_ORDER == __BIG_ENDIAN
        if ((d_value->type == INT || d_value->type == UINT) && d_value->size > 1) {
            switch (d_value->size) {
                case 2: { /* uint_16 */
                    uint_16 v = __bswap_16(*((uint_16 *) d_value->value));
                    add_bytes(value, &v, d_value->type == INT ? 'I' : 'U', d_value->size);
                    continue;
                }
                case 4: { /* uint_32 */
                    uint_32 v = __bswap_32(*((uint_32 *) d_value->value));
                    add_bytes(value, &v, d_value->type == INT ? 'I' : 'U', d_value->size);
                    continue;
                }
                case 8: { /* uint_64 */
                    uint_64 v = __bswap_64(*((uint_64 *) d_value->value));
                    add_bytes(value, &v, d_value->type == INT ? 'I' : 'U', d_value->size);
                    continue;
                }
            }
        }
#endif

        add_bytes(value, d_value->value, d_value->type == BYTES ? 'B' : (d_value->type == INT ? 'I' : 'U'), d_value->size);
    }
}

void unmarshall(const struct value *const value, char **const service, char **const method, struct data **const data) {
    uint_8 i;
    void *bytes = NULL;
    byte marker;
    uint_8 size = 0;
    enum type type;

    if (value->size > 0)
        *data = data_new(1);

    for (i = 0; i < value->size; i += (2 + size)) {
        get_bytes(((byte *) value->value) + i, &bytes, &marker, &size);

        switch (marker) {
            case 'S':
                if (service != NULL) {
                    *service = malloc(size + 1);
                    memcpy(*service, bytes, size);
                    (*service)[size] = '\0';
                }
                continue;
            case 'M':
                if (method != NULL) {
                    *method = malloc(size + 1);
                    memcpy(*method, bytes, size);
                    (*method)[size] = '\0';
                }
                continue;
            case 'B':
                type = BYTES;
                break;
            case 'I':
                type = INT;
                break;
            case 'U':
                type = UINT;
                break;
            default:
                data_destroy(*data);
                *data = NULL;
                free(bytes);
                errno = EINVAL;
                return;
        }

#if __BYTE_ORDER == __BIG_ENDIAN
        if ((type == INT || type == UINT) && size > 1) {
            switch (size) {
                case 2: { /* uint_16 */
                    uint_16 v = __bswap_16(*((uint_16 *) bytes));
                    data_push(*data, type, size, &v);
                    continue;
                }
                case 4: { /* uint_32 */
                    uint_32 v = __bswap_32(*((uint_32 *) bytes));
                    data_push(*data, type, size, &v);
                    continue;
                }
                case 8: { /* uint_64 */
                    uint_64 v = __bswap_64(*((uint_64 *) bytes));
                    data_push(*data, type, size, &v);
                    continue;
                }
            }
        }
#endif

        data_push(*data, type, size, bytes);
    }

    free(bytes);
}
