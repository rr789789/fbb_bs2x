import { access } from '@kit.ConnectivityKit'
import { abilityAccessCtrl, common } from '@kit.AbilityKit'
import { router } from '@kit.ArkUI'
import { JumpToSettingDialog } from '../dialogs/JumpToSettingDialog'
import { askUserAuthorize, SleDevice, enableSle, getGrantStatus, getSleState, jumpToSetting,
    startSleScan,
    stopSleScan } from '../sle/helper'
import { manager } from '@kit.NearLinkKit'
import { SleAuthorizeDialog } from '../dialogs/SleAuthorizeDialog'
import { SleSwitchDialog } from '../dialogs/SleSwitchDialog'

@Entry
@Component
struct Index {
    @State bleItemList: Array<SleDevice> = []
    @State sleScanning: boolean = false
    private scanTimer = -1
    sleAuthorizeDialogController: CustomDialogController = new CustomDialogController({
        builder: SleAuthorizeDialog({
            confirm: () => {
                //用户点击去授权，拉起系统授权
                this.onSleAuthorizeDialogConfirm()
            }
        }),
        alignment: DialogAlignment.Bottom
    })
    sleSwitchDialogController: CustomDialogController = new CustomDialogController({
        builder: SleSwitchDialog({
            confirm: () => {
                this.onSleSwitchDialogConfirm()
            }
        }),
        alignment: DialogAlignment.Bottom
    })
    jumpToSettingDialogController: CustomDialogController = new CustomDialogController({
        builder: JumpToSettingDialog({
            confirm: () => {
                this.onJumpToSettingDialogConfirm()
            }
        }),
        alignment: DialogAlignment.Bottom
    })

    onSleAuthorizeDialogConfirm() {
        askUserAuthorize(getContext(this) as common.UIAbilityContext, () => {
            //用户同意
            this.onSleState(getSleState())
        }, () => {
            // 弹窗了，用户没有同意，这里不作处理

        }, () => {
            // 没有弹窗，用户已经拒绝过了
            this.jumpToSettingDialogController.open()
        })
    }

    onSleSwitchDialogConfirm() {
        enableSle(() => {
            this.startScan()
        })
    }

    onJumpToSettingDialogConfirm() {
        jumpToSetting(getContext() as common.UIAbilityContext)
    }

    stopScan() {
        this.sleScanning = false
        stopSleScan()
        if (this.scanTimer > 0) {
            clearTimeout(this.scanTimer)
            this.scanTimer = -1
        }
    }

    // 退出页面的时候必须停止搜索关闭定时器，防止空指针问题
    onPageHide() {
        this.stopScan()
    }

    onPageShow() {
        this.bleItemList = []
    }

    //搜索入口
    startScan() {
        this.sleScanning = true
        startSleScan((bleDeviceList: Array<SleDevice>) => {
            // index维护了一份蓝牙设备列表
            // helper传过来的蓝牙设备列表需要对比，如果已在列表中只需要更新rssi，否则需要添加进列表中
            bleDeviceList.forEach(bleDevice => {
                let foundDevice = this.bleItemList.find(item => item.name == bleDevice.name)
                if (foundDevice) {
                    foundDevice.rssi = bleDevice.rssi
                } else {
                    this.bleItemList.push({
                        name: bleDevice.name,
                        rssi: bleDevice.rssi,
                        macAddress: bleDevice.macAddress
                    })
                }
                // }
            })

        })
        // 20秒后关闭搜索
        this.scanTimer = setTimeout(() => {
            this.stopScan()
        }, 20 * 1000)
    }

    onSleState(sleState: manager.NearlinkState) {
        if (sleState == manager.NearlinkState.STATE_OFF) {
            //打开窗口提示打开蓝牙
            this.sleSwitchDialogController.open()
        } else if (sleState === manager.NearlinkState.STATE_ON) {
            //开始扫描
            this.startScan()
        }
    }

    onGetGrantStatus(grantStatus: abilityAccessCtrl.GrantStatus) {
        if (grantStatus === abilityAccessCtrl.GrantStatus.PERMISSION_GRANTED) {
            // 已经授权，查询蓝牙开关状态
            this.onSleState(getSleState())
        } else if (grantStatus === abilityAccessCtrl.GrantStatus.PERMISSION_DENIED) {
            // 未授权，弹出授权提示窗
            this.sleAuthorizeDialogController.open()
        }
    }

    build() {
        Column() {
            Flex({
                direction: FlexDirection.Row,
                alignItems: ItemAlign.Auto,
                justifyContent: FlexAlign.SpaceAround
            }) {
                Button("开始扫描").onClick(async () => {
                    if (this.sleScanning) {
                        return
                    }
                    this.onGetGrantStatus(await getGrantStatus())
                }).enabled(!this.sleScanning)
            }

            List() {
                ForEach(this.bleItemList, (item: SleDevice) => {
                    ListItem() {
                        Row() {
                            Text(item.name.length > 0 ? item.name : "noname")
                            Text(item.rssi.toString())
                            Text(item.macAddress).fontSize(8)
                        }
                        .width('100%')
                        .justifyContent(FlexAlign.SpaceBetween)
                    }.onClick(() => {
                        this.stopScan()

                        router.pushUrl({
                            url: 'pages/Device',
                            params: item
                        }, err => {
                            if (err) {
                                console.error(`Invoke pushUrl failed, code is ${err.code}, message is ${err.message}`)
                                return
                            }
                        })
                    })
                    .margin(10)
                    .padding(15)
                    .border({ width: 1, color: Color.Black })
                }, (item: SleDevice) => `${item.macAddress}${item.rssi}`)
            }.width("90%").backgroundColor("#ff93deff").margin({ top: 30 }).height("90%")
        }
        .padding({ top: 30, bottom: 50 })
        .height('100%')
        .width('100%')
    }
}