# Included ROMs

The repository includes these 4096-byte ROM dumps:

| File | SHA-1 |
| --- | --- |
| BOOTROM.BIN | eefe5ad6b1ef77a1caf0af743b74de5fa1c4c19d |
| CHAROM.BIN | 50132b759a6d84c22c387c39c0f57535cd380411 |

The user's existing [Bondwell repository](https://github.com/royedmund/Bondwell-12-14-PCGET-and-PCPUT)
contains these dumps. They match the boot and character ROM identities in
[MAME's Bondwell driver](https://github.com/mamedev/mame/blob/master/src/mame/bondwell/bw12.cpp).
These dumps are included at the repository owner's request.
An alternate 4096-byte character ROM can be selected with `--chargen`.
