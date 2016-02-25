
#ifndef __DRV_CLK_MTK_H
#define __DRV_CLK_MTK_H


#include <linux/bitops.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>

#define CLK_DEBUG		0
#define DUMMY_REG_TEST		0
#if !defined(CONFIG_MTK_LEGACY)
#define Bring_Up /* FIXME: disable after CCF is ready */
#endif /* !defined(CONFIG_MTK_LEGACY) */

extern spinlock_t *get_mtk_clk_lock(void);

#define mtk_clk_lock(flags)	spin_lock_irqsave(get_mtk_clk_lock(), flags)
#define mtk_clk_unlock(flags)	\
	spin_unlock_irqrestore(get_mtk_clk_lock(), flags)

#define MAX_MUX_GATE_BIT	31
#define INVALID_MUX_GATE_BIT	(MAX_MUX_GATE_BIT + 1)

struct clk *mtk_clk_register_mux(
		const char *name,
		const char **parent_names,
		u8 num_parents,
		void __iomem *base_addr,
		u8 shift,
		u8 width,
		u8 gate_bit);

#endif /* __DRV_CLK_MTK_H */
