
.global IcoAppMain_data
.global IcoAppMain_size
.global IcoBtnNo_data
.global IcoBtnNo_size
.global IcoBtnOk_data
.global IcoBtnOk_size
.global IcoToolAttach_data
.global IcoToolAttach_size
.global IcoToolConfig_data
.global IcoToolConfig_size
.global IcoToolCreate_data
.global IcoToolCreate_size
.global IcoToolDelete_data
.global IcoToolDelete_size
.global IcoToolEdit_data
.global IcoToolEdit_size
.global IcoToolExport_data
.global IcoToolExport_size
.global IcoToolLayout_data
.global IcoToolLayout_size
.global IcoToolPrint_data
.global IcoToolPrint_size
.global IcoToolRefresh_data
.global IcoToolRefresh_size
.global IcoToolSave_data
.global IcoToolSave_size
.global IcoToolStop_data
.global IcoToolStop_size
.global IcoToolView_data
.global IcoToolView_size

.section .rodata

/* Binary resources data blocks */

IcoAppMain_data: .incbin "../Resource/AppMain.ico"

IcoBtnNo_data: .incbin "../Resource/BtnNo.ico"
IcoBtnOk_data: .incbin "../Resource/BtnOk.ico"

IcoToolAttach_data: .incbin "../Resource/ToolAttach.ico"
IcoToolConfig_data: .incbin "../Resource/ToolConfig.ico"
IcoToolCreate_data: .incbin "../Resource/ToolCreate.ico"
IcoToolDelete_data: .incbin "../Resource/ToolDelete.ico"
IcoToolEdit_data: .incbin "../Resource/ToolEdit.ico"
IcoToolExport_data: .incbin "../Resource/ToolExport.ico"
IcoToolLayout_data: .incbin "../Resource/ToolLayout.ico"
IcoToolPrint_data: .incbin "../Resource/ToolPrint.ico"
IcoToolRefresh_data: .incbin "../Resource/ToolRefresh.ico"
IcoToolSave_data: .incbin "../Resource/ToolSave.ico"
IcoToolStop_data: .incbin "../Resource/ToolStop.ico"
IcoToolView_data: .incbin "../Resource/ToolView.ico"

/* Binary resources size definitions */

IcoAppMain_size: .int IcoBtnNo_data - IcoAppMain_data

IcoBtnNo_size: .int IcoBtnOk_data - IcoBtnNo_data
IcoBtnOk_size: .int IcoToolAttach_data - IcoBtnOk_data

IcoToolAttach_size: .int IcoToolConfig_data - IcoToolAttach_data
IcoToolConfig_size: .int IcoToolCreate_data - IcoToolConfig_data
IcoToolCreate_size: .int IcoToolDelete_data - IcoToolCreate_data
IcoToolDelete_size: .int IcoToolEdit_data - IcoToolDelete_data
IcoToolEdit_size: .int IcoToolExport_data - IcoToolEdit_data
IcoToolExport_size: .int IcoToolLayout_data - IcoToolExport_data
IcoToolLayout_size: .int IcoToolPrint_data - IcoToolLayout_data
IcoToolPrint_size: .int IcoToolRefresh_data - IcoToolPrint_data
IcoToolRefresh_size: .int IcoToolSave_data - IcoToolRefresh_data
IcoToolSave_size: .int IcoToolStop_data - IcoToolSave_data
IcoToolStop_size: .int IcoToolView_data - IcoToolStop_data
IcoToolView_size: .int IcoAppMain_size - IcoToolView_data
