/*
 * Virtio Block Device
 *
 * Copyright IBM, Corp. 2007
 *
 * Authors:
 *  Anthony Liguori   <aliguori@us.ibm.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 *
 */

#ifndef QEMU_VIRTIO_BLK_H
#define QEMU_VIRTIO_BLK_H

#include "standard-headers/linux/virtio_blk.h"
#include "hw/virtio/virtio.h"
#include "hw/block/block.h"
#include "sysemu/iothread.h"
#include "sysemu/block-backend.h"

#define TYPE_VIRTIO_BLK "virtio-blk-device"
#define VIRTIO_BLK(obj) \
        OBJECT_CHECK(VirtIOBlock, (obj), TYPE_VIRTIO_BLK)

/* This is the last element of the write scatter-gather list */
struct virtio_blk_inhdr
{
    unsigned char status;
};

struct VirtIOBlkConf
{
    BlockConf conf;
    IOThread *iothread;
    char *serial;
    uint32_t scsi;
    uint32_t config_wce;
    uint32_t request_merging;
    uint16_t num_queues;
};

struct VirtIOBlockDataPlane;

typedef struct ReqRecord {
    int *list;
    int *idx;
    void **reqs;
    bool *completed;
    int len;
    int size;
    int left;
    QTAILQ_ENTRY(ReqRecord) node;
} ReqRecord;

typedef QTAILQ_HEAD(, ReqRecord) ReqRecordList;

typedef struct WReqrecord {
		int list;
		int idx;
		void *reqs;
		QTAILQ_ENTRY(WReqrecord) node;

} Wreqrecord;

typedef QTAILQ_HEAD(, WReqrecord) WReqList;

struct VirtIOBlockReq;
typedef struct VirtIOBlock {
    VirtIODevice parent_obj;
    BlockBackend *blk;
    void *rq;
    QEMUBH *bh;
    VirtIOBlkConf conf;
    unsigned short sector_mask;
    bool original_wce;
    VMChangeStateEntry *change;
    bool dataplane_disabled;
    bool dataplane_started;
    struct VirtIOBlockDataPlane *dataplane;

    // For CUJU-FT
    void *pending_rq;   	// pending read  requests
    ReqRecord *temp_list;
    ReqRecordList record_list;
		WReqList pending_wrq;	// pending write request
		int pending_wlen;


} VirtIOBlock;

typedef struct ReqRecordCommit {
    ReqRecord *req;
    QEMUBH *bh;
    VirtIOBlock *block;
} ReqRecordCommit;

typedef struct VirtIOBlockReq {
    VirtQueueElement elem;
    int64_t sector_num;
    VirtIOBlock *dev;
    VirtQueue *vq;
    struct virtio_blk_inhdr *in;
    struct virtio_blk_outhdr out;
    QEMUIOVector qiov;
    size_t in_len;
    struct VirtIOBlockReq *next;
    struct VirtIOBlockReq *mr_next;
    BlockAcctCookie acct;

    // For CUJU-FT
    ReqRecord *record;
} VirtIOBlockReq;

#define VIRTIO_BLK_MAX_MERGE_REQS 32

typedef struct MultiReqBuffer {
    VirtIOBlockReq *reqs[VIRTIO_BLK_MAX_MERGE_REQS];
    unsigned int num_reqs;
    bool is_write;
} MultiReqBuffer;

void virtio_blk_handle_vq(VirtIOBlock *s, VirtQueue *vq);

#endif
