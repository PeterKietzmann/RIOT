import puf_sram_if

puf_sram = puf_sram_if.PufSram()
print(puf_sram.get_seed_list(n=10, allow_print=True))
