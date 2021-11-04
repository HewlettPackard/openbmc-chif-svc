# HPE GXP Channel Interface (CHIF) Service for OpenBMC

This OpenBMC service handles incoming Channel Interface (CHIF) requests from the host UEFI firmware.  This enables capabilities such as SMBIOS download to OpenBMC.

This depends upon a kernel driver to create `/dev/chif24`
