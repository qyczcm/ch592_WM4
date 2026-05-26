::ÇÐ»»µ±Ç°Ä¿Â¼
::cd  %~dp0
::copy "..\project\mdk5\objects\app.hex" "%cd%\"
::mergehex --merge adv_vendor_self_provision_JumpIAP.hex adv_vendor_self_provision_IAP.hex  --output bootloader.hex
::mergehex --merge bootloader.hex CH59xBLE_ROM_MESH.hex --output bl_temp2.hex
mergehex --merge bootloader.hex BLE_Mesh_App.hex --output CH592_Mesh_All_In_One.hex
del BLE_Mesh_App_OTA.bin
hex2bin -c BLE_Mesh_App.hex
ren "BLE_Mesh_App.bin" "BLE_Mesh_App_OTA.bin"
::del bl_temp.hex
::del bl_temp2.hex
exit