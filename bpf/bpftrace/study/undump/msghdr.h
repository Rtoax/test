#ifndef __MSG_HDR_H
#define __MSG_HDR_H 1


struct MsgHdr {
    int src;
    int dst;
    int id;
    char data[];
};


#define DUMP_HDR(hdr) \
    printf("(%d->%d) id %d\n", hdr->src, hdr->dst, hdr->id)

#endif /*<__MSG_HDR_H>*/