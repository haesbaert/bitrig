/* $OpenBSD: armv7.c,v 1.2 2013/11/20 13:32:40 rapha Exp $ */
/*
 * Copyright (c) 2005,2008 Dale Rahn <drahn@openbsd.com>
 * Copyright (c) 2012-2013 Patrick Wildt <patrick@blueri.se>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/param.h>
#include <sys/systm.h>
#define _ARM32_BUS_DMA_PRIVATE
#include <machine/bus.h>
#include <arm/armv7/armv7var.h>
#include <armv7/armv7/armv7var.h>
#include <armv7/sunxi/sunxireg.h>

#include "exynos.h"
#include "imx.h"
#include "omap.h"
#include "sunxi.h"

struct arm32_bus_dma_tag armv7_bus_dma_tag = {
	0,
	0,
	NULL,
	_bus_dmamap_create,
	_bus_dmamap_destroy,
	_bus_dmamap_load,
	_bus_dmamap_load_mbuf,
	_bus_dmamap_load_uio,
	_bus_dmamap_load_raw,
	_bus_dmamap_unload,
	_bus_dmamap_sync,
	_bus_dmamem_alloc,
	_bus_dmamem_free,
	_bus_dmamem_map,
	_bus_dmamem_unmap,
	_bus_dmamem_mmap,
};

struct armv7_dev *armv7_devs = NULL;

#define DEVNAME(sc)	(sc)->sc_dv.dv_xname

extern struct board_dev chromebook_devs[];
extern struct board_dev hummingboard_devs[];
extern struct board_dev sabrelite_devs[];
extern struct board_dev utilite_devs[];
extern struct board_dev wandboard_devs[];
extern struct board_dev beagleboard_devs[];
extern struct board_dev beaglebone_devs[];
extern struct board_dev overo_devs[];
extern struct board_dev pandaboard_devs[];
extern struct board_dev sun4i_devs[];
extern struct board_dev sun7i_devs[];
/*
 * We do direct configuration of devices on this SoC "bus", so we
 * never call the child device's match function at all (it can be
 * NULL in the struct cfattach).
 */
int
armv7_submatch(struct device *parent, void *child, void *aux)
{
	struct cfdata *cf = child;
	struct armv7_attach_args *aa = aux;

	if (strcmp(cf->cf_driver->cd_name, aa->aa_dev->name) == 0)
		return (1);

	/* "These are not the droids you are looking for." */
	return (0);
}

void
armv7_set_devs(struct armv7_dev *devs)
{
	armv7_devs = devs;
}

struct armv7_dev *
armv7_find_dev(const char *name, int unit)
{
	struct armv7_dev *ad;

	if (armv7_devs == NULL)
		panic("%s: armv7_devs == NULL", __func__);

	for (ad = armv7_devs; ad->name != NULL; ad++) {
		if (ad->unit == unit && strcmp(ad->name, name) == 0)
			return (ad);
	}

	return (NULL);
}

int
armv7_match(struct device *parent, void *cfdata, void *aux)
{
	return (1);
}

void
armv7_attach(struct device *parent, struct device *self, void *aux)
{
	struct armv7_softc *sc = (struct armv7_softc *)self;
	struct board_dev *bd;
	uint32_t issunxi = 0;
	bus_space_handle_t ioh;

	switch (board_id) {
#if NEXYNOS > 0
	case BOARD_ID_EXYNOS5_CHROMEBOOK:
		printf(": Exynos 5 Chromebook\n");
		exynos5_init();
		sc->sc_board_devs = chromebook_devs;
		break;
#endif
#if NIMX > 0
	case BOARD_ID_IMX6_HUMMINGBOARD:
		printf(": SolidRun HummingBoard\n");
		imx6_init();
		sc->sc_board_devs = hummingboard_devs;
		break;
	case BOARD_ID_IMX6_SABRELITE:
		printf(": i.MX6 SABRE Lite\n");
		imx6_init();
		sc->sc_board_devs = sabrelite_devs;
		break;
	case BOARD_ID_IMX6_UTILITE:
		printf(": i.MX6 Utilite\n");
		imx6_init();
		sc->sc_board_devs = utilite_devs;
		break;
	case BOARD_ID_IMX6_WANDBOARD:
		printf(": i.MX6 Wandboard\n");
		imx6_init();
		sc->sc_board_devs = wandboard_devs;
		break;
#endif
#if NOMAP > 0
	case BOARD_ID_OMAP3_BEAGLE:
		printf(": BeagleBoard\n");
		omap3_init();
		sc->sc_board_devs = beagleboard_devs;
		break;
	case BOARD_ID_AM335X_BEAGLEBONE:
		printf(": BeagleBone\n");
		am335x_init();
		sc->sc_board_devs = beaglebone_devs;
		break;
	case BOARD_ID_OMAP3_OVERO:
		printf(": Gumstix Overo\n");
		omap3_init();
		sc->sc_board_devs = overo_devs;
		break;
	case BOARD_ID_OMAP4_PANDA:
		printf(": PandaBoard\n");
		omap4_init();
		sc->sc_board_devs = pandaboard_devs;
		break;
#endif
#if NSUNXI > 0
	case BOARD_ID_SUN4I_A10:
		printf(": A1X\n");
		sxia1x_init();
		sc->sc_board_devs = sun4i_devs;
		issunxi = 1;
		break;
	case BOARD_ID_SUN7I_A20:
		printf(": A20\n");
		sxia20_init();
		sc->sc_board_devs = sun7i_devs;
		issunxi = 1;
		break;
#endif
	default:
		printf("\n");
		panic("%s: board type 0x%x unknown", __func__, board_id);
	}

	if (issunxi) {
		/*
		 * XXX think of a better place to do this, as there might
		 * be need for access by other drivers later.
		 */
		if (bus_space_map(&armv7_bs_tag, SYSCTRL_ADDR, SYSCTRL_SIZE, 0,
		    &ioh))
		panic("sunxi_attach: bus_space_map failed!");
		/* map the part of SRAM dedicated to EMAC to EMAC */
		bus_space_write_4(&armv7_bs_tag, ioh, 4,
		    bus_space_read_4(&armv7_bs_tag, ioh, 4) | (5 << 2));
	}

	/* Directly configure on-board devices (dev* in config file). */
	for (bd = sc->sc_board_devs; bd->name != NULL; bd++) {
		struct armv7_dev *ad = armv7_find_dev(bd->name, bd->unit);
		struct armv7_attach_args aa;

		if (ad == NULL) {
			printf("%s: device %s unit %d not found\n",
			    DEVNAME(sc), bd->name, bd->unit);
			continue;
		}

		memset(&aa, 0, sizeof(aa));
		aa.aa_dev = ad;
		aa.aa_iot = &armv7_bs_tag;
		aa.aa_dmat = &armv7_bus_dma_tag;

		if (config_found_sm(self, &aa, NULL, armv7_submatch) == NULL)
			printf("%s: device %s unit %d not configured\n",
			    DEVNAME(sc), bd->name, bd->unit);
	}
}

