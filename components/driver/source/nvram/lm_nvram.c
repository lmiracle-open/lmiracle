#include "lmiracle.h"
#include "lm_nvram.h"

/*
 * todo:这里暂时使用指针,以后需要修改成链表
 */
static lm_nvram_dev_t                        *__gp_nvram_dev;
const static lm_nvram_info_t                  *__gp_nvram_info;


//static int __nvram_find_for_name (char *p_name)
//{
//
//
//    return LM_OK;
//}

/*
 * NVRAM写
 */
int lm_nvram_write (char *name, uint8_t *p_buf, uint32_t offset, size_t len)
{
    int  ret = LM_OK;

    if ((name == NULL) ||(p_buf == NULL)) {
        return -LM_EFAULT;
    }

    /* todo: 之后要用链表管理 */
    const lm_nvram_info_t                   *p_info = __gp_nvram_info;
    lm_nvram_dev_t                   *p_nvram = __gp_nvram_dev;

    const lm_nvram_segment_t               *p_zone  = NULL;
    int                                     tmp_len = 0;
    struct erase_info                       instr;

    for (int i = 0; i < p_info->zone_num; ++i) {
        p_zone  =  &p_info->p_zone[i];

        /* 查找区域 */
        if (0 == strcmp(name, p_zone->name)) {
            tmp_len = len;
            if (offset + len > p_zone->size) {
                tmp_len = p_zone->size - offset;
                if (tmp_len < 0) {
                    return -LM_EIO;
                }
            }

            uint32_t block_addr_pre  = 0;
            uint32_t block_addr_next = 0;
            uint32_t current_addr    = 0;
            uint32_t copy_size       = 0;
            uint32_t j               = 0;
            uint32_t k               = 0;
            uint8_t *p_txbuf         = 0;

            /* 计算当前的存储地址 */
            current_addr = j + p_zone->addr + offset;
            block_addr_pre  = p_zone->addr / p_nvram->erasesize *
                              p_nvram->erasesize;
            block_addr_next = block_addr_pre + p_nvram->erasesize;

            /*
             * 需要判断当前数据是否与擦除地址对齐,如果对齐则直接擦除写,如果不对齐需要线读出
             * 当前数据,合并后在写
             */
            for (j = 0; j < tmp_len; ) {
                copy_size = (tmp_len - j) < (block_addr_next - current_addr) ? \
                        (tmp_len - j): \
                        (block_addr_next - current_addr);

                if (copy_size < p_nvram->erasesize) {
                    /* 读取数据 */
                    ret = p_nvram->pfunc_read(p_nvram,
                                              block_addr_pre,
                                              p_info->buf,
                                              p_nvram->erasesize,
                                              NULL);
                    if (ret) {
                        return ret;
                    }

                    memcpy(&p_info->buf[current_addr%p_nvram->erasesize],
                           &p_buf[j], copy_size);

                    p_txbuf = p_info->buf;
                } else {
                    p_txbuf = (uint8_t*)(p_buf +j);
                }

                /* 擦除 */
                instr.addr = block_addr_pre;
                instr.len = p_nvram->erasesize;
                p_nvram->pfunc_erase(p_nvram, &instr);

                /* 写入数据 */
                for (k = 0; k < p_nvram->erasesize; k += p_nvram->writebufsize) {
                    ret = p_nvram->pfunc_write(p_nvram,
                                               block_addr_pre +k,
                                               p_txbuf + k,
                                               p_nvram->writebufsize,
                                               NULL);
                    if (ret) {
                        return ret;
                    }
                }
                j+= copy_size;
                current_addr += copy_size;
                block_addr_pre += p_nvram->erasesize;
                block_addr_next = block_addr_pre + p_nvram->erasesize;
            }
            return LM_OK;
        }
    }

    return ret;
}

/*
 * NVRAM读
 */
int lm_nvram_read (char *name, uint8_t *p_buf, uint32_t offset, size_t len)
{
    int  ret = LM_OK;

    if ((name == NULL) ||(p_buf == NULL)) {
        return -LM_EFAULT;
    }

    /* todo: 之后要用链表管理 */
    const lm_nvram_info_t                   *p_info = __gp_nvram_info;

    const lm_nvram_segment_t         *p_zone  = NULL;
    int                               tmp_len = 0;


    for (int i = 0; i < p_info->zone_num; ++i) {
        p_zone = &p_info->p_zone[i];

        /* 查找区域 */
        if (0 == strcmp(name, p_zone->name)) {
            tmp_len = len;
            if (offset + len > p_zone->size) {
                tmp_len = p_zone->size - offset;
                if (tmp_len < 0) {
                    return -LM_EIO;
                }
            }

            ret = __gp_nvram_dev->pfunc_read(__gp_nvram_dev,
                                  p_zone->addr + offset,
                                  p_buf,
                                  tmp_len,
                                  NULL);

        }
    }

    return ret;
}



/*
 * 注册NVRAM
 */
int lm_nvram_register (lm_nvram_dev_t *p_dev)
{
    int ret = LM_OK;

    __gp_nvram_dev = p_dev;
    __gp_nvram_info = p_dev->p_info;

    return ret;
}






/* end of file */



