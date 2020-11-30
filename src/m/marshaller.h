#ifndef CSOCKET_MARSHALLER_H
#define CSOCKET_MARSHALLER_H

#include "data.h"

void marshall(const struct data *, const char *service, const char *method, struct value *);

void unmarshall(const struct value *, char **service, char **method, struct data **);

void marshall_free(struct value *value);

void unmarshall_free(char **service, char **method, struct data **);

#endif /* CSOCKET_MARSHALLER_H */
