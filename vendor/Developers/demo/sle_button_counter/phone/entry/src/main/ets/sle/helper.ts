/**
 * Copyright (c) EndyTsang 2025. All rights reserved. \n
 *
 * Description: SLE Phone Demo. \n
 * 
 */

import { abilityAccessCtrl, common, Want } from '@kit.AbilityKit'
import { bundleManager } from '@kit.AbilityKit'
import { BusinessError, Callback } from '@kit.BasicServicesKit'
import { buffer } from '@kit.ArkTS'
import { manager, scan, ssap, constant } from '@kit.NearLinkKit';

export function getSleState(): manager.NearlinkState {
    let sleState = manager.NearlinkState.STATE_OFF
    try {
        sleState = manager.getState()
        console.debug(`get state sle state :${sleState}`)
    } catch (err) {
        // 无权限
        console.error(`getSleState errCode: ${err.code}, errMessage: ${err.message}`)
    }
    return sleState
}

export async function getGrantStatus(): Promise<abilityAccessCtrl.GrantStatus> {
    // 获取应用程序的accessTokenID
    let tokenId: number = 0
    let grantStatus: abilityAccessCtrl.GrantStatus = abilityAccessCtrl.GrantStatus.PERMISSION_DENIED

    try {
        tokenId =
            (await bundleManager.getBundleInfoForSelf(bundleManager.BundleFlag.GET_BUNDLE_INFO_WITH_APPLICATION)).appInfo.accessTokenId
        // 校验应用是否被授予权限
        grantStatus =
            await abilityAccessCtrl.createAtManager().checkAccessToken(tokenId, 'ohos.permission.ACCESS_NEARLINK')
    } catch (err) {
        console.error(`getGrantStatus errCode: ${err.code}, errMessage: ${err.message}`);
    }
    return grantStatus
}

export async function jumpToSetting(context: common.UIAbilityContext) {
    let bundleName =
        (await bundleManager.getBundleInfoForSelf(bundleManager.BundleFlag.GET_BUNDLE_INFO_WITH_APPLICATION)).appInfo.name
    console.log(`bundle name: ${bundleName}`)
    let wantInfo: Want = {
        bundleName: 'com.huawei.hmos.settings',
        abilityName: 'com.huawei.hmos.settings.MainAbility',
        uri: 'application_info_entry',
        parameters: {
            pushParams: bundleName // 应用的 bundleName
        }
    }

    await context.startAbility(wantInfo).catch((err: BusinessError) => {
        console.error(`startAbility errCode: ${err.code}, errMessage: ${err.message}`)
    })
}

export async function askUserAuthorize(context: common.UIAbilityContext,
    userAgreeCallback: Function = null,
    userDenyCallback: Function = null,
    notPopCallback: Function = null) {
    let result = await abilityAccessCtrl.createAtManager()
        .requestPermissionsFromUser(context, ['ohos.permission.ACCESS_NEARLINK'])
        .catch((err: BusinessError) => console.error(`errCode: ${err.code}, errMessage: ${err.message}`))
    if (result) {
        let grantStatus = result.authResults[0]
        console.info(`grant status: ${grantStatus}`)

        if (grantStatus === 0) {
            if (userAgreeCallback) {
                userAgreeCallback()
            }
        } else {
            if (result.dialogShownResults && result.dialogShownResults[0]) {
                // 弹窗了，用户没有同意
                if (userDenyCallback) {
                    userDenyCallback()
                }
            } else {
                if (notPopCallback) {
                    notPopCallback()
                }
            }
        }
    } else {
        console.info("no result")
    }
}

export async function enableSle(bleOnCallback: Function = null) {
    manager.on('stateChange', state => {
        if (state == manager.NearlinkState.STATE_ON) {
            manager.off('stateChange')
            if (bleOnCallback) {
                bleOnCallback()
            }
        }
        console.debug(`nearlink statues: ${state}`)
    })

    try {
        console.debug("enableSle is not enable right now")
        // manager.enableBluetooth()
    } catch (err) {
        console.error(`enableSle errCode: ${err.code}, errMessage: ${err.message}`)
    }
}

export interface SleDevice {
    name: string
    rssi: number
    macAddress: string
}

export async function startSleScan(scanResultCallback: Callback<Array<SleDevice>>) {
    scan.on('deviceFound', scanResult => {
        console.log("deviceFound")
        scanResultCallback(scanResult.filter(result => result.isConnectable).map(result => ({
            name: result.deviceName, rssi: result.rssi, macAddress: result.address
        })
        ))
    })
    try {
        console.log("scan.startScan")
        scan.startScan([{ deviceName: "COUNTER" }], {
            scanMode: scan.ScanMode.SCAN_MODE_LOW_POWER
        })
    } catch (err) {
        console.error(`errCode: ${err.code}, errMessage: ${err.message}`)
    }
}

