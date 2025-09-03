/**
 * Copyright (c) EndyTsang 2025. All rights reserved. \n
 *
 * Description: SLE Phone Demo. \n
 * 
 */

import { router } from '@kit.ArkUI'
import { SleWriteDialog } from '../dialogs/SleWriteDialog'
import {
    connectToDevice,
    disconnectDevice,
    getShortUUID,
    readProperty,
    SleDevice,
    subscribeProperty,
    writeProperty
} from '../sle/helper'
import { ssap } from '@kit.NearLinkKit'


@Entry
@Component
struct Device {
    @State bleName: string = "Gi888888"
    @State macAddress: string = "00:00:00:00:00:00"
    @State deviceLog: string = ""
    @State serviceList: ssap.Service[] = []
    
    private propertyReadyToWrite: string = ""
    sleWriteDialogController: CustomDialogController = new CustomDialogController({
        builder: SleWriteDialog({
            confirm: (sentValue: string) => {
                this.writeSle(this.propertyReadyToWrite, sentValue)
            }
        }),
        alignment: DialogAlignment.Bottom
    })

    appendLog(logText: string) {
        this.deviceLog = logText + "\n" + this.deviceLog
    }

    aboutToAppear(): void {
        this.bleName = (router.getParams() as SleDevice)?.name || this.bleName
        this.macAddress = (router.getParams() as SleDevice)?.macAddress || this.macAddress

        connectToDevice(this.macAddress, () => {
            this.appendLog(`${this.bleName} connected`)
        }, () => {
            //断开返回
            this.onBackPress()
        }, (serviceList: Array<ssap.Service>) => {
            this.serviceList = []
            serviceList.forEach(service => {
                let serviceItem: ssap.Service = { serviceUuid: service.serviceUuid, properties: [] }
                service.properties.forEach(property => {
                    // let properties = property.descriptors
                    let propertyItem: ssap.Property =
                        {
                            propertyUuid: property.propertyUuid,
                            serviceUuid: service.serviceUuid,
                            operation: property.operation,
                            descriptors: property.descriptors,
                            value: property.value
                        }
                    serviceItem.properties.push(propertyItem)
                })
                this.serviceList.push(serviceItem)
            })
        }, (shortUUID: string, propertyValue: string) => {
            this.appendLog(`${shortUUID} notify: ${propertyValue}`)
        })
    }

    aboutToDisappear() {
        disconnectDevice()
    }

    onBackPress(): boolean | void {
        router.back({
            url: 'pages/Index'
        })
        return true
    }

    @Builder
    serviceHead(text: string) {
        Text(getShortUUID(text))
            .width("100%")
            .backgroundColor("#ff5f74ff")
            .padding(10)
            .borderRadius(5)
            .fontColor(Color.White)
    }

    writeSle(propertyUuid: string, writeValue: string) {
        this.appendLog(`write ${propertyUuid}: ${writeValue}`)
        writeProperty(propertyUuid, writeValue)
    }

    async readSle(propertyUuid: string) {
        let result = await readProperty(propertyUuid)
        this.appendLog(`read ${propertyUuid}: ${result}`)
    }

    subscribeSle(propertyUuid: string) {
        this.appendLog(`subscribe ${propertyUuid}`)
        subscribeProperty(propertyUuid)
    }

    build() {
        Column() {
            Row() {
                Text(this.bleName)
                Blank()
                Text(this.macAddress)
            }
            .padding({ left: 15, right: 15 })
            .width("100%")
            .height("10%")

            List({ space: 10 }) {
                ForEach(this.serviceList, (service: ssap.Service) => {
                    ListItemGroup({ header: this.serviceHead(service.serviceUuid) }) {
                        ForEach(service.properties, (property: ssap.Property) => {
                            ListItem() {
                                Row() {
                                    Text(getShortUUID(property.propertyUuid))
                                    Blank()
                                    Flex({
                                        direction: FlexDirection.Row,
                                        alignItems: ItemAlign.Auto,
                                        justifyContent: FlexAlign.SpaceAround
                                    }) {
                                        if (property.operation! & 0x04) {
                                            Button("写", { type: ButtonType.Circle })
                                                .fontSize(13)
                                                .padding(5)
                                                .onClick(() => {
                                                    this.propertyReadyToWrite = property.propertyUuid
                                                    this.sleWriteDialogController.open()
                                                })
                                        }
                                        if (property.operation! & 0x01) {
                                            Button("读", { type: ButtonType.Circle })
                                                .fontSize(13)
                                                .padding(5)
                                                .onClick(async () => {
                                                    await this.readSle(property.propertyUuid)
                                                })
                                        }
                                        if (property.operation! & 0x08) {
                                            Button("通", { type: ButtonType.Circle })
                                                .fontSize(13)
                                                .padding(5)
                                                .onClick(() => {
                                                    this.subscribeSle(property.propertyUuid)
                                                })
                                        }

                                    }.width("50%")
                                }.width("90%").padding(8)
                            }.width("100%")
                        }
                        )
                    }
                    .divider({
                        strokeWidth: 1,
                        color: Color.Gray,
                        startMargin: 10,
                        endMargin: 10
                    })
                })
            }.padding({ left: 10, right: 10 })
            .width("100%")
            .height("50%")

            Blank()

            TextArea({ text: this.deviceLog })
                .padding(10)
                .width("100%")
                .height('30%')
                .backgroundColor(Color.Transparent)
        }
        .height('100%')
        .width('100%')
    }
}