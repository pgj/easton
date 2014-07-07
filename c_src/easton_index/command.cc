
#include <stdlib.h>

#include "command.hh"
#include "config.hh"
#include "io.hh"
#include "util.hh"


static void
close_idx(easton_idx_t* idx, const unsigned char* cmd, size_t cmdlen)
{
    if(cmdlen != 0) {
        exit(EASTON_ERROR_TRAILING_DATA);
    }

    if(!easton_index_close(idx)) {
        exit(EASTON_ERROR_CLOSE_FAIL);
    }

    easton_send_ok(NULL, 0);

    exit(EASTON_OK);
}


static void
flush_idx(easton_idx_t* idx, const unsigned char* cmd, size_t cmdlen)
{
    if(cmdlen != 0) {
        exit(EASTON_ERROR_TRAILING_DATA);
    }

    if(!easton_index_flush(idx)) {
        exit(EASTON_ERROR_FLUSH_FAIL);
    }
    
    easton_send_ok(NULL, 0);
}


static void
put_user_kv(easton_idx_t* idx, const unsigned char* cmd, size_t cmdlen)
{
    unsigned char* key = NULL;
    unsigned char* val = NULL;
    size_t klen;
    size_t vlen;

    if(!easton_read_binary(&cmd, &cmdlen, (const void**) &key, &klen)) {
        exit(EASTON_ERROR_BAD_USER_KEY);
    }
    
    if(!easton_read_binary(&cmd, &cmdlen, (const void**) &val, &vlen)) {
        exit(EASTON_ERROR_BAD_USER_VAL);
    }

    if(cmdlen != 0) {
        exit(EASTON_ERROR_TRAILING_DATA);
    }
    
    if(!easton_index_put_kv(idx, key, klen, val, vlen)) {
        exit(EASTON_ERROR_BAD_PUT_USER_KV);
    }
    
    easton_send_ok(NULL, 0);
}


static void
get_user_kv(easton_idx_t* idx, const unsigned char* cmd, size_t cmdlen)
{
    unsigned char* key;
    unsigned char* val;
    size_t klen;
    size_t vlen;

    if(!easton_read_binary(&cmd, &cmdlen, (const void**) &key, &klen)) {
        exit(EASTON_ERROR_BAD_USER_KEY);
    }

    if(cmdlen != 0) {
        exit(EASTON_ERROR_TRAILING_DATA);
    }
    
    val = (unsigned char*) easton_index_get_kv(idx, key, klen, &vlen);
    if(val != NULL) {
        easton_send_ok(val, vlen);
        free(val);
    } else {
        easton_send_error(NULL, 0);
    }
}


static void
del_user_kv(easton_idx_t* idx, const unsigned char* cmd, size_t cmdlen)
{
    unsigned char* key;
    size_t klen;

    if(!easton_read_binary(&cmd, &cmdlen, (const void**) &key, &klen)) {
        exit(EASTON_ERROR_BAD_USER_KEY);
    }

    if(cmdlen != 0) {
        exit(EASTON_ERROR_TRAILING_DATA);
    }

    if(easton_index_del_kv(idx, key, klen)) {
        easton_send_ok(NULL, 0);
    } else {
        easton_send_error(NULL, 0);
    }
}


void
easton_handle_command(easton_idx_t* idx,
        const unsigned char* cmd, size_t cmdlen)
{
    int op;

    if(cmdlen < 4) {
        exit(EASTON_ERROR_BAD_COMMAND);
    }

    memcpy(&op, cmd, 4);
    op = ntohl(op);

    // Bump to after the op code
    cmd += 4;
    cmdlen -= 4;

    switch(op) {
        case EASTON_COMMAND_CLOSE:
            close_idx(idx, cmd, cmdlen);
            break;
        case EASTON_COMMAND_FLUSH:
            flush_idx(idx, cmd, cmdlen);
            break;
        case EASTON_COMMAND_PUT_USER_KV:
            put_user_kv(idx, cmd, cmdlen);
            break;
        case EASTON_COMMAND_GET_USER_KV:
            get_user_kv(idx, cmd, cmdlen);
            break;
        case EASTON_COMMAND_DEL_USER_KV:
            del_user_kv(idx, cmd, cmdlen);
            break;
        default:
            exit(EASTON_ERROR_BAD_COMMAND);
    }
}
