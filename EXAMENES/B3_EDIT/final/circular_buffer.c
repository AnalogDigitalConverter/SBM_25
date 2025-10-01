typedef struct {
    uint8_t *buffer;
    uint16_t head;
    uint16_t tail;
    uint16_t size;
} circular_buffer_t;

void circular_buffer_init(circular_buffer_t *cb, uint8_t *buffer, uint16_t size) {
    cb->buffer = buffer;
    cb->head = 0;
    cb->tail = 0;
    cb->size = size;
}

uint16_t circular_buffer_is_empty(circular_buffer_t *cb) {
    return cb->head == cb->tail;
}

uint16_t circular_buffer_is_full(circular_buffer_t *cb) {
    return (cb->head + 1) % cb->size == cb->tail;
}

uint16_t circular_buffer_write(circular_buffer_t *cb, uint8_t data) {
    if (circular_buffer_is_full(cb)) {
        return 0; // Buffer is full
    }

    cb->buffer[cb->head] = data;
    cb->head = (cb->head + 1) % cb->size;
    return 1;
}

uint16_t circular_buffer_read(circular_buffer_t *cb, uint8_t *data) {
    if (circular_buffer_is_empty(cb)) {
        return 0; // Buffer is empty
    }

    *data = cb->buffer[cb->tail];
    cb->tail = (cb->tail + 1) % cb->size;
    return 1;
}