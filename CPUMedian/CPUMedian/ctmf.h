#ifndef CTMF_H
#define CTMF_H


#if defined(_MSC_VER)
#ifdef __cplusplus
extern "C" {
#endif
#endif

void ctmf(
        const unsigned char* src, unsigned char* dst,
        int width, int height,
        int src_step_row, int dst_step_row,
        int r, int channels, unsigned long memsize
        );

#if defined(_MSC_VER)
#ifdef __cplusplus
}
#endif
#endif

#endif
