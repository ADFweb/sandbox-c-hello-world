#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <modbus/modbus.h>

#define NB_REGS 2

//Copy libmodbus.so.5.1.0 to the sandbox and run with the following environment variable set
//LD_PRELOAD=/home/sandbox/libmodbus.so.5.1.0

int modbus_master() {
    modbus_t *ctx = modbus_new_rtu("/dev/uart0", 9600, 'N', 8, 1);
    if (ctx == nullptr) {
        perror("Unable to create the libmodbus context\n");
        return -1;
    }

    modbus_set_debug(ctx, TRUE);

    modbus_set_response_timeout(ctx, 3, 0);

    int ret = modbus_set_slave(ctx, 1);
    if (ret < 0) {
        perror("modbus_set_slave error\n");
        return -1;
    }

    ret = modbus_connect(ctx);
    if (ret < 0) {
        perror("modbus_connect error\n");
        return -1;
    }

    ret = modbus_rtu_set_serial_mode(ctx, MODBUS_RTU_RS485);
    if (ret < 0) {
        perror("modbus_rtu_set_serial_mode error\n");
        return -1;
    }

    uint16_t dest[NB_REGS];
    // Read 2 registers from adress 0
    ret = modbus_read_registers(ctx, 0, NB_REGS, dest);
    if (ret < 0) {
        perror("modbus_read_regs error\n");
        return -1;
    }

    printf("Register 1 = %d\n", dest[0]);
    printf("Register 2 = %d\n", dest[1]);

    dest[0] = 1;
    dest[1] = 2;
    ret = modbus_write_registers(ctx, 0, NB_REGS, dest);
    if (ret < 0) {
        perror("modbus_write_regs error\n");
        return -1;
    }

    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}

int modbus_slave() {
    modbus_t *ctx = modbus_new_rtu("/dev/uart0", 9600, 'N', 8, 1);
    if (ctx == nullptr) {
        perror("Unable to create the libmodbus context");
        return -1;
    }

    std::vector<uint8_t> request(MODBUS_RTU_MAX_ADU_LENGTH);
    modbus_set_debug(ctx, TRUE);

    int ret = modbus_set_slave(ctx, 1); //Set slave address
    if (ret < 0) {
        perror("modbus_set_slave error");
        return -1;
    }

    ret = modbus_connect(ctx);
    if (ret < 0) {
        perror("modbus_connect error");
        return -1;
    }

    ret = modbus_rtu_set_serial_mode(ctx, MODBUS_RTU_RS485);
    if (ret < 0) {
        perror("modbus_rtu_set_serial_mode error\n");
        return -1;
    }

    modbus_mapping_t *mb_mapping = modbus_mapping_new(MODBUS_MAX_READ_BITS, 0,
                                                      MODBUS_MAX_READ_REGISTERS, 0);
    if (mb_mapping == nullptr) {
        perror("Cannot allocate mb_mapping");
        return -1;
    }

    uint16_t counter = 0;
    while (true) {
        int rc;
        do {
            rc = modbus_receive(ctx, request.data());
        } while (rc == 0);
        if (rc < 0) {
            perror("Error in modbus receive");
        }
        printf("Request received rc = %d\n", rc);

        mb_mapping->tab_registers[0] = counter++;
        mb_mapping->tab_registers[1] = counter++;

        ret = modbus_reply(ctx, request.data(), rc, mb_mapping);
        if (ret < 0) {
            perror("modbus reply error");
            return -1;
        }
    }

    modbus_mapping_free(mb_mapping);
    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}

int main() {
    return modbus_slave();
}
