/**
 * @file
 *
 * @author
 *
 * @date
 *
 * @brief
 *
 * @see
 *
 * @copyright
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */
#include "util/input_buf.h"

#include "presentation/print.h"

volatile InputBuffer g_input_buf;

static bool putIntoBuf(volatile InputBuffer *ib, uint8_t byte);

void input_onInterrupt(const uint8_t input)
{
    if(!putIntoBuf(&g_input_buf, input))
    {
        //!TODO Handle error better
        print("INPUT BUFFER OVERFLOW");
    }
}

void input_init(volatile InputBuffer *ib)
{
    ib->head = 0;
    ib->tail = 0;
}

bool input_isEmpty(const volatile InputBuffer *ib)
{
    return ib->head == ib->tail;
}

bool input_isFull(const volatile InputBuffer *ib)
{
    return (ib->head + 1) % INPUT_BUF_SIZE == ib->tail;
}

bool input_getNext(volatile InputBuffer *ib, uint8_t *byte)
{
    if(input_isEmpty(ib))
    {
        return false;
    }
    *byte = ib->data[ib->tail];
    ib->tail = (ib->tail + 1) % INPUT_BUF_SIZE;
    return true;
}

/**
 * @brief Put the next byte into the buffer
 * @param[inout] ib
 * InputBuffer
 * @param[out] byte
 * Next Byte
 * @return False if full
 */
bool putIntoBuf(volatile InputBuffer *ib, const uint8_t byte)
{
    if(input_isFull(ib))
    {
        return false;
    }

    ib->data[ib->head] = byte;
    ib->head = (ib->head + 1) % INPUT_BUF_SIZE;
    return true;
}