export function stopSleScan() {
    try {
        console.log("scan.stopScan")
        scan.stopScan();
    } catch (err) {
        console.error(`errCode: ${err.code}, errMessage: ${err.message}`)
    }
}

let connectedDevice: ssap.Client | undefined = undefined
let serviceList: Array<ssap.Service> = []

export async function connectToDevice(macAddress: string, connectedCallback: Function = null,
    disconnectedCallback: Function = null,
    servicesCallback: Callback<Array<ssap.Service>> = null,
    propertyNotifyCallback: Function = null) {
    connectedDevice = ssap.createClient(macAddress)
    connectedDevice?.on('connectionStateChange', async (state: ssap.ConnectionChangeState) => {
        let connectState: ssap.ConnectionChangeState = state
        if (connectState.state === constant.ConnectionState.STATE_CONNECTED) {
            if (connectedCallback) {
                connectedCallback()
            }
            connectedDevice?.on('propertyChange', (characteristic: ssap.Property) => {
                if (propertyNotifyCallback) {
                    propertyNotifyCallback(getShortUUID(characteristic.propertyUuid),
                        arrayBufferToString(characteristic.value))
                }
            })
            serviceList = await connectedDevice?.getServices()
            if (servicesCallback) {
                servicesCallback(serviceList)
            }
        } else if (connectState.state === constant.ConnectionState.STATE_DISCONNECTED) {
            if (disconnectedCallback) {
                disconnectedCallback()
            }
            // 这句很关键，取消订阅所有事件
            connectedDevice?.close()
            connectedDevice = null
        }
    })

    try {
        connectedDevice?.connect()
    } catch (err) {
        console.error(`errCode: ${err.code}, errMessage: ${err.message}`)
    }
    return connectedDevice
}

export async function disconnectDevice() {
    try {
        connectedDevice?.disconnect()
    } catch (err) {
        console.error(`errCode: ${err.code}, errMessage: ${err.message}`)
    }
}

// 根据SLE协议，把UUID改为短写模式，方便操作
export const getShortUUID = (src: string): string => {
    // const regex = /0000(\w{4})-/g
    // const matches = regex.exec(src)
    // return matches != null ? matches[1] : src
    return src.slice(src.length - 4)
}

const strToArrayBuffer = (str: string): ArrayBuffer => {
    return buffer.from(str.toUpperCase().match(/[0-9A-F]{2}/g).map(s => parseInt(s, 16))).buffer
}

const arrayBufferToString = (buff: ArrayBuffer): string => {
    let temp = ""
    let arr = new Uint8Array(buff)
    for (let i = 0; i < arr.byteLength; i++) {
        temp += arr[i].toString(16)
    }
    return temp
}


const findTargetProperty = (propertyUuid: string): ssap.Property | undefined => {
    let targetProperty: ssap.Property | undefined = undefined

    serviceList.forEach(service => {
        service.properties.forEach(property => {
            if (property.propertyUuid == propertyUuid) {
                targetProperty = property
            }
        })
    })

    if (targetProperty == undefined) {
        console.error(`property ${propertyUuid} not found`)
    }

    return targetProperty
}


export const readProperty = async (propertyUuid: string): Promise<string | undefined> => {
    try {
        let result = await connectedDevice?.readProperty(findTargetProperty(propertyUuid))
        return arrayBufferToString(result?.value)
    } catch (err) {
        console.error(`errCode: ${err.code}, errMessage: ${err.message}`)
    }
    return undefined
}

export const writeProperty = async (propertyUuid: string, writeValue: string) => {
    let targetProperty = findTargetProperty(propertyUuid)
    // 把writeValue按照每两个字符串分割后按照16进制转换为数字后写入
    if (targetProperty) {
        targetProperty.value = strToArrayBuffer(writeValue)
    }

    try {
        await connectedDevice?.writeProperty(targetProperty, ssap.PropertyWriteType.WRITE)
    } catch (err) {
        console.error(`errCode: ${err.code}, errMessage: ${err.message}`)
    }
}

export const subscribeProperty = (propertyUuid: string) => {
    try {
        connectedDevice?.setPropertyNotification(findTargetProperty(propertyUuid), true)
    } catch (err) {
        console.error(`errCode: ${err.code}, errMessage: ${err.message}`)
    }
}
