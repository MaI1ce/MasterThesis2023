#ifndef VERIFY_H
#define VERIFY_H

#include <stdlib.h>

#include "ds2/params.h"
#include "ds2/party.h"
#include "ds2/poly.h"
#include "ds2/sign.h"

uint8_t verify(const party_t *party, const uint8_t *msg, size_t msg_len, const sign_t *sig);

#endif