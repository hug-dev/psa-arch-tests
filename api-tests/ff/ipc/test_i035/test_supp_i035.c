/** @file
 * Copyright (c) 2018, Arm Limited or its affiliates. All rights reserved.
 * SPDX-License-Identifier : Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
**/

#include "val/common/val_client_defs.h"
#include "val/spe/val_partition_common.h"

int32_t server_test_psa_skip_at_ipc_disconnect();

server_test_t test_i035_server_tests_list[] = {
    NULL,
    server_test_psa_skip_at_ipc_disconnect,
    NULL,
};

int32_t server_test_psa_skip_at_ipc_disconnect()
{
    int32_t         status = VAL_STATUS_SUCCESS;
    psa_msg_t       msg = {0};

   /* Test is targeting fatal error condition and it will expect an error recovery(reboot)
    * to happen. To decide, a reboot happened was intended as per test scenario or it happended
    * due to other reasons, test is setting a boot signature into non-volatile memory before and
    * after targeted test check. After a reboot, these boot signatures are being read by the
    * VAL APIs to decide test status.
    *
    * Note: If SPM is not capable of rebooting (just hangs or power down) in fatal error condition,
    * a watchdog timer enabled by val_test_init can reboot the system on timeout event.
    * If programmed timeout value isn't sufficient for your system, it can be reconfigured using
    * timeout entries available in target.cfg.
    */

    status = val_process_connect_request(SERVER_RELAX_MINOR_VERSION_SIG, &msg);
    if (val_err_check_set(TEST_CHECKPOINT_NUM(201), status))
    {
        psa_reply(msg.handle, PSA_CONNECTION_REFUSED);
        return status;
    }
    psa_reply(msg.handle, PSA_SUCCESS);

    status = val_process_disconnect_request(SERVER_RELAX_MINOR_VERSION_SIG, &msg);
    if (val_err_check_set(TEST_CHECKPOINT_NUM(202), status))
    {
        psa_reply(msg.handle, PSA_SUCCESS);
        return status;
    }

    status = val_set_boot_flag(BOOT_EXPECTED_NS);
    if (val_err_check_set(TEST_CHECKPOINT_NUM(203), status))
    {
        val_print(PRINT_ERROR, "\tFailed to set boot flag before check\n", 0);
        psa_reply(msg.handle, PSA_SUCCESS);
        return status;
    }

    /* Test check- psa_skip at PSA_IPC_DISCONNECT */
    psa_skip(msg.handle, 0, 0);

    /* Shouldn't have reached here */
    val_print(PRINT_ERROR,"\tpsa_skip should failed but successed\n", 0);

    /* Resetting boot.state to catch unwanted reboot */
    if (val_set_boot_flag(BOOT_EXPECTED_BUT_FAILED))
    {
        val_print(PRINT_ERROR, "\tFailed to set boot flag after check\n", 0);
    }
    psa_reply(msg.handle, PSA_SUCCESS);
    return VAL_STATUS_INVALID;
}
